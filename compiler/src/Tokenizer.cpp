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


Tokenizer::Tokenizer(std::string const& filename): currentPosition(0), line(1), column(1), filename(filename) {
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

bool Tokenizer::match(int offset, std::string str) const {
    return !isAtEnd(offset) && source.substr(currentPosition+offset, str.size()) == str;
}
bool Tokenizer::match(int offset, char c) const {
    return !isAtEnd(offset) && getChar(offset) == c;
}
bool Tokenizer::matchAny(int offset, std::string str) const {
    for (char c : str) {
        if (match(offset, c)) {
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
std::string Tokenizer::consume(std::string str) {
    if (!match(str)) {
        logError("Expected: \"", str, "\"");
        return "\0";
    }
    std::string result;
    for (int i = 0; i < str.size(); i++) {
        result += consume();
    }
    return result;
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

#define SIMPLE_TOKEN(characters, type) \
    if (match(characters)) { \
        int line_ = line; \
        int column_ = column; \
        size_t pos_ = currentPosition; \
        auto chars = consume(characters); \
        token = Token(type, chars, {pos_, currentPosition, line_, column_}); \
    }
#define KEYWORD_TOKEN(characters, type) \
    if (match(characters) && !matchAny(std::string(characters).size(), validIdentifierChars)) { \
        int line_ = line; \
        int column_ = column; \
        size_t pos_ = currentPosition; \
        auto chars = consume(characters); \
        token = Token(type, chars, {pos_, currentPosition, line_, column_}); \
    }

#define ENCLOSING_TOKEN(startChars, endChars, type) \
    if (match(startChars)) { \
        int line_ = line; \
        int column_ = column; \
        size_t pos_ = currentPosition; \
        auto chars = consumeUntil(endChars); \
        token = Token(type, chars, {pos_, currentPosition, line_, column_}); \
    }
#define SET_TOKEN(characters, type) \
    if (matchAny(characters)) { \
        int line_ = line; \
        int column_ = column; \
        size_t pos_ = currentPosition; \
        auto chars = consumeAny(characters); \
        token = Token(type, chars, {pos_, currentPosition, line_, column_}); \
    }
#define COMPLEX_SET_TOKEN(beginCharacters, characters, type) \
    if (matchAny(beginCharacters)) { \
        int line_ = line; \
        int column_ = column; \
        size_t pos_ = currentPosition; \
        auto chars = consumeAny(characters); \
        token = Token(type, chars, {pos_, currentPosition, line_, column_}); \
    }

Token Tokenizer::nextToken(){
    while (!isAtEnd()) {
        Token token;
        token.type = TokenType::None;

        ENCLOSING_TOKEN("//", "\n", TokenType::Comment)
        else ENCLOSING_TOKEN("/*", "*/", TokenType::Comment)

        else KEYWORD_TOKEN("if", TokenType::If)
        else KEYWORD_TOKEN("elif", TokenType::Elif)
        else KEYWORD_TOKEN("else", TokenType::Else)

        else KEYWORD_TOKEN("return", TokenType::Return)

        else KEYWORD_TOKEN("while", TokenType::While)
        else KEYWORD_TOKEN("for", TokenType::For)
        else KEYWORD_TOKEN("do", TokenType::Do)
        else KEYWORD_TOKEN("break", TokenType::Break)
        else KEYWORD_TOKEN("skip", TokenType::Skip)

        else KEYWORD_TOKEN("i32", TokenType::Typename)
        else KEYWORD_TOKEN("i64", TokenType::Typename)
        else KEYWORD_TOKEN("char", TokenType::Typename)
        else KEYWORD_TOKEN("const", TokenType::TypeModifier)

        else SIMPLE_TOKEN(';', TokenType::Semicolon)
        else SIMPLE_TOKEN(',', TokenType::Comma)
        else SIMPLE_TOKEN(':', TokenType::Colon)

        else SIMPLE_TOKEN('(', TokenType::LeftParen)
        else SIMPLE_TOKEN(')', TokenType::RightParen)
        else SIMPLE_TOKEN('[', TokenType::LeftBracket)
        else SIMPLE_TOKEN(']', TokenType::RightBracket)
        else SIMPLE_TOKEN('{', TokenType::LeftBrace)
        else SIMPLE_TOKEN('}', TokenType::RightBrace)
        
        else SIMPLE_TOKEN("&&", TokenType::DoubleAmpersand)
        else SIMPLE_TOKEN("||", TokenType::DoublePipe)
        else SIMPLE_TOKEN("==", TokenType::EqualEqual)
        else SIMPLE_TOKEN("!=", TokenType::NotEqual)
        else SIMPLE_TOKEN("<=", TokenType::LessThanEqual)
        else SIMPLE_TOKEN(">=", TokenType::GreaterThanEqual)
        else SIMPLE_TOKEN("<", TokenType::LessThan)
        else SIMPLE_TOKEN(">", TokenType::GreaterThan)

        else SIMPLE_TOKEN('!', TokenType::Bang)
        else SIMPLE_TOKEN('~', TokenType::Tilde)
        else SIMPLE_TOKEN('?', TokenType::QuestionMark)

        else SIMPLE_TOKEN('+', TokenType::Plus)
        else SIMPLE_TOKEN('-', TokenType::Minus)
        else SIMPLE_TOKEN('*', TokenType::Star)
        else SIMPLE_TOKEN('/', TokenType::Slash)
        else SIMPLE_TOKEN('%', TokenType::Percent)

        else SIMPLE_TOKEN('=', TokenType::Assign)

        else COMPLEX_SET_TOKEN(validIdentifierBegin, validIdentifierChars, TokenType::Identifier)
        else SET_TOKEN(numbers, TokenType::Number)
        else if (std::isspace(getCurrentChar())) { consume(); }
        else {
            logError("Unexpected character '", consume(), "'");
        }

        if (token.type != TokenType::None) {
            return token;
        }
    }

    return Token(TokenType::EndOfFile, "", {currentPosition, currentPosition, line, column});
}

#undef SIMPLE_TOKEN
#undef ENCLOSING_TOKEN
#undef SET_TOKEN
#undef COMPLEX_SET_TOKEN


std::vector<Token> Tokenizer::tokenize(){
    resetErrorCount();
    setErrorLimit(20);

    std::vector<Token> tokens;
    while (!isAtEnd()) {
        tokens.push_back(nextToken());
    }
    tokens.push_back(Token(TokenType::EndOfFile, "", {currentPosition, currentPosition, line, column}));
    if (getErrorCount()){
        Fatal("Encountered ", getErrorCount(), " error", getErrorCount() == 1 ? "" : "s");
        return {};
    }
    return tokens;
}