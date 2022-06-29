#pragma once

#include "R-Sharp/AstVisitor.hpp"

#include <memory>
#include <string>

class CCodeGenerator : public AstVisitor {
    public:
        CCodeGenerator(std::shared_ptr<AstNode> root);

        std::string generate();

        void visit(AstProgram* node) override;
        void visit(AstParameterList* node) override;

        void visit(AstFunctionDeclaration* node) override;

        void visit(AstBlock* node) override;
        void visit(AstReturn* node) override;
        void visit(AstExpressionStatement* node) override;
        void visit(AstConditionalStatement* node) override;
        void visit(AstForLoopDeclaration* node) override;
        void visit(AstForLoopExpression* node) override;
        void visit(AstWhileLoop* node) override;
        void visit(AstDoWhileLoop* node) override;
        void visit(AstBreak* node) override;
        void visit(AstSkip* node) override;
        void visit(AstErrorStatement* node) override;

        void visit(AstUnary* node) override;
        void visit(AstBinary* node) override;
        void visit(AstInteger* node) override;
        void visit(AstVariableAccess* node) override;
        void visit(AstVariableAssignment* node) override;
        void visit(AstConditionalExpression* node) override;
        void visit(AstFunctionCall* node) override;

        void visit(AstVariableDeclaration* node) override;
        void visit(AstType* node) override;

    private:
        void indent();
        void dedent();
        // don't indent the next emit
        void blockNextIndentedEmit();
        
        void emit(std::string const& str);
        void emitIndented(std::string const& str);

        std::string source;
        int indentLevel;
        bool indentedEmitBlocked = false;
};