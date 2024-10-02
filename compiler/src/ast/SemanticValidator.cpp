#include "R-Sharp/ast/SemanticValidator.hpp"
#include "R-Sharp/ast/AstNodes.hpp"
#include "R-Sharp/Utils.hpp"
#include "R-Sharp/Logging.hpp"

#include <set>
#include <queue>

SemanticValidator::SemanticValidator(std::shared_ptr<AstNode> root, std::string const& filename, std::string const& source) {
    this->root = root;
    this->filename = filename;
    this->source = source;
}

void SemanticValidator::validate() {
    variableContexts = {};
    functions = {};
    collapseContexts = false;
    loops = {};

    root->accept(this);
}
bool SemanticValidator::hasErrors() {
    return hasError;
}

void SemanticValidator::pushContext(std::shared_ptr<AstBlock> block) {
    if (collapseContexts && !variableContexts.empty()) {
        variableContexts.back()->isMerged = true;

        block->variables.insert(
            block->variables.begin(),
            variableContexts.back()->variables.begin(),
            variableContexts.back()->variables.end()
        );
        variableContexts.back() = block;
        collapseContexts = false;
    }
    else {
        variableContexts.push_back(block);
    }
}
void SemanticValidator::popContext() {
    if (variableContexts.empty()) {
        Error("INTERNAL ERROR: Invalid context pop");
    }
    variableContexts.pop_back();
}
bool SemanticValidator::isVariableDeclared(std::string const& name) const {
    for (auto it = variableContexts.rbegin(); it != variableContexts.rend(); it++) {
        // TODO: use ContainerTools::contains
        auto found = std::find_if((*it)->variables.begin(), (*it)->variables.end(), [&](auto other) {
            if (other->name.size() == 0)
                Error("INTERNAL ERROR: Variable without name detected");
            return other->name == name;
        });
        if (found != (*it)->variables.end()) {
            return true;
        }
    }
    return false;
}
bool SemanticValidator::isVariableDefinable(AstVariableDeclaration const& testVar) const {
    // TODO: use ContainerTools::contains
    auto it = std::find_if(variableContexts.back()->variables.begin(), variableContexts.back()->variables.end(), [&](auto other) {
        return other->name == testVar.name;
    });
    if (it == variableContexts.back()->variables.end()) {
        return true;
    }
    else {
        if ((*it)->isGlobal && testVar.variable->isGlobal) {
            return !(*it)->isDefined;
        }
        else {
            return false;
        }
    }
}
void SemanticValidator::addVariable(std::shared_ptr<AstVariableDeclaration> var) {
    // TODO: use ContainerTools::contains
    auto it = std::find_if(variableContexts.back()->variables.begin(), variableContexts.back()->variables.end(), [&](auto other) {
        return other->name == var->name;
    });
    if (it == variableContexts.back()->variables.end()) {
        var->variable->isDefined = (bool)var->value;
        var->variable->name = var->name;
        var->variable->type = var->semanticType;

        variableContexts.back()->variables.push_back(var->variable);
    }
    else if (var->value) {
        (*it)->isDefined = true;
        var->variable = *it;
    }
}
std::shared_ptr<SemanticVariableData> SemanticValidator::getVariable(std::string const& name) {
    for (auto it = variableContexts.rbegin(); it != variableContexts.rend(); it++) {
        // TODO: use ContainerTools::contains
        auto it2 = std::find_if((*it)->variables.begin(), (*it)->variables.end(), [&](auto other) {
            return other->name == name;
        });
        if (it2 != (*it)->variables.end()) {
            return *it2;
        }
    }
    return nullptr;
}


