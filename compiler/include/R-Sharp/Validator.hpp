#pragma once

#include "R-Sharp/AstVisitor.hpp"

#include <vector>

struct ValidatorVariable{
    std::string name;
    std::string type;
    bool defined = false;
    bool isGlobal = false;

    bool operator==(ValidatorVariable const& other) const{
        if (name != other.name){
            return false;
        }
        if (type != "" && other.type != ""){
            return type == other.type;
        }
        return true;
    }
    bool operator!=(ValidatorVariable const& other) const{
        return !(*this == other);
    }
};

struct ValidatorFunction{
    std::string name;
    std::string returnType;
    std::vector<ValidatorVariable> parameters;
    bool defined;

    bool operator==(ValidatorFunction const& other) const{
        if (name != other.name){
            return false;
        }
        if (returnType != "" && other.returnType != ""){
            if (returnType != other.returnType){
                return false;
            }
        }
        if (parameters.size() != other.parameters.size()){
            return false;
        }
        return true;
    }
};

class Validator : public AstVisitor{
    public:
        Validator(std::shared_ptr<AstNode> root, std::string const& filename, std::string const& source);
        void validate();
        bool hasErrors();

        void visit(AstBlock* node) override;

        void visit(AstForLoopDeclaration* node) override;
        void visit(AstForLoopExpression* node) override;
        void visit(AstWhileLoop* node) override;
        void visit(AstDoWhileLoop* node) override;
        void visit(AstBreak* node) override;
        void visit(AstSkip* node) override;

        void visit(AstVariableAccess* node) override;
        void visit(AstVariableAssignment* node) override;
        void visit(AstVariableDeclaration* node) override;
        void visit(AstFunctionCall* node) override;
        void visit(AstFunctionDeclaration* node) override;
        void visit(AstFunction* node) override;

    private:
        void pushContext();
        void popContext();
        void forceContextCollapse(){
            collapseContexts = true;
        };
        
        bool isVariableDeclared(ValidatorVariable testVar);
        bool isVariableDefinable(ValidatorVariable testVar);
        void addVariable(ValidatorVariable var);

        bool isFunctionDeclared(ValidatorFunction testFunc);
        bool isFunctionDeclarable(ValidatorFunction testFunc);
        bool isFunctionDefinable(ValidatorFunction testFunc);
        void addFunction(ValidatorFunction func);

        void printErrorToken(Token token);

    private:
        std::shared_ptr<AstNode> root;
        std::string filename;
        std::string source;

        std::vector<std::vector<ValidatorVariable>> variableContexts;
        std::vector<ValidatorFunction> functions;
        bool collapseContexts = false;

        int numLoops = 0;

        bool hasError = false;
};