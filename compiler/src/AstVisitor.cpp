#include "R-Sharp/AstVisitor.hpp"
#include "R-Sharp/AstNodes.hpp"

#define VISTITOR_FN(NAME) void AstVisitor::visit(NAME* node) {visit(static_cast<AstNode*>(node));}

void AstVisitor::visit(AstNode* node) {
    for (auto& child : node->getChildren()) {
        if (child) child->accept(this);
    }
}

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