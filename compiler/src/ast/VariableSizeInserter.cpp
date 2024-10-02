#include "R-Sharp/ast/VariableSizeInserter.hpp"
#include "R-Sharp/ast/AstNodes.hpp"
#include "R-Sharp/Logging.hpp"

VariableSizeInserter::VariableSizeInserter(std::shared_ptr<AstProgram> root) {
    this->root = root;
}

void VariableSizeInserter::insert(std::function<int(std::shared_ptr<AstType>)> typeToSize) {
    this->typeToSize = typeToSize;
    stackOffset = 0;
    scopes.clear();
    root->accept(this);
}

void VariableSizeInserter::visit(std::shared_ptr<AstFunctionDefinition> node) {
    this->stackOffset = 0;
    scopes.push_back(node->parameters->parameterBlock);

    if (std::find(node->tags->tags.begin(), node->tags->tags.end(), AstTags::Value::Extern)
        == node->tags->tags.end()) {
        // The body will contain all parameters from semanitc
        // analysis. They are just not visited automatically.

        // visit parameters to generate stack data
        node->parameters->accept(this);
        node->body->accept(this);
    }
    scopes.pop_back();
}
void VariableSizeInserter::visit(std::shared_ptr<AstForLoopDeclaration> node) {
    node->initializationContext->accept(this);
    // the body is nested in the initialization context
    // node->body->accept(this);
}
// statements
void VariableSizeInserter::visit(std::shared_ptr<AstBlock> node) {
    node->sizeOfLocalVariables = 0;

    scopes.push_back(node);
    for (auto child : node->getChildren()) {
        if (child)
            child->accept(this);
    }
    scopes.pop_back();

    for (auto var : node->variables) {
        node->sizeOfLocalVariables += var->sizeInBytes;
    }
    stackOffset -= node->sizeOfLocalVariables;

    Print("Size of scope \"", node->name, "\": ", node->sizeOfLocalVariables, "\n");
}


void VariableSizeInserter::visit(std::shared_ptr<AstVariableDeclaration> node) {
    node->variable->sizeInBytes = typeToSize(node->semanticType);
    stackOffset += node->variable->sizeInBytes;
    node->variable->accessor = stackOffset;
}
