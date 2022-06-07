#pragma once

#include "R-Sharp/Logging.hpp"
#include "R-Sharp/Token.hpp"

#include <vector>
#include <string>
#include <memory>

// forward declarations for all the AST nodes
struct AstProgram;
struct AstFunction;
struct AstBlock;
struct AstStatement;
struct AstReturn;
struct AstExpression;
struct AstExpressionStatement;
struct AstUnary;
struct AstBinary;
struct AstInteger;
struct AstVariableAccess;
struct AstVariableAssignment;
struct AstVariableDeclaration;
struct AstType;
struct AstBuiltinType;
struct AstTypeModifier;
struct AstParameterList;
struct AstArray;


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

    virtual void generateCCode(std::string& output);
};

template<typename... Args>
std::vector<std::shared_ptr<AstNode>> combineChildren(Args... args){
    std::vector<std::shared_ptr<AstNode>> children;
    (children.push_back(std::static_pointer_cast<AstNode>(args)), ...);
    return children;
}

// Helper macros to make it easier to define AST nodes
#define BASE(NAME) \
    NAME() = default; \
    AstNodeType getType() const override { return AstNodeType::NAME; } \
    void generateCCode(std::string& output) override;

#define CHILD_INIT(NAME, TYPE, VARIABLE_NAME) \
    NAME(std::shared_ptr<TYPE> child) : VARIABLE_NAME(child) {}


#define SINGLE_CHILD(TYPE, VARIABLE_NAME) \
    std::shared_ptr<TYPE> VARIABLE_NAME;

#define GET_SINGLE_CHILDREN(...) \
    std::vector<std::shared_ptr<AstNode>> getChildren() const override{ \
        return combineChildren(__VA_ARGS__); \
    }

#define MULTI_CHILD(TYPE, VARIABLE_NAME) std::vector<std::shared_ptr<TYPE>> VARIABLE_NAME;

#define GET_MULTI_CHILD(VARIABLE_NAME)\
    std::vector<std::shared_ptr<AstNode>> getChildren() const override{ \
        std::vector<std::shared_ptr<AstNode>> internal_children; \
        for(auto& child : VARIABLE_NAME) \
            internal_children.push_back(std::static_pointer_cast<AstNode>(child)); \
        return internal_children; \
    } \


#define TO_STRING(NAME) \
    std::string toString() const override{ \
        return #NAME; \
    }

