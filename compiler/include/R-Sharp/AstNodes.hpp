#pragma once

#include "R-Sharp/Logging.hpp"

#include <vector>
#include <string>
#include <memory>

class AstNode;
class AstProgram;
class AstFunction;
class AstBlock;
class AstStatement;
class AstExpression;
class AstVariable;
class AstReturn;
class AstNumber;
class AstType;
class AstTypeModifier;
class AstArray;
class AstParameterList;

class AstNode{
    public:
        virtual ~AstNode() = default;

        virtual std::string toString() const = 0;

        void printTree(std::string prefix="", bool isTail=true) const;

        virtual std::vector<std::shared_ptr<AstNode>> getChildren() const {return {};}
};

class AstProgram : public AstNode{
    public:
        std::string toString() const override{return "Program";}

        std::vector<std::shared_ptr<AstNode>> getChildren() const override{
            std::vector<std::shared_ptr<AstNode>> children;
            children.reserve(functions.size());
            for (int i = 0; i < functions.size(); i++) {
                children.push_back(std::static_pointer_cast<AstNode>(functions.at(i)));
            }
            return children;
        }
    
    public:
        std::vector<std::shared_ptr<AstFunction>> functions;
};

class AstFunction : public AstNode{
    public:
        AstFunction(std::string name): name(name){}
        std::string toString() const override{return "Function: " + name;}

        std::vector<std::shared_ptr<AstNode>> getChildren() const override{
            return {std::static_pointer_cast<AstNode>(body), std::static_pointer_cast<AstNode>(parameters)};
        }
    
    public:
        std::string name;

        std::shared_ptr<AstBlock> body;
        std::shared_ptr<AstParameterList> parameters;
};


class AstStatement : public AstNode{
    public:
        virtual ~AstStatement() = default;
};

class AstBlock : public AstStatement{
    // children are statements
    public:
        std::string toString() const override{return "Block";}

        std::vector<std::shared_ptr<AstNode>> getChildren() const override{
            std::vector<std::shared_ptr<AstNode>> children;
            children.reserve(statements.size());
            for (int i = 0; i < statements.size(); i++) {
                children.push_back(std::static_pointer_cast<AstNode>(statements.at(i)));
            }
            return children;
        }
    
    public:
        std::vector<std::shared_ptr<AstStatement>> statements;
};

class AstReturn : public AstStatement{
    public:
        std::string toString() const override{return "Return";}

        std::vector<std::shared_ptr<AstNode>> getChildren() const override{
            return {std::static_pointer_cast<AstNode>(value)};
        }

    public:
        std::shared_ptr<AstExpression> value;
};

class AstExpression : public AstNode{
    public:
        virtual ~AstExpression() = default;
};

class AstNumber : public AstExpression{
    public:
        AstNumber(double value): value(value){}
        std::string toString() const override{return "Number: " + std::to_string(value);}

    public:
        double value;
};

class AstVariable : public AstNode{
    public:
        AstVariable(std::string name): name(name){}
        std::string toString() const override{return "Variable: " + name;}

        std::vector<std::shared_ptr<AstNode>> getChildren() const override{
            return {std::static_pointer_cast<AstNode>(type)};
        }

    public:
        std::string name;

        std::shared_ptr<AstType> type;
};

class AstType : public AstNode{
    public:
        AstType(): name(""){}
        AstType(std::string name): name(name){}
        std::string toString() const override{return "Type: " + name;}

        std::vector<std::shared_ptr<AstNode>> getChildren() const override{
            std::vector<std::shared_ptr<AstNode>> children;
            children.reserve(modifiers.size());
            for (int i = 0; i < modifiers.size(); i++) {
                children.push_back(std::static_pointer_cast<AstNode>(modifiers.at(i)));
            }
            return children;
        }

    public:
        std::string name;

        std::vector<std::shared_ptr<AstTypeModifier>> modifiers;
};

class AstTypeModifier : public AstNode{
    public:
        AstTypeModifier(std::string name): name(name){}
        std::string toString() const override{return "TypeModifier: " + name;}

    public:
        std::string name;
};

class AstParameterList : public AstNode{
    public:
        std::string toString() const override{return "Parameters";}

        std::vector<std::shared_ptr<AstNode>> getChildren() const override{
            std::vector<std::shared_ptr<AstNode>> children;
            children.reserve(parameters.size());
            for (int i = 0; i < parameters.size(); i++) {
                children.push_back(std::static_pointer_cast<AstNode>(parameters.at(i)));
            }
            return children;
        }
    
    public:
        std::vector<std::shared_ptr<AstVariable>> parameters;
};

class AstArray : public AstType{
    public:
        std::string toString() const override{return "Array";}

        std::vector<std::shared_ptr<AstNode>> getChildren() const override{
            return {std::static_pointer_cast<AstNode>(type)};
        }
    
    public:
        std::shared_ptr<AstType> type;
};