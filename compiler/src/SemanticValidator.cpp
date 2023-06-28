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
        var->variable->type = var->semanticType;

        switch(var->variable->type.lock()->getType()){
            case AstNodeType::AstPrimitiveType:{
                auto primitive_type = std::static_pointer_cast<AstPrimitiveType>(var->variable->type.lock())->type;
                switch(primitive_type){
                    case RSharpPrimitiveType::I8:  var->variable->sizeInBytes = 1; break;
                    case RSharpPrimitiveType::I16: var->variable->sizeInBytes = 2; break;
                    case RSharpPrimitiveType::I32: var->variable->sizeInBytes = 4; break;
                    case RSharpPrimitiveType::I64: var->variable->sizeInBytes = 8; break;

                    default:
                        hasError = true;
                        Error("INTERNAL ERROR: type nr. " + std::to_string(static_cast<int>(primitive_type)) + " (" + typeToString(primitive_type) + ") isn't implemented.");
                        break;
                }
                break;
            }
            default: throw std::runtime_error("Unimplemented type used");
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


bool SemanticValidator::isFunctionDefined(std::string name) const{
    const auto it = std::find_if(functions.begin(), functions.end(), [&](auto other){
        return other->name == name;
    });
    return it != functions.end();
}
std::shared_ptr<SemanticFunctionData> SemanticValidator::getFunction(std::string name, std::shared_ptr<AstParameterList> params){
    auto it = std::find_if(functions.begin(), functions.end(), [&](auto other){
        if (other->name != name)
            return false;
        if (*other->parameters == *params)
            return true;
        
        // allow function if the parameters can be casted
        if (other->parameters->parameters.size() != params->parameters.size()){
            return false;
        }

        for (int i=0; i<other->parameters->parameters.size(); i++){
            if (!areEquivalentTypes(other->parameters->parameters.at(i), params->parameters.at(i))){
                return false;
            }
        }
        return true;
    });
    if (it == functions.end()){
        return nullptr;
    }
    else{
        return *it;
    }
}

bool isEqualTypeInSharedPointer (std::shared_ptr<AstType> a, std::shared_ptr<AstType> b){
    if (!a.get() || !b.get()) return false;
    if (a->getType() != b->getType()) return false;
    switch(a->getType()){
        case AstNodeType::AstPrimitiveType: return std::static_pointer_cast<AstPrimitiveType>(a)->type == std::static_pointer_cast<AstPrimitiveType>(b)->type;
        case AstNodeType::AstPointerType: return isEqualTypeInSharedPointer(std::static_pointer_cast<AstPointerType>(a)->subtype, std::static_pointer_cast<AstPointerType>(b));
        default: throw std::runtime_error("Unknown type to test equality");
    }
}

bool SemanticValidator::areEquivalentTypes(std::shared_ptr<AstNode> expected, std::shared_ptr<AstNode> found){
    static const std::vector<RSharpPrimitiveType> integerTypes = {RSharpPrimitiveType::I8, RSharpPrimitiveType::I16, RSharpPrimitiveType::I32, RSharpPrimitiveType::I64};
    requireType(expected);
    requireType(found);

    // don't issue further errors if the type is unknown already
    if (expected->semanticType->isErrorType() || found->semanticType->isErrorType()) return true;

    switch(expected->semanticType->getType()){
        default: throw std::runtime_error("Unimplemented type used"); break;
        case AstNodeType::AstPrimitiveType:{
            auto expected_type = std::static_pointer_cast<AstPrimitiveType>(expected->semanticType)->type;
            auto found_type = std::static_pointer_cast<AstPrimitiveType>(found->semanticType)->type;

            if (expected_type == found_type) return true;

            if (std::find(integerTypes.begin(), integerTypes.end(), expected_type) != integerTypes.end()
             && std::find(integerTypes.begin(), integerTypes.end(), found_type) != integerTypes.end()){
                Warning("Converting between integer types. (", typeToString(found_type), " --> ", typeToString(expected_type), ")");
                return true;
            }

            break;
        }

        case AstNodeType::AstPointerType:{
            auto expected_type = std::static_pointer_cast<AstPointerType>(expected->semanticType);
            auto found_type = std::static_pointer_cast<AstPointerType>(found->semanticType);

            return isEqualTypeInSharedPointer(expected_type->subtype, found_type->subtype);
        }
    }

    return false;
}

void SemanticValidator::requireIdenticalTypes(std::shared_ptr<AstNode> expected, std::shared_ptr<AstNode> found, std::string msg){
    if (!areEquivalentTypes(expected, found)){
        hasError = true;
        Error(msg, " (expected: ", expected->semanticType->toString(), "   found: ", found->semanticType->toString(), ")");
        printErrorToken(expected->token, source);
        printErrorToken(found->token, source);
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
    // collect all functions
    for (auto item : node->items){
        if (item->getType() != AstNodeType::AstFunctionDefinition) continue;
        auto function = std::dynamic_pointer_cast<AstFunctionDefinition>(item);

        if (isVariableDeclared(function->name)){
            hasError = true;
            Error("function \"", function->name, "\" is already declared as a variable");
            printErrorToken(node->token, source);
        }
        if (!isFunctionDefined(function->name)){
            functions.push_back(function->functionData);
        }
        else{
            hasError = true;
            Error("function \"", function->name, "\" is already defined");
            printErrorToken(node->token, source);
        }
        
    }

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
    
    requireIdenticalTypes(currentFunction, node, "return type and returned type don't match");
        
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
    
    if (node->left->semanticType->isErrorType() || node->right->semanticType->isErrorType()){
        node->semanticType = std::make_shared<AstPrimitiveType>(RSharpPrimitiveType::ErrorType);
    }
    else{
        node->semanticType = node->left->semanticType;
    }
}
void SemanticValidator::visit(std::shared_ptr<AstVariableAccess> node){
    if (isVariableDeclared(node->name)){
        auto var = getVariable(node->name);
        if (var){
            node->variable = var;
            node->semanticType = var->type.lock();
        }
        else{
            Fatal("INTERNAL ERROR: variable declared but not found in variable stack");
        }
    }
    else{
        hasError = true;
        Error("variable \"", node->name, "\" is not declared");
        printErrorToken(node->token, source);

        node->semanticType = std::make_shared<AstPrimitiveType>(RSharpPrimitiveType::ErrorType);
    }
}
void SemanticValidator::visit(std::shared_ptr<AstVariableAssignment> node){
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    if (isVariableDeclared(node->name)){
        requireType(node->value);
        node->variable = getVariable(node->name);
        node->semanticType = node->variable->type.lock();
    }
    else{
        hasError = true;
        Error("variable \"", node->name, "\" is not declared");
        printErrorToken(node->token, source);
        node->semanticType = std::make_shared<AstPrimitiveType>(RSharpPrimitiveType::ErrorType);
    }
}
void SemanticValidator::visit(std::shared_ptr<AstVariableDeclaration> node){
    // visit the children
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    
    if (node->value){
            requireIdenticalTypes(node, node->value, "value assigned to variable of different semantical type");
    }
    if (node->variable->isGlobal){
        if (isFunctionDefined(node->name)){
            hasError = true;
            Error("global variable \"", node->name, "\" is already declared as a function");
            printErrorToken(node->token, source);
        }
        if (node->value && node->value->getType() != AstNodeType::AstInteger){
            hasError = true;
            Error("global variable \"", node->name, "\" must be initialized to a constant value (no expression)");
            printErrorToken(node->token, source);
        }
    }
    if (!isVariableDefinable(*node)){
        hasError = true;
        Error("variable \"", node->name, "\" is defined multiple times");
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
        arg->accept(this);
        parameters->parameters.push_back(std::make_shared<AstVariableDeclaration>());
        parameters->parameters.back()->semanticType = arg->semanticType;
    }

    node->function = getFunction(node->name, parameters);
    if (node->function){
        node->semanticType = node->function->returnType;

        AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    }
    else{
        hasError = true;
        Error("function \"", node->name, "\" is not declared (wrong number of arguments?)");
        printErrorToken(node->token, source);
        node->semanticType = std::make_shared<AstPrimitiveType>(RSharpPrimitiveType::ErrorType);
    }
}
void SemanticValidator::visit(std::shared_ptr<AstFunctionDefinition> node){
    // push the function context to include parameters
    node->parameters->parameterBlock = std::make_shared<AstBlock>();
    node->parameters->parameterBlock->name = "parameters " + node->name;
    currentFunction = node;
    pushContext(node->parameters->parameterBlock);
    node->parameters->accept(this);

    if(std::find(node->tags->tags.begin(), node->tags->tags.end(), AstTags::Value::Extern) == node->tags->tags.end()){
        node->body->name = node->name;
        forceContextCollapse();
        node->body->accept(this);
        // the parameterContext will have been popped because it's collapsed
    }
}