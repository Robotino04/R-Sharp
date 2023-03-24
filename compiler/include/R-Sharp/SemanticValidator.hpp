#pragma once

#include "R-Sharp/AstVisitor.hpp"

#include <vector>
#include <stack>

class SemanticValidator : public AstVisitor{
    public:
        SemanticValidator(std::shared_ptr<AstNode> root, std::string const& filename, std::string const& source);
        void validate();
        bool hasErrors();

        void visit(std::shared_ptr<AstProgram> node) override;

        void visit(std::shared_ptr<AstFunctionDefinition> node) override;

        void visit(std::shared_ptr<AstBlock> node);
        void visit(std::shared_ptr<AstReturn> node) override;
        void visit(std::shared_ptr<AstExpressionStatement> node) override;
        void visit(std::shared_ptr<AstForLoopDeclaration> node) override;
        void visit(std::shared_ptr<AstForLoopExpression> node) override;
        void visit(std::shared_ptr<AstWhileLoop> node) override;
        void visit(std::shared_ptr<AstDoWhileLoop> node) override;
        void visit(std::shared_ptr<AstBreak> node) override;
        void visit(std::shared_ptr<AstSkip> node) override;

        void visit(std::shared_ptr<AstUnary> node) override;
        void visit(std::shared_ptr<AstBinary> node) override;
        void visit(std::shared_ptr<AstVariableAccess> node) override;
        void visit(std::shared_ptr<AstVariableAssignment> node) override;
        void visit(std::shared_ptr<AstConditionalExpression> node) override;
        void visit(std::shared_ptr<AstFunctionCall> node) override;

        void visit(std::shared_ptr<AstVariableDeclaration> node) override;

    private:
        void pushContext(std::shared_ptr<AstBlock> block);
        void popContext();
        void forceContextCollapse(){
            collapseContexts = true;
        };
        
        bool isVariableDeclared(std::string const& name) const;
        bool isVariableDefinable(AstVariableDeclaration const& testVar) const;
        void addVariable(std::shared_ptr<AstVariableDeclaration> var);
        std::shared_ptr<SemanticVariableData> getVariable(std::string const& name);

        bool isFunctionDefined(std::string name) const;
        std::shared_ptr<SemanticFunctionData> getFunction(std::string name, std::shared_ptr<AstParameterList> params);

        void requireIdenticalTypes(std::shared_ptr<AstNode> a, std::shared_ptr<AstNode> b, std::string msg="operands don't match");
        void requireType(std::shared_ptr<AstNode> node);

    private:
        std::shared_ptr<AstNode> root;
        std::string filename;
        std::string source;

        std::vector<std::shared_ptr<AstBlock>> variableContexts;
        bool collapseContexts = false;

        std::stack<std::shared_ptr<SemanticLoopData>> loops;
        std::vector<std::shared_ptr<SemanticFunctionData>> functions;

        bool hasError = false;
};