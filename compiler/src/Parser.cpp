#include "R-Sharp/Parser.hpp"

Parser::Parser(std::vector<Token> const& tokens, std::string const& filename) : tokens(tokens), filename(filename){
    program = std::make_shared<AstProgram>();
}

std::shared_ptr<AstProgram> Parser::parse() {
    return parseProgram();
}


bool Parser::match(TokenType type) const{
    return !isAtEnd() && getCurrentToken().type == type;
}
bool Parser::match(TokenType type, std::string value) const{
    return !isAtEnd() && getCurrentToken().type == type && getCurrentToken().value == value;
}
bool Parser::match(std::vector<TokenType> types) const{
    if (isAtEnd(types.size()-1)) return false;
    for (int i = 0; i < types.size(); i++) {
        if (getToken(i).type == types[i]) return true;
    }
    return false;
}

bool Parser::match(int offset, TokenType type) const{
    return !isAtEnd(offset) && getToken(offset).type == type;
}
bool Parser::match(int offset,TokenType type, std::string value) const{
    return !isAtEnd(offset) && getToken(offset).type == type && getToken(offset).value == value;
}
bool Parser::match(int offset,std::vector<TokenType> types) const{
    if (isAtEnd(offset + types.size()-1)) return false;
    for (int i = 0; i < types.size(); i++) {
        if (getToken(offset + i).type == types[i]) return true;
    }
    return false;
}

Token Parser::consume(){
    Token token = getCurrentToken();
    currentTokenIndex++;
    return token;
}
Token Parser::Parser::consume(TokenType type){
    if (!match(type)){
        logError("Expected ", type, " but got ", getCurrentToken());
    }
    return consume();
}
Token Parser::consume(TokenType type, std::string value){
    if (!match(type, value))
        logError("Expected ", Token(type, value), " but got ", getCurrentToken());
    return consume();
}
Token Parser::consume(std::vector<TokenType> types){
    for (int i = 0; i < types.size(); i++) {
        if (!match(types[i])) {
            logError("Expected ", types[i], " but got ", getCurrentToken());
        }
        consume();
    }
    return getCurrentToken();
}

bool Parser::isAtEnd(int offset) const{
    return ((currentTokenIndex + offset) >= tokens.size() || tokens[currentTokenIndex].type == TokenType::EndOfFile) && (currentTokenIndex + offset) >= 0;
}

Token Parser::getCurrentToken() const{
    if (isAtEnd()){
        logError("Unexpected end of file");
        return Token(TokenType::EndOfFile, "");
    }
    if (currentTokenIndex < 0){
        Fatal("Tried to get current token before any tokens were consumed");
        return Token(TokenType::EndOfFile, "");
    }
    return tokens[currentTokenIndex];
}
Token Parser::getToken(int offset) const{
    if (isAtEnd(offset)) return Token(TokenType::EndOfFile, "");
    return tokens[currentTokenIndex + offset];
}

std::shared_ptr<AstProgram> Parser::parseProgram() {
    while (!isAtEnd()) {
        program->functions.push_back(parseFunction());
    }
    return program;
}

std::shared_ptr<AstFunction> Parser::parseFunction() {
    std::shared_ptr<AstFunction> function = std::make_shared<AstFunction>();
    function->name = consume(TokenType::ID).value;
    function->parameters = parseParameterList();
    consume(TokenType::Colon);
    function->returnType = parseType();
    function->body = parseBlock();
    return function;
}
std::shared_ptr<AstBlock> Parser::parseBlock() {
    std::shared_ptr<AstBlock> block = std::make_shared<AstBlock>();
    consume(TokenType::LeftBrace);
    while (!match(TokenType::RightBrace)) {
        block->statements.push_back(parseStatement());
    }
    consume(TokenType::RightBrace);
    return block;
}

std::shared_ptr<AstStatement> Parser::parseStatement() {
    if (match(TokenType::LeftBrace)) {
        return parseBlock();
    }
    // if (match(TokenType::TokenType_If)) {
    //     return parseIf();
    // }
    // if (match(TokenType::TokenType_While)) {
    //     return parseWhile();
    // }
    // if (match(TokenType::TokenType_For)) {
    //     return parseFor();
    // }
    // if (match(TokenType::TokenType_Break)) {
    //     return parseBreak();
    // }
    // if (match(TokenType::TokenType_Continue)) {
    //     return parseContinue();
    // }
    if (match(TokenType::Return)) {
        return parseReturn();
    }
    // if (match(TokenType::LeftParen)) {
    //     return parseExpression();
    // }
    // if (match(TokenType::Identifier)) {
    //     return parseAssignment();
    // }
    logError("Expected statement but got ", getCurrentToken());
    return nullptr;
}

std::shared_ptr<AstReturn> Parser::parseReturn() {
    std::shared_ptr<AstReturn> returnStatement = std::make_shared<AstReturn>();
    consume(TokenType::Return);
    returnStatement->value = parseExpression();
    consume(TokenType::Semicolon);
    return returnStatement;
}

std::shared_ptr<AstExpression> Parser::parseExpression() {
    return parseNumber();
}

std::shared_ptr<AstInteger> Parser::parseNumber() {
    std::shared_ptr<AstInteger> number = std::make_shared<AstInteger>();
    number->value = std::stoi(consume(TokenType::Number).value);
    return number;
}

std::shared_ptr<AstVariableDeclaration> Parser::parseVariable() {
    std::shared_ptr<AstVariableDeclaration> variable = std::make_shared<AstVariableDeclaration>();
    variable->name = consume(TokenType::Identifier).value;
    consume(TokenType::Colon);
    variable->type = parseType();
    return variable;
}

std::shared_ptr<AstType> Parser::parseType() {
    std::vector<std::shared_ptr<AstTypeModifier>> typeModifiers;
    while (match(TokenType::TypeModifier)) {
        typeModifiers.push_back(parseTypeModifier());
    }
    if (match(TokenType::LeftBracket)) {
        auto type = parseArray();
        type->modifiers = typeModifiers;
        return type;
    }
    else if (match(TokenType::Typename)) {
        auto type = parseBuiltinType();
        type->modifiers = typeModifiers;
        return type;
    }
    else {
        logError("Expected typename, type modifier or array but got ", getCurrentToken());
        return nullptr;
    }
}

std::shared_ptr<AstBuiltinType> Parser::parseBuiltinType() {
    std::shared_ptr<AstBuiltinType> type = std::make_shared<AstBuiltinType>();
    type->name = consume(TokenType::Typename).value;
    return type;
}

std::shared_ptr<AstTypeModifier> Parser::parseTypeModifier() {
    std::shared_ptr<AstTypeModifier> typeModifier = std::make_shared<AstTypeModifier>();
    typeModifier->name = consume(TokenType::TypeModifier).value;
    return typeModifier;
}

std::shared_ptr<AstParameterList> Parser::parseParameterList() {
    std::shared_ptr<AstParameterList> parameterList = std::make_shared<AstParameterList>();
    consume(TokenType::LeftParen);
    while (!match(TokenType::RightParen)) {
        parameterList->parameters.push_back(parseVariable());
        if (!match(TokenType::RightParen)) {
            consume(TokenType::Comma);
        }
    }
    consume(TokenType::RightParen);
    return parameterList;
}

std::shared_ptr<AstArray> Parser::parseArray() {
    std::shared_ptr<AstArray> array = std::make_shared<AstArray>();
    consume(TokenType::LeftBracket);
    array->type = parseType();
    consume(TokenType::RightBracket);
    return array;
}