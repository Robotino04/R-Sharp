#pragma once

#include "R-Sharp/AstVisitor.hpp"

#include <memory>
#include <string>

class CCodeGenerator : public AstVisitor {
    public:
        CCodeGenerator(std::shared_ptr<AstNode> root);

        std::string generate();

        void visitAstProgram(AstProgram* node) override;
        void visitAstFunction(AstFunction* node) override;
        void visitAstParameterList(AstParameterList* node) override;

        void visitAstBlock(AstBlock* node) override;
        void visitAstReturn(AstReturn* node) override;
        void visitAstExpressionStatement(AstExpressionStatement* node) override;
        void visitAstConditionalStatement(AstConditionalStatement* node) override;

        void visitAstUnary(AstUnary* node) override;
        void visitAstBinary(AstBinary* node) override;
        void visitAstInteger(AstInteger* node) override;
        void visitAstVariableAccess(AstVariableAccess* node) override;
        void visitAstVariableAssignment(AstVariableAssignment* node) override;
        void visitAstConditionalExpression(AstConditionalExpression* node) override;

        void visitAstBuiltinType(AstBuiltinType* node) override;
        void visitAstTypeModifier(AstTypeModifier* node) override;
        void visitAstArray(AstArray* node) override;

        void visitAstVariableDeclaration(AstVariableDeclaration* node) override;

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