bool SemanticValidator::isFunctionDefined(std::string name) const {
    // TODO: use ContainerTools::contains
    const auto it = std::find_if(functions.begin(), functions.end(), [&](auto other) {
        return other->name == name;
    });
    return it != functions.end();
}
std::shared_ptr<SemanticFunctionData> SemanticValidator::getFunction(std::string name, std::shared_ptr<AstParameterList> params) {
    // TODO: use ContainerTools::contains
    auto it = std::find_if(functions.begin(), functions.end(), [&](auto other) {
        if (other->name != name)
            return false;
        if (*other->parameters == *params)
            return true;

        // allow function if the parameters can be casted
        if (other->parameters->parameters.size() != params->parameters.size()) {
            return false;
        }

        for (int i = 0; i < other->parameters->parameters.size(); i++) {
            if (!areEquivalentTypes(other->parameters->parameters.at(i), params->parameters.at(i))) {
                return false;
            }
        }
        return true;
    });
    if (it == functions.end()) {
        return nullptr;
    }
    else {
        return *it;
    }
}

bool isEqualTypeInSharedPointer(std::shared_ptr<AstType> a, std::shared_ptr<AstType> b) {
    if (!a.get() || !b.get())
        return false;
    if (a->getType() != b->getType())
        return false;
    switch (a->getType()) {
        case AstNodeType::AstPrimitiveType:
            return std::static_pointer_cast<AstPrimitiveType>(a)->type
                == std::static_pointer_cast<AstPrimitiveType>(b)->type;
        case AstNodeType::AstPointerType:
            return isEqualTypeInSharedPointer(
                std::static_pointer_cast<AstPointerType>(a)->subtype,
                std::static_pointer_cast<AstPointerType>(b)
            );

        case AstNodeType::AstArrayType: {
            auto A = std::dynamic_pointer_cast<AstArrayType>(a);
            auto B = std::dynamic_pointer_cast<AstArrayType>(b);
            if (!A->size.has_value()) {
                return false;
            }
            if (!B->size.has_value()) {
                return false;
            }

            if (std::dynamic_pointer_cast<AstInteger>(A->size.value())->value
                != std::dynamic_pointer_cast<AstInteger>(B->size.value())->value) {
                return false;
            }
            return true;
        }
        default: throw std::runtime_error("Unknown type to test equality");
    }
}

bool SemanticValidator::areEquivalentTypes(std::shared_ptr<AstNode> expected, std::shared_ptr<AstNode> found) {
    static const std::vector<RSharpPrimitiveType> integerTypes = {
        RSharpPrimitiveType::I8,
        RSharpPrimitiveType::I16,
        RSharpPrimitiveType::I32,
        RSharpPrimitiveType::I64,
    };
    requireType(expected);
    requireType(found);

    // don't issue further errors if the type is unknown already
    if (expected->semanticType->isErrorType() || found->semanticType->isErrorType())
        return true;

    switch (expected->semanticType->getType()) {
        default: {
            throw std::runtime_error("Unimplemented type used");
            break;
        }
        case AstNodeType::AstPrimitiveType: {
            auto expected_type = std::static_pointer_cast<AstPrimitiveType>(expected->semanticType)->type;
            auto found_type = std::static_pointer_cast<AstPrimitiveType>(found->semanticType)->type;

            if (expected_type == found_type)
                return true;

            // TODO: use ContainerTools::contains
            if (std::find(integerTypes.begin(), integerTypes.end(), expected_type) != integerTypes.end()
                && std::find(integerTypes.begin(), integerTypes.end(), found_type) != integerTypes.end()) {
                return true;
            }

            break;
        }

        case AstNodeType::AstPointerType: {
            auto expected_type = std::static_pointer_cast<AstPointerType>(expected->semanticType);
            if (found->semanticType->getType() == AstNodeType::AstPrimitiveType) {
                // temporarily allow int to pointer conversions
                // TODO: use ContainerTools::contains
                const bool isValid = std::find(
                                         integerTypes.begin(),
                                         integerTypes.end(),
                                         std::static_pointer_cast<AstPrimitiveType>(found->semanticType)->type
                                     )
                                  != integerTypes.end();
                if (isValid)
                    Warning("Performing integer to pointer conversion.");
                return isValid;
            }
            else if (found->semanticType->getType() == AstNodeType::AstPointerType) {
                auto found_type = std::static_pointer_cast<AstPointerType>(found->semanticType);
                if (isEqualTypeInSharedPointer(expected_type->subtype, found_type->subtype)) {
                    return true;
                }
                else if (expected_type->subtype->getType() == AstNodeType::AstPrimitiveType
                         && std::static_pointer_cast<AstPrimitiveType>(expected_type->subtype)->type
                                == RSharpPrimitiveType::C_void) {
                    // a *c_void is expected
                    return true;
                }
                else if (found_type->subtype->getType() == AstNodeType::AstPrimitiveType
                         && std::static_pointer_cast<AstPrimitiveType>(found_type->subtype)->type
                                == RSharpPrimitiveType::C_void) {
                    // a *c_void is found
                    return true;
                }
            }
            else {
                return false;
            }
        }
        case AstNodeType::AstArrayType: {
            auto expected_type = std::static_pointer_cast<AstArrayType>(expected->semanticType);
            if (found->semanticType->getType() != AstNodeType::AstArrayType) {
                return false;
            }
            auto found_type = std::static_pointer_cast<AstArrayType>(found->semanticType);
            if (!isEqualTypeInSharedPointer(expected_type->subtype, found_type->subtype)) {
                return false;
            }
            if (!expected_type->size.has_value()) {
                return true;
            }
            if (!found_type->size.has_value()) {
                return true;
            }

            if (std::dynamic_pointer_cast<AstInteger>(expected_type->size.value())->value
                != std::dynamic_pointer_cast<AstInteger>(found_type->size.value())->value) {
                return false;
            }

            return true;
        }
    }

    return false;
}

