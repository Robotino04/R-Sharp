#pragma once

#include <vector>
#include <memory>
#include <string>

struct AstVisitor;

enum class AstNodeType {
    AstProgram,
    AstFunction,
    AstParameterList,

    AstBlock,
    AstReturn,
    AstExpressionStatement,
    AstConditionalStatement,
    AstForLoopDeclaration,
    AstForLoopExpression,
    AstWhileLoop,
    AstDoWhileLoop,
    AstBreak,
    AstSkip,

    AstVariableAccess,
    AstVariableAssignment,
    AstUnary,
    AstBinary,
    AstInteger,
    AstConditionalExpression,
    AstEmptyExpression,

    AstVariableDeclaration,
    AstBuiltinType,
    AstTypeModifier,
    AstArray,
};

struct AstNode{
    virtual ~AstNode() = default;

    void printTree(std::string prefix="", bool isTail=true) const;

    virtual std::vector<std::shared_ptr<AstNode>> getChildren() const{return {};};
    virtual AstNodeType getType() const = 0;
    virtual std::string toString() const = 0;
    virtual void accept(AstVisitor* visitor) = 0;
};

// forward declarations for all the AST nodes
struct AstProgram;
struct AstFunction;

struct AstStatement;
struct AstExpression;
struct AstType;
struct AstDeclaration;
struct AstBlockItem;

struct AstBlock;
struct AstReturn;
struct AstExpressionStatement;
struct AstConditionalStatement;
struct AstForLoopDeclaration;
struct AstForLoopExpression;
struct AstWhileLoop;
struct AstDoWhileLoop;
struct AstBreak;
struct AstSkip;

struct AstUnary;
struct AstBinary;
struct AstInteger;
struct AstVariableAccess;
struct AstVariableAssignment;
struct AstConditionalExpression;
struct AstEmptyExpression;

struct AstBuiltinType;
struct AstTypeModifier;
struct AstParameterList;
struct AstArray;

struct AstVariableDeclaration;