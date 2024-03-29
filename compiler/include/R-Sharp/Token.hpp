#pragma once

#include <string>
#include <memory>

enum class TokenType {
    None,      //
    EndOfFile, // "\0"

    Comment,   // "//" | "/*" ... "*/"
    ID,        // [a-zA-Z_][a-zA-Z0-9_]*
    Number,    // [0-9]+
    Typename,  // "i8" | "i16" | "i32" | "i64"


    DoubleColon,      // "::"
    At,               // "@"

    Semicolon,        // ";"
    Colon,            // ":"
    Comma,            // ","

    LeftParen,        // "("
    RightParen,       // ")"
    LeftBracket,      // "["
    RightBracket,     // "]"
    LeftBrace,        // "{"
    RightBrace,       // "}"

    ExclamationPoint, // "!"
    Tilde,            // "~"
    QuestionMark,     // "?"

    Plus,             // "+"
    Minus,            // "-"
    Star,             // "*"
    Slash,            // "/"
    Percent,          // "%"

    DoubleAmpersand,  // "&&"
    DoublePipe,       // "||"
    EqualEqual,       // "=="
    NotEqual,         // "!="
    LessThan,         // "<"
    LessThanEqual,    // "<="
    GreaterThan,      // ">"
    GreaterThanEqual, // ">="

    Return,           // "return"
    If,               // "if"
    Elif,             // "elif"
    Else,             // "else"

    While,            // "while"
    For,              // "for"
    Do,               // "do"
    Break,            // "break"
    Skip,             // "skip"

    Assign,           // "="
    DollarSign,       // "$"

    CharacterLiteral, // "'" . "'"
    StringLiteral,    // '"' .* '"'

    Identifier = ID,
    Bang = ExclamationPoint,
};

struct TokenLocation {
    size_t startPos;
    size_t endPos;
    int line;
    int column;
};

struct Token {
    Token(): type(TokenType::None), value("") {}
    Token(TokenType type, std::string value, TokenLocation position, std::shared_ptr<std::string> source) {
        this->type = type;
        this->value = value;
        this->position = position;
        this->source = source;
    }

    Token(TokenType type, char value, TokenLocation position, std::shared_ptr<std::string> source) {
        this->type = type;
        this->value = std::string(1, value);
        this->position = position;
        this->source = source;
    }
    Token(TokenType type, std::string value): type(type), value(value) {}

    std::string toString() const;

    TokenType type;
    std::string value = "";

    TokenLocation position;

    std::shared_ptr<std::string> source;
};

std::string tokenTypeToString(TokenType type);