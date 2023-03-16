#include <iostream>
#include <string>
#include <fstream>
#include <array>
#include <memory>
#include <functional>

enum class ReturnValue{
    NormalExit = 0,
    UnknownError = 1,
    SyntaxError = 2,
    SemanticError = 3,
    AssemblingError = 4,
};

struct ExecutionResults{
    int compilationReturnValue = static_cast<int>(ReturnValue::NormalExit);
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
    ExecutionResults expectedResult;
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


        if (line.find("executionExitCode: ") != std::string::npos) {
            // extract the return value
            expectedResult.returnValue = std::stoi(line.substr(line.find(": ") + 2));
        }
        else if (line.find("compilationExitCode: ") != std::string::npos) {
            // the program is expected to fail
            expectedResult.compilationReturnValue = std::stoi(line.substr(line.find(": ") + 2));
        }
        else if (line.find("skip: ") != std::string::npos) {
            // the program should be ignored until the missing features are implemented
            expectedResult.skipped = std::stoi(line.substr(line.find(": ") + 2));
        }
        else if (line.find("output: ") != std::string::npos) {
            // parse a string enclosed in quotes by itterating over the characters
            std::string output = line.substr(line.find(": \"") + 3);
            for (int i = 0; i < output.size(); i++) {
                if (output[i] == '\\') {
                    i++;
                    if (output[i] == 'n') {
                        expectedResult.output += '\n';
                    }
                    else if (output[i] == 't') {
                        expectedResult.output += '\t';
                    }
                    else if (output[i] == '\\') {
                        expectedResult.output += '\\';
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
                        expectedResult.output += '*' + output[i];
                    }
                }
                else {
                    expectedResult.output += output[i];
                }
            }
        }
    }

    return expectedResult;
}

bool validate(ExecutionResults real, ExecutionResults expected){
    bool isValid = true;
    std::cout << "Validation:";

    std::cout << "\n\tCompilation exit code: " << real.compilationReturnValue << " (expected " << expected.compilationReturnValue << ") ";
    if (real.compilationReturnValue != expected.compilationReturnValue) {
        isValid = false;
        std::cout << "✗";
    }
    else{
        std::cout << "✓";
    }
    bool printExecution = (expected.compilationReturnValue == static_cast<int>(ReturnValue::NormalExit));
    
    if (printExecution){
        std::cout << "\n\tExecution exit code: " << real.returnValue << " (expected " << expected.returnValue << ") ";
        if (real.returnValue != expected.returnValue) {
            isValid = false;
            std::cout << "✗";
        }
        else
            std::cout << "✓";
    }
    else{
        std::cout << "\n\tExecution exit code: [not executed] (expected " << expected.returnValue << ") ✗";
    }


    if (printExecution){
        std::cout << "\n\tExecution output: \"" << escapeString(real.output) << "\" (expected \"" << escapeString(expected.output) << "\") ";
        if (real.output != expected.output) {
            isValid = false;
            std::cout << "✗";
        }
        else
            std::cout << "✓";
    }
    else{
        std::cout << "\n\tExecution output: [not executed] (expected " << expected.returnValue << ") ✗";
    }


    if (isValid) {
        std::cout << "\nPASSED\n" << std::endl;
    }
    else{
        std::cout << "\nFAILED\n" << std::endl;
    }
    return isValid;
}

constexpr int8_t getExitStatus(int status) {
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

    // the output language is needed to allow for parrallel testing over multiple languages
    std::string outputFile = outputDir + "/" + filename + "_" + outputLanguage;

    std::string command = compilerPath + " -o " + outputFile + " " + inputFile + " -f " + outputLanguage;

    ExecutionResults realResults;

    std::cout << "Compiling: " << command << std::endl;
    auto compilerResult = exec(command);

    realResults.compilationReturnValue = compilerResult.returnCode;

    if (realResults.compilationReturnValue == static_cast<int>(ReturnValue::NormalExit)){
        auto programResult = exec(outputFile);

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
        
        if (realResults.compilationReturnValue != static_cast<int>(ReturnValue::NormalExit)) {
            std::cout << "Compiling Failed: \n" << std::endl;
            std::cout << compilerResult.output << std::endl;
        }
    }
    return 1;
}