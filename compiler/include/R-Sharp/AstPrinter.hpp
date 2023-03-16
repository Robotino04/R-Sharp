#pragma once

#include "R-Sharp/AstVisitor.hpp"
#include "R-Sharp/Logging.hpp"

class AstPrinter: public AstVisitor{
    public:
        AstPrinter(std::shared_ptr<AstNode> root){
            this->root = root;
        }
        void print(){
            prefix = "";
            isTail = true;
            root->accept(this);
        }

        void visit(std::shared_ptr<AstNode> node) override{
            std::string oldPrefix = prefix;
            bool isThisTail = isTail;

            std::string nodeConnection = isThisTail ? "└── " : "├── ";
            AstNodeType t = node->getType();
            Print(oldPrefix, nodeConnection, node->toString());


            auto const& children = node->getChildren();
            for (int i = 0; i < children.size(); i++) {
                if (!children.at(i)) continue;
                prefix = oldPrefix + (isThisTail ? "    " : "│   ");
                isTail = i == children.size()-1;
                children.at(i)->accept(this);
            }
        }
    private:
        // basically arguments
        std::string prefix = "";
        bool isTail = false;
};