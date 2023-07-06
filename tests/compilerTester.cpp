#include <iostream>
#include <string>
#include <fstream>
#include <array>
#include <memory>
#include <optional>
#include <chrono>
#include <thread>
#include <atomic>

#include <unistd.h>
#include <signal.h>
#include <wait.h>

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
            std::cout << "No further validation required\n";
            break;
        }
        if (line.find("/*") != std::string::npos) continue;


        if (line.find(":") == std::string::npos) {
            std::cout << "Error: no validation present on line " << lineNumber << "\n";
            exit(1);
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
                        std::cout << "Error: unknown escape sequence \\" << output[i] << " on line " << lineNumber << "\n";
                    }
                }
                else if (output[i] == '"') {
                    break;
                }
                else if (output[i] == '*') {
                    i++;
                    if (output[i] == '/') {
                        std::cout << "Error: unterminated string on line " << lineNumber << "\n";
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
        else{
            std::cout << "Error: unknown test parameter on line " << lineNumber << "\n";
            exit(1);
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
        std::cout << "\nPASSED\n\n";
    }
    else{
        std::cout << "\nFAILED\n\n";
    }
    return isValid;
}

struct CommandResult {
    int returnCode;
    std::string output;
    bool wasKilled = false;
};

template<typename T>
void printBits(T const& value) {
    for (int i = sizeof(T) * 8 - 1; i >= 0; i--) {
        std::cout << ((value >> i) & 1);
    }
}

enum class Popen2Type{
    Reading,
    Writing,
};

FILE* popen2(std::string command, Popen2Type type, int& pid){
    static const int READ = 0;
    static const int WRITE = 1;
    pid_t child_pid;

    int fd[2];
    if(pipe(fd) == -1){
        throw std::runtime_error("pipe() failed!");
    }

    if((child_pid = fork()) == -1){
        throw std::runtime_error("fork() failed!");
    }

    /* child process */
    if (child_pid == 0){
        if (type == Popen2Type::Reading){
            close(fd[READ]);    //Close the READ end of the pipe since the child's fd is write-only
            dup2(fd[WRITE], 1); //Redirect stdout to pipe
            close(fd[WRITE]);
        }
        else{
            close(fd[WRITE]);    //Close the WRITE end of the pipe since the child's fd is read-only
            dup2(fd[READ], 0);   //Redirect stdin to pipe
            close(fd[READ]);
        }

        setpgid(child_pid, child_pid); //Needed so negative PIDs can kill children of /bin/sh
        execl("/bin/sh", "sh", "-c", command.c_str(), NULL);
        exit(127);
    }
    else{
        if (type == Popen2Type::Reading){
            close(fd[WRITE]); //Close the WRITE end of the pipe since parent's fd is read-only
        }
        else{
            close(fd[READ]); //Close the READ end of the pipe since parent's fd is write-only
        }
    }

    pid = child_pid;

    if (type == Popen2Type::Reading){
        return fdopen(fd[READ], "r");
    }

    return fdopen(fd[WRITE], "w");
}

CommandResult execute(std::string const& cmd, std::optional<std::chrono::seconds> timeout) {
    std::array<char, 128> buffer;
    CommandResult result;

    int childPid = 0;

    std::unique_ptr<FILE, decltype(&fclose)> pipe(popen2(cmd + " 2>&1", Popen2Type::Reading, childPid), fclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    std::cout << "Child PID is " << childPid << "\n";
    
    // start a timeout
    std::atomic<bool> watchdog_stop = false;
    std::thread timeout_watchdog([&](){
        if (!timeout.has_value()) return;
        auto endTime = std::chrono::system_clock::now() + timeout.value();

        while (std::chrono::system_clock::now() < endTime){
            if (watchdog_stop){
                return;
            }
            // don't use 100% cpu
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        if (childPid != 0){
            // the program is probably stuck somehow. Kill it with SIGKILL.
            std::cout << "Timeout reached. Killing with SIGKILL.\n";
            result.wasKilled = true;
            killpg(childPid, SIGKILL);
        }
        else{
            std::cout << "Child PID is 0. Not killing.\n";
        }
    });

    // read stdout of child
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
            result.output += buffer.data();
        else{
            break;
        }
    }
    watchdog_stop = true;
    timeout_watchdog.join();

    int status;
    waitpid(childPid, &status, 0);
    result.returnCode = (int8_t)WEXITSTATUS(status);

    return result;
}

int main(int argc, char** argv) {
    if (argc < 7) {
        std::cout << "Usage: " << argv[0] << " <compiler> <input file> <output dir> <output language> <test library source> <stdlib directory> [proxy] [gcc compiler]\n";
        std::cout << "  proxy is something like qemu\n";
        return 1;
    }

    int i=0;

    std::string compilerPath = argv[++i];
    std::string inputFile = argv[++i];
    std::string outputDir = argv[++i];
    std::string outputLanguage = argv[++i];
    std::string testLibrarySource = argv[++i];
    std::string standardLibrary = argv[++i];
    std::string proxy = "";
    std::string gccCompiler = "gcc";
    if (i+1 < argc) proxy = argv[++i];
    if (i+1 < argc) gccCompiler = argv[++i];
    std::cout << "proxy: " << proxy << "\n";
    std::cout << "gccCompiler: " << gccCompiler << "\n";

    
    ExecutionResults expectedResults = parseExpectations(inputFile);

    // get the filename without the directory and extension
    std::string filename = inputFile.substr(inputFile.find_last_of("/") + 1);
    filename = filename.substr(0, filename.find_last_of("."));

    // the output language is needed to allow for parrallel testing over multiple languages
    std::string outputFile = outputDir + "/" + filename + "_" + outputLanguage;
    std::string test_lib_outfile = outputFile + "_tlib.o";

    std::string test_lib_command = gccCompiler + " -c -g " + testLibrarySource + " -o " + test_lib_outfile;

    std::string rsharp_command = compilerPath + " -o " + outputFile + " " + inputFile + " -f " + outputLanguage + " --compiler " + gccCompiler + " --link " + test_lib_outfile + " --stdlib " + standardLibrary;

    ExecutionResults realResults;

    std::cout << "Compiling Test Library: " << test_lib_command << "\n";
    auto test_lib_result = execute(test_lib_command, std::optional<std::chrono::seconds>());
    if (test_lib_result.returnCode){
        std::cout << test_lib_result.output << "\n";
        return 1;
    }

    std::cout << "Compiling: " << rsharp_command << "\n";
    auto compilerResult = execute(rsharp_command, std::optional<std::chrono::seconds>());

    realResults.compilationReturnValue = compilerResult.returnCode;

    if (realResults.compilationReturnValue == static_cast<int>(ReturnValue::NormalExit)){
        std::string executionCommand;
        if (proxy.size())
            executionCommand = proxy + " " + outputFile;
        else
            executionCommand = outputFile;
        
        auto timeout = std::chrono::seconds(10);
        std::cout << "Runnning: " << executionCommand << " (Timeout: " << timeout.count() << "s)\n";
        auto programResult = execute(executionCommand, timeout);
        if (programResult.output.length() > 1000){
            std::cout << "Output is more than 1000 characters. It will be truncated!\n";
            programResult.output = programResult.output.substr(0, 1000);
        }
        if (programResult.wasKilled){
            std::cout << "Testee was killed by timeout.\nFAILED\n";
            return 1;
        }

        realResults.returnValue = programResult.returnCode;
        realResults.output = programResult.output;
    }

    if (validate(realResults, expectedResults)) {
        return 0;
    }
    else {
        if (expectedResults.skipped) {
            std::cout << "Program skipped\n";
            return 0;
        }
        
        if (realResults.compilationReturnValue != static_cast<int>(ReturnValue::NormalExit)) {
            std::cout << "Compiling Failed: \n\n";
            std::cout << compilerResult.output << "\n";
        }
    }
    return 1;
}