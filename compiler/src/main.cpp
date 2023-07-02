#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "R-Sharp/Logging.hpp"
#include "R-Sharp/Tokenizer.hpp"
#include "R-Sharp/Token.hpp"
#include "R-Sharp/AstNodes.hpp"
#include "R-Sharp/Parser.hpp"
#include "R-Sharp/Utils.hpp"
#include "R-Sharp/ParsingCache.hpp"

#include "R-Sharp/AstPrinter.hpp"
#include "R-Sharp/CCodeGenerator.hpp"
#include "R-Sharp/NASMCodeGenerator.hpp"
#include "R-Sharp/AArch64CodeGenerator.hpp"
#include "R-Sharp/ErrorPrinter.hpp"
#include "R-Sharp/SemanticValidator.hpp"

enum class ReturnValue{
    NormalExit = 0,
    UnknownError = 1,
    SyntaxError = 2,
    SemanticError = 3,
    AssemblingError = 4,
};

void printHelp(const char* programName) {
    std::cout << "Usage: " << programName << " [options] [input file]\n";
    std::cout <<
R"(Options:
  -h, --help                Print this help message
  -o, --output <file>       Output file
  -f, --format <format>     Output format (c, nasm)
  --compiler <path>         Use this compiler. Default: "gcc"
  --link <file>             Additionally link <file> into the output. Can be repeated.
  --stdlib <path>           Use the standard library at <path>.

Return values:
  0     Everything OK
  1     Something unknown went wrong
  2     There were syntax errors
  3     There were semantic errors
  4     There were assembling errors
)";
}

std::stringstream& indent(std::stringstream& ss, int indentLevel) {
    for (int i = 0; i < indentLevel; i++) {
        ss << "    ";
    }
    return ss;
}

std::string tokensToString(std::vector<Token> const& tokens) {
    std::stringstream ss;
    int braceCount = 0;
    for (auto const& token : tokens) {
        switch (token.type) {
            case TokenType::LeftBrace: braceCount++; break;
            case TokenType::RightBrace: braceCount--; break;
            default: break;
        }
        if (!ss.str().empty() && ss.str().back() == '\n') {
            indent(ss, braceCount);
        }
        switch (token.type) {
            case TokenType::ID: ss << token.value; break;
            case TokenType::Number: ss << token.value; break;
            case TokenType::Typename: ss << token.value << " "; break;
            case TokenType::Semicolon: ss << token.value << "\n"; break;
            case TokenType::Colon: ss << token.value << " "; break;
            case TokenType::Comma: ss << token.value << " "; break;
            case TokenType::LeftParen: ss << token.value; break;
            case TokenType::RightParen: ss << token.value; break;
            case TokenType::LeftBracket: ss << token.value; break;
            case TokenType::RightBracket: ss << token.value; break;
            case TokenType::LeftBrace: ss << token.value << "\n"; break;
            case TokenType::RightBrace: ss << token.value; break;
            case TokenType::Star: ss << token.value; break;
            case TokenType::Comment: ss << "/*" << token.value << "*/"; break;

            case TokenType::Return: ss << token.value << " "; break;
            default: break;
        }
    }

    for (auto const& token : tokens) {
        switch (token.type) {
            case TokenType::LeftBrace: braceCount++; break;
            case TokenType::RightBrace: braceCount--; break;
            default: break;
        }
    }

    return ss.str();
}

enum class OutputFormat {
    C,
    NASM,
    AArch64,
};

