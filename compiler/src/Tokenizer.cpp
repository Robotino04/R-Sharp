#include "R-Sharp/Tokenizer.hpp"
#include "R-Sharp/Logging.hpp"

Tokenizer::Tokenizer(std::string const& source) : source(source), currentPosition(0) {
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
                        Error("Unterminated multiline comment");
                    }
                    advance(); // consume the '*'
                    advance(); // consume the '/'
                    break;
                
                default:
                    Error("Division is not yet supported!");
                    break;
            }
            break;
            
        default:
            if (std::isspace(c)) {
                while (!isDone() && std::isspace(c = advance()));
                return nextToken();
            }
            else{
                Error("Unexpected character '", c, "'");
            }
            token.type = TokenType_None;
            break;
    }

    return token;
}


std::vector<Token> Tokenizer::tokenize(){
    LoggingContext("Tokenizer");
    std::vector<Token> tokens;
    while (!isDone()) {
        tokens.push_back(nextToken());
    }
    return tokens;
}