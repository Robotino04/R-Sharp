#pragma once

#include "R-Sharp/Logging.hpp"

#include <vector>
#include <string>
#include <memory>

struct AstNode;
struct AstProgram;
struct AstFunction;
struct AstBlock;
struct AstStatement;
struct AstExpression;
struct AstVariableDeclaration;
struct AstReturn;
struct AstInteger;
struct AstType;
struct AstBuiltinType;
struct AstTypeModifier;
struct AstArray;
struct AstParameterList;

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
    NEGATION,
    LOGICAL_NOT,
    BITWISE_NOT,
};

struct AstNode{
    virtual ~AstNode() = default;

    virtual std::string toString() const = 0;

    void printTree(std::string prefix="", bool isTail=true) const{
        std::string nodeConnection = isTail ? "└── " : "├── ";
        Print(prefix, nodeConnection, toString());

        auto const& children = getChildren();
        for (int i = 0; i < children.size(); i++) {
            std::string newPrefix = prefix + (isTail ? "    " : "│   ");
            children.at(i)->printTree(newPrefix, i == children.size()-1);
        }
    }

    virtual void generateCCode(std::string& output){output += "!!!Unimplemented Node!!!";}

    virtual std::vector<std::shared_ptr<AstNode>> getChildren() const {return {};}

    virtual AstNodeType getType() const = 0;
};

struct AstProgram : public AstNode{
    std::string toString() const override{return "Program";}

    std::vector<std::shared_ptr<AstNode>> getChildren() const override{
        std::vector<std::shared_ptr<AstNode>> children;
        children.reserve(functions.size());
        for (int i = 0; i < functions.size(); i++) {
            children.push_back(std::static_pointer_cast<AstNode>(functions.at(i)));
        }
        return children;
    }

    virtual void generateCCode(std::string& output) override;

    AstNodeType getType() const override{return AstNodeType::PROGRAM;}

    std::vector<std::shared_ptr<AstFunction>> functions;
};

struct AstFunction : public AstNode{
    AstFunction() = default;
    AstFunction(std::string name): name(name){}
    std::string toString() const override{return "Function: " + name;}

    std::vector<std::shared_ptr<AstNode>> getChildren() const override{
        return {std::static_pointer_cast<AstNode>(parameters), std::static_pointer_cast<AstNode>(returnType), std::static_pointer_cast<AstNode>(body)};
    }

    virtual void generateCCode(std::string& output) override;

    AstNodeType getType() const override{return AstNodeType::FUNCTION;}

    std::string name;

    std::shared_ptr<AstParameterList> parameters;
    std::shared_ptr<AstType> returnType;
    std::shared_ptr<AstBlock> body;
};


struct AstStatement : public AstNode{
    virtual ~AstStatement() = default;
};

struct AstBlock : public AstStatement{
    std::string toString() const override{return "Block";}

    std::vector<std::shared_ptr<AstNode>> getChildren() const override{
        std::vector<std::shared_ptr<AstNode>> children;
        children.reserve(statements.size());
        for (int i = 0; i < statements.size(); i++) {
            children.push_back(std::static_pointer_cast<AstNode>(statements.at(i)));
        }
        return children;
    }

    virtual void generateCCode(std::string& output) override;

    AstNodeType getType() const override{return AstNodeType::BLOCK;}

    std::vector<std::shared_ptr<AstStatement>> statements;
};

struct AstReturn : public AstStatement{
    std::string toString() const override{return "Return";}

    std::vector<std::shared_ptr<AstNode>> getChildren() const override{
        return {std::static_pointer_cast<AstNode>(value)};
    }

    virtual void generateCCode(std::string& output) override;

    AstNodeType getType() const override{return AstNodeType::RETURN;}

    std::shared_ptr<AstExpression> value;
};

struct AstExpression : public AstNode{
    virtual ~AstExpression() = default;
};

struct AstUnary : public AstExpression{
    virtual ~AstUnary() = default;
};

struct AstNegation : public AstUnary{
    std::string toString() const override{return "Negative";}

