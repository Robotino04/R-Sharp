#include <iostream>
#include <string>
#include <fstream>
#include <array>
#include <memory>
#include <functional>

struct ExecutionResults{
    bool failed = false;
    bool skipped = false;
    int returnValue = 0;
    std::string output = "";
};

std::string escapeString(std::string const& str) {
    std::string result;
    for (char c : str) {
        switch (c) {
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            case '\'': result += "\\'"; break;
            case '\"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            default: result += c; break;
        }
    }
    return result;
}

ExecutionResults parseExpectations(std::string filename){
    ExecutionResults validations;
    std::ifstream input(filename);
    
    std::string line;
    int lineNumber = 0;
    while (std::getline(input, line)) {
        lineNumber++;
        if (line.find("*/") != std::string::npos) {
            std::cout << "No further validation required" << std::endl;
            break;
        }
        if (line.find("/*") != std::string::npos) continue;


        if (line.find(":") == std::string::npos) {
            std::cout << "Error: no validation present on line " << lineNumber << std::endl;
        }


        if (line.find("return: ") != std::string::npos) {
            // extract the return value
            validations.returnValue = std::stoi(line.substr(line.find(": ") + 2));
        }
        else if (line.find("fail: ") != std::string::npos) {
            // the program is expected to fail
            validations.failed = std::stoi(line.substr(line.find(": ") + 2));
        }
        else if (line.find("skip: ") != std::string::npos) {
            // the program should be ignored until the missing features are implemented
            validations.skipped = std::stoi(line.substr(line.find(": ") + 2));
        }
        else if (line.find("output: ") != std::string::npos) {
            // parse a string enclosed in quotes by itterating over the characters
            std::string output = line.substr(line.find(": \"") + 3);
            for (int i = 0; i < output.size(); i++) {
                if (output[i] == '\\') {
                    i++;
                    if (output[i] == 'n') {
                        validations.output += '\n';
                    }
                    else if (output[i] == 't') {
                        validations.output += '\t';
                    }
                    else if (output[i] == '\\') {
                        validations.output += '\\';
                    }
                    else {
                        std::cout << "Error: unknown escape sequence \\" << output[i] << " on line " << lineNumber << std::endl;
                    }
                }
                else if (output[i] == '"') {
                    break;
                }
                else if (output[i] == '*') {
                    i++;
                    if (output[i] == '/') {
                        std::cout << "Error: unterminated string on line " << lineNumber << std::endl;
                    exit(1);
                    }
                    else {
                        validations.output += '*' + output[i];
                    }
                }
                else {
                    validations.output += output[i];
                }
            }
        }
    }

    return validations;
}

bool validate(ExecutionResults real, ExecutionResults expected){
    bool isValid = true;
    std::cout << "Validation:";
    std::cout << "\n\texpected program to fail: " << std::boolalpha << expected.failed << std::noboolalpha;
    std::cout << "\n\tactual program failed: " << std::boolalpha << real.failed << std::noboolalpha;
    if (real.failed != expected.failed) {
        isValid = false;
    }
    std::cout << "\n\texpected return value: " << expected.returnValue << " (interpreted as " << (int)((int8_t)expected.returnValue) << ")";
    std::cout << "\n\tactual return value: " << real.returnValue << " (interpreted as " << ((int)(int8_t)real.returnValue) << ")";
    if ((int8_t)real.returnValue != (int8_t)expected.returnValue) {
        isValid = false;
    }
    std::cout << "\n\texpected output: \"" << escapeString(expected.output) << "\"";
    std::cout << "\n\tactual output: \"" << escapeString(real.output) << "\"";
    if (real.output != expected.output) {
        isValid = false;
    }

    if (isValid) {
        std::cout << "\nPASSED\n" << std::endl;
    }
    else{
        std::cout << "\nFAILED\n" << std::endl;
    }
    return isValid;
}

constexpr int getExitStatus(int status) {
    return (int)WEXITSTATUS(status);
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
        std::shared_ptr<FILE> pipe(popen((cmd + " 2>&1").c_str(), "r"), custom_pclose);
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
    if (argc < 5) {
        std::cout << "Usage: " << argv[0] << " <compiler> <input file> <output dir> <output language>" << std::endl;
        return 1;
    }

    std::string compilerPath = argv[1];
    std::string inputFile = argv[2];
    std::string outputDir = argv[3];
    std::string outputLanguage = argv[4];

    
    ExecutionResults expectedResults = parseExpectations(inputFile);

    // get the filename without the directory and extension
    std::string filename = inputFile.substr(inputFile.find_last_of("/") + 1);
    filename = filename.substr(0, filename.find_last_of("."));

    std::string command = compilerPath + " -o " + outputDir + "/" + filename + " " + inputFile + " -f " + outputLanguage;

    ExecutionResults realResults;

    std::cout << "Compiling: " << command << std::endl;
    auto compilerResult = exec(command);

    if (compilerResult.returnCode != 0) {
        realResults.failed = true;
    }

    if (!realResults.failed){
        auto programResult = exec(outputDir + "/" + filename);

        realResults.returnValue = programResult.returnCode;
        realResults.output = programResult.output;
    }

    if (validate(realResults, expectedResults)) {
        return 0;
    }
    else {
        if (expectedResults.skipped) {
            std::cout << "Program skipped" << std::endl;
            return 0;
        }
        
        if (realResults.failed) {
            std::cout << "Compiling Failed: \n" << std::endl;
            std::cout << compilerResult.output << std::endl;
        }
    }
    return 1;
}