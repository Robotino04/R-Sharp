#include "R-Sharp/AstVisitor.hpp"
#include "R-Sharp/AstNodes.hpp"

#define VISITOR_FN(NAME) void AstVisitor::visit(std::shared_ptr<NAME> node) {visit(std::dynamic_pointer_cast<AstNode>(node));}

void AstVisitor::visit(std::shared_ptr<AstNode> node) {
    for (auto& child : node->getChildren()) {
        if (child) child->accept(this);
    }
}

VISITOR_FN(AstProgram)

VISITOR_FN(AstErrorStatement)
VISITOR_FN(AstErrorProgramItem)

VISITOR_FN(AstFunctionDefinition)
VISITOR_FN(AstParameterList)

VISITOR_FN(AstBlock)
VISITOR_FN(AstReturn)
VISITOR_FN(AstExpressionStatement)
VISITOR_FN(AstConditionalStatement)
VISITOR_FN(AstForLoopDeclaration)
VISITOR_FN(AstForLoopExpression)
VISITOR_FN(AstWhileLoop)
VISITOR_FN(AstDoWhileLoop)
VISITOR_FN(AstBreak)
VISITOR_FN(AstSkip)

VISITOR_FN(AstUnary)
VISITOR_FN(AstBinary)
VISITOR_FN(AstInteger)
VISITOR_FN(AstAssignment)
VISITOR_FN(AstConditionalExpression)
VISITOR_FN(AstEmptyExpression)
VISITOR_FN(AstFunctionCall)

VISITOR_FN(AstVariableAccess)
VISITOR_FN(AstDereference)

VISITOR_FN(AstVariableDeclaration)
VISITOR_FN(AstPointerType)
VISITOR_FN(AstPrimitiveType)
VISITOR_FN(AstTypeConversion)

VISITOR_FN(AstTags)


#undef VISITOR_FN