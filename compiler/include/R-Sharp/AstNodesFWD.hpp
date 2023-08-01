#pragma once

#include "R-Sharp/Token.hpp"

#include <vector>
#include <memory>
#include <string>

struct AstVisitor;

enum class AstNodeType {
    AstProgram,
    AstParameterList,

    AstFunctionDefinition,

    AstErrorStatement,
    AstErrorProgramItem,
    AstErrorExpression,

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
    AstAssignment,
    AstUnary,
    AstBinary,
    AstInteger,
    AstConditionalExpression,
    AstEmptyExpression,
    AstFunctionCall,
    AstDereference,
    AstTypeConversion,
    AstAddressOf,
    AstArrayAccess,
    

    AstVariableDeclaration,
    AstPrimitiveType,
    AstPointerType,
    AstArrayType,
    AstTags,
};

struct SemanticVariableData;
struct SemanticLoopData;
struct SemanticFunctionData;

// forward declarations for all the AST nodes
struct AstProgram;

struct AstErrorStatement;
struct AstErrorProgramItem;
struct AstErrorExpression;

struct AstStatement;
struct AstExpression;
struct AstDeclaration;
struct AstBlockItem;
struct AstProgramItem;

struct AstFunctionDefinition;

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
struct AstAssignment;
struct AstConditionalExpression;
struct AstEmptyExpression;
struct AstFunctionCall;
struct AstTypeConversion;
struct AstAddressOf;
struct AstArrayAccess;

struct AstLValue;
struct AstDereference;
struct AstVariableAccess;

struct AstVariableDeclaration;
struct AstParameterList;
struct AstType;
struct AstPointerType;
struct AstArrayType;
struct AstPrimitiveType;
struct AstTags;

struct AstNode{
    virtual ~AstNode() = default;
    AstNode() = default;

    virtual std::vector<std::shared_ptr<AstNode>> getChildren() const{return {};};
    virtual AstNodeType getType() const = 0;
    virtual std::string toString() const = 0;
    virtual void accept(AstVisitor* visitor) = 0;

    std::shared_ptr<AstType> semanticType = nullptr;
    Token token;
};


enum class RSharpPrimitiveType{
    NONE,
    I8,
    I16,
    I32,
    I64,
    ErrorType,
    C_void,
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
bool operator==(AstFunctionDefinition const& a, AstFunctionDefinition const& b);
bool operator==(AstParameterList const& a, AstParameterList const& b);
bool operator!=(AstParameterList const& a, AstParameterList const& b);


AstUnaryType toUnaryOperator(TokenType type);
AstBinaryType toBinaryOperator(TokenType type);