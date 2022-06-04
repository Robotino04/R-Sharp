#include "R-Sharp/Tokenizer.hpp"

#include <fstream>
#include <sstream>

Tokenizer::Tokenizer(std::string const& filename): currentPosition(0), line(1), column(1), numErrors(0), filename(filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        Fatal("Could not open file: \"", filename, "\"");
    }
    std::stringstream ss;
    ss << file.rdbuf();
    source = ss.str();
}

bool Tokenizer::isDone() const {
    return currentPosition >= source.size();
}

char Tokenizer::getCurrentChar() const{
    return source[currentPosition];
}
char Tokenizer::getNextChar() const{
    return source[currentPosition + 1];
}
char Tokenizer::advance(){
    if (getCurrentChar() == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    currentPosition++;
    return getCurrentChar();
}

std::string Tokenizer::advanceUntil(std::string str){
    std::string result;
    while (!isDone() && source.substr(currentPosition, str.size()) != str){
        result += getCurrentChar();
        advance();
    }
    return result;
}

Token Tokenizer::nextToken(){
    Token token;
    if (isDone()) {
        token.type = TokenType::EndOfFile;
        token.line = line; token.column = column;
        return token;
    }
    char c = getCurrentChar();
    switch (c) {
        case 'a'...'z':
        case 'A'...'Z':
        case '_':
            token.line = line; token.column = column;
            while (!isDone() && (
                (c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') ||
                c == '_')) {
                token.value += c;
                c = advance();
            }
            if(token.value == "int" || token.value == "char"){
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
            break;
        case '-':
        case '0'...'9':
            token.line = line; token.column = column;
            token.type = TokenType::Number;
            token.value += c;
            c = advance();
            while (!isDone() && (
                (c >= '0' && c <= '9') ||
                c == '.')) {
                token.value += c;
                c = advance();
            }
            break;
        case ';':
            token.line = line; token.column = column;
            token.type = TokenType::Semicolon;
            token.value = c;
            advance();
            break;
        case ':':
            token.line = line; token.column = column;
            token.type = TokenType::Colon;
            token.value = c;
            advance();
            break;
        case ',':
            token.line = line; token.column = column;
            token.type = TokenType::Comma;
            token.value = c;
            advance();
            break;
        case '(':
            token.line = line; token.column = column;
            token.type = TokenType::LeftParen;
            token.value = c;
            advance();
            break;
        case ')':
            token.line = line; token.column = column;
            token.type = TokenType::RightParen;
            token.value = c;
            advance();
            break;
        case '[':
            token.line = line; token.column = column;
            token.type = TokenType::LeftBracket;
            token.value = c;
            advance();
            break;
        case ']':
            token.line = line; token.column = column;
            token.type = TokenType::RightBracket;
            token.value = c;
            advance();
            break;
        case '{':
            token.line = line; token.column = column;
            token.type = TokenType::LeftBrace;
            token.value = c;
            advance();
            break;
        case '}':
            token.line = line; token.column = column;
            token.type = TokenType::RightBrace;
            token.value = c;
            advance();
            break;
        case '*':
            token.line = line; token.column = column;
            token.type = TokenType::Star;
            token.value = c;
            advance();
            break;
        case '/':
            token.line = line; token.column = column;
            advance();
            switch (getCurrentChar()) {
                case '/':
                    advance();  // consume the second '/'
                    token.type = TokenType::Comment;
                    token.value = advanceUntil("\n");
                    break;
                case '*':
                    advance();
                    token.type = TokenType::MultilineComment;
                    token.value = advanceUntil("*/"); 
                    if (isDone()){
                        logError("Unterminated multiline comment");
                    }
                    else{
                        advance(); // consume the '*'
                        advance(); // consume the '/'
                    }
                    break;
                
                default:
                    logError("Division is not yet supported!");
                    break;
            }
            break;
            
        default:
            token.line = line; token.column = column;
            if (std::isspace(c)) {
                while (!isDone() && std::isspace(c = advance()));
                return nextToken();
            }
            else{
                logError("Unexpected character '", c, "'");
            }
            advance();
            break;
    }

    return token;
}


std::vector<Token> Tokenizer::tokenize(){
    std::vector<Token> tokens;
    while (!isDone()) {
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