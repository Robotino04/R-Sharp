#pragma once

#include "R-Sharp/AstNodesFWD.hpp"

#include <memory>

#define VISTITOR_FN(NAME) virtual void visit(NAME* node);

class AstVisitor {
public:
    virtual ~AstVisitor() = default;

    virtual void visit(AstNode* node);

    VISTITOR_FN(AstProgram);
    
    VISTITOR_FN(AstErrorStatement);
    VISTITOR_FN(AstErrorProgramItem);

    VISTITOR_FN(AstFunctionDeclaration);
    VISTITOR_FN(AstParameterList);

    VISTITOR_FN(AstBlock);
    VISTITOR_FN(AstReturn);
    VISTITOR_FN(AstExpressionStatement);
    VISTITOR_FN(AstConditionalStatement);
    VISTITOR_FN(AstForLoopDeclaration);
    VISTITOR_FN(AstForLoopExpression);
    VISTITOR_FN(AstWhileLoop);
    VISTITOR_FN(AstDoWhileLoop);
    VISTITOR_FN(AstBreak);
    VISTITOR_FN(AstSkip);

    VISTITOR_FN(AstUnary);
    VISTITOR_FN(AstBinary);
    VISTITOR_FN(AstInteger);
    VISTITOR_FN(AstVariableAccess);
    VISTITOR_FN(AstVariableAssignment);
    VISTITOR_FN(AstConditionalExpression);
    VISTITOR_FN(AstEmptyExpression);
    VISTITOR_FN(AstFunctionCall);
    
    VISTITOR_FN(AstVariableDeclaration);
    VISTITOR_FN(AstType);

protected:
    // this will keep the nodes alive
    std::shared_ptr<AstNode> root;
};

#undef VISTITOR_FN