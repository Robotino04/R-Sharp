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
struct AstUnary;
struct AstBinary;
struct AstInteger;
struct AstVariableDeclaration;
struct AstType;
struct AstBuiltinType;
struct AstTypeModifier;
struct AstParameterList;
struct AstArray;


enum class AstNodeType {
    PROGRAM,
    FUNCTION,
    BLOCK,
    VARIABLE_DECLARATION,
    RETURN,
    INTEGER,
    BUILTIN_TYPE,
    TYPE_MODIFIER,
    ARRAY,
    PARAMETER_LIST,

    UNARY,
    BINARY,
};

struct AstNode{
    virtual ~AstNode() = default;

    void printTree(std::string prefix="", bool isTail=true) const;

    virtual std::vector<std::shared_ptr<AstNode>> getChildren() const;
    virtual AstNodeType getType() const = 0;
    virtual std::string toString() const = 0;

    virtual void generateCCode(std::string& output);
};

struct AstProgram : public AstNode{
    std::vector<std::shared_ptr<AstNode>> getChildren() const;
    AstNodeType getType() const override;
    std::string toString() const;

    void generateCCode(std::string& output) override;

    std::vector<std::shared_ptr<AstFunction>> functions;
};

struct AstFunction : public AstNode{
    AstFunction() = default;
    AstFunction(std::string name): name(name){}

    std::vector<std::shared_ptr<AstNode>> getChildren() const override;
    AstNodeType getType() const override;
    std::string toString() const override;

    void generateCCode(std::string& output) override;

    std::string name;
    std::shared_ptr<AstParameterList> parameters;
    std::shared_ptr<AstType> returnType;
    std::shared_ptr<AstBlock> body;
};


struct AstStatement : public AstNode{
    virtual ~AstStatement() = default;
};

struct AstBlock : public AstStatement{
    std::vector<std::shared_ptr<AstNode>> getChildren() const override;
    AstNodeType getType() const override;
    std::string toString() const override;

    void generateCCode(std::string& output) override;

    std::vector<std::shared_ptr<AstStatement>> statements;
};

struct AstReturn : public AstStatement{
    std::vector<std::shared_ptr<AstNode>> getChildren() const;
    AstNodeType getType() const override;
    std::string toString() const override;

    void generateCCode(std::string& output) override;

    std::shared_ptr<AstExpression> value;
};

struct AstExpression : public AstNode{
    virtual ~AstExpression() = default;
};

struct AstUnary : public AstExpression{
    enum class Type{
        Negate,
        LogicalNot,
        BinaryNot,
        None,
    };
    AstUnary(Type type, std::shared_ptr<AstExpression> value)
        : type(type), value(value){}
    AstUnary() = default;

    std::vector<std::shared_ptr<AstNode>> getChildren() const override;
    AstNodeType getType() const override;
    std::string toString() const override;

    void generateCCode(std::string& output) override;

    std::shared_ptr<AstExpression> value;
    Type type = Type::None;
};

struct AstBinary : public AstExpression{
    enum class Type{
        Add,
        Subtract,
        Multiply,
        Divide,
        None,
    };
    AstBinary(std::shared_ptr<AstExpression> left, Type type, std::shared_ptr<AstExpression> right)
        : left(left), type(type), right(right){}
    AstBinary() = default;

    std::vector<std::shared_ptr<AstNode>> getChildren() const override;
    AstNodeType getType() const override;
    std::string toString() const override;

    void generateCCode(std::string& output) override;

    std::shared_ptr<AstExpression> left;
    std::shared_ptr<AstExpression> right;
    Type type = Type::None;
};

struct AstInteger : public AstExpression{
    AstInteger() = default;
    AstInteger(double value): value(value){}

    AstNodeType getType() const override;
    std::string toString() const override;

    void generateCCode(std::string& output) override;

    int value;
};

struct AstVariableDeclaration : public AstNode{
    AstVariableDeclaration() = default;
    AstVariableDeclaration(std::string name): name(name){}

    std::vector<std::shared_ptr<AstNode>> getChildren() const override;
    AstNodeType getType() const override;
    std::string toString() const override;

    void generateCCode(std::string& output) override;

    std::string name;
    std::shared_ptr<AstType> type;
};

struct AstType : public AstNode{
    virtual ~AstType(){};

    std::vector<std::shared_ptr<AstTypeModifier>> modifiers;
};

struct AstBuiltinType : public AstType{
    AstBuiltinType() = default;
    AstBuiltinType(std::string name): name(name){}

    std::vector<std::shared_ptr<AstNode>> getChildren() const override;
    AstNodeType getType() const override;
    std::string toString() const override;

    void generateCCode(std::string& output) override;

    std::string name;
};

struct AstTypeModifier : public AstNode{
    AstTypeModifier() = default;
    AstTypeModifier(std::string name): name(name){}

    AstNodeType getType() const override;
    std::string toString() const override;

    void generateCCode(std::string& output) override;

    std::string name;
};

struct AstParameterList : public AstNode{
    std::vector<std::shared_ptr<AstNode>> getChildren() const override;
    AstNodeType getType() const override;
    std::string toString() const override;

    void generateCCode(std::string& output) override;

    std::vector<std::shared_ptr<AstVariableDeclaration>> parameters;
};

struct AstArray : public AstType{
    std::vector<std::shared_ptr<AstNode>> getChildren() const override;
    AstNodeType getType() const override;
    std::string toString() const override;

    void generateCCode(std::string& output) override;

    std::shared_ptr<AstType> type;
};

// some helper functions

AstUnary::Type toUnaryOperator(TokenType type);
AstBinary::Type toBinaryOperator(TokenType type);

namespace std{
    std::string to_string(AstUnary::Type type);
    std::string to_string(AstBinary::Type type);
}
