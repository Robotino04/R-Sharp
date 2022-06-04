#include <iostream>
#include <string>
#include <fstream>
#include <array>
#include <memory>
#include <functional>

constexpr int getExitStatus(int status) {
    return (int8_t)WEXITSTATUS(status);
}

struct CommandResult {
    int returnCode;
    std::string output;
};

template<typename T>
void printBits(T const& value) {
    for (int i = sizeof(T) * 8 - 1; i >= 0; i--) {
        std::cout << ((value >> i) & 1);
    }
}

void linked_pclose(int& returnCode, FILE *__stream){
    if (__stream) {
        returnCode = getExitStatus(pclose(__stream));
    }
}

CommandResult exec(std::string const& cmd) {
    using namespace std::placeholders;
    std::array<char, 128> buffer;
    CommandResult result;

    auto custom_pclose = std::bind(linked_pclose, std::ref(result.returnCode), _1);

    {
        std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), custom_pclose);
        if (!pipe) {
            throw std::runtime_error("popen() failed!");
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result.output += buffer.data();
        }
    }
    return result;
}

int main(int argc, char** argv) {
    
    if (argc < 4) {
        std::cout << "Usage: " << argv[0] << " <compiler> <input file> <output dir>" << std::endl;
        return 1;
    }

    std::string compilerPath = argv[1];
    std::string inputFile = argv[2];
    std::string outputDir = argv[3];

    // get the filename without the directory and extension
    std::string filename = inputFile.substr(inputFile.find_last_of("/") + 1);
    filename = filename.substr(0, filename.find_last_of("."));

    std::string command = compilerPath + " -o " + outputDir + "/" + filename +  " " + inputFile;

    std::cout << "Compiling: " << command << std::endl;
    auto result = exec(command);
    std::cout << "Compiler output: " << result.output;
    if (result.returnCode != 0) {
        if (inputFile.find("skip_on_failure") != std::string::npos) {
            std::cout << " (skipped)" << std::endl;
        }
        else{
            std::cout << "Compiler return code: " << result.returnCode << std::endl;
            return 1;
        }
    }

    result = exec(outputDir + "/" + filename);

    std::cout << "Validating result: " << std::endl;
    std::ifstream input(inputFile);
    
    std::string line;
    while (std::getline(input, line)) {
        if (line.find("return: ") != std::string::npos) {
            // extract the return value
            int expectedReturnValue = std::stoi(line.substr(line.find(": ") + 2));

            std::cout << "Expected return value: " << expectedReturnValue;
            if (result.returnCode != expectedReturnValue) {
                // if the input file contains "skip_on_failure" then we skip the test
                if (inputFile.find("skip_on_failure") != std::string::npos) {
                    std::cout << " (skipped)" << std::endl;
                    continue;
                }
                std::cout << " - FAILED (It was " << result.returnCode << ")" << std::endl;
                return 1;
            }
            std::cout << " - PASSED" << std::endl;
        }

        if (line.find("*/") != std::string::npos) {
            std::cout << "No further validation required" << std::endl;
        }
    }

    

    return 0;
}