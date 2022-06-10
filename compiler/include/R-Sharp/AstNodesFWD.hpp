#pragma once

#include <vector>
#include <memory>
#include <string>

struct AstVisitor;

enum class AstNodeType {
    AstProgram,
    AstParameterList,

    AstFunction,
    AstFunctionDeclaration,

    AstErrorStatement,
    AstErrorProgramItem,

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
    AstFunctionCall,

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

struct AstErrorStatement;
struct AstErrorProgramItem;

struct AstStatement;
struct AstExpression;
struct AstType;
struct AstDeclaration;
struct AstBlockItem;
struct AstProgramItem;

struct AstFunction;
struct AstFunctionDeclaration;

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
struct AstFunctionCall;

struct AstBuiltinType;
struct AstTypeModifier;
struct AstParameterList;
struct AstArray;

struct AstVariableDeclaration;