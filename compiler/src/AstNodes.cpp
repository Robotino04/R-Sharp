#include "R-Sharp/AstNodes.hpp"

void AstNode::printTree(std::string prefix, bool isTail) const{
    std::string nodeConnection = isTail ? "└── " : "├── ";
    Print(prefix, nodeConnection, toString());

    auto const& children = getChildren();
    for (int i = 0; i < children.size(); i++) {
        std::string newPrefix = prefix + (isTail ? "    " : "│   ");
        children.at(i)->printTree(newPrefix, i == children.size()-1);
    }
}