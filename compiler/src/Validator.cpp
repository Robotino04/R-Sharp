#include "R-Sharp/Validator.hpp"
#include "R-Sharp/AstNodes.hpp"

#include <sstream>


std::string to_string(std::shared_ptr<AstTypeModifier> node){
    return node->name;
}
std::string to_string(std::shared_ptr<AstType> node){
    std::string result;
    if (node->getType() == AstNodeType::AstArray){
        auto node2 = std::static_pointer_cast<AstArray>(node);
        result =  "[" + to_string(node2->type) + "]";
    }
    else if (node->getType() == AstNodeType::AstBuiltinType){
        auto node2 = std::static_pointer_cast<AstBuiltinType>(node);
        result = node2->name;
    }

    for (auto const& modifier : node->modifiers){
        result += " " + to_string(modifier);
    }
    return result;
}

Validator::Validator(std::shared_ptr<AstNode> root, std::string const& filename, std::string const& source){
    this->root = root;
    this->filename = filename;
    this->source = source;
}

void Validator::validate(){
    variableContexts.clear();
    variableContexts.emplace_back();
    functions.clear();
    collapseContexts = false;
    numLoops = 0;

    root->accept(this);
}
bool Validator::hasErrors(){
    return hasError;
}

void Validator::pushContext(){
    if (!collapseContexts){
        variableContexts.emplace_back();
    }
    collapseContexts = false;
}
void Validator::popContext(){
    if (variableContexts.empty()){
        Error("Invalid context pop (internal error)");
    }
    variableContexts.pop_back();
}
bool Validator::isVariableDeclared(Variable testVar){
    for (auto it = variableContexts.rbegin(); it != variableContexts.rend(); it++){
        if (it->end() != std::find(it->begin(), it->end(), testVar)){
            return true;
        }
    }
    return false;
}
bool Validator::isVariableDefinable(Variable testVar){
    if (isVariableDeclared(testVar)){
        auto it = std::find(variableContexts.back().begin(), variableContexts.back().end(), testVar);
        if (it == variableContexts.back().end()){
            return true;
        }
        if (it->defined){
            return false;
        }
    }
    return true;
}
void Validator::addVariable(Variable var){
    auto it = std::find(variableContexts.back().begin(), variableContexts.back().end(), var);
    if (it == variableContexts.back().end()){
    variableContexts.back().push_back(var);
    }
    else if (var.defined){
        it->defined = true;
    }
}


bool Validator::isFunctionDeclared(Function testFunc){
    return std::find(functions.begin(), functions.end(), testFunc) != functions.end();
}
bool Validator::isFunctionDeclarable(Function testFunc){
    for (auto func : functions){
        if (func.name == testFunc.name){
            if (testFunc.parameters.size() == func.parameters.size()){
                return true;
            }   
            else{
                return false;
            }
        }
    }
    return true;
}
bool Validator::isFunctionDefinable(Function testFunc){
    bool isDeclarable = isFunctionDeclarable(testFunc);
    if (isFunctionDeclared(testFunc)){
        auto it = std::find(functions.begin(), functions.end(), testFunc);
        if (it->defined){
            return false;
        }
    }
    return isDeclarable;
}
void Validator::addFunction(Function func){
    auto it = std::find(functions.begin(), functions.end(), func);
    if (it == functions.end()){
        functions.push_back(func);
    }
    else if (func.defined){
        it->defined = true;
    }
}


void Validator::visit(AstBlock* node){
    pushContext();
    for (auto const& child : node->getChildren()){
        child->accept(this);
    }
    popContext();
}
void Validator::visit(AstForLoopDeclaration* node){
    pushContext();
    numLoops++;
    for (auto const& child : node->getChildren()){
        child->accept(this);
    }
    numLoops--;
    popContext();
}
void Validator::visit(AstForLoopExpression* node){
    numLoops++;
    for (auto const& child : node->getChildren()){
        child->accept(this);
    }
    numLoops--;
}
void Validator::visit(AstWhileLoop* node){
    numLoops++;
    for (auto const& child : node->getChildren()){
        child->accept(this);
    }
    numLoops--;
}
void Validator::visit(AstDoWhileLoop* node){
    numLoops++;
    for (auto const& child : node->getChildren()){
        child->accept(this);
    }
    numLoops--;
}
void Validator::visit(AstBreak* node){
    if (numLoops == 0){
        hasError = true;
        Error("Break statement outside loop");
        printErrorToken(node->token);
    }
}
void Validator::visit(AstSkip* node){
    if (numLoops == 0){
        hasError = true;
        Error("Skip statement outside loop");
        printErrorToken(node->token);
    }
}

