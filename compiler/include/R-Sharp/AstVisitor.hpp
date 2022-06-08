#pragma once

#include "R-Sharp/AstNodesFWD.hpp"

#include <memory>

#define VISTITOR_FN(NAME) virtual void visit##NAME(NAME* node) = 0

class AstVisitor {
public:
    virtual ~AstVisitor() = default;

    virtual void visit(AstNode* node) {node->accept(this);}
    virtual void visit(std::shared_ptr<AstNode> node) {node->accept(this);}

    VISTITOR_FN(AstProgram);
    VISTITOR_FN(AstFunction);

    VISTITOR_FN(AstBlock);
    VISTITOR_FN(AstReturn);
    VISTITOR_FN(AstExpressionStatement);
    VISTITOR_FN(AstConditionalStatement);

    VISTITOR_FN(AstUnary);
    VISTITOR_FN(AstBinary);
    VISTITOR_FN(AstInteger);
    VISTITOR_FN(AstVariableAccess);
    VISTITOR_FN(AstVariableAssignment);
    VISTITOR_FN(AstConditionalExpression);

    VISTITOR_FN(AstBuiltinType);
    VISTITOR_FN(AstTypeModifier);
    VISTITOR_FN(AstParameterList);
    VISTITOR_FN(AstArray);
    
    VISTITOR_FN(AstVariableDeclaration);

protected:
    // this will keep the nodes alive
    std::shared_ptr<AstNode> root;
};

#undef VISTITOR_FN