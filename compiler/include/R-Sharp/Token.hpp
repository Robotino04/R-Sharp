#pragma once

#include <string>

enum class TokenType{
    None,                       // 
    EndOfFile,                  // "\0"

    Comment,                    // "//" | "/*" ... "*/"
    ID,                         // [a-zA-Z_][a-zA-Z0-9_]*
    Number,                     // [0-9]+
    Typename,                   // "int" | "char"
    TypeModifier,               // "const"

    Semicolon,                  // ";"
    Colon,                      // ":"
    Comma,                      // ","

    LeftParen,                  // "("
    RightParen,                 // ")"
    LeftBracket,                // "["
    RightBracket,               // "]"
    LeftBrace,                  // "{"
    RightBrace,                 // "}"

    ExclamationPoint,           // "!"
    Tilde,                      // "~"

    Plus,                       // "+"
    Minus,                      // "-"
    Star,                       // "*"
    Slash,                      // "/"

    DoubleAmpersand,            // "&&"
    DoublePipe,                 // "||"
    EqualEqual,                 // "=="
    NotEqual,                   // "!="
    LessThan,                   // "<"
    LessThanEqual,              // "<="
    GreaterThan,                // ">"
    GreaterThanEqual,           // ">="

    Return,                     // "return"

    Identifier = ID,
    Bang = ExclamationPoint,
};

struct Token{
    Token(): type(TokenType::None), value(""), line(0), column(0){}
    Token(TokenType type, std::string value, int line, int column)
        : type(type), value(value), line(line), column(column){}
    Token(TokenType type, char value, int line, int column)
        : type(type), line(line), column(column)
    {
        this->value = value;
    }
    Token(TokenType type, std::string value) : type(type), value(value), line(0), column(0){}
    TokenType type;
    std::string value = "";

    int line = 0, column = 0;
};

namespace std{
    string to_string(Token const& token);
    string to_string(TokenType const& type);

    ostream& operator<<(ostream& os, Token const& token);
    ostream& operator<<(ostream& os, TokenType const& type);
}