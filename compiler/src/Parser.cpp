#include "R-Sharp/Parser.hpp"
#include "R-Sharp/Tokenizer.hpp"

#include <filesystem>

Parser::Parser(std::vector<Token> const& tokens, std::string const& filename, std::string const& importSearchPath, ParsingCache& cache) : tokens(tokens), filename(filename), importSearchPath(importSearchPath), cache(cache){
    program = std::make_shared<AstProgram>();
}

std::shared_ptr<AstProgram> Parser::parse() {
    cache.addWildcard(filename);
    hasError = false;
    isRecovering = false;
    Print("Parsing ", std::filesystem::absolute(filename));
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
Token Parser::consume(TokenType type){
    if (!match(type)){
        parserError("Expected ", tokenTypeToString(type), " but got ", getCurrentToken().toString());
    }
    return consume();
}
Token Parser::consume(TokenType type, std::string value){
    if (!match(type, value))
        parserError("Expected ", Token(type, value).toString(), " but got ", getCurrentToken().toString());
    return consume();
}
Token Parser::consume(std::vector<TokenType> types){
    for (int i = 0; i < types.size(); i++) {
        if (!match(types[i])) {
            parserError("Expected ", tokenTypeToString(types[i]), " but got ", getCurrentToken().toString());
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
        error += tokenTypeToString(types[i]);
        if (i != types.size()-1) error += ", ";
    }
    parserError(error, " but got ", getCurrentToken().toString());
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

        while (match(TokenType::Comment)) consume(TokenType::Comment);

        try{
            TokenRestorer _(*this);
            auto importedThings = parseImportStatement();
            program->items.insert(program->items.end(), importedThings.begin(), importedThings.end());
        }
        catch(ParsingError){
            auto item = parseProgramItem();

            if (item->getType() == AstNodeType::AstErrorProgramItem && !wereRecovering){
                program->items.push_back(item);
            }
            else if (item->getType() != AstNodeType::AstErrorProgramItem){
                program->items.push_back(item);
                isRecovering = false;
            }
        }
    }
    return program;
}

// program items
std::shared_ptr<AstProgramItem> Parser::parseProgramItem(){
    try{
        try{
            TokenRestorer _(*this);
            return parseGlobalVariableDefinition();
        }
        catch(ParsingError const& e){}

        return parseFunctionDefinition();
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
std::shared_ptr<AstFunctionDefinition> Parser::parseFunctionDefinition(){
    auto function = std::make_shared<AstFunctionDefinition>();
    function->tags = parseTags();
    function->token = consume(TokenType::ID);
    function->name = function->token.value;

    function->parameters = parseParameterList();
    consume(TokenType::Colon);
    function->semanticType = parseType();


    function->functionData = std::make_shared<SemanticFunctionData>();
    function->functionData->name = function->name;
    function->functionData->returnType = function->semanticType;
    function->functionData->parameters = function->parameters;

    if(std::find(function->tags->tags.begin(), function->tags->tags.end(), AstTags::Value::Extern) == function->tags->tags.end()){
        auto body = parseStatement();
        if (body->getType() == AstNodeType::AstExpressionStatement && std::dynamic_pointer_cast<AstExpressionStatement>(body)->expression->getType() == AstNodeType::AstEmptyExpression){
            parserError("Function cannot only contain an empty expression. Use {} instead.");
        }
        if (body->getType() == AstNodeType::AstBlock)
            function->body = std::dynamic_pointer_cast<AstBlock>(body);
        else{
            function->body = std::make_shared<AstBlock>();
            function->body->items.push_back(body);
        }
    }
    else{
        consume(TokenType::Semicolon);
    }
    return function;

}
std::shared_ptr<AstVariableDeclaration> Parser::parseGlobalVariableDefinition(){
    auto var = parseVariableDeclaration();
    var->variable = std::make_shared<SemanticVariableData>();
    var->variable->isGlobal = true;
    var->variable->name = var->name;
    consume(TokenType::Semicolon);
    return var;
}
std::vector<std::shared_ptr<AstProgramItem>> Parser::parseImportStatement(){
    bool importEverything = false;

    std::vector<Token> identifiersToImport = {};
    
    if (match(TokenType::Star)){
        consume(TokenType::Star);
        importEverything = true;
    }
    else{
        identifiersToImport.push_back(consume(TokenType::Identifier));
        while(match(TokenType::Comma)){
            consume(TokenType::Comma);
            identifiersToImport.push_back(consume(TokenType::Identifier));
        }
    }

    consume(TokenType::At);

    std::vector<Token> importPath = {};
    importPath.push_back(consume(TokenType::Identifier));
    while(match(TokenType::DoubleColon)){
        consume(TokenType::DoubleColon);
        importPath.push_back(consume(TokenType::Identifier));
    }
    consume(TokenType::Semicolon);
    
    // preprocessing

    std::string path;
    if (importPath.at(0).value == "std")
        importPath.at(0).value = importSearchPath;
    else{
        path = std::filesystem::absolute(filename).remove_filename();
    }

    for (auto tok : importPath){
        path += tok.value + "/";
    }
    // remove the trailing slash
    path = path.substr(0, path.size()-1);
    path += ".rs";
    path = std::filesystem::absolute(path);

    if (!identifiersToImport.empty()){
        identifiersToImport.erase(std::remove_if(identifiersToImport.begin(), identifiersToImport.end(), [&](auto const& ident){
            return cache.contains(path, ident.value);
        }), identifiersToImport.end());
    }
    
    if (cache.containsWildcard(path)){
        // this file was already fully imported
        return {};
    }

    if (importEverything) {
        cache.addWildcard(path);
    }
    else{
        if (identifiersToImport.empty()){
            // everything that is to import, was already imported
            return {};
        }
    }


    // Tokenize and parse
    Tokenizer tokenizer(path);
    auto tokens = tokenizer.tokenize();
    Parser parser(tokens, path, importSearchPath, cache);
    auto importedRoot = parser.parse();

    if (parser.hasErrors()){
        hasError = true;
        return importedRoot->items;
    }

    if (importEverything){
        return importedRoot->items;
    }


    std::vector<std::shared_ptr<AstProgramItem>> importedItems;

    for (auto ident : identifiersToImport){
        const auto filterForName = [&](auto other){
            if (other->getType() == AstNodeType::AstFunctionDefinition){
                auto func = std::static_pointer_cast<AstFunctionDefinition>(other);
                return func->name == ident.value;
            }
            else if (other->getType() == AstNodeType::AstVariableDeclaration){
                auto var = std::static_pointer_cast<AstVariableDeclaration>(other);
                return var->name == ident.value;
            }
            else{
                return false;
            }
        };
        if (cache.containsNonWildcard(path, ident.value)){
            continue;
        }
        auto item = std::find_if(importedRoot->items.begin(), importedRoot->items.end(), filterForName);
        if (item == importedRoot->items.end()){
            auto error = std::make_shared<AstErrorProgramItem>(stringify("Cannot find program item named '", ident.value, "' in ", std::filesystem::absolute(path)));
            error->token = ident;
            importedItems.push_back(error);
            hasError = true;
        }
        else{
            cache.add(path, ident.value);
            importedItems.push_back(*item);
        }
    }
    return importedItems;
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
    try{
        TokenRestorer _(*this);
        return parseAssignment();
    }
    catch(ParsingError){
        return parseConditionalExpression();
    }
}
std::shared_ptr<AstDeclaration> Parser::parseDeclaration() {
    if (match({TokenType::ID, TokenType::Colon})){
        auto decl = parseVariableDeclaration();
        decl->variable = std::make_shared<SemanticVariableData>();
        decl->variable->name = decl->name;
        decl->variable->type = decl->semanticType;
        consume(TokenType::Semicolon);
        return decl;
    }
    else {
        parserError("Expected typename but got ", getCurrentToken().toString());
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
    std::shared_ptr<AstReturn> returnStatement = std::make_shared<AstReturn>(consume(TokenType::Return));
    try{
        TokenRestorer _(*this);
        returnStatement->value = parseOptionalExpression();
        consume(TokenType::Semicolon);
    }
    catch(ParsingError){
        // it is neither an expression nor a simple "return;". still try to parse an expression for a proper error
        returnStatement->value = parseExpression();
        consume(TokenType::Semicolon);
    }
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
    std::shared_ptr<AstConditionalStatement> main_conditional = std::make_shared<AstConditionalStatement>(consume(TokenType::If));
    auto current_conditional = main_conditional;
    
    consume(TokenType::LeftParen);
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
    std::shared_ptr<AstForLoopDeclaration> forLoop = std::make_shared<AstForLoopDeclaration>(consume(TokenType::For));
    consume(TokenType::LeftParen);
    forLoop->initialization = parseVariableDeclaration();
    consume(TokenType::Semicolon);
    forLoop->condition = parseOptionalExpression();
    consume(TokenType::Semicolon);
    forLoop->increment = parseOptionalExpression();
    consume(TokenType::RightParen);
    forLoop->body = parseStatement();
    if (forLoop->body->getType() == AstNodeType::AstBlock)
        std::dynamic_pointer_cast<AstBlock>(forLoop->body)->name = "for loop declaration";


    forLoop->initializationContext = std::make_shared<AstBlock>();
    forLoop->initializationContext->items.push_back(forLoop->initialization);
    forLoop->initializationContext->items.push_back(std::make_shared<AstExpressionStatement>(forLoop->condition));
    forLoop->initializationContext->items.push_back(std::make_shared<AstExpressionStatement>(forLoop->increment));
    forLoop->initializationContext->items.push_back(forLoop->body);
    return forLoop;
}
std::shared_ptr<AstForLoopExpression> Parser::parseForLoopExpression() {
    std::shared_ptr<AstForLoopExpression> forLoop = std::make_shared<AstForLoopExpression>(consume(TokenType::For));
    consume(TokenType::LeftParen);
    forLoop->variable = parseOptionalExpression();
    consume(TokenType::Semicolon);
    forLoop->condition = parseOptionalExpression();
    consume(TokenType::Semicolon);
    forLoop->increment = parseOptionalExpression();
    consume(TokenType::RightParen);
    forLoop->body = parseStatement();
    if (forLoop->body->getType() == AstNodeType::AstBlock)
        std::dynamic_pointer_cast<AstBlock>(forLoop->body)->name = "for loop expression";
        
    return forLoop;
}
std::shared_ptr<AstWhileLoop> Parser::parseWhileLoop() {
    std::shared_ptr<AstWhileLoop> whileLoop = std::make_shared<AstWhileLoop>(consume(TokenType::While));
    consume(TokenType::LeftParen);
    whileLoop->condition = parseExpression();
    consume(TokenType::RightParen);
    whileLoop->body = parseStatement();
    return whileLoop;
}
std::shared_ptr<AstDoWhileLoop> Parser::parseDoWhileLoop() {
    std::shared_ptr<AstDoWhileLoop> doWhileLoop = std::make_shared<AstDoWhileLoop>(consume(TokenType::Do));
    doWhileLoop->body = parseStatement();
    consume({TokenType::While, TokenType::LeftParen});
    doWhileLoop->condition = parseExpression();
    consume({TokenType::RightParen, TokenType::Semicolon});
    return doWhileLoop;
}
std::shared_ptr<AstBreak> Parser::parseBreak() {
    Token tok = consume(TokenType::Break);
    consume(TokenType::Semicolon);
    return std::make_shared<AstBreak>(tok);
}
std::shared_ptr<AstSkip> Parser::parseSkip() {
    Token tok = consume(TokenType::Skip);
    consume(TokenType::Semicolon);
    return std::make_shared<AstSkip>(tok);
}



std::shared_ptr<AstExpression> Parser::parseConditionalExpression() {
    std::shared_ptr<AstExpression> condition = parseLogicalOrExp();
    if (!match(TokenType::QuestionMark)){
        return condition;
    }
    std::shared_ptr<AstConditionalExpression> conditional = std::make_shared<AstConditionalExpression>();
    conditional->condition = condition;
    conditional->token = consume(TokenType::QuestionMark);
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
        andExp->token = operatorToken;
    }
    return andExp;
}
std::shared_ptr<AstExpression> Parser::parseLogicalAndExp() {
    auto equalityExp = parseEqualityExp();

    while (match(TokenType::DoubleAmpersand)) {
        Token operatorToken = consume(TokenType::DoubleAmpersand);
        auto next_equalityExp = parseEqualityExp();

        equalityExp = std::make_shared<AstBinary>(equalityExp, toBinaryOperator(operatorToken.type), next_equalityExp);
        equalityExp->token = operatorToken;
    }
    return equalityExp;
}
std::shared_ptr<AstExpression> Parser::parseEqualityExp() {
    auto relationalExp = parseRelationalExp();

    while (matchAny({TokenType::EqualEqual, TokenType::NotEqual})) {
        Token operatorToken = consumeAnyOne({TokenType::EqualEqual, TokenType::NotEqual});
        auto next_relationalExp = parseRelationalExp();

        relationalExp = std::make_shared<AstBinary>(relationalExp, toBinaryOperator(operatorToken.type), next_relationalExp);
        relationalExp->token = operatorToken;
    }
    return relationalExp;
}
std::shared_ptr<AstExpression> Parser::parseRelationalExp() {
    auto additiveExp = parseAdditiveExp();

    while (matchAny({TokenType::GreaterThan, TokenType::GreaterThanEqual, TokenType::LessThan, TokenType::LessThanEqual})) {
        Token operatorToken = consumeAnyOne({TokenType::GreaterThan, TokenType::GreaterThanEqual, TokenType::LessThan, TokenType::LessThanEqual});
        auto next_additiveExp = parseAdditiveExp();

        additiveExp = std::make_shared<AstBinary>(additiveExp, toBinaryOperator(operatorToken.type), next_additiveExp);
        additiveExp->token = operatorToken;
    }
    return additiveExp;
}
std::shared_ptr<AstExpression> Parser::parseAdditiveExp() {
    auto term = parseTerm();

    while (matchAny({TokenType::Plus, TokenType::Minus})) {
        Token operatorToken = consumeAnyOne({TokenType::Plus, TokenType::Minus});
        auto next_term = parseTerm();

        term = std::make_shared<AstBinary>(term, toBinaryOperator(operatorToken.type), next_term);
        term->token = operatorToken;
    }
    return term;
}
std::shared_ptr<AstExpression> Parser::parseTerm() {
    auto factor = parseFactor();

    while (matchAny({TokenType::Star, TokenType::Slash, TokenType::Percent})) {
        Token operatorToken = consumeAnyOne({TokenType::Star, TokenType::Slash, TokenType::Percent});
        auto next_factor = parseFactor();

        factor = std::make_shared<AstBinary>(factor, toBinaryOperator(operatorToken.type), next_factor);
        factor->token = operatorToken;
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
        auto unary = std::make_shared<AstUnary>(toUnaryOperator(operatorToken.type), factor);
        unary->token = operatorToken;
        return unary;
    }
    else if (match(TokenType::Number))
        return parseNumber();
    else if (match({TokenType::ID, TokenType::LeftParen}))
        return parseFunctionCall();
    else if (matchAny({TokenType::ID, TokenType::Star}))
        return parseLValue();
    else if (match(TokenType::DollarSign)){
        return parseAstAddressOf();
    }
    else{
        parserError("Expected expression but got ", getCurrentToken().toString());
        return nullptr;
    }
}
std::shared_ptr<AstInteger> Parser::parseNumber() {
    std::shared_ptr<AstInteger> number = std::make_shared<AstInteger>(consume(TokenType::Number));
    try{
        number->value = std::stoull(number->token.value);
    }
    catch(std::out_of_range){
        parserError("Number doesn't fit into 64 bits: ", number->token.value);
    }
    number->semanticType = std::make_shared<AstPrimitiveType>(RSharpPrimitiveType::I64);
    return number;
}
std::shared_ptr<AstLValue> Parser::parseLValue(){
    if (match(TokenType::Identifier)){
        return parseVariableAccess();
    }
    else if (match(TokenType::Star)){
        return parseDereference();
    }
    else{
        parserError("Expected lvalue but got ", getCurrentToken().toString());
        return nullptr;
    }
}

std::shared_ptr<AstAssignment> Parser::parseAssignment() {
    std::shared_ptr<AstAssignment> assignment = std::make_shared<AstAssignment>();
    assignment->lvalue = parseLValue();
    assignment->token = consume(TokenType::Assign);
    assignment->rvalue = parseExpression();
    return assignment;
}
std::shared_ptr<AstFunctionCall> Parser::parseFunctionCall() {
    std::shared_ptr<AstFunctionCall> functionCall = std::make_shared<AstFunctionCall>(consume(TokenType::ID));
    functionCall->name = functionCall->token.value;
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
std::shared_ptr<AstAddressOf> Parser::parseAstAddressOf(){
    std::shared_ptr<AstAddressOf> address_of = std::make_shared<AstAddressOf>(consume(TokenType::DollarSign));
    address_of->operand = parseVariableAccess();
    return address_of;
}


std::shared_ptr<AstVariableAccess> Parser::parseVariableAccess() {
    std::shared_ptr<AstVariableAccess> variableAccess = std::make_shared<AstVariableAccess>(consume(TokenType::ID));
    variableAccess->name = variableAccess->token.value;
    return variableAccess;
}

std::shared_ptr<AstDereference> Parser::parseDereference(){
    auto tok = consume(TokenType::Star);
    auto lvalue = std::make_shared<AstDereference>(parseFactor());
    lvalue->token = tok;
    return lvalue;
}



std::shared_ptr<AstVariableDeclaration> Parser::parseVariableDeclaration() {
    std::shared_ptr<AstVariableDeclaration> variable = std::make_shared<AstVariableDeclaration>(consume(TokenType::Identifier));
    variable->name = variable->token.value;
    consume(TokenType::Colon);
    variable->semanticType = parseType();
    if (match(TokenType::Assign)) {
        consume(TokenType::Assign);
        variable->value = parseExpression();
    }
    variable->variable = std::make_shared<SemanticVariableData>();
    variable->variable->name = variable->name;
    variable->variable->type = variable->semanticType;
    return variable;
}


std::shared_ptr<AstType> Parser::parseType() {
    if (match(TokenType::Typename)) {
        auto typename_tok = consume(TokenType::Typename);
        auto type = stringToType(typename_tok.value);
        if (type == RSharpPrimitiveType::NONE) {
            parserError("Unknown type ", getCurrentToken().toString());
        }
        auto type_ast = std::make_shared<AstPrimitiveType>(type);
        type_ast->token = typename_tok;
        return type_ast;
    }
    else if (match(TokenType::Star)){
        auto star = consume(TokenType::Star);
        auto pointer = std::make_shared<AstPointerType>(parseType());
        pointer->token = star;
        return pointer;
    }
    else {
        parserError("Expected typename or '*' (pointer) but got ", getCurrentToken().toString());
        return nullptr;
    }
}

std::shared_ptr<AstTags> Parser::parseTags() {
    auto tags = std::make_shared<AstTags>();
    if(match(TokenType::LeftBracket)){
        consume(TokenType::LeftBracket);
        do{
            auto identifier = consume(TokenType::Identifier);
            if (identifier.value == "extern"){
                tags->tags.push_back(AstTags::Value::Extern);
            }
            else{
                parserError("Expected tag identifier but got \"", identifier.value, "\"");
            }
        } while(match(TokenType::Comma) && (consume(TokenType::Comma), true));
        consume(TokenType::RightBracket);
    }
    
    return tags;
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

// helpers
std::shared_ptr<AstExpression> Parser::parseOptionalExpression(){
    try{
        TokenRestorer _(*this);
        return parseExpression();
    }
    catch(ParsingError& e){
        auto exp = std::make_shared<AstEmptyExpression>();
        exp->semanticType = std::make_shared<AstPrimitiveType>(RSharpPrimitiveType::C_void);
        return exp;
    }
}