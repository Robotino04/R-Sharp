#pragma once

#include "R-Sharp/AstNodesFWD.hpp"

#include <memory>

#define VISITOR_FN(NAME) virtual void visit(std::shared_ptr<NAME> node);

class AstVisitor {
public:
    virtual ~AstVisitor() = default;

    virtual void visit(std::shared_ptr<AstNode> node);

    VISITOR_FN(AstProgram);
    
    VISITOR_FN(AstErrorStatement);
    VISITOR_FN(AstErrorProgramItem);

    VISITOR_FN(AstFunctionDefinition);
    VISITOR_FN(AstParameterList);

    VISITOR_FN(AstBlock);
    VISITOR_FN(AstReturn);
    VISITOR_FN(AstExpressionStatement);
    VISITOR_FN(AstConditionalStatement);
    VISITOR_FN(AstForLoopDeclaration);
    VISITOR_FN(AstForLoopExpression);
    VISITOR_FN(AstWhileLoop);
    VISITOR_FN(AstDoWhileLoop);
    VISITOR_FN(AstBreak);
    VISITOR_FN(AstSkip);

    VISITOR_FN(AstUnary);
    VISITOR_FN(AstBinary);
    VISITOR_FN(AstInteger);
    VISITOR_FN(AstVariableAccess);
    VISITOR_FN(AstAssignment);
    VISITOR_FN(AstConditionalExpression);
    VISITOR_FN(AstEmptyExpression);
    VISITOR_FN(AstFunctionCall);
    VISITOR_FN(AstDereference);
    VISITOR_FN(AstTypeConversion);
    
    VISITOR_FN(AstVariableDeclaration);
    VISITOR_FN(AstPointerType);
    VISITOR_FN(AstPrimitiveType);

    VISITOR_FN(AstTags);

protected:
    // this will keep the nodes alive
    std::shared_ptr<AstProgram> root;
};

#undef VISITOR_FN