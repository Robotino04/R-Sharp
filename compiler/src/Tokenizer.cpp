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
        token.type = TokenType_EndOfFile;
        return token;
    }
    char c = getCurrentChar();
    switch (c) {
        case 'a'...'z':
        case 'A'...'Z':
        case '_':
            while (!isDone() && (
                (c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') ||
                c == '_')) {
                token.value += c;
                c = advance();
            }
            if (token.value == "const") {
                token.type = TokenType_TypeModifier;
            }
            else if(token.value == "return") {
                token.type = TokenType_Return;
            }
            else {
                token.type = TokenType_ID;
            }
            break;
        case '0'...'9':
            token.type = TokenType_Number;
            while (!isDone() && (
                (c >= '0' && c <= '9') ||
                c == '.')) {
                token.value += c;
                c = advance();
            }
            break;
        case ';':
            token.type = TokenType_Semicolon;
            token.value = c;
            advance();
            break;
        case ':':
            token.type = TokenType_Colon;
            token.value = c;
            advance();
            break;
        case ',':
            token.type = TokenType_Comma;
            token.value = c;
            advance();
            break;
        case '(':
            token.type = TokenType_OpenParenthesis;
            token.value = c;
            advance();
            break;
        case ')':
            token.type = TokenType_CloseParenthesis;
            token.value = c;
            advance();
            break;
        case '[':
            token.type = TokenType_OpenBracket;
            token.value = c;
            advance();
            break;
        case ']':
            token.type = TokenType_CloseBracket;
            token.value = c;
            advance();
            break;
        case '{':
            token.type = TokenType_OpenBrace;
            token.value = c;
            advance();
            break;
        case '}':
            token.type = TokenType_CloseBrace;
            token.value = c;
            advance();
            break;
        case '*':
            token.type = TokenType_Star;
            token.value = c;
            advance();
            break;
        case '/':
            advance();
            switch (getCurrentChar()) {
                case '/':
                    advance();  // consume the second '/'
                    token.type = TokenType_Comment;
                    token.value = advanceUntil("\n");
                    break;
                case '*':
                    advance();
                    token.type = TokenType_MultilineComment;
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