#define TO_STRING_NAME(NAME) \
    NAME(std::string const& name): name(name){} \
    std::string toString() const override{ \
        return std::string(#NAME) + ": " + name; \
    }\
    std::string name;

#define DESTRUCTOR(NAME)\
    virtual ~NAME() = default;


// The actual AST nodes
// ----------------------------------| Program |---------------------------------- //
struct AstProgram : public AstNode {
    BASE(AstProgram)
    TO_STRING(AstProgram)

    GET_MULTI_CHILD(functions)

    MULTI_CHILD(AstFunction, functions)
};

struct AstParameterList : public AstNode {
    BASE(AstParameterList)
    TO_STRING(AstParameterList)

    GET_MULTI_CHILD(parameters)

    MULTI_CHILD(AstVariableDeclaration, parameters)
};

struct AstFunction : public AstNode {
    BASE(AstFunction)
    TO_STRING_NAME(AstFunction)

    GET_SINGLE_CHILDREN(parameters, body, returnType)

    SINGLE_CHILD(AstParameterList, parameters)
    SINGLE_CHILD(AstType, returnType)
    SINGLE_CHILD(AstBlock, body)
};


// ----------------------------------| Groups |---------------------------------- //
struct AstStatement : public AstNode{
    DESTRUCTOR(AstStatement)
};
struct AstExpression : public AstNode{
    DESTRUCTOR(AstExpression)
};
struct AstType : public AstNode{
    DESTRUCTOR(AstType)

    GET_MULTI_CHILD(modifiers)

    MULTI_CHILD(AstTypeModifier, modifiers)
};

// ----------------------------------| Statements |---------------------------------- //
struct AstBlock : public AstStatement {
    BASE(AstBlock)
    TO_STRING(AstBlock)

    GET_MULTI_CHILD(statements)

    MULTI_CHILD(AstStatement, statements)
};

struct AstReturn : public AstStatement {
    BASE(AstReturn)
    TO_STRING(AstReturn)

    CHILD_INIT(AstReturn, AstExpression, value)
    GET_SINGLE_CHILDREN(value)

    SINGLE_CHILD(AstExpression, value)
};

struct AstExpressionStatement : public AstStatement {
    BASE(AstExpressionStatement)
    TO_STRING(AstExpressionStatement)

    CHILD_INIT(AstExpressionStatement, AstExpression, expression)
    GET_SINGLE_CHILDREN(expression)

    SINGLE_CHILD(AstExpression, expression)
};

struct AstVariableDeclaration : public AstStatement {
    BASE(AstVariableDeclaration)
    TO_STRING_NAME(AstVariableDeclaration)

    GET_SINGLE_CHILDREN(type, value)

    SINGLE_CHILD(AstType, type)
    SINGLE_CHILD(AstExpression, value)
};

// ----------------------------------| Expressions |---------------------------------- //
struct AstVariableAccess : public AstExpression {
    BASE(AstVariableAccess)
    TO_STRING_NAME(AstVariableAccess)
};

struct AstInteger : public AstExpression {
    BASE(AstInteger)
    AstInteger(int value): value(value){}
    
    std::string toString() const override{
        return "AstInteger: " + std::to_string(value);
    }
    int value;
};

struct AstVariableAssignment : AstExpression{
    BASE(AstVariableAssignment)
    TO_STRING_NAME(AstVariableAssignment)

    CHILD_INIT(AstVariableAssignment, AstExpression, value)
    GET_SINGLE_CHILDREN(value)

    SINGLE_CHILD(AstExpression, value)
};


enum class AstUnaryType{
    Negate,
    LogicalNot,
    BinaryNot,
    None,
};

AstUnaryType toUnaryOperator(TokenType type);
namespace std{
    std::string to_string(AstUnaryType type);
}

struct AstUnary : public AstExpression {
    BASE(AstUnary)
    AstUnary(AstUnaryType op, std::shared_ptr<AstExpression> value): type(op), value(value){}
    std::string toString() const override{
        return "AstUnary: " + std::to_string(type);
    }
    GET_SINGLE_CHILDREN(value)

    SINGLE_CHILD(AstExpression, value)
    AstUnaryType type = AstUnaryType::None;
};



enum class AstBinaryType{
    Add,
    Subtract,
    Multiply,
    Divide,

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

AstBinaryType toBinaryOperator(TokenType type);
namespace std{
    std::string to_string(AstBinaryType type);
}

struct AstBinary : public AstExpression {
    BASE(AstBinary)
    AstBinary(std::shared_ptr<AstExpression> left, AstBinaryType type, std::shared_ptr<AstExpression> right)
        : left(left), type(type), right(right){}
    std::string toString() const override{
        return "AstBinary: " + std::to_string(type);
    }

    GET_SINGLE_CHILDREN(left, right)

    SINGLE_CHILD(AstExpression, left)
    SINGLE_CHILD(AstExpression, right)

    AstBinaryType type = AstBinaryType::None;
};



// ----------------------------------| Types |---------------------------------- //
struct AstTypeModifier : public AstNode {
    BASE(AstTypeModifier)
    TO_STRING_NAME(AstTypeModifier)
};

struct AstBuiltinType : public AstType {
    BASE(AstBuiltinType)
    TO_STRING_NAME(AstBuiltinType)
};

struct AstArray : public AstType {
    BASE(AstArray)
    TO_STRING(AstArray)

    CHILD_INIT(AstArray, AstType, type)
    GET_SINGLE_CHILDREN(type)

    SINGLE_CHILD(AstType, type)
};


#undef BASE
#undef GET_SINGLE_CHILDREN
#undef SINGLE_CHILD
#undef GET_MULTI_CHILD
#undef MULTI_CHILD
#undef TO_STRING
#undef TO_STRING_NAME
#undef DESTRUCTOR