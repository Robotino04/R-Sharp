#include "R-Sharp/Parser.hpp"

Parser::Parser(std::vector<Token> const& tokens, std::string const& filename) : tokens(tokens), filename(filename){
    program = std::make_shared<AstProgram>();
}

std::shared_ptr<AstProgram> Parser::parse() {
    auto prog = parseProgram();
    if (numErrors){
        Fatal("Parser error");
    }
    return prog;
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
        if (getToken(i).type != types[i]) return false;
    }
    return true;
}
bool Parser::matchAny(std::vector<TokenType> types) const{
    for (TokenType type : types) {
        if (match(type)) {
            return true;
        }
    }
    return false;
}

bool Parser::match(int offset, TokenType type) const{
    return !isAtEnd(offset) && getToken(offset).type == type;
}
bool Parser::match(int offset, TokenType type, std::string value) const{
    return !isAtEnd(offset) && getToken(offset).type == type && getToken(offset).value == value;
}
bool Parser::match(int offset, std::vector<TokenType> types) const{
    if (isAtEnd(offset + types.size()-1)) return false;
    for (int i = 0; i < types.size(); i++) {
        if (getToken(offset + i).type != types[i]) return false;
    }
    return true;
}
bool Parser::matchAny(int offset, std::vector<TokenType> types) const{
    for (TokenType type : types) {
        if (match(offset, type)) {
            return true;
        }
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
Token Parser::consumeAnyOne(std::vector<TokenType> types){
    for (TokenType type : types) {
        if (match(type)) {
            return consume();
        }
    }
    std::string error = "Expected one of ";
    for (int i = 0; i < types.size(); i++) {
        error += std::to_string(types[i]);
        if (i != types.size()-1) error += ", ";
    }
    logError(error, " but got ", getCurrentToken());
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


void Parser::testErrorLimit() const{
    if (numErrors > maxErrors) {
        Fatal("Too many errors");
    }
}

std::shared_ptr<AstProgram> Parser::parseProgram() {
    testErrorLimit();
    while (!isAtEnd()) {
        program->functions.push_back(parseFunction());
    }
    return program;
}

std::shared_ptr<AstFunction> Parser::parseFunction() {
    testErrorLimit();
    std::shared_ptr<AstFunction> function = std::make_shared<AstFunction>();
    function->name = consume(TokenType::ID).value;
    function->parameters = parseParameterList();
    consume(TokenType::Colon);
    function->returnType = parseType();
    function->body = parseBlock();
    return function;
}
std::shared_ptr<AstBlock> Parser::parseBlock() {
    testErrorLimit();
    std::shared_ptr<AstBlock> block = std::make_shared<AstBlock>();
    consume(TokenType::LeftBrace);
    while (!match(TokenType::RightBrace)) {
        block->statements.push_back(parseStatement());
    }
    consume(TokenType::RightBrace);
    return block;
}

std::shared_ptr<AstStatement> Parser::parseStatement() {
    testErrorLimit();
    if (match(TokenType::LeftBrace)) {
        return parseBlock();
    }
    else if (match(TokenType::Return)) {
        return parseReturn();
    }
    else if (match({TokenType::ID, TokenType::Colon})){
        return parseVariableDeclaration();
    }
    else{
        auto exp = parseExpression();
        consume(TokenType::Semicolon);
        return std::make_shared<AstExpressionStatement>(exp);
    }
}

std::shared_ptr<AstReturn> Parser::parseReturn() {
    testErrorLimit();
    std::shared_ptr<AstReturn> returnStatement = std::make_shared<AstReturn>();
    consume(TokenType::Return);
    returnStatement->value = parseExpression();
    consume(TokenType::Semicolon);
    return returnStatement;
}

std::shared_ptr<AstExpression> Parser::parseExpression() {
    testErrorLimit();
    
    if (match({TokenType::ID, TokenType::Assign})){
        return parseVariableAssignment();
    }
    else{
        return parseLogicalOrExp();
    }
}

std::shared_ptr<AstExpression> Parser::parseLogicalOrExp() {
    testErrorLimit();
    auto andExp = parseLogicalAndExp();

    while (match(TokenType::DoublePipe)) {
        Token operatorToken = consume(TokenType::DoublePipe);
        auto next_andExp = parseLogicalAndExp();

        andExp = std::make_shared<AstBinary>(andExp, toBinaryOperator(operatorToken.type), next_andExp);
    }
    return andExp;
}


std::shared_ptr<AstExpression> Parser::parseLogicalAndExp() {
    testErrorLimit();
    auto equalityExp = parseEqualityExp();

    while (match(TokenType::DoubleAmpersand)) {
        Token operatorToken = consume(TokenType::DoubleAmpersand);
        auto next_equalityExp = parseEqualityExp();

        equalityExp = std::make_shared<AstBinary>(equalityExp, toBinaryOperator(operatorToken.type), next_equalityExp);
    }
    return equalityExp;
}

std::shared_ptr<AstExpression> Parser::parseEqualityExp() {
    testErrorLimit();
    auto relationalExp = parseRelationalExp();

    while (matchAny({TokenType::EqualEqual, TokenType::NotEqual})) {
        Token operatorToken = consumeAnyOne({TokenType::EqualEqual, TokenType::NotEqual});
        auto next_relationalExp = parseRelationalExp();

        relationalExp = std::make_shared<AstBinary>(relationalExp, toBinaryOperator(operatorToken.type), next_relationalExp);
    }
    return relationalExp;
}

std::shared_ptr<AstExpression> Parser::parseRelationalExp() {
    testErrorLimit();
    auto additiveExp = parseAdditiveExp();

    while (matchAny({TokenType::GreaterThan, TokenType::GreaterThanEqual, TokenType::LessThan, TokenType::LessThanEqual})) {
        Token operatorToken = consumeAnyOne({TokenType::GreaterThan, TokenType::GreaterThanEqual, TokenType::LessThan, TokenType::LessThanEqual});
        auto next_additiveExp = parseAdditiveExp();

        additiveExp = std::make_shared<AstBinary>(additiveExp, toBinaryOperator(operatorToken.type), next_additiveExp);
    }
    return additiveExp;
}

std::shared_ptr<AstExpression> Parser::parseAdditiveExp() {
    testErrorLimit();
    auto term = parseTerm();

    while (matchAny({TokenType::Plus, TokenType::Minus})) {
        Token operatorToken = consumeAnyOne({TokenType::Plus, TokenType::Minus});
        auto next_term = parseTerm();

        term = std::make_shared<AstBinary>(term, toBinaryOperator(operatorToken.type), next_term);
    }
    return term;
}

std::shared_ptr<AstExpression> Parser::parseTerm() {
    testErrorLimit();
    auto factor = parseFactor();

    while (matchAny({TokenType::Star, TokenType::Slash})) {
        Token operatorToken = consumeAnyOne({TokenType::Star, TokenType::Slash});
        auto next_factor = parseFactor();

        factor = std::make_shared<AstBinary>(factor, toBinaryOperator(operatorToken.type), next_factor);
    }
    return factor;
}

std::shared_ptr<AstExpression> Parser::parseFactor() {
    testErrorLimit();
    
    if (match(TokenType::LeftParen)) {
        consume(TokenType::LeftParen);
        auto expression = parseExpression();
        consume(TokenType::RightParen);
        return expression;
    }
    else if (matchAny({TokenType::Bang, TokenType::Minus, TokenType::Tilde})) {
        Token operatorToken = consumeAnyOne({TokenType::Bang, TokenType::Minus, TokenType::Tilde});
        auto factor = parseFactor();
        return std::make_shared<AstUnary>(toUnaryOperator(operatorToken.type), factor);
    }
    else if (match(TokenType::Number))
        return parseNumber();
    else if (match(TokenType::ID)){
        return std::make_shared<AstVariableAccess>(consume(TokenType::ID).value);
    }
    else{
        logError("Expected expression but got ", getCurrentToken());
        return nullptr;
    }
}

std::shared_ptr<AstInteger> Parser::parseNumber() {
    testErrorLimit();
    std::shared_ptr<AstInteger> number = std::make_shared<AstInteger>();
    number->value = std::stoi(consume(TokenType::Number).value);
    return number;
}

std::shared_ptr<AstVariableAccess> Parser::parseVariableAccess() {
    testErrorLimit();

    std::shared_ptr<AstVariableAccess> variableAccess = std::make_shared<AstVariableAccess>();
    variableAccess->name = consume(TokenType::ID).value;
    return variableAccess;
}

std::shared_ptr<AstVariableAssignment> Parser::parseVariableAssignment() {
    testErrorLimit();
    std::shared_ptr<AstVariableAssignment> variableAssignment = std::make_shared<AstVariableAssignment>();
    variableAssignment->name = consume(TokenType::ID).value;
    consume(TokenType::Assign);
    variableAssignment->value = parseExpression();
    return variableAssignment;
}

std::shared_ptr<AstVariableDeclaration> Parser::parseVariableDeclaration() {
    testErrorLimit();
    std::shared_ptr<AstVariableDeclaration> variable = std::make_shared<AstVariableDeclaration>();
    variable->name = consume(TokenType::Identifier).value;
    consume(TokenType::Colon);
    variable->type = parseType();
    if (match(TokenType::Assign)) {
        consume(TokenType::Assign);
        variable->value = parseExpression();
    }
    consume(TokenType::Semicolon);
    return variable;
}

std::shared_ptr<AstType> Parser::parseType() {
    testErrorLimit();
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
    testErrorLimit();
    std::shared_ptr<AstBuiltinType> type = std::make_shared<AstBuiltinType>();
    type->name = consume(TokenType::Typename).value;
    return type;
}

std::shared_ptr<AstTypeModifier> Parser::parseTypeModifier() {
    testErrorLimit();
    std::shared_ptr<AstTypeModifier> typeModifier = std::make_shared<AstTypeModifier>();
    typeModifier->name = consume(TokenType::TypeModifier).value;
    return typeModifier;
}

std::shared_ptr<AstParameterList> Parser::parseParameterList() {
    testErrorLimit();
    std::shared_ptr<AstParameterList> parameterList = std::make_shared<AstParameterList>();
    consume(TokenType::LeftParen);
    while (!match(TokenType::RightParen)) {
        parameterList->parameters.push_back(parseVariableDeclaration());
        if (!match(TokenType::RightParen)) {
            consume(TokenType::Comma);
        }
    }
    consume(TokenType::RightParen);
    return parameterList;
}

std::shared_ptr<AstArray> Parser::parseArray() {
    testErrorLimit();
    std::shared_ptr<AstArray> array = std::make_shared<AstArray>();
    consume(TokenType::LeftBracket);
    array->type = parseType();
    consume(TokenType::RightBracket);
    return array;
}