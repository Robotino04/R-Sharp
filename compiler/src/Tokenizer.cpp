#include "R-Sharp/Tokenizer.hpp"

#include <fstream>
#include <sstream>

std::string characterRange(char start, char end){
    if (end < start){
        std::swap(start, end);
    }
    std::string str;
    for (char c = start; c <= end; c++) {
        str += c;
    }
    return str;
}


static const std::string numbers = characterRange('0', '9');
static const std::string validIdentifierBegin = characterRange('a', 'z') + characterRange('A', 'Z') + "_";
static const std::string validIdentifierChars = validIdentifierBegin + characterRange('0', '9');


Tokenizer::Tokenizer(std::string const& filename): currentPosition(0), line(1), column(1), numErrors(0), filename(filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        Fatal("Could not open file: \"", filename, "\"");
    }
    std::stringstream ss;
    ss << file.rdbuf();
    source = ss.str();
}

bool Tokenizer::match(std::string str) const {
    return !isAtEnd() && source.substr(currentPosition, str.size()) == str;
}
bool Tokenizer::match(char c) const {
    return !isAtEnd() && getCurrentChar() == c;
}
bool Tokenizer::matchAny(std::string str) const {
    for (char c : str) {
        if (match(c)) {
            return true;
        }
    }
    return false;
}

char Tokenizer::consume() {
    char c = getCurrentChar();
    if (getCurrentChar() == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    currentPosition++;
    return c;
}
char Tokenizer::consume(char c) {
    if (match(c)) {
        return consume();
    }
    return '\0';
}
char Tokenizer::consume(std::string str) {
    if (!match(str)) {
        logError("Expected: \"", str, "\"");
        return '\0';
    }
    for (int i = 0; i < str.size(); i++) {
        consume();
    }
    return getCurrentChar();
}
char Tokenizer::consumeAnyOne(std::string str) {
    for (char c : str) {
        if (match(c)) {
            return c;
        }
    }
    logError("Expected: \"", str, "\"");
    return '\0';
}
std::string Tokenizer::consumeAny(std::string str) {
    std::string result;
    while (matchAny(str)) {
        result += consume();
    }
    return result;
}
std::string Tokenizer::consumeUntil(std::string str) {
    std::string result;
    while (!match(str)) {
        result += consume();
    }
    // consume the delimiter
    for (int i = 0; i < str.size(); i++) {
        consume();
    }
    return result;
}

bool Tokenizer::isAtEnd(int offset) const {
    return currentPosition + offset >= source.size();
}

char Tokenizer::getCurrentChar() const{
    return source[currentPosition];
}
char Tokenizer::getChar(int offset) const{
    if (isAtEnd(offset)) return '\0';
    return source[currentPosition + offset];
}

void Tokenizer::testErrorLimit() const{
    if (numErrors > maxErrors) {
        Fatal("Too many errors");
    }
}

#define SIMPLE_TOKEN(characters, type) \
    if (match(characters)) { \
        int line_ = line; \
        int column_ = column; \
        token = Token(type, consume(characters), line_, column_); \
    }

#define ENCLOSING_TOKEN(startChars, endChars, type) \
    if (match(startChars)) { \
        int line_ = line; \
        int column_ = column; \
        token = Token(type, consumeUntil(endChars), line_, column_); \
    }
#define SET_TOKEN(characters, type) \
    if (matchAny(characters)) { \
        int line_ = line; \
        int column_ = column; \
        token = Token(type, consumeAny(characters), line_, column_); \
    }
#define COMPLEX_SET_TOKEN(beginCharacters, characters, type) \
    if (matchAny(beginCharacters)) { \
        int line_ = line; \
        int column_ = column; \
        token = Token(type, consumeAny(characters), line_, column_); \
    }

Token Tokenizer::nextToken(){
    while (!isAtEnd()) {
        Token token;
        token.type = TokenType::None;
        testErrorLimit();

        COMPLEX_SET_TOKEN(validIdentifierBegin, validIdentifierChars, TokenType::Identifier)
        else ENCLOSING_TOKEN("//", "\n", TokenType::Comment)
        else ENCLOSING_TOKEN("/*", "*/", TokenType::Comment)
        else SIMPLE_TOKEN('!', TokenType::Bang)
        else SIMPLE_TOKEN('~', TokenType::Tilde)
        else SIMPLE_TOKEN('+', TokenType::Plus)
        else SIMPLE_TOKEN('-', TokenType::Minus)
        else SIMPLE_TOKEN('*', TokenType::Star)
        else SIMPLE_TOKEN('/', TokenType::Slash)
        else SIMPLE_TOKEN(';', TokenType::Semicolon)
        else SIMPLE_TOKEN(',', TokenType::Comma)
        else SIMPLE_TOKEN(':', TokenType::Colon)
        else SIMPLE_TOKEN('(', TokenType::LeftParen)
        else SIMPLE_TOKEN(')', TokenType::RightParen)
        else SIMPLE_TOKEN('{', TokenType::LeftBrace)
        else SIMPLE_TOKEN('}', TokenType::RightBrace)
        else SIMPLE_TOKEN('[', TokenType::LeftBracket)
        else SIMPLE_TOKEN(']', TokenType::RightBracket)

        else SET_TOKEN(numbers, TokenType::Number)
        else if (std::isspace(getCurrentChar())) { consume(); }
        else {
            logError("Unexpected character '", consume(), "'");
        }

        // filter out keywords, typenames, and type modifiers
        if (token.type == TokenType::ID){
            if (token.value == "int" || token.value == "char"){
                token.type = TokenType::Typename;
            }
            else if (token.value == "const") {
                token.type = TokenType::TypeModifier;
            }
            else if(token.value == "return") {
                token.type = TokenType::Return;
            }
            else {
                token.type = TokenType::ID;
            }
        }

        if (token.type != TokenType::None) {
            return token;
        }
    }

    logError("Unexpected end of file");
    return Token(TokenType::EndOfFile, "", line, column);
}

#undef SIMPLE_TOKEN
#undef ENCLOSING_TOKEN
#undef SET_TOKEN
#undef COMPLEX_SET_TOKEN


std::vector<Token> Tokenizer::tokenize(){
    std::vector<Token> tokens;
    while (!isAtEnd()) {
        tokens.push_back(nextToken());
    }
    if (numErrors){
        if (numErrors == 1) {
            Error("There was 1 error in the file!");
        }
        else {
            Error("There were ", numErrors, " errors in the file!");
        }
        return {};
    }
    return tokens;
}