int main(int argc, const char** argv) {
    std::string inputFilename;
    std::string outputFilename = "a.out";
    OutputFormat outputFormat = OutputFormat::C;
    std::string compiler = "gcc";
    std::vector<std::string> additionalyLinkedFiles;
    std::string stdlibIncludePath = std::filesystem::path(argv[0]).replace_filename("stdlib/");


    if (argc < 2) {
        printHelp(argv[0]);
        return static_cast<int>(ReturnValue::UnknownError);
    }

    for (int i=1; i<argc; i++){
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            printHelp(argv[0]);
            return static_cast<int>(ReturnValue::NormalExit);
        }
        else if (arg == "-o" || arg == "--output") {
            if (i+1 < argc) {
                outputFilename = argv[i+1];
                i++;
            } else {
                Error("Missing output file");
                return static_cast<int>(ReturnValue::UnknownError);
            }
        }
        else if (arg == "--link") {
            if (i+1 < argc) {
                additionalyLinkedFiles.push_back(argv[i+1]);
                i++;
            } else {
                Error("Missing file to link");
                return static_cast<int>(ReturnValue::UnknownError);
            }
        }
        else if (arg == "-f" || arg == "--format") {
            if (i+1 < argc) {
                std::string format = argv[i+1];
                if (format == "c") {
                    outputFormat = OutputFormat::C;
                }
                else if (format == "nasm") {
                    outputFormat = OutputFormat::NASM;
                }
                else if (format == "aarch64") {
                    outputFormat = OutputFormat::AArch64;
                }
                else {
                    Error("Unknown output format \"" + format + "\"");
                    return static_cast<int>(ReturnValue::UnknownError);
                }
                i++;
            } else {
                Error("Missing output format");
                return static_cast<int>(ReturnValue::UnknownError);
            }
        }
        else if (arg == "--compiler"){
            if (i+1 < argc) {
                compiler = argv[++i];
            } else {
                Error("Missing compiler path");
                return static_cast<int>(ReturnValue::UnknownError);
            }
        }
        else if (arg == "--stdlib"){
            if (i+1 < argc) {
                stdlibIncludePath = argv[++i];
            } else {
                Error("Missing standard library path");
                return static_cast<int>(ReturnValue::UnknownError);
            }
        }
        else {
            // test if it is a filename
            if (std::filesystem::exists(arg)) {
                inputFilename = arg;
            } else {
                Error("Invalid argument: ", arg);
                return static_cast<int>(ReturnValue::UnknownError);
            }
        }
    }

    std::vector<Token> tokens;
    std::shared_ptr<AstProgram> ast;
    std::string outputSource;
    std::string R_Sharp_Source;

    Print("--------------| Tokenizing |--------------");
    {
        Tokenizer tokenizer(inputFilename);
        tokens = tokenizer.tokenize();
        R_Sharp_Source = tokenizer.getSource();

        for (auto const& token : tokens){
            Print(token.toString());
        }

        tokens = cleanTokens(tokens);
    }

    Print("--------------| Parsing |--------------");
    {
        ParsingCache cache;
        Parser parser = Parser(tokens, inputFilename, stdlibIncludePath, cache);
        ast = parser.parse();

        Print("--------------| Syntax Errors |--------------");
        if (parser.hasErrors()) {
            ErrorPrinter printer(ast, inputFilename, R_Sharp_Source);
            printer.print();
            Error("Parsing errors.");
            return static_cast<int>(ReturnValue::SyntaxError);
        }
        else{
            Print("No errors"); 
        }
    }
    Print("--------------| Raw AST |--------------");
    {
        AstPrinter printer(ast);
        printer.print();
    }
    
    Print("--------------| Semantic Errors |--------------");
    {
        SemanticValidator validator(ast, inputFilename, R_Sharp_Source);
        validator.validate();

        if (validator.hasErrors()) {
            Error("Semantic errors.");
            return static_cast<int>(ReturnValue::SemanticError);
        }
        else{
            Print("No errors"); 
        }
    }
    Print("--------------| Typed AST |--------------");
    {
        AstPrinter printer(ast);
        printer.print();
    }

    Print("--------------| Generated code |--------------");
    {
        switch(outputFormat) {
            case OutputFormat::C:
                outputSource = CCodeGenerator(ast).generate();
                break;
            case OutputFormat::NASM:
                outputSource = NASMCodeGenerator(ast, R_Sharp_Source).generate();
                break;
            case OutputFormat::AArch64:
                outputSource = AArch64CodeGenerator(ast, R_Sharp_Source).generate();
                break;
        }
        Print(outputSource);
    }
    std::string temporaryFile = outputFilename;
    switch(outputFormat) {
        case OutputFormat::C:
            temporaryFile += ".c";
            break;
        case OutputFormat::NASM:
            temporaryFile += ".asm";
            break;
        case OutputFormat::AArch64:
            temporaryFile += ".S";
            break;
        default:
            Error("Unknown output format");
            return static_cast<int>(ReturnValue::UnknownError);
            break;
    }

    Print("Writing to file: ", temporaryFile);
    std::ofstream outputFile(temporaryFile);
    if (outputFile.is_open()) {
        outputFile << outputSource;
        outputFile.close();
    } else {
        Error("Could not open file: ", temporaryFile);
        return static_cast<int>(ReturnValue::UnknownError);
    }

    std::string additionalyLinkedFiles_str;
    for (auto file : additionalyLinkedFiles){
        additionalyLinkedFiles_str += file + " ";
    }

    const std::string gccArgumentsCompile = "-g -Werror -Wall -Wno-unused-variable -Wno-unused-value -Wno-unused-but-set-variable";
    const std::string gccArgumentsLink = "-g -Werror -Wall -no-pie ";
    const std::string nasmArgumentsCompile = "-g -w+error -w+all";

    switch(outputFormat) {
        case OutputFormat::C:{
            Print("--------------| Compiling using gcc |--------------");
            std::string command = compiler + " " + gccArgumentsCompile + " " + temporaryFile + " " + additionalyLinkedFiles_str + " -o " + outputFilename;
            Print("Executing: ", command);
            int success = !system(command.c_str());
            if (success)
                Print("Compilation successful.");
            else{
                Error("Compilation failed.");
                return static_cast<int>(ReturnValue::AssemblingError);
            }
            break;
        }
        case OutputFormat::AArch64:{
            Print("--------------| Compiling using gcc |--------------");
            std::string command = compiler + " " + gccArgumentsCompile + " " + temporaryFile + " " + additionalyLinkedFiles_str + " -o " + outputFilename;
            Print("Executing: ", command);
            int success = !system(command.c_str());
            if (success)
                Print("Compilation successful.");
            else{
                Error("Compilation failed.");
                return static_cast<int>(ReturnValue::AssemblingError);
            }
            break;
        }
        case OutputFormat::NASM:{
            Print("--------------| Assembling using nasm |--------------");
            std::string command = "nasm " + nasmArgumentsCompile + " -f elf64 " + temporaryFile + " -o " + outputFilename + ".o";
            Print("Executing: ", command);
            int success = !system(command.c_str());
            if (success)
                Print("Assembling successful.");
            else{
                Error("Assembling failed.");
                return static_cast<int>(ReturnValue::AssemblingError);
            }

            Print("--------------| Linking using gcc |--------------");
            command = compiler + " " + gccArgumentsLink + " " + outputFilename + ".o " + additionalyLinkedFiles_str + " -o " + outputFilename;
            Print("Executing: ", command);
            success = !system(command.c_str());
            if (success)
                Print("Linking successful.");
            else{
                Error("Linking failed.");
                return static_cast<int>(ReturnValue::AssemblingError);
            }
            
            break;
        }
        default:
            Error("Unsupported output format");
            return static_cast<int>(ReturnValue::UnknownError);
            break;
    }

    return static_cast<int>(ReturnValue::NormalExit);
}