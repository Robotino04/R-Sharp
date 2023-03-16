#include "R-Sharp/SemanticValidator.hpp"
#include "R-Sharp/AstNodes.hpp"
#include "R-Sharp/Utils.hpp"

#include <sstream>

SemanticValidator::SemanticValidator(std::shared_ptr<AstNode> root, std::string const& filename, std::string const& source){
    this->root = root;
    this->filename = filename;
    this->source = source;
}

void SemanticValidator::validate(){
    variableContexts.clear();
    functions.clear();
    collapseContexts = false;
    numLoops = 0;

    root->accept(this);
}
bool SemanticValidator::hasErrors(){
    return hasError;
}

void SemanticValidator::pushContext(std::shared_ptr<AstBlock> block){
    if (!collapseContexts) variableContexts.push_back(block);
    collapseContexts = false;
}
void SemanticValidator::popContext(){
    if (variableContexts.empty()){
        Error("INTERNAL ERROR: Invalid context pop");
    }
    variableContexts.pop_back();
}
bool SemanticValidator::isVariableDeclared(std::string const& name) const{
    for (auto it = variableContexts.rbegin(); it != variableContexts.rend(); it++){
        auto found = std::find_if((*it)->variables.begin(), (*it)->variables.end(), [&](auto other){
            if (other->name.size() == 0) Error("INTERNAL ERROR: Variable without name detected");
            return other->name == name;
        });
        if (found != (*it)->variables.end()){
            return true;
        }
    }
    return false;
}
bool SemanticValidator::isVariableDefinable(AstVariableDeclaration const& testVar) const{
    // :
    auto it = std::find_if(variableContexts.back()->variables.begin(), variableContexts.back()->variables.end(), [&](auto other){
        return other->name == testVar.name;
    });
    if (it == variableContexts.back()->variables.end()){
        return true;
    }
    else{
        if ((*it)->isGlobal && testVar.variable->isGlobal){
            return !(*it)->isDefined;
        }
        else{
            return false;
        }
    }
}
void SemanticValidator::addVariable(AstVariableDeclaration const& var){
    // :
    auto it = std::find_if(variableContexts.back()->variables.begin(), variableContexts.back()->variables.end(), [&](auto other){
        return other->name == var.name;
    });
    if (it == variableContexts.back()->variables.end()){
        auto varPtr = std::make_shared<SemanticVariableData>();
        varPtr->isDefined = (bool)var.value;
        varPtr->name = var.name;
        varPtr->type = var.semanticType->type;
        varPtr->isGlobal = var.variable->isGlobal;
        variableContexts.back()->variables.push_back(varPtr);
    }
    else if (var.value){
        (*it)->isDefined = (bool)var.value;
    }
}
std::shared_ptr<SemanticVariableData> SemanticValidator::getVariable(std::string const& name){
    for (auto it = variableContexts.rbegin(); it != variableContexts.rend(); it++){
    // :
        auto it2 = std::find_if((*it)->variables.begin(), (*it)->variables.end(), [&](auto other){
            return other->name == name;
        });
        if (it2 != (*it)->variables.end()){
            return *it2;
        }
    }
    return nullptr;
}