bool SemanticValidator::requireEquivalentTypes(std::shared_ptr<AstNode> expected, std::shared_ptr<AstNode> found, std::string msg) {
    if (!areEquivalentTypes(expected, found)) {
        hasError = true;
        Error(
            msg,
            " (expected: ",
            expected->semanticType->toString(),
            "   found: ",
            found->semanticType->toString(),
            ")"
        );
        printErrorToken(expected->token, source);
        printErrorToken(found->token, source);
        return true;
    }
    return false;
}
void SemanticValidator::requireType(std::shared_ptr<AstNode> node) {
    if (!node->semanticType) {
        hasError = true;
        Error("INTERNAL ERROR: operand doesn't have a semanticType");
        printErrorToken(node->token, source);
        exit(1);
    }
}

void SemanticValidator::visit(std::shared_ptr<AstProgram> node) {
    // collect all functions
    for (auto item : node->items) {
        if (item->getType() != AstNodeType::AstFunctionDefinition)
            continue;
        auto function = std::dynamic_pointer_cast<AstFunctionDefinition>(item);

        if (isVariableDeclared(function->name)) {
            hasError = true;
            Error("function \"", function->name, "\" is already declared as a variable");
            printErrorToken(node->token, source);
        }
        if (!isFunctionDefined(function->name)) {
            functions.push_back(function->functionData);
        }
        else {
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

    for (auto var : node->globalScope->variables) {
        if (!var->isDefined)
            node->uninitializedGlobalVariables.push_back(var);
    }
}

void SemanticValidator::visit(std::shared_ptr<AstBlock> node) {
    pushContext(node);
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    popContext();
}
void SemanticValidator::visit(std::shared_ptr<AstReturn> node) {
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    node->semanticType = node->value->semanticType;

    if (requireEquivalentTypes(currentFunction, node, "return type and returned type don't match")) {
        node->semanticType = std::make_shared<AstPrimitiveType>(RSharpPrimitiveType::ErrorType);
    }
    if (!isEqualTypeInSharedPointer(currentFunction->semanticType, node->semanticType)) {
        // apply an automaic type conversion
        node->value = std::make_shared<AstTypeConversion>(node->value, currentFunction->semanticType);
    }

    // don't copy the global scope
    node->containedScopes
        .insert(node->containedScopes.begin(), std::next(variableContexts.begin()), variableContexts.end());
}
void SemanticValidator::visit(std::shared_ptr<AstExpressionStatement> node) {
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    node->semanticType = node->expression->semanticType;
}

void SemanticValidator::visit(std::shared_ptr<AstForLoopDeclaration> node) {
    loops.push(node->loop = std::make_shared<SemanticLoopData>());
    loops.top()->hasAdditionalCleanup = true;
    node->initializationContext->name = "for loop counter";
    variableContexts.back()->hasLoopCurrently = true;
    node->initializationContext->accept(this);
    variableContexts.back()->hasLoopCurrently = false;
    loops.pop();
}
void SemanticValidator::visit(std::shared_ptr<AstForLoopExpression> node) {
    loops.push(node->loop = std::make_shared<SemanticLoopData>());
    loops.top()->hasAdditionalCleanup = false;
    variableContexts.back()->hasLoopCurrently = true;
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    variableContexts.back()->hasLoopCurrently = false;
    loops.pop();
}
void SemanticValidator::visit(std::shared_ptr<AstWhileLoop> node) {
    loops.push(node->loop = std::make_shared<SemanticLoopData>());
    loops.top()->hasAdditionalCleanup = false;
    variableContexts.back()->hasLoopCurrently = true;
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    variableContexts.back()->hasLoopCurrently = false;
    loops.pop();
}
void SemanticValidator::visit(std::shared_ptr<AstDoWhileLoop> node) {
    loops.push(node->loop = std::make_shared<SemanticLoopData>());
    loops.top()->hasAdditionalCleanup = false;
    variableContexts.back()->hasLoopCurrently = true;
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    variableContexts.back()->hasLoopCurrently = false;
    loops.pop();
}
void SemanticValidator::visit(std::shared_ptr<AstBreak> node) {
    // TODO: use ContainerTools::contains
    auto it = std::find_if(variableContexts.rbegin(), variableContexts.rend(), [&](auto varContext) {
        return varContext->hasLoopCurrently;
    });
    if (loops.empty() || it == variableContexts.rend()) {
        hasError = true;
        Error("Break statement outside of loop");
        printErrorToken(node->token, source);
    }
    else {
        node->loop = loops.top();
        if (node->loop->hasAdditionalCleanup) {
            // advance by one to NOT cleanup the additional context.
            // that is handled after the end label.
            node->containedScopes = {std::next(it.base(), 1), variableContexts.end()};
        }
        else {
            // otherwise, cleanup everything
            node->containedScopes = {it.base(), variableContexts.end()};
        }
    }
}
void SemanticValidator::visit(std::shared_ptr<AstSkip> node) {
    // TODO: use ContainerTools::contains
    auto it = std::find_if(variableContexts.rbegin(), variableContexts.rend(), [&](auto varContext) {
        return varContext->hasLoopCurrently;
    });
    if (loops.empty() || it == variableContexts.rend()) {
        hasError = true;
        Error("Skip statement outside of loop");
        printErrorToken(node->token, source);
    }
    else {
        node->loop = loops.top();
        node->containedScopes = {std::next(it.base(), 1), variableContexts.end()};
    }
}

void SemanticValidator::visit(std::shared_ptr<AstUnary> node) {
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    requireType(node->value);
    node->semanticType = node->value->semanticType;
}
void SemanticValidator::visit(std::shared_ptr<AstBinary> node) {
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    if (node->left->semanticType->getType() == AstNodeType::AstPrimitiveType
        && std::static_pointer_cast<AstPrimitiveType>(node->left->semanticType)->type == RSharpPrimitiveType::C_void) {
        hasError = true;
        Error("expressions may not have type c_void");
        printErrorToken(node->left->token, source);
        node->semanticType = std::make_shared<AstPrimitiveType>(RSharpPrimitiveType::ErrorType);
        return;
    }
    if (node->right->semanticType->getType() == AstNodeType::AstPrimitiveType
        && std::static_pointer_cast<AstPrimitiveType>(node->right->semanticType)->type == RSharpPrimitiveType::C_void) {
        hasError = true;
        Error("expressions may not have type c_void");
        printErrorToken(node->right->token, source);
        node->semanticType = std::make_shared<AstPrimitiveType>(RSharpPrimitiveType::ErrorType);
        return;
    }
    if (requireEquivalentTypes(node->left, node->right, "Binary operands don't match in semantical type")) {
        node->semanticType = std::make_shared<AstPrimitiveType>(RSharpPrimitiveType::ErrorType);
        return;
    }
    if (!isEqualTypeInSharedPointer(node->left->semanticType, node->right->semanticType)) {
        if (node->left->semanticType->getType() != AstNodeType::AstPointerType
            && node->right->semanticType->getType() != AstNodeType::AstPointerType) {
            // apply an automaic type conversion
            node->right = std::make_shared<AstTypeConversion>(node->right, node->left->semanticType);
        }
        else {
            // one of the types is a pointer
            if (node->type != AstBinaryType::Subtract && node->type != AstBinaryType::Add) {
                hasError = true;
                Error("Two pointers can only be added and subtracted.");
                printErrorToken(node->token, source);
                node->semanticType = std::make_shared<AstPrimitiveType>(RSharpPrimitiveType::ErrorType);
                return;
            }
            else {
                if (node->left->semanticType->getType() == AstNodeType::AstPointerType) {
                    // apply an automaic type conversion
                    node->right = std::make_shared<AstTypeConversion>(
                        node->right,
                        std::make_shared<AstPrimitiveType>(RSharpPrimitiveType::I64)
                    );
                }
                if (node->right->semanticType->getType() == AstNodeType::AstPointerType) {
                    // apply an automaic type conversion
                    node->left = std::make_shared<AstTypeConversion>(
                        node->left,
                        std::make_shared<AstPrimitiveType>(RSharpPrimitiveType::I64)
                    );
                }
            }
        }
    }

    if (node->left->semanticType->isErrorType() || node->right->semanticType->isErrorType()) {
        node->semanticType = std::make_shared<AstPrimitiveType>(RSharpPrimitiveType::ErrorType);
    }
    else {
        node->semanticType = node->left->semanticType;
    }
}
void SemanticValidator::visit(std::shared_ptr<AstVariableAccess> node) {
    if (isVariableDeclared(node->name)) {
        auto var = getVariable(node->name);
        if (var) {
            node->variable = var;
            node->semanticType = var->type.lock();
        }
        else {
            Fatal("INTERNAL ERROR: variable declared but not found in variable stack");
        }
    }
    else {
        hasError = true;
        Error("variable \"", node->name, "\" is not declared");
        printErrorToken(node->token, source);

        node->semanticType = std::make_shared<AstPrimitiveType>(RSharpPrimitiveType::ErrorType);
    }
}
void SemanticValidator::visit(std::shared_ptr<AstDereference> node) {
    node->operand->accept(this);
    if (node->operand->semanticType->isErrorType()) {
        node->semanticType = std::make_shared<AstPrimitiveType>(RSharpPrimitiveType::ErrorType);
        return;
    }
    if (node->operand->semanticType->getType() != AstNodeType::AstPointerType) {
        hasError = true;
        Error("Cannot dereference here! Not a pointer.");
        printErrorToken(node->token, source);
        node->semanticType = std::make_shared<AstPrimitiveType>(RSharpPrimitiveType::ErrorType);
        return;
    }
    if (std::static_pointer_cast<AstPointerType>(node->operand->semanticType)->subtype->getType() == AstNodeType::AstPrimitiveType
        && std::static_pointer_cast<AstPrimitiveType>(
               std::static_pointer_cast<AstPointerType>(node->operand->semanticType)->subtype
           )
                   ->type
               == RSharpPrimitiveType::C_void) {
        hasError = true;
        Error("Cannot dereference pointer of type *c_void");
        printErrorToken(node->operand->token, source);
        node->semanticType = std::make_shared<AstPrimitiveType>(RSharpPrimitiveType::ErrorType);
        return;
    }
    else {
        node->semanticType = std::static_pointer_cast<AstPointerType>(node->operand->semanticType)->subtype;
    }
}
void SemanticValidator::visit(std::shared_ptr<AstArrayAccess> node) {
    node->array->accept(this);
    node->index->accept(this);

    if (node->array->semanticType->getType() != AstNodeType::AstArrayType) {
        hasError = true;
        Error("Can only index into array, but got ", node->array->semanticType->toString(), "\n");
        printErrorToken(node->array->token, source);
        node->semanticType = std::make_shared<AstPrimitiveType>(RSharpPrimitiveType::ErrorType);
        return;
    }

    std::set<AstNodeType> validTypesToIndexInto = {
        AstNodeType::AstVariableAccess,
        AstNodeType::AstDereference,
        AstNodeType::AstArrayLiteral,
        AstNodeType::AstArrayAccess,
    };

    if (validTypesToIndexInto.count(node->array->getType()) == 0) {
        hasError = true;
        Error("Indexing into ", node->array->toString(), " is not currently supported\n");
        printErrorToken(node->array->token, source);
        node->semanticType = std::make_shared<AstPrimitiveType>(RSharpPrimitiveType::ErrorType);
        return;
    }

    node->semanticType = std::dynamic_pointer_cast<AstArrayType>(node->array->semanticType)->subtype;
}

void SemanticValidator::visit(std::shared_ptr<AstArrayLiteral> node) {
    if (node->elements.size() == 0) {
        hasError = true;
        Error("Zero length array literals aren't supported yet.\n");
        printErrorToken(node->token, source);
        node->semanticType = std::make_shared<AstPrimitiveType>(RSharpPrimitiveType::ErrorType);
        return;
    }

    std::shared_ptr<AstNode> firstElement = nullptr;
    std::shared_ptr<AstType> elementType = nullptr;
    for (int i = 0; i < node->elements.size(); i++) {
        auto element = node->elements.at(i);
        element->accept(this);
        requireType(element);
        if (element->semanticType->isErrorType()) {
            node->semanticType = std::make_shared<AstPrimitiveType>(RSharpPrimitiveType::ErrorType);
            return;
        }
        if (i == 0) {
            elementType = element->semanticType;
            node->semanticType = std::make_shared<AstArrayType>(elementType);
            std::static_pointer_cast<AstArrayType>(node->semanticType)->size = std::make_shared<AstInteger>(
                node->elements.size()
            );
            node->semanticType->accept(this);
            firstElement = element;
        }

        requireEquivalentTypes(firstElement, element, "Array element doesn't have the correct type");
        if (!isEqualTypeInSharedPointer(elementType, element->semanticType)) {
            node->elements.at(i) = std::make_shared<AstTypeConversion>(element, elementType);
        }
    }
}


void SemanticValidator::visit(std::shared_ptr<AstAssignment> node) {
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    if (node->lvalue->getType() == AstNodeType::AstVariableAccess) {
        auto var = std::static_pointer_cast<AstVariableAccess>(node->lvalue);
        if (isVariableDeclared(var->name)) {
            requireEquivalentTypes(node->lvalue, node->lvalue);
            var->variable = getVariable(var->name);
            if (!isEqualTypeInSharedPointer(node->lvalue->semanticType, node->rvalue->semanticType)) {
                // apply an automaic type conversion
                node->rvalue = std::make_shared<AstTypeConversion>(node->rvalue, node->lvalue->semanticType);
                node->semanticType = node->rvalue->semanticType;
            }
            else {
                node->semanticType = var->semanticType;
            }
        }
        else {
            hasError = true;
            Error("variable \"", var->name, "\" is not declared");
            printErrorToken(var->token, source);
            node->semanticType = std::make_shared<AstPrimitiveType>(RSharpPrimitiveType::ErrorType);
        }
    }
    else if (node->lvalue->getType() == AstNodeType::AstDereference) {
        node->semanticType = node->lvalue->semanticType;
    }
    else if (node->lvalue->getType() == AstNodeType::AstArrayAccess) {
        node->semanticType = node->lvalue->semanticType;
    }
    else {
        hasError = true;
        Error("Unimplemented type of assignment");
        printErrorToken(node->token, source);
        node->semanticType = std::make_shared<AstPrimitiveType>(RSharpPrimitiveType::ErrorType);
    }
}
void SemanticValidator::visit(std::shared_ptr<AstVariableDeclaration> node) {
    // visit the children
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    if (node->semanticType->getType() == AstNodeType::AstPrimitiveType
        && std::static_pointer_cast<AstPrimitiveType>(node->semanticType)->type == RSharpPrimitiveType::C_void) {
        hasError = true;
        Error("Variables cannot be of type c_void.");
        printErrorToken(node->semanticType->token, source);
        return;
    }

    if (node->value) {
        if (requireEquivalentTypes(node, node->value, "value assigned to variable of different semantical type")) {
            node->semanticType = std::make_shared<AstPrimitiveType>(RSharpPrimitiveType::ErrorType);
        }
        if (!isEqualTypeInSharedPointer(node->semanticType, node->value->semanticType)) {
            // apply an automaic type conversion
            node->value = std::make_shared<AstTypeConversion>(node->value, node->semanticType);
        }
    }
    if (node->variable->isGlobal) {
        if (isFunctionDefined(node->name)) {
            hasError = true;
            Error("global variable \"", node->name, "\" is already declared as a function");
            printErrorToken(node->token, source);
        }

        if (node->value && node->value->getType() == AstNodeType::AstArrayLiteral) {
            std::queue<std::shared_ptr<AstNode>> children;
            node->value->accept(this);
            children.push(node->value);
            while (!children.empty()) {
                auto child = children.front();
                children.pop();
                if (child->getType() == AstNodeType::AstArrayType) {
                    continue;
                }
                if (child->getType() != AstNodeType::AstInteger && child->getType() != AstNodeType::AstArrayLiteral
                    && child->getType() != AstNodeType::AstTypeConversion) {
                    hasError = true;
                    Error("expression isn't constant");
                    printErrorToken(child->token, source);
                }
                else {
                    auto subchildren = child->getChildren();
                    for (auto subchild : subchildren) {
                        children.push(subchild);
                    }
                }
            }
        }
        else if (node->value && node->value->getType() != AstNodeType::AstInteger) {
            hasError = true;
            Error(
                "global variable \"",
                node->name,
                "\" must be initialized to a constant value (no expression)"
            );
            printErrorToken(node->token, source);
        }
    }
    if (!isVariableDefinable(*node)) {
        hasError = true;
        Error("variable \"", node->name, "\" is defined multiple times");
        printErrorToken(node->token, source);
    }
    else {
        addVariable(node);
    }
}
void SemanticValidator::visit(std::shared_ptr<AstConditionalExpression> node) {
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    requireType(node->condition);
    requireType(node->trueExpression);
    requireType(node->falseExpression);

    if (requireEquivalentTypes(
            node->falseExpression,
            node->trueExpression,
            "true and false ternary expressions don't match in semantic type"
        )) {
        node->semanticType = std::make_shared<AstPrimitiveType>(RSharpPrimitiveType::ErrorType);
    }
    if (!isEqualTypeInSharedPointer(node->falseExpression->semanticType, node->trueExpression->semanticType)) {
        // apply an automaic type conversion
        node->falseExpression =
            std::make_shared<AstTypeConversion>(node->falseExpression, node->trueExpression->semanticType);
    }

    node->semanticType = node->trueExpression->semanticType;
}
void SemanticValidator::visit(std::shared_ptr<AstFunctionCall> node) {
    auto parameters = std::make_shared<AstParameterList>();
    for (auto arg : node->arguments) {
        arg->accept(this);
        parameters->parameters.push_back(std::make_shared<AstVariableDeclaration>());
        parameters->parameters.back()->semanticType = arg->semanticType;
    }

    node->function = getFunction(node->name, parameters);
    if (node->function) {
        node->semanticType = node->function->returnType;
        for (int i = 0; i < node->function->parameters->parameters.size(); i++) {
            if (!isEqualTypeInSharedPointer(
                    node->arguments.at(i)->semanticType,
                    node->function->parameters->parameters.at(i)->semanticType
                )) {
                // apply an automaic type conversion
                node->arguments.at(i) = std::make_shared<AstTypeConversion>(
                    node->arguments.at(i),
                    node->function->parameters->parameters.at(i)->semanticType
                );
            }
        }

        AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    }
    else {
        hasError = true;
        Error("function \"", node->name, "\" is not declared (wrong number of arguments?)");
        printErrorToken(node->token, source);
        node->semanticType = std::make_shared<AstPrimitiveType>(RSharpPrimitiveType::ErrorType);
    }
}

void SemanticValidator::visit(std::shared_ptr<AstAddressOf> node) {
    node->operand->accept(this);
    if (node->operand->getType() != AstNodeType::AstVariableAccess) {
        hasError = true;
        Error("Tried to get address of non-variable.\n");
        printErrorToken(node->token, source);
        node->semanticType = std::make_shared<AstPrimitiveType>(RSharpPrimitiveType::ErrorType);
    }
    node->semanticType = std::make_shared<AstPointerType>(node->operand->semanticType);
}


void SemanticValidator::visit(std::shared_ptr<AstFunctionDefinition> node) {
    // push the function context to include parameters
    node->parameters->parameterBlock = std::make_shared<AstBlock>();
    node->parameters->parameterBlock->name = "parameters " + node->name;
    currentFunction = node;
    pushContext(node->parameters->parameterBlock);
    node->parameters->accept(this);

    // TODO: use ContainerTools::contains
    if (std::find(node->tags->tags.begin(), node->tags->tags.end(), AstTags::Value::Extern)
        == node->tags->tags.end()) {
        node->body->name = node->name;
        forceContextCollapse();
        node->body->accept(this);
        // the parameterContext will have been popped because it's collapsed
    }
    else {
        // extern functions don't have a body that does the cleanup.
        popContext();
    }
}

void SemanticValidator::visit(std::shared_ptr<AstArrayType> node) {
    AstVisitor::visit(std::dynamic_pointer_cast<AstNode>(node));
    if (node->subtype->getType() == AstNodeType::AstPrimitiveType
        && std::static_pointer_cast<AstPrimitiveType>(node->subtype)->type == RSharpPrimitiveType::C_void) {
        hasError = true;
        Error("Arrays cannot contain the type c_void.");
        printErrorToken(node->semanticType->token, source);
        return;
    }

    if (node->size.has_value()) {
        if (node->size.value()->getType() != AstNodeType::AstInteger) {
            hasError = true;
            Error("Sized arrays must have a size known at compile-time (currently only integer literals).");
            printErrorToken(node->size.value()->token, source);
            return;
        }
        if (std::static_pointer_cast<AstInteger>(node->size.value())->value <= 0) {
            hasError = true;
            Error("Sized arrays must have a positive, non-zero size.");
            printErrorToken(node->size.value()->token, source);
            return;
        }
    }
    else {
        hasError = true;
        Error("Unsized arrays must be assigned an array literal.");
        printErrorToken(node->token, source);
        return;
    }
}