void Validator::visit(AstVariableAccess* node){
    if (!isVariableDeclared({node->name, ""})){
        hasError = true;
        Error("Error: variable \"", node->name, "\" is not declared");
        printErrorToken(node->token);
    }
}
void Validator::visit(AstVariableAssignment* node){
    if (!isVariableDeclared({node->name, ""})){
        hasError = true;
        Error("Error: variable \"", node->name, "\" is not declared");
        printErrorToken(node->token);
    }
}
void Validator::visit(AstVariableDeclaration* node){
    Variable var = {node->name, to_string(node->type)};
    if (node->isGlobal){
        if (isFunctionDeclared({var.name, ""})){
            hasError = true;
            Error("Error: global variable \"", var.name, "\" is already declared as a function");
            printErrorToken(node->token);
        }
        if (node->value && node->value->getType() != AstNodeType::AstInteger){
            hasError = true;
            Error("Error: global variable \"", var.name, "\" must be initialized to a constant value");
            printErrorToken(node->token);
        }
    }
    if (!isVariableDefinable(var)){
        hasError = true;
        Error("Error: variable \"", node->name, "\" is defined multiple times");
        printErrorToken(node->token);
    }else{
        addVariable(var);
    }
}
void Validator::visit(AstFunctionCall* node){
    Function thisFunc = {node->name, "", {}, false};
    for (auto arg : node->arguments){
        thisFunc.parameters.push_back({"", ""});
    }
    if (!isFunctionDeclared(thisFunc)){
        hasError = true;
        Error("Error: function \"", node->name, "\" is not declared (wrong number of arguments?)");
        printErrorToken(node->token);
    }
}
void Validator::visit(AstFunctionDeclaration* node){
    Function thisFunc = {node->name, to_string(node->returnType), {}, false};
    for (auto param : node->parameters->parameters){
        thisFunc.parameters.push_back({param->name, to_string(param->type)});
    }

    if (isVariableDeclared({node->name, ""})){
        hasError = true;
        Error("Error: function \"", node->name, "\" is already declared as a variable");
        printErrorToken(node->token);
    }

    if (isFunctionDeclarable(thisFunc)){
        addFunction(thisFunc);
    }
    else{
        hasError = true;
        Error("Error: function \"", node->name, "\" is already declared (possibly with different parameters)");
        printErrorToken(node->token);
    }
    addFunction(thisFunc);
}
void Validator::visit(AstFunction* node){
    Function thisFunc = {node->name, to_string(node->returnType), {}, true};
    for (auto param : node->parameters->parameters){
        thisFunc.parameters.push_back({param->name, to_string(param->type)});
    }

    if (isVariableDeclared({node->name, ""})){
        hasError = true;
        Error("Error: function \"", node->name, "\" is already declared as a variable");
        printErrorToken(node->token);
    }

    if (isFunctionDefinable(thisFunc)){
        addFunction(thisFunc);
    }
    else{
        hasError = true;
        Error("Error: function \"", node->name, "\" is already declared with different parameters");
        printErrorToken(node->token);
    }
    // push the function context to include parameters
    pushContext();
    for (auto const& child : node->getChildren()){
        child->accept(this);
    }
    // force the function body to use the same context as the parameters
    forceContextCollapse();
    node->body->accept(this);
    // since the context is collapsed, the function body has alredy popped the context
}

void Validator::printErrorToken(Token token){
    int start = token.position.startPos;
    int end = token.position.endPos;

    std::string src = source;
    src.replace(start, end - start, "\033[31m" + src.substr(start, end - start) + "\033[0m");

    // print the error and 3 lines above it
    std::stringstream ss;
    int line = 1;
    int column = 1;
    int pos = 0;

    for (char c : src) {
        if (line >= token.position.line - 3 && line <= token.position.line) {
            if (column == 1){
                ss << line << "| ";
            }
            if (line == token.position.line && c == '\n') break;
            ss << c;
        }
        pos++;
        if (c == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
    }

    int prefixLen = (std::to_string(token.position.line) + "| ").length();

    ss << "\n\033[31m" // enable red text
        << std::string(prefixLen + token.position.column - 1, ' ') // print spaces before the error
        << "^";
    try {
        ss << std::string(end - start - 1, '~'); // underline the error
    }
    catch(std::length_error){}

    ss << "\033[0m"; // disable red text
    Print(ss.str());
}