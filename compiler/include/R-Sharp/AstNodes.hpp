#pragma once

#include "R-Sharp/Logging.hpp"
#include "R-Sharp/Token.hpp"
#include "R-Sharp/AstNodesFWD.hpp"
#include "R-Sharp/AstVisitor.hpp"

#include <vector>
#include <string>
#include <memory>

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
    void accept(AstVisitor* visitor) override{ visitor->visit(this); }

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
struct AstProgram : public virtual AstNode {
    BASE(AstProgram)
    TO_STRING(AstProgram)

    GET_MULTI_CHILD(items)

    MULTI_CHILD(AstProgramItem, items)
};

struct AstParameterList : public virtual AstNode {
    BASE(AstParameterList)
    TO_STRING(AstParameterList)

    GET_MULTI_CHILD(parameters)

    MULTI_CHILD(AstVariableDeclaration, parameters)
};


// ----------------------------------| Groups |---------------------------------- //
struct AstExpression : public virtual AstNode{
    DESTRUCTOR(AstExpression)
};
struct AstType : public virtual AstNode{
    DESTRUCTOR(AstType)

    GET_MULTI_CHILD(modifiers)

    MULTI_CHILD(AstTypeModifier, modifiers)
};
struct AstBlockItem : public virtual AstNode{
    DESTRUCTOR(AstBlockItem)
};
struct AstDeclaration : public AstBlockItem{
    DESTRUCTOR(AstDeclaration)
};
struct AstStatement : public AstBlockItem{
    DESTRUCTOR(AstStatement)
};
struct AstErrorNode : public virtual AstNode{
    DESTRUCTOR(AstErrorNode)

    Token token;
};
struct AstProgramItem : public virtual AstNode{
    DESTRUCTOR(AstProgramItem)
};

// ----------------------------------| Program Items |---------------------------------- //

struct AstFunction : public AstProgramItem {
    BASE(AstFunction)
    TO_STRING_NAME(AstFunction)

    GET_SINGLE_CHILDREN(parameters, body, returnType)

    SINGLE_CHILD(AstParameterList, parameters)
    SINGLE_CHILD(AstType, returnType)
    SINGLE_CHILD(AstStatement, body)
};

struct AstFunctionDeclaration : public AstProgramItem {
    BASE(AstFunctionDeclaration)
    TO_STRING_NAME(AstFunctionDeclaration)

    GET_SINGLE_CHILDREN(parameters, returnType)

    SINGLE_CHILD(AstParameterList, parameters)
    SINGLE_CHILD(AstType, returnType)
};

// ----------------------------------| Errors |---------------------------------- //

struct AstErrorStatement : public AstStatement, public AstErrorNode {
    BASE(AstErrorStatement)
    TO_STRING_NAME(AstErrorStatement)
};
struct AstErrorProgramItem : public AstProgramItem, public AstErrorNode {
    BASE(AstErrorProgramItem)
    TO_STRING_NAME(AstErrorProgramItem)
};
// ----------------------------------| Statements |---------------------------------- //
struct AstBlock : public AstStatement {
    BASE(AstBlock)
    TO_STRING(AstBlock)

    GET_MULTI_CHILD(items)

    MULTI_CHILD(AstBlockItem, items)
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


struct AstConditionalStatement : public AstStatement {
    BASE(AstConditionalStatement)
    TO_STRING(AstConditionalStatement)

    GET_SINGLE_CHILDREN(condition, trueStatement, falseStatement)

    SINGLE_CHILD(AstExpression, condition)
    SINGLE_CHILD(AstStatement, trueStatement)
    SINGLE_CHILD(AstStatement, falseStatement)
};


struct AstWhileLoop : public AstStatement {
    BASE(AstWhileLoop)
    TO_STRING(AstWhileLoop)

    GET_SINGLE_CHILDREN(condition, body)

    SINGLE_CHILD(AstExpression, condition)
    SINGLE_CHILD(AstStatement, body)
};


struct AstForLoopDeclaration : public AstStatement {
    BASE(AstForLoopDeclaration)
    TO_STRING(AstForLoopDeclaration)

    GET_SINGLE_CHILDREN(variable, condition, increment, body)

    SINGLE_CHILD(AstVariableDeclaration, variable)
    SINGLE_CHILD(AstExpression, condition)
    SINGLE_CHILD(AstExpression, increment)
    SINGLE_CHILD(AstStatement, body)
};


struct AstForLoopExpression : public AstStatement {
    BASE(AstForLoopExpression)
    TO_STRING(AstForLoopExpression)

    GET_SINGLE_CHILDREN(variable, condition, increment, body)

    SINGLE_CHILD(AstExpression, variable)
    SINGLE_CHILD(AstExpression, condition)
    SINGLE_CHILD(AstExpression, increment)
    SINGLE_CHILD(AstStatement, body)
};


struct AstDoWhileLoop : public AstStatement {
    BASE(AstDoWhileLoop)
    TO_STRING(AstDoWhileLoop)

    GET_SINGLE_CHILDREN(body, condition)

    SINGLE_CHILD(AstStatement, body)
    SINGLE_CHILD(AstExpression, condition)
};


struct AstBreak : public AstStatement {
    BASE(AstBreak)
    TO_STRING(AstBreak)
};


struct AstSkip : public AstStatement {
    BASE(AstSkip)
    TO_STRING(AstSkip)
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
struct AstConditionalExpression : public AstExpression {
    BASE(AstConditionalExpression)
    TO_STRING(AstConditionalExpression)

    GET_SINGLE_CHILDREN(condition, trueExpression, falseExpression)

    SINGLE_CHILD(AstExpression, condition)
    SINGLE_CHILD(AstExpression, trueExpression)
    SINGLE_CHILD(AstExpression, falseExpression)
};

struct AstEmptyExpression : public AstExpression {
    BASE(AstEmptyExpression)
    TO_STRING(AstEmptyExpression)
};

struct AstFunctionCall : public AstExpression {
    BASE(AstFunctionCall)
    TO_STRING_NAME(AstFunctionCall)

    GET_MULTI_CHILD(arguments)

    MULTI_CHILD(AstExpression, arguments)
};

// ----------------------------------| Types |---------------------------------- //
struct AstTypeModifier : public virtual AstNode {
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

// ----------------------------------| Declarations |---------------------------------- //
struct AstVariableDeclaration : public AstDeclaration {
    BASE(AstVariableDeclaration)
    TO_STRING_NAME(AstVariableDeclaration)

    GET_SINGLE_CHILDREN(type, value)

    SINGLE_CHILD(AstType, type)
    SINGLE_CHILD(AstExpression, value)
};

#undef BASE
#undef GET_SINGLE_CHILDREN
#undef SINGLE_CHILD
#undef GET_MULTI_CHILD
#undef MULTI_CHILD
#undef TO_STRING
#undef TO_STRING_NAME
#undef DESTRUCTOR