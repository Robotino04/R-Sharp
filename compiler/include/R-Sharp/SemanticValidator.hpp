#pragma once

#include "R-Sharp/AstVisitor.hpp"

#include <vector>

class SemanticValidator : public AstVisitor{
    public:
        SemanticValidator(std::shared_ptr<AstNode> root, std::string const& filename, std::string const& source);
        void validate();
        bool hasErrors();

        void visit(AstFunctionDeclaration* node) override;

        void visit(AstBlock* node) override;
        void visit(AstReturn* node) override;
        void visit(AstExpressionStatement* node) override;
        void visit(AstForLoopDeclaration* node) override;
        void visit(AstForLoopExpression* node) override;
        void visit(AstWhileLoop* node) override;
        void visit(AstDoWhileLoop* node) override;
        void visit(AstBreak* node) override;
        void visit(AstSkip* node) override;

        void visit(AstUnary* node) override;
        void visit(AstBinary* node) override;
        void visit(AstVariableAccess* node) override;
        void visit(AstVariableAssignment* node) override;
        void visit(AstConditionalExpression* node) override;
        void visit(AstFunctionCall* node) override;

        void visit(AstVariableDeclaration* node) override;

    private:
        void pushContext();
        void popContext();
        void forceContextCollapse(){
            collapseContexts = true;
        };
        
        bool isVariableDeclared(AstVariableDeclaration const& testVar);
        bool isVariableDefinable(AstVariableDeclaration const& testVar);
        void addVariable(AstVariableDeclaration const& var);
        AstVariableDeclaration* getVariable(AstVariableDeclaration const& var);

        bool isFunctionDeclared(AstFunctionDeclaration const& testFunc);
        bool isFunctionDeclarable(AstFunctionDeclaration const& testFunc);
        bool isFunctionDefinable(AstFunctionDeclaration const& testFunc);
        void addFunction(AstFunctionDeclaration const& func);
        AstFunctionDeclaration* getFunction(AstFunctionDeclaration const& func);

        void requireIdenticalTypes(AstNode* a, AstNode* b, std::string msg="operands don't match");
        void requireType(AstNode* node);

    private:
        std::shared_ptr<AstNode> root;
        std::string filename;
        std::string source;

        std::vector<std::vector<AstVariableDeclaration>> variableContexts;
        std::vector<AstFunctionDeclaration> functions;
        bool collapseContexts = false;

        int numLoops = 0;

        bool hasError = false;
};