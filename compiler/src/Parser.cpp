#include "R-Sharp/Parser.hpp"

Parser::Parser(std::vector<Token> const& tokens, std::string const& filename) : tokens(tokens), filename(filename){
    program = std::make_shared<AstProgram>();
}

std::shared_ptr<AstProgram> Parser::parse() {
    hasError = false;
    isRecovering = false;
    auto prog = parseProgram();
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
        parserError("Expected ", type, " but got ", getCurrentToken());
    }
    return consume();
}
Token Parser::consume(TokenType type, std::string value){
    if (!match(type, value))
        parserError("Expected ", Token(type, value), " but got ", getCurrentToken());
    return consume();
}
Token Parser::consume(std::vector<TokenType> types){
    for (int i = 0; i < types.size(); i++) {
        if (!match(types[i])) {
            parserError("Expected ", types[i], " but got ", getCurrentToken());
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
    parserError(error, " but got ", getCurrentToken());
    return getCurrentToken();
}

bool Parser::isAtEnd(int offset) const{
    return ((currentTokenIndex + offset) >= tokens.size() || tokens[currentTokenIndex + offset].type == TokenType::EndOfFile) && (currentTokenIndex + offset) >= 0;
}

Token Parser::getCurrentToken() const{
    if (currentTokenIndex >= tokens.size()){
        Fatal("Unexpected end of file");
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
        bool wereRecovering = isRecovering;
        auto item = parseProgramItem();

        if (item->getType() == AstNodeType::AstErrorProgramItem && !wereRecovering){
            program->items.push_back(item);
        }
        else if (item->getType() != AstNodeType::AstErrorProgramItem){
            program->items.push_back(item);
            isRecovering = false;
        }
    }
    return program;
}

// program items
std::shared_ptr<AstProgramItem> Parser::parseProgramItem(){
    std::shared_ptr<AstFunction> function = std::make_shared<AstFunction>();
    try{
        function->name = consume(TokenType::ID).value;
        function->parameters = parseParameterList();
        consume(TokenType::Colon);
        function->returnType = parseType();

        if (match(TokenType::Semicolon)){
            // it is a function declaration
            consume(TokenType::Semicolon);
            auto decl = std::make_shared<AstFunctionDeclaration>();
            decl->name = function->name;
            decl->parameters = function->parameters;
            decl->returnType = function->returnType;
            return decl;
        }
        else{
            // a full function definition
            function->body = parseStatement();
            return function;
        }
    }
    catch(ParsingError const& e){
        hasError = true;
        // consume one token to try to recover
        if (isRecovering){
            consume();
        }
        isRecovering = true;
        auto err = std::make_shared<AstErrorProgramItem>(e.what());
        err->token = getCurrentToken();
        return err;
    }
}

std::shared_ptr<AstStatement> Parser::parseStatement() {
    // If an error occurs, the parser will try to recover by skipping this statement and returning an error statement.
    try{
        if (match(TokenType::LeftBrace)) {
            return parseBlock();
        }
        else if (match(TokenType::Return)) {
            return parseReturn();
        }
        else if (match(TokenType::If)){
            return parseConditionalStatement();
        }
        else if (match(TokenType::While)){
            return parseWhileLoop();
        }
        else if (match(TokenType::Do)){
            return parseDoWhileLoop();
        }
        else if (match(TokenType::For)){
            try{
                TokenRestorer _(*this);
                return parseForLoopDeclaration();
            }
            catch(ParsingError const& e){
                return parseForLoopExpression();
            }
        }
        else if (match(TokenType::Break)){
            return parseBreak();
        }
        else if (match(TokenType::Skip)){
            return parseSkip();
        }
        else{
            auto exp = parseOptionalExpression();
            consume(TokenType::Semicolon);
            return std::make_shared<AstExpressionStatement>(exp);
        }
    }
    catch(ParsingError const& e){
        hasError = true;
        // consume one token to try to recover
        if (isRecovering){
            consume();
        }
        isRecovering = true;
        auto err = std::make_shared<AstErrorStatement>(e.what());
        err->token = getCurrentToken();
        return err;
    }
}
std::shared_ptr<AstExpression> Parser::parseExpression() {
    if (match({TokenType::ID, TokenType::Assign})){
        return parseVariableAssignment();
    }
    else{
        return parseConditionalExpression();
    }
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
        parserError("Expected typename, type modifier or array but got ", getCurrentToken());
        return nullptr;
    }
}
std::shared_ptr<AstDeclaration> Parser::parseDeclaration() {
    if (match({TokenType::ID, TokenType::Colon})){
        auto decl = parseVariableDeclaration();
        consume(TokenType::Semicolon);
        return decl;
    }
    else {
        parserError("Expected typename, type modifier or array but got ", getCurrentToken());
        return nullptr;
    }
}

std::shared_ptr<AstBlockItem> Parser::parseBlockItem() {
    if (match({TokenType::ID, TokenType::Colon})){
        return parseDeclaration();
    }
    else{
        return parseStatement();
    }
}


std::shared_ptr<AstReturn> Parser::parseReturn() {
    std::shared_ptr<AstReturn> returnStatement = std::make_shared<AstReturn>();
    consume(TokenType::Return);
    returnStatement->value = parseExpression();
    consume(TokenType::Semicolon);
    return returnStatement;
}
std::shared_ptr<AstBlock> Parser::parseBlock() {
    std::shared_ptr<AstBlock> block = std::make_shared<AstBlock>();
    consume(TokenType::LeftBrace);
    while (!match(TokenType::RightBrace) && !isAtEnd()) {
        bool wereRecovering = isRecovering;
        auto item = parseBlockItem();
        

        if (item->getType() == AstNodeType::AstErrorStatement && !wereRecovering){
            block->items.push_back(item);
        }
        else if (item->getType() != AstNodeType::AstErrorStatement){
            block->items.push_back(item);
            isRecovering = false;
        }
    }
    consume(TokenType::RightBrace);
    return block;
}
std::shared_ptr<AstConditionalStatement> Parser::parseConditionalStatement() {
    std::shared_ptr<AstConditionalStatement> main_conditional = std::make_shared<AstConditionalStatement>();
    auto current_conditional = main_conditional;
    
    consume({TokenType::If, TokenType::LeftParen});
    main_conditional->condition = parseExpression();
    consume(TokenType::RightParen);
    main_conditional->trueStatement = parseStatement();

    while (match(TokenType::Elif)) {
        consume(TokenType::Elif);
        current_conditional->falseStatement = std::make_shared<AstConditionalStatement>();
        current_conditional = std::dynamic_pointer_cast<AstConditionalStatement>(current_conditional->falseStatement);
        consume(TokenType::LeftParen);
        current_conditional->condition = parseExpression();
        consume(TokenType::RightParen);
        current_conditional->trueStatement = parseStatement();
    }

    if (match(TokenType::Else)) {
        consume(TokenType::Else);
        current_conditional->falseStatement = parseStatement();
    }

    return main_conditional;
}
std::shared_ptr<AstForLoopDeclaration> Parser::parseForLoopDeclaration() {
    std::shared_ptr<AstForLoopDeclaration> forLoop = std::make_shared<AstForLoopDeclaration>();
    consume({TokenType::For, TokenType::LeftParen});
    forLoop->variable = parseVariableDeclaration();
    consume(TokenType::Semicolon);
    forLoop->condition = parseOptionalExpression();
    consume(TokenType::Semicolon);
    forLoop->increment = parseOptionalExpression();
    consume(TokenType::RightParen);
    forLoop->body = parseStatement();
    return forLoop;
}
std::shared_ptr<AstForLoopExpression> Parser::parseForLoopExpression() {
    std::shared_ptr<AstForLoopExpression> forLoop = std::make_shared<AstForLoopExpression>();
    consume({TokenType::For, TokenType::LeftParen});
    forLoop->variable = parseOptionalExpression();
    consume(TokenType::Semicolon);
    forLoop->condition = parseOptionalExpression();
    consume(TokenType::Semicolon);
    forLoop->increment = parseOptionalExpression();
    consume(TokenType::RightParen);
    forLoop->body = parseStatement();
    return forLoop;
}
std::shared_ptr<AstWhileLoop> Parser::parseWhileLoop() {
    std::shared_ptr<AstWhileLoop> whileLoop = std::make_shared<AstWhileLoop>();
    consume({TokenType::While, TokenType::LeftParen});
    whileLoop->condition = parseExpression();
    consume(TokenType::RightParen);
    whileLoop->body = parseStatement();
    return whileLoop;
}
std::shared_ptr<AstDoWhileLoop> Parser::parseDoWhileLoop() {
    std::shared_ptr<AstDoWhileLoop> doWhileLoop = std::make_shared<AstDoWhileLoop>();
    consume(TokenType::Do);
    doWhileLoop->body = parseStatement();
    consume({TokenType::While, TokenType::LeftParen});
    doWhileLoop->condition = parseExpression();
    consume({TokenType::RightParen, TokenType::Semicolon});
    return doWhileLoop;
}
std::shared_ptr<AstBreak> Parser::parseBreak() {
    consume({TokenType::Break, TokenType::Semicolon});
    return std::make_shared<AstBreak>();
}
std::shared_ptr<AstSkip> Parser::parseSkip() {
    consume({TokenType::Skip, TokenType::Semicolon});
    return std::make_shared<AstSkip>();
}



std::shared_ptr<AstExpression> Parser::parseConditionalExpression() {
    std::shared_ptr<AstExpression> condition = parseLogicalOrExp();
    if (!match(TokenType::QuestionMark)){
        return condition;
    }
    std::shared_ptr<AstConditionalExpression> conditional = std::make_shared<AstConditionalExpression>();
    conditional->condition = condition;
    consume(TokenType::QuestionMark);
    conditional->trueExpression = parseExpression();
    consume(TokenType::Colon);
    conditional->falseExpression = parseExpression();
    return conditional;
}

std::shared_ptr<AstExpression> Parser::parseLogicalOrExp() {
    auto andExp = parseLogicalAndExp();

    while (match(TokenType::DoublePipe)) {
        Token operatorToken = consume(TokenType::DoublePipe);
        auto next_andExp = parseLogicalAndExp();

        andExp = std::make_shared<AstBinary>(andExp, toBinaryOperator(operatorToken.type), next_andExp);
    }
    return andExp;
}
std::shared_ptr<AstExpression> Parser::parseLogicalAndExp() {
    auto equalityExp = parseEqualityExp();

    while (match(TokenType::DoubleAmpersand)) {
        Token operatorToken = consume(TokenType::DoubleAmpersand);
        auto next_equalityExp = parseEqualityExp();

        equalityExp = std::make_shared<AstBinary>(equalityExp, toBinaryOperator(operatorToken.type), next_equalityExp);
    }
    return equalityExp;
}
std::shared_ptr<AstExpression> Parser::parseEqualityExp() {
    auto relationalExp = parseRelationalExp();

    while (matchAny({TokenType::EqualEqual, TokenType::NotEqual})) {
        Token operatorToken = consumeAnyOne({TokenType::EqualEqual, TokenType::NotEqual});
        auto next_relationalExp = parseRelationalExp();

        relationalExp = std::make_shared<AstBinary>(relationalExp, toBinaryOperator(operatorToken.type), next_relationalExp);
    }
    return relationalExp;
}
std::shared_ptr<AstExpression> Parser::parseRelationalExp() {
    auto additiveExp = parseAdditiveExp();

    while (matchAny({TokenType::GreaterThan, TokenType::GreaterThanEqual, TokenType::LessThan, TokenType::LessThanEqual})) {
        Token operatorToken = consumeAnyOne({TokenType::GreaterThan, TokenType::GreaterThanEqual, TokenType::LessThan, TokenType::LessThanEqual});
        auto next_additiveExp = parseAdditiveExp();

        additiveExp = std::make_shared<AstBinary>(additiveExp, toBinaryOperator(operatorToken.type), next_additiveExp);
    }
    return additiveExp;
}
std::shared_ptr<AstExpression> Parser::parseAdditiveExp() {
    auto term = parseTerm();

    while (matchAny({TokenType::Plus, TokenType::Minus})) {
        Token operatorToken = consumeAnyOne({TokenType::Plus, TokenType::Minus});
        auto next_term = parseTerm();

        term = std::make_shared<AstBinary>(term, toBinaryOperator(operatorToken.type), next_term);
    }
    return term;
}
std::shared_ptr<AstExpression> Parser::parseTerm() {
    auto factor = parseFactor();

    while (matchAny({TokenType::Star, TokenType::Slash})) {
        Token operatorToken = consumeAnyOne({TokenType::Star, TokenType::Slash});
        auto next_factor = parseFactor();

        factor = std::make_shared<AstBinary>(factor, toBinaryOperator(operatorToken.type), next_factor);
    }
    return factor;
}
std::shared_ptr<AstExpression> Parser::parseFactor() {
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
    else if (match({TokenType::ID, TokenType::LeftParen}))
        return parseFunctionCall();
    else if (match(TokenType::ID))
        return std::make_shared<AstVariableAccess>(consume(TokenType::ID).value);
    else{
        parserError("Expected expression but got ", getCurrentToken());
        return nullptr;
    }
}
std::shared_ptr<AstInteger> Parser::parseNumber() {
    std::shared_ptr<AstInteger> number = std::make_shared<AstInteger>();
    number->value = std::stoi(consume(TokenType::Number).value);
    return number;
}
std::shared_ptr<AstVariableAccess> Parser::parseVariableAccess() {
    std::shared_ptr<AstVariableAccess> variableAccess = std::make_shared<AstVariableAccess>();
    variableAccess->name = consume(TokenType::ID).value;
    return variableAccess;
}
std::shared_ptr<AstVariableAssignment> Parser::parseVariableAssignment() {
    std::shared_ptr<AstVariableAssignment> variableAssignment = std::make_shared<AstVariableAssignment>();
    variableAssignment->name = consume(TokenType::ID).value;
    consume(TokenType::Assign);
    variableAssignment->value = parseExpression();
    return variableAssignment;
}
std::shared_ptr<AstFunctionCall> Parser::parseFunctionCall() {
    std::shared_ptr<AstFunctionCall> functionCall = std::make_shared<AstFunctionCall>();
    functionCall->name = consume(TokenType::ID).value;
    consume(TokenType::LeftParen);
    while (!match(TokenType::RightParen)) {
        functionCall->arguments.push_back(parseExpression());
        if (match(TokenType::Comma))
            consume(TokenType::Comma);
        else{
            break;
        }
    }
    consume(TokenType::RightParen);
    return functionCall;
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
        parameterList->parameters.push_back(parseVariableDeclaration());
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


std::shared_ptr<AstVariableDeclaration> Parser::parseVariableDeclaration() {
    std::shared_ptr<AstVariableDeclaration> variable = std::make_shared<AstVariableDeclaration>();
    variable->name = consume(TokenType::Identifier).value;
    consume(TokenType::Colon);
    variable->type = parseType();
    if (match(TokenType::Assign)) {
        consume(TokenType::Assign);
        variable->value = parseExpression();
    }
    return variable;
}


// helpers
std::shared_ptr<AstExpression> Parser::parseOptionalExpression(){
    try{
        return parseExpression();
    }
    catch(ParsingError& e){
        return std::make_shared<AstEmptyExpression>();
    }
}