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

void printHelp(const char* programName) {
    std::cout << "Usage: " << programName << " [options] [input file]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help\t\t\t\tPrint this help message" << std::endl;
    std::cout << "  -o, --output <file>\t\t\tOutput file" << std::endl;
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

int main(int argc, const char** argv) {
    std::string inputFilename;
    std::string outputFilename = "a.out";

    if (argc < 2) {
        printHelp(argv[0]);
        return 1;
    }

    for (int i=1; i<argc; i++){
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            printHelp(argv[0]);
            return 0;
        } else if (arg == "-o" || arg == "--output") {
            if (i+1 < argc) {
                outputFilename = argv[i+1];
                i++;
            } else {
                Error("Missing output file");
                return 1;
            }
        } else {
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
    std::string C_Source;
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

        if (parser.hasErrors()) {
            ErrorPrinter printer(ast, inputFilename, R_Sharp_Source);
            printer.print();
            Fatal("Parsing errors.");
        }
    }

    Print("--------------| Generated code |--------------");
    {
        CCodeGenerator generator(ast);
        C_Source = generator.generate();
        Print(C_Source);
    }

    Print("Writing to file: ", outputFilename, ".c");
    std::ofstream outputFile(outputFilename + ".c");
    if (outputFile.is_open()) {
        outputFile << C_Source;
        outputFile.close();
    } else {
        Error("Could not open file: ", outputFilename, ".c");
        return 1;
    }

    Print("--------------| Compiling using gcc |--------------");
    std::string command = "gcc " + outputFilename + ".c -o " + outputFilename;
    Print("Executing: ", command);
    int success = system(command.c_str());
    if (success != 0) {
        Error("Compilation failed");
        return 1;
    }
    else {
        Print("Compilation successful");
    }

    return 0;
}