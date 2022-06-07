#pragma once

#include <vector>
#include <memory>
#include <string>

struct AstVisitor;

enum class AstNodeType {
    AstProgram,
    AstFunction,
    AstBlock,
    AstReturn,
    AstInteger,
    AstBuiltinType,
    AstTypeModifier,
    AstArray,
    AstParameterList,

    AstUnary,
    AstBinary,

    AstExpressionStatement,

    AstVariableDeclaration,
    AstVariableAccess,
    AstVariableAssignment,
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

struct AstBlock;
struct AstReturn;
struct AstExpressionStatement;
struct AstVariableDeclaration;

struct AstUnary;
struct AstBinary;
struct AstInteger;
struct AstVariableAccess;
struct AstVariableAssignment;

struct AstBuiltinType;
struct AstTypeModifier;
struct AstParameterList;
struct AstArray;
