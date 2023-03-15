#pragma once

#include "R-Sharp/Token.hpp"

#include <vector>
#include <memory>
#include <string>

struct AstVisitor;

enum class AstNodeType {
    AstProgram,
    AstParameterList,

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
    AstType
};


// forward declarations for all the AST nodes
struct AstProgram;

struct AstErrorStatement;
struct AstErrorProgramItem;

struct AstStatement;
struct AstExpression;
struct AstDeclaration;
struct AstBlockItem;
struct AstProgramItem;

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

struct AstVariableDeclaration;
struct AstParameterList;
struct AstType;

struct AstNode{
    virtual ~AstNode() = default;
    AstNode() = default;

    virtual std::vector<std::shared_ptr<AstNode>> getChildren() const{return {std::static_pointer_cast<AstNode>(semanticType)};};
    virtual AstNodeType getType() const = 0;
    virtual std::string toString() const = 0;
    virtual void accept(AstVisitor* visitor) = 0;

    std::shared_ptr<AstType> semanticType = nullptr;
    Token token;
};


enum class RSharpType{
    NONE,
    I32,
    I64,
    ErrorType,
};

enum class AstBinaryType{
    Add,
    Subtract,
    Multiply,
    Divide,
    Modulo,

    Equal,
    NotEqual,
    LessThan,
    LessThanOrEqual,
    GreaterThan,
    GreaterThanOrEqual,

    LogicalAnd,
    LogicalOr,
    
    None,
};
enum class AstUnaryType{
    Negate,
    LogicalNot,
    BinaryNot,
    None,
};


bool operator==(AstType const& a, AstType const& b);
bool operator!=(AstType const& a, AstType const& b);
bool operator==(AstVariableDeclaration const& a, AstVariableDeclaration const& b);
bool operator!=(AstVariableDeclaration const& a, AstVariableDeclaration const& b);
bool operator==(AstFunctionDeclaration const& a, AstFunctionDeclaration const& b);
bool operator==(AstParameterList const& a, AstParameterList const& b);
bool operator!=(AstParameterList const& a, AstParameterList const& b);


AstUnaryType toUnaryOperator(TokenType type);
AstBinaryType toBinaryOperator(TokenType type);

// TODO: remove because undefined behaviour 
namespace std{
    std::string to_string(AstUnaryType type);
    std::string to_string(AstBinaryType type);
    std::string to_string(const AstType* type);
}