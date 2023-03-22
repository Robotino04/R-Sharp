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
    variableContexts = {};
    functions = {};
    collapseContexts = false;
    loops = {};

    root->accept(this);
}
bool SemanticValidator::hasErrors(){
    return hasError;
}

void SemanticValidator::pushContext(std::shared_ptr<AstBlock> block){
    if (collapseContexts && !variableContexts.empty()){
        variableContexts.back()->isMerged = true;

        block->variables.insert(block->variables.begin(), variableContexts.back()->variables.begin(), variableContexts.back()->variables.end());
        block->sizeOfLocalVariables += variableContexts.back()->sizeOfLocalVariables;
        variableContexts.back() = block;
        collapseContexts = false;
    }
    else{
        variableContexts.push_back(block);
    }
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
void SemanticValidator::addVariable(std::shared_ptr<AstVariableDeclaration> var){
    auto it = std::find_if(variableContexts.back()->variables.begin(), variableContexts.back()->variables.end(), [&](auto other){
        return other->name == var->name;
    });
    if (it == variableContexts.back()->variables.end()){
        var->variable->isDefined = (bool)var->value;
        var->variable->name = var->name;
        var->variable->type = var->semanticType->type;

        switch(var->variable->type){
            case RSharpType::I32: var->variable->sizeInBytes = 4; break;
            case RSharpType::I64: var->variable->sizeInBytes = 8; break;

            default:
                Error("INTERNAL ERROR: type nr. " + std::to_string(static_cast<int>(var->variable->type)) + " isn't implemented.");
        }
        variableContexts.back()->variables.push_back(var->variable);
        variableContexts.back()->sizeOfLocalVariables += var->variable->sizeInBytes;
    }
    else if (var->value){
        (*it)->isDefined = true;
        var->variable = *it;
    }
}
std::shared_ptr<SemanticVariableData> SemanticValidator::getVariable(std::string const& name){
    for (auto it = variableContexts.rbegin(); it != variableContexts.rend(); it++){
        auto it2 = std::find_if((*it)->variables.begin(), (*it)->variables.end(), [&](auto other){
            return other->name == name;
        });
        if (it2 != (*it)->variables.end()){
            return *it2;
        }
    }
    return nullptr;
}


bool SemanticValidator::isFunctionDeclared(std::string name) const{
    return std::find_if(functions.begin(), functions.end(), [&](auto other){
        return other->name == name;
    }) != functions.end();
}
bool SemanticValidator::isFunctionDeclarable(std::shared_ptr<AstFunctionDeclaration> testFunc) const{
    for (auto func : functions){
        if (func->name == testFunc->name){
            if (testFunc->parameters->parameters.size() == func->parameters->parameters.size()){
                return true;
            }
            else{
                return false;
            }
        }
    }
    return true;
}
bool SemanticValidator::isFunctionDefinable(std::shared_ptr<AstFunctionDeclaration> testFunc) const{
    auto it = std::find_if(functions.begin(), functions.end(), [&](auto other){
        return other->name == testFunc->name;
    });
    if (it == functions.end())
        return true;
    if ((*it)->isDefined)
        return false;
    
    return isFunctionDeclarable(testFunc);
}
void SemanticValidator::addFunction(std::shared_ptr<AstFunctionDeclaration> func){
    auto it = std::find_if(functions.begin(), functions.end(), [&](auto other){
        return other->name == func->name;
    });
    if (it == functions.end()){
        functions.push_back(func->function);
    }
    else {
        func->function = *it;
        func->function->isDefined = (bool)func->body;
    }
}
std::shared_ptr<SemanticFunctionData> SemanticValidator::getFunction(std::string name, std::shared_ptr<AstParameterList> params){
    auto it = std::find_if(functions.begin(), functions.end(), [&](auto other){
        return other->name == name && *other->parameters == *params;
    });
    if (it == functions.end()){
        return nullptr;
    }
    else{
        return *it;
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
    node->globalScope = std::make_shared<AstBlock>();
    node->globalScope->name = "Global Scope";
    // it isn't actually merged, but just marked so to avoid assembly outside of a function
    node->globalScope->isMerged = true;
    pushContext(node->globalScope);
    // visit children
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    popContext();

    for (auto var : node->globalScope->variables){
        if (!var->isDefined)
            node->uninitializedGlobalVariables.push_back(var);
    }
}

void SemanticValidator::visit(std::shared_ptr<AstBlock> node){
    pushContext(node);
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    popContext();
}
void SemanticValidator::visit(std::shared_ptr<AstReturn> node){
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    node->semanticType = node->value->semanticType;
    // don't copy the global scope
    node->containedScopes.insert(node->containedScopes.begin(), std::next(variableContexts.begin()), variableContexts.end());
}
void SemanticValidator::visit(std::shared_ptr<AstExpressionStatement> node){
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    node->semanticType = node->expression->semanticType;
}

void SemanticValidator::visit(std::shared_ptr<AstForLoopDeclaration> node){
    loops.push(node->loop = std::make_shared<SemanticLoopData>());
    node->initializationContext->name = "for loop counter";
    variableContexts.back()->hasLoopCurrently = true;
    node->initializationContext->accept(this);
    variableContexts.back()->hasLoopCurrently = false;
    loops.pop();
}
void SemanticValidator::visit(std::shared_ptr<AstForLoopExpression> node){
    loops.push(node->loop = std::make_shared<SemanticLoopData>());
    variableContexts.back()->hasLoopCurrently = true;
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    variableContexts.back()->hasLoopCurrently = false;
    loops.pop();
}
void SemanticValidator::visit(std::shared_ptr<AstWhileLoop> node){
    loops.push(node->loop = std::make_shared<SemanticLoopData>());
    variableContexts.back()->hasLoopCurrently = true;
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    variableContexts.back()->hasLoopCurrently = false;
    loops.pop();
}
void SemanticValidator::visit(std::shared_ptr<AstDoWhileLoop> node){
    loops.push(node->loop = std::make_shared<SemanticLoopData>());
    variableContexts.back()->hasLoopCurrently = true;
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    variableContexts.back()->hasLoopCurrently = false;
    loops.pop();
}
void SemanticValidator::visit(std::shared_ptr<AstBreak> node){
    auto it = std::find_if(variableContexts.rbegin(), variableContexts.rend(), [&](auto varContext){
        return varContext->hasLoopCurrently;
    });
    if (loops.empty() || it == variableContexts.rend()){
        hasError = true;
        Error("Break statement outside of loop");
        printErrorToken(node->token, source);
    }
    else{
        node->loop = loops.top();
        node->containedScopes = {std::next(it.base(), 1), variableContexts.end()};
    }
}
void SemanticValidator::visit(std::shared_ptr<AstSkip> node){
    auto it = std::find_if(variableContexts.rbegin(), variableContexts.rend(), [&](auto varContext){
        return varContext->hasLoopCurrently;
    });
    if (loops.empty() || it == variableContexts.rend()){
        hasError = true;
        Error("Skip statement outside of loop");
        printErrorToken(node->token, source);
    }
    else{
        node->loop = loops.top();
        node->containedScopes = {std::next(it.base(), 1), variableContexts.end()};
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
    if (isVariableDeclared(node->name)){
        requireType(node->value);
        node->variable = getVariable(node->name);
        node->semanticType = std::make_shared<AstType>();
        node->semanticType->type = node->variable->type;
    }
    else{
        hasError = true;
        Error("Error: variable \"", node->name, "\" is not declared");
        printErrorToken(node->token, source);
        node->semanticType = std::make_shared<AstType>();
        node->semanticType->type = RSharpType::ErrorType;
    }
}
void SemanticValidator::visit(std::shared_ptr<AstVariableDeclaration> node){
    // visit the children
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    
    if (node->value){
        requireIdenticalTypes(node, node->value, "value assigned to variable of different semantical type");
    }
    if (node->variable->isGlobal){
        if (isFunctionDeclared(node->name)){
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
        addVariable(node);
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
    auto parameters = std::make_shared<AstParameterList>();
    for (auto arg : node->arguments){
        parameters->parameters.push_back(std::make_shared<AstVariableDeclaration>());
        parameters->parameters.back()->semanticType = arg->semanticType;
    }

    node->function = getFunction(node->name, parameters);
    if (node->function){
        node->semanticType = std::make_shared<AstType>();
        node->semanticType->type = node->function->returnType;

        AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
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
        if (isFunctionDefinable(node)){
            addFunction(node);
        }
        else{
            hasError = true;
            Error("Error: function \"", node->name, "\" is already defined (possibly with different parameters)");
            printErrorToken(node->token, source);
        }
    }
    else{
        if (isFunctionDeclarable(node)){
            addFunction(node);
        }
        else{
            hasError = true;
            Error("Error: function \"", node->name, "\" is already declared (possibly with different parameters)");
            printErrorToken(node->token, source);
        }
    }
    
    // push the function context to include parameters
    if (node->body){
        node->parameters->parameterBlock = std::make_shared<AstBlock>();
        node->parameters->parameterBlock->name = "parameters " + node->name;
        node->body->name = node->name;
        pushContext(node->parameters->parameterBlock);
        node->parameters->accept(this);

        forceContextCollapse();
        node->body->accept(this);

        // the parameterContext will have been popped because it's collapsed
    }
}