#pragma once

#include <string>

enum TokenType{
    TokenType_None,                     // 
    TokenType_ID,                       // [a-zA-Z_][a-zA-Z0-9_]*
    TokenType_Number,                   // [0-9]+(\.[0-9]+)?
    TokenType_Typename,                 // "int" | "char"
    TokenType_TypeModifier,             // "const"
    TokenType_Semicolon,                // ";"
    TokenType_Colon,                    // ":"
    TokenType_Comma,                    // ","
    TokenType_OpenParenthesis,          // "("
    TokenType_CloseParenthesis,         // ")"
    TokenType_OpenBracket,              // "["
    TokenType_CloseBracket,             // "]"
    TokenType_OpenBrace,                // "{"
    TokenType_CloseBrace,               // "}"
    TokenType_Star,                     // "*"
    TokenType_Comment,                  // "//"
    TokenType_MultilineComment,         // "/*" .* "*/"
    TokenType_EndOfFile,                // "\0"

    TokenType_Return,                   // "return"
};

struct Token{
    TokenType type;
    std::string value = "";
};

namespace std{
    string to_string(Token const& token);
    string to_string(TokenType const& type);

    ostream& operator<<(ostream& os, Token const& token);
}