bool SemanticValidator::isFunctionDeclared(AstFunctionDeclaration const& testFunc) const{
    return std::find(functions.begin(), functions.end(), testFunc) != functions.end();
}
bool SemanticValidator::isFunctionDeclarable(AstFunctionDeclaration const& testFunc) const{
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
bool SemanticValidator::isFunctionDefinable(AstFunctionDeclaration const& testFunc) const{
    bool isDeclarable = isFunctionDeclarable(testFunc);
    if (isFunctionDeclared(testFunc)){
        auto it = std::find(functions.begin(), functions.end(), testFunc);
        if (it->body){
            return false;
        }
    }
    return isDeclarable;
}
void SemanticValidator::addFunction(AstFunctionDeclaration const& func){
    auto it = std::find(functions.begin(), functions.end(), func);

    if (it == functions.end()){
        functions.push_back(func);
    }
    else if (func.body){
        it->body = func.body;
    }
}
AstFunctionDeclaration* SemanticValidator::getFunction(AstFunctionDeclaration const& func){
    auto it = std::find(functions.begin(), functions.end(), func);
    if (it == functions.end()){
        return nullptr;
    }
    else{
        return &(*it);
    }
}

void SemanticValidator::requireIdenticalTypes(std::shared_ptr<AstNode> a, std::shared_ptr<AstNode> b, std::string msg){
    requireType(a);
    requireType(b); 
    if (*a->semanticType == RSharpType::ErrorType || *b->semanticType == RSharpType::ErrorType) return;
    if (*a->semanticType != *b->semanticType){
        if ((a->semanticType->type == RSharpType::I32 && a->semanticType->type == RSharpType::I64)
         || (b->semanticType->type == RSharpType::I32 && b->semanticType->type == RSharpType::I64)
        ){
            Warning("using conversion between i32 and i64");
            return;
        }

        hasError = true;
        Error("Error: ", msg);
        printErrorToken(a->token, source);
        printErrorToken(b->token, source);
    }
}
void SemanticValidator::requireType(std::shared_ptr<AstNode> node){
    if (!node->semanticType){
        hasError = true;
        Error("INTERNAL ERROR: operand doesn't have a semanticType");
        printErrorToken(node->token, source);
        exit(1);
    }
}

void SemanticValidator::visit(std::shared_ptr<AstProgram> node){
    auto globalScope = std::make_shared<AstBlock>();
    pushContext(globalScope);
    // visit children
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    popContext();
}

void SemanticValidator::visit(std::shared_ptr<AstBlock> node){
    pushContext(node);
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    popContext();
}
void SemanticValidator::visit(std::shared_ptr<AstReturn> node){
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    node->semanticType = node->value->semanticType;
}
void SemanticValidator::visit(std::shared_ptr<AstExpressionStatement> node){
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    node->semanticType = node->expression->semanticType;
}
void SemanticValidator::visit(std::shared_ptr<AstForLoopDeclaration> node){
    pushContext(node->body);
    numLoops++;
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    numLoops--;
    popContext();
}
void SemanticValidator::visit(std::shared_ptr<AstForLoopExpression> node){
    numLoops++;
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    numLoops--;
}
void SemanticValidator::visit(std::shared_ptr<AstWhileLoop> node){
    numLoops++;
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    numLoops--;
}
void SemanticValidator::visit(std::shared_ptr<AstDoWhileLoop> node){
    numLoops++;
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    numLoops--;
}
void SemanticValidator::visit(std::shared_ptr<AstBreak> node){
    if (numLoops == 0){
        hasError = true;
        Error("Break statement outside loop");
        printErrorToken(node->token, source);
    }
}
void SemanticValidator::visit(std::shared_ptr<AstSkip> node){
    if (numLoops == 0){
        hasError = true;
        Error("Skip statement outside loop");
        printErrorToken(node->token, source);
    }
}

void SemanticValidator::visit(std::shared_ptr<AstUnary> node){
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    requireType(node->value);
    node->semanticType = node->value->semanticType;
}
void SemanticValidator::visit(std::shared_ptr<AstBinary> node){
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    requireIdenticalTypes(node->left, node->right, "Binary operands don't match in semantical type");
    
    if (node->left->semanticType->type == RSharpType::ErrorType || node->right->semanticType->type == RSharpType::ErrorType){
        node->semanticType = std::make_shared<AstType>();
        node->semanticType->type = RSharpType::ErrorType;
    }
    else{
        node->semanticType = std::make_shared<AstType>();
        node->semanticType = node->left->semanticType;
    }
}
void SemanticValidator::visit(std::shared_ptr<AstVariableAccess> node){
    if (isVariableDeclared(node->name)){
        auto var = getVariable(node->name);
        if (var){
            node->variable = var;
            node->semanticType = std::make_shared<AstType>();
            node->semanticType->type = var->type;
        }
        else{
            Fatal("INTERNAL ERROR: variable declared but not found in variable stack");
        }
    }
    else{
        hasError = true;
        Error("Error: variable \"", node->name, "\" is not declared");
        printErrorToken(node->token, source);

        node->semanticType = std::make_shared<AstType>();
        node->semanticType->type = RSharpType::ErrorType;
    }
}
void SemanticValidator::visit(std::shared_ptr<AstVariableAssignment> node){
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    if (!isVariableDeclared(node->name)){
        hasError = true;
        Error("Error: variable \"", node->name, "\" is not declared");
        printErrorToken(node->token, source);
    }
    requireType(node->value);
    node->variable = getVariable(node->name);
    node->semanticType = std::make_shared<AstType>();
    node->semanticType->type = node->variable->type;
}
void SemanticValidator::visit(std::shared_ptr<AstVariableDeclaration> node){
    // visit the children
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    
    if (node->value){
        requireIdenticalTypes(node, node->value, "value assigned to variable of different semantical type");
    }
    if (node->variable->isGlobal){
        AstFunctionDeclaration testFunc;
        testFunc.name = node->name;
        if (isFunctionDeclared(testFunc)){
            hasError = true;
            Error("Error: global variable \"", node->name, "\" is already declared as a function");
            printErrorToken(node->token, source);
        }
        if (node->value && node->value->getType() != AstNodeType::AstInteger){
            hasError = true;
            Error("Error: global variable \"", node->name, "\" must be initialized to a constant value (no expression)");
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
void SemanticValidator::visit(std::shared_ptr<AstConditionalExpression> node){
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    requireType(node->condition);
    requireType(node->trueExpression);
    requireType(node->falseExpression);

    requireIdenticalTypes(node->falseExpression, node->trueExpression,
        "true and false ternary expressions don't match in semantic type");

    node->semanticType = node->trueExpression->semanticType;
}
void SemanticValidator::visit(std::shared_ptr<AstFunctionCall> node){
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
void SemanticValidator::visit(std::shared_ptr<AstFunctionDeclaration> node){
    if (isVariableDeclared(node->name)){
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
        auto parameterBlock = std::make_shared<AstBlock>();
        parameterBlock->items.insert(parameterBlock->items.begin(), node->parameters->parameters.begin(), node->parameters->parameters.begin());
        
        pushContext(parameterBlock);
        node->parameters->accept(this);
        // force the function body to use the same context as the parameters
        forceContextCollapse();
        node->body->accept(this);
        // since the context is collapsed, the function body has alredy popped the context
    }
}