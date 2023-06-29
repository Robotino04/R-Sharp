#include "R-Sharp/VariableSizeInserter.hpp"
#include "R-Sharp/Logging.hpp"
#include "R-Sharp/AstNodes.hpp"
#include "R-Sharp/Utils.hpp"

VariableSizeInserter::VariableSizeInserter(std::shared_ptr<AstProgram> root){
    this->root = root;
}

void VariableSizeInserter::insert(std::function<int(std::shared_ptr<AstType>)> typeToSize){
    this->typeToSize = typeToSize;
    this->root->accept(this);
    scopes.clear();
}

void VariableSizeInserter::visit(std::shared_ptr<AstForLoopDeclaration> node){
    node->initializationContext->accept(this);
    node->body->accept(this);
}
// statements
void VariableSizeInserter::visit(std::shared_ptr<AstBlock> node){
    node->sizeOfLocalVariables = 0;

    scopes.push_back(node);
    for (auto child : node->getChildren()){
        if (child) child->accept(this);
    }
    scopes.pop_back();

    for (auto var : node->variables){
        node->sizeOfLocalVariables += var->sizeInBytes;
    }

    Print("Size of scope \"", node->name, "\": ", node->sizeOfLocalVariables, "\n");
}


void VariableSizeInserter::visit(std::shared_ptr<AstVariableDeclaration> node){
    node->variable->sizeInBytes = typeToSize(node->semanticType);
}