    std::vector<std::shared_ptr<AstNode>> getChildren() const override{
        return {std::static_pointer_cast<AstNode>(value)};
    }

    virtual void generateCCode(std::string& output) override;

    AstNodeType getType() const override{return AstNodeType::NEGATION;}

    std::shared_ptr<AstExpression> value;
};

struct AstLogicalNot : public AstUnary{
    std::string toString() const override{return "Logical Not";}

    std::vector<std::shared_ptr<AstNode>> getChildren() const override{
        return {std::static_pointer_cast<AstNode>(value)};
    }

    virtual void generateCCode(std::string& output) override;

    AstNodeType getType() const override{return AstNodeType::LOGICAL_NOT;}

    std::shared_ptr<AstExpression> value;
};

struct AstBitwiseNot : public AstUnary{
    std::string toString() const override{return "Bitwise Not";}

    std::vector<std::shared_ptr<AstNode>> getChildren() const override{
        return {std::static_pointer_cast<AstNode>(value)};
    }

    virtual void generateCCode(std::string& output) override;

    AstNodeType getType() const override{return AstNodeType::BITWISE_NOT;}

    std::shared_ptr<AstExpression> value;
};


struct AstInteger : public AstExpression{
    AstInteger() = default;
    AstInteger(double value): value(value){}
    std::string toString() const override{return "Int: " + std::to_string(value);}

    virtual void generateCCode(std::string& output) override;

    AstNodeType getType() const override{return AstNodeType::INTEGER;}

    int value;
};

struct AstVariableDeclaration : public AstNode{
    AstVariableDeclaration() = default;
    AstVariableDeclaration(std::string name): name(name){}
    std::string toString() const override{return "Variable: " + name;}

    std::vector<std::shared_ptr<AstNode>> getChildren() const override{
        return {std::static_pointer_cast<AstNode>(type)};
    }

    virtual void generateCCode(std::string& output) override;

    AstNodeType getType() const override{return AstNodeType::VARIABLE_DECLARATION;}

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
    std::string toString() const override{return "Builtin Type: " + name;}

    virtual void generateCCode(std::string& output) override;

    AstNodeType getType() const override{return AstNodeType::BUILTIN_TYPE;}

    std::vector<std::shared_ptr<AstNode>> getChildren() const override{
        std::vector<std::shared_ptr<AstNode>> children;
        children.reserve(modifiers.size());
        for (int i = 0; i < modifiers.size(); i++) {
            children.push_back(std::static_pointer_cast<AstNode>(modifiers.at(i)));
        }
        return children;
    }

    std::string name;
};

struct AstTypeModifier : public AstNode{
    AstTypeModifier() = default;
    AstTypeModifier(std::string name): name(name){}
    std::string toString() const override{return "TypeModifier: " + name;}

    virtual void generateCCode(std::string& output) override;

    AstNodeType getType() const override{return AstNodeType::TYPE_MODIFIER;}

    std::string name;
};

struct AstParameterList : public AstNode{
    std::string toString() const override{return "Parameters";}

    std::vector<std::shared_ptr<AstNode>> getChildren() const override{
        std::vector<std::shared_ptr<AstNode>> children;
        children.reserve(parameters.size());
        for (int i = 0; i < parameters.size(); i++) {
            children.push_back(std::static_pointer_cast<AstNode>(parameters.at(i)));
        }
        return children;
    }

    virtual void generateCCode(std::string& output) override;

    AstNodeType getType() const override{return AstNodeType::PARAMETER_LIST;}

    std::vector<std::shared_ptr<AstVariableDeclaration>> parameters;
};

struct AstArray : public AstType{
    std::string toString() const override{return "Array";}

    std::vector<std::shared_ptr<AstNode>> getChildren() const override{
        return {std::static_pointer_cast<AstNode>(type)};
    }

    virtual void generateCCode(std::string& output) override;

    AstNodeType getType() const override{return AstNodeType::ARRAY;}

    std::shared_ptr<AstType> type;
};