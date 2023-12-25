#pragma once

#include "R-Sharp/AstNodes.hpp"
#include "R-Sharp/AstVisitor.hpp"
#include "R-Sharp/Logging.hpp"
#include "R-Sharp/Utils.hpp"

class ErrorPrinter : public AstVisitor {
public:
    ErrorPrinter(std::shared_ptr<AstNode> root, std::string const& filename, std::string const& source)
        : root(root), filename(filename), source(source) {}

    void print() {
        root->accept(this);
    }

    void visit(std::shared_ptr<AstErrorStatement> node) override {
        Error(node->name);
        printErrorToken(node->token, *(node->token.source));
    };
    void visit(std::shared_ptr<AstErrorExpression> node) override {
        Error(node->name);
        printErrorToken(node->token, *(node->token.source));
    };
    void visit(std::shared_ptr<AstErrorProgramItem> node) override {
        Error(node->name);
        printErrorToken(node->token, *(node->token.source));
    };

private:
    std::string filename;
    std::string source;
    std::shared_ptr<AstNode> root;
};