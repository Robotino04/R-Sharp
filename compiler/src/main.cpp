#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "R-Sharp/Logging.hpp"
#include "R-Sharp/Tokenizer.hpp"
#include "R-Sharp/Token.hpp"
#include "R-Sharp/AstNodes.hpp"
#include "R-Sharp/Parser.hpp"
#include "R-Sharp/Utils.hpp"
#include "R-Sharp/CCodeGenerator.hpp"
#include "R-Sharp/ErrorPrinter.hpp"
#include "R-Sharp/Validator.hpp"

void printHelp(const char* programName) {
    std::cout << "Usage: " << programName << " [options] [input file]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help\t\t\t\tPrint this help message" << std::endl;
    std::cout << "  -o, --output <file>\t\t\tOutput file" << std::endl;
    std::cout << "  -f, --format <format>\t\t\tOutput format (c)" << std::endl;
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
            case TokenType::TypeModifier: ss << token.value << " "; break;
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
};

int main(int argc, const char** argv) {
    std::string inputFilename;
    std::string outputFilename = "a.out";
    OutputFormat outputFormat = OutputFormat::C;

    if (argc < 2) {
        printHelp(argv[0]);
        return 1;
    }

    for (int i=1; i<argc; i++){
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            printHelp(argv[0]);
            return 0;
        }
        else if (arg == "-o" || arg == "--output") {
            if (i+1 < argc) {
                outputFilename = argv[i+1];
                i++;
            } else {
                Error("Missing output file");
                return 1;
            }
        }
        else if (arg == "-f" || arg == "--format") {
            if (i+1 < argc) {
                std::string format = argv[i+1];
                if (format == "c") {
                    outputFormat = OutputFormat::C;
                }
                else {
                    Error("Unknown output format \"" + format + "\"");
                    return 1;
                }
                i++;
            } else {
                Error("Missing output format");
                return 1;
            }
        }
        else {
            // test if it is a filename
            std::ifstream testFile(arg);
            if (testFile.is_open()) {
                inputFilename = arg;
            } else {
                Error("Invalid argument: ", arg);
                return 1;
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
            Print(token);
        }

        tokens = cleanTokens(tokens);
    }

    Print("--------------| Parsed tree |--------------");
    {
        Parser parser = Parser(tokens, inputFilename);
        ast = parser.parse();
        ast->printTree();

        Print("--------------| Syntax Errors |--------------");
        if (parser.hasErrors()) {
            ErrorPrinter printer(ast, inputFilename, R_Sharp_Source);
            printer.print();
            Fatal("Parsing errors.");
        }
        else{
            Print("No errors"); 
        }
    }

    Print("--------------| Semantic Errors |--------------");
    {
        Validator validator(ast, inputFilename, R_Sharp_Source);
        validator.validate();

        if (validator.hasErrors()) {
            Fatal("Semantic errors.");
        }
        else{
            Print("No errors"); 
        }
    }

    Print("--------------| Generated code |--------------");
    {
        switch(outputFormat) {
            case OutputFormat::C:
                outputSource = CCodeGenerator(ast).generate();
                break;
        }
        Print(outputSource);
    }
    std::string temporaryFile = outputFilename;
    switch(outputFormat) {
        case OutputFormat::C:
            temporaryFile += ".c";
            break;
        default:
            Fatal("Unknown output format");
            break;
    }

    Print("Writing to file: ", temporaryFile);
    std::ofstream outputFile(temporaryFile);
    if (outputFile.is_open()) {
        outputFile << outputSource;
        outputFile.close();
    } else {
        Error("Could not open file: ", temporaryFile);
        return 1;
    }

    switch(outputFormat) {
        case OutputFormat::C:{
            Print("--------------| Compiling using gcc |--------------");
            std::string command = "gcc " + temporaryFile + " -o " + outputFilename;
            Print("Executing: ", command);
            int success = !system(command.c_str());
            if (success)
                Print("Compilation successful.");
            else
                Fatal("Compilation failed.");
            break;
        }
        default:
            Fatal("Unsupported output format");
            break;
    }

    return 0;
}