#include "R-Sharp/Validator.hpp"
#include "R-Sharp/AstNodes.hpp"
#include "R-Sharp/Utils.hpp"

#include <sstream>

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
bool Validator::isVariableDeclared(AstVariableDeclaration const& testVar){
    for (auto it = variableContexts.rbegin(); it != variableContexts.rend(); it++){
        if (it->end() != std::find(it->begin(), it->end(), testVar)){
            return true;
        }
    }
    return false;
}
bool Validator::isVariableDefinable(AstVariableDeclaration const& testVar){
    auto it = std::find(variableContexts.back().begin(), variableContexts.back().end(), testVar);

    if (it == variableContexts.back().end()){
        return true;
    }
    else{
        if (it->isGlobal && testVar.isGlobal){
            return !it->value;
        }
        else{
            return false;
        }
    }
}
void Validator::addVariable(AstVariableDeclaration const& var){
    auto it = std::find(variableContexts.back().begin(), variableContexts.back().end(), var);
    if (it == variableContexts.back().end()){
    variableContexts.back().push_back(var);
    }
    else if (var.value){
        it->value = var.value;
    }
}
AstVariableDeclaration* Validator::getVariable(AstVariableDeclaration const& var){
    for (auto it = variableContexts.rbegin(); it != variableContexts.rend(); it++){
        auto it2 = std::find(it->begin(), it->end(), var);
        if (it2 != it->end()){
            return &(*it2);
        }
    }
    return nullptr;
}


bool Validator::isFunctionDeclared(AstFunctionDeclaration const& testFunc){
    return std::find(functions.begin(), functions.end(), testFunc) != functions.end();
}
bool Validator::isFunctionDeclarable(AstFunctionDeclaration const& testFunc){
    for (auto const& func : functions){
        if (func.name == testFunc.name){
            if (testFunc.parameters->parameters.size() == func.parameters->parameters.size()){
                return true;
            }   
            else{
                return false;
            }
        }
    }
    return true;
}
bool Validator::isFunctionDefinable(AstFunctionDeclaration const& testFunc){
    bool isDeclarable = isFunctionDeclarable(testFunc);
    if (isFunctionDeclared(testFunc)){
        auto it = std::find(functions.begin(), functions.end(), testFunc);
        if (it->body){
            return false;
        }
    }
    return isDeclarable;
}
void Validator::addFunction(AstFunctionDeclaration const& func){
    auto it = std::find(functions.begin(), functions.end(), func);

    if (it == functions.end()){
        functions.push_back(func);
    }
    else if (func.body){
        it->body = func.body;
    }
}
AstFunctionDeclaration* Validator::getFunction(AstFunctionDeclaration const& func){
    auto it = std::find(functions.begin(), functions.end(), func);
    if (it == functions.end()){
        return nullptr;
    }
    else{
        return &(*it);
    }
}

void Validator::requireIdenticalTypes(AstNode* a, AstNode* b, std::string msg){
    requireType(a);
    requireType(b);
    if (*a->semanticType != *b->semanticType){
        if ((a->semanticType->type == RSharpType::I32 || a->semanticType->type == RSharpType::I64)
         && (b->semanticType->type == RSharpType::I32 || b->semanticType->type == RSharpType::I64)
        ){
            Warning("using conversion between ", std::to_string(a->semanticType.get()), " and ", std::to_string(b->semanticType.get()));
            return;
        }

        hasError = true;
        Error("Error: ", msg);
        printErrorToken(a->token, source);
        printErrorToken(b->token, source);
    }
}
void Validator::requireType(AstNode* node){
    if (!node->semanticType){
        hasError = true;
        Error("INTERNAL ERROR: operand doesn't have a semanticType");
        printErrorToken(node->token, source);
        exit(1);
    }
}

void Validator::visit(AstBlock* node){
    pushContext();
    AstVisitor::visit((AstNode*)node);
    popContext();
}
void Validator::visit(AstReturn* node){
    AstVisitor::visit((AstNode*)node);
    node->semanticType = node->value->semanticType;
}
void Validator::visit(AstExpressionStatement* node){
    AstVisitor::visit((AstNode*)node);
    node->semanticType = node->expression->semanticType;
}
void Validator::visit(AstForLoopDeclaration* node){
    pushContext();
    numLoops++;
    AstVisitor::visit((AstNode*)node);
    numLoops--;
    popContext();
}
void Validator::visit(AstForLoopExpression* node){
    numLoops++;
    AstVisitor::visit((AstNode*)node);
    numLoops--;
}
void Validator::visit(AstWhileLoop* node){
    numLoops++;
    AstVisitor::visit((AstNode*)node);
    numLoops--;
}
void Validator::visit(AstDoWhileLoop* node){
    numLoops++;
    AstVisitor::visit((AstNode*)node);
    numLoops--;
}
void Validator::visit(AstBreak* node){
    if (numLoops == 0){
        hasError = true;
        Error("Break statement outside loop");
        printErrorToken(node->token, source);
    }
}
void Validator::visit(AstSkip* node){
    if (numLoops == 0){
        hasError = true;
        Error("Skip statement outside loop");
        printErrorToken(node->token, source);
    }
}

void Validator::visit(AstUnary* node){
    AstVisitor::visit((AstNode*)node);
    requireType(node->value.get());
    node->semanticType = node->value->semanticType;
}
void Validator::visit(AstBinary* node){
    AstVisitor::visit((AstNode*)node);
    requireIdenticalTypes(node->left.get(), node->right.get(), "Binary operands don't match in semantical type");
    node->semanticType = node->left->semanticType;
}
void Validator::visit(AstVariableAccess* node){
    AstVariableDeclaration testVar;
    testVar.name = node->name;
    if (isVariableDeclared(testVar)){
        auto var = getVariable(testVar);
        if (var){
            node->semanticType = var->semanticType;
        }
        else{
            Fatal("INTERNAL ERROR: variable declared but not found in variable stack");
        }
    }
    else{
        hasError = true;
        Error("Error: variable \"", node->name, "\" is not declared");
        printErrorToken(node->token, source);
    }
}
void Validator::visit(AstVariableAssignment* node){
    AstVisitor::visit((AstNode*)node);
    AstVariableDeclaration testVar;
    testVar.name = node->name;
    requireType(node->value.get());
    testVar.semanticType = node->value->semanticType;
    node->semanticType = node->value->semanticType;
    if (!isVariableDeclared(testVar)){
        hasError = true;
        Error("Error: variable \"", node->name, "\" is not declared");
        printErrorToken(node->token, source);
    }
}
void Validator::visit(AstVariableDeclaration* node){
    AstVisitor::visit((AstNode*)node);
    if (node->value){
        requireIdenticalTypes(node, node->value.get(),
            "value assigned to variable of different semantical type");
    }
    if (node->isGlobal){
        AstFunctionDeclaration testFunc;
        testFunc.name = node->name;
        if (isFunctionDeclared(testFunc)){
            hasError = true;
            Error("Error: global variable \"", node->name, "\" is already declared as a function");
            printErrorToken(node->token, source);
        }
        if (node->value && node->value->getType() != AstNodeType::AstInteger){
            hasError = true;
            Error("Error: global variable \"", node->name, "\" must be initialized to a constant value");
            printErrorToken(node->token, source);
        }
    }
    if (!isVariableDefinable(*node)){
        hasError = true;
        Error("Error: variable \"", node->name, "\" is defined multiple times");
        printErrorToken(node->token, source);
    }else{
        addVariable(*node);
    }
}
void Validator::visit(AstConditionalExpression* node){
    AstVisitor::visit((AstNode*)node);
    requireType(node->condition.get());
    requireType(node->trueExpression.get());
    requireType(node->falseExpression.get());

    requireIdenticalTypes(node->falseExpression.get(), node->trueExpression.get(),
        "true and false ternary expressions don't match in semantic type");

    node->semanticType = node->trueExpression->semanticType;
}
void Validator::visit(AstFunctionCall* node){
    AstFunctionDeclaration testFunc;
    testFunc.name = node->name;
    testFunc.parameters = std::make_shared<AstParameterList>();
    for (auto arg : node->arguments){
        testFunc.parameters->parameters.push_back(std::make_shared<AstVariableDeclaration>());
        testFunc.parameters->parameters.back()->semanticType = arg->semanticType;
    }

    if (isFunctionDeclared(testFunc)){
        node->semanticType = getFunction(testFunc)->semanticType;
    }
    else{
        hasError = true;
        Error("Error: function \"", node->name, "\" is not declared (wrong number of arguments?)");
        printErrorToken(node->token, source);
    }
}
void Validator::visit(AstFunctionDeclaration* node){
    AstVariableDeclaration testVar;
    testVar.name = node->name;
    if (isVariableDeclared(testVar)){
        hasError = true;
        Error("Error: function \"", node->name, "\" is already declared as a variable");
        printErrorToken(node->token, source);
    }

    if (node->body){
        if (isFunctionDefinable(*node)){
            addFunction(*node);
        }
        else{
            hasError = true;
            Error("Error: function \"", node->name, "\" is already defined");
            printErrorToken(node->token, source);
        }
    }
    else{
        if (isFunctionDeclarable(*node)){
            addFunction(*node);
        }
        else{
            hasError = true;
            Error("Error: function \"", node->name, "\" is already declared (possibly with different parameters)");
            printErrorToken(node->token, source);
        }
    }
    
    // push the function context to include parameters
    if (node->body){
        pushContext();
        AstVisitor::visit((AstNode*)node);
        // force the function body to use the same context as the parameters
        forceContextCollapse();
        node->body->accept(this);
        // since the context is collapsed, the function body has alredy popped the context
    }
}