#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "R-Sharp/Logging.hpp"
#include "R-Sharp/Tokenizer.hpp"
#include "R-Sharp/Token.hpp"

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
            case TokenType::TokenType_OpenBrace: braceCount++; break;
            case TokenType::TokenType_CloseBrace: braceCount--; break;
            default: break;
        }
        if (!ss.str().empty() && ss.str().back() == '\n') {
            indent(ss, braceCount);
        }
        switch (token.type) {
            case TokenType::TokenType_ID: ss << token.value; break;
            case TokenType::TokenType_Number: ss << token.value; break;
            case TokenType::TokenType_Typename: ss << token.value << " "; break;
            case TokenType::TokenType_TypeModifier: ss << token.value << " "; break;
            case TokenType::TokenType_Semicolon: ss << token.value << "\n"; break;
            case TokenType::TokenType_Colon: ss << token.value << " "; break;
            case TokenType::TokenType_Comma: ss << token.value << " "; break;
            case TokenType::TokenType_OpenParenthesis: ss << token.value; break;
            case TokenType::TokenType_CloseParenthesis: ss << token.value; break;
            case TokenType::TokenType_OpenBracket: ss << token.value; break;
            case TokenType::TokenType_CloseBracket: ss << token.value; break;
            case TokenType::TokenType_OpenBrace: ss << token.value << "\n"; break;
            case TokenType::TokenType_CloseBrace: ss << token.value; break;
            case TokenType::TokenType_Star: ss << token.value; break;
            case TokenType::TokenType_Comment: ss << "//" << token.value << "\n"; break;
            case TokenType::TokenType_MultilineComment: ss << "/*" << token.value << "*/\n"; break;

            case TokenType::TokenType_Return: ss << token.value << " "; break;
            default: break;
        }
    }

    for (auto const& token : tokens) {
        switch (token.type) {
            case TokenType::TokenType_OpenBrace: braceCount++; break;
            case TokenType::TokenType_CloseBrace: braceCount--; break;
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
                Log("Output file: ", argv[i+1]);
                outputFilename = argv[i+1];
            } else {
                Log("Missing output file");
                return 1;
            }
        } else {
            // test if it is a filename
            std::ifstream testFile(arg);
            if (testFile.is_open()) {
                Log("Input file: ", arg);
                inputFilename = arg;
            } else {
                Error("Invalid argument: ", arg);
                return 1;
            }
        }
    }

    std::ifstream inputFile(inputFilename);

    if (!inputFile.is_open()) {
        Error("Could not open input file: \"", inputFilename, "\"");
        return 1;
    }

    std::string source;
    std::string line;
    while (std::getline(inputFile, line)) {
        source += line + "\n";
    }

    Tokenizer tokenizer(source);
    std::vector<Token> tokens = tokenizer.tokenize();

    for (auto const& token : tokens){
        Log(token);
    }
    Log("Reconstructed source:\n", tokensToString(tokens));

    return 0;
}