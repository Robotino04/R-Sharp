#pragma once

#include "R-Sharp/AstVisitor.hpp"

#include <vector>

struct Variable{
    std::string name;
    std::string type;
    bool defined = false;

    bool operator==(Variable const& other) const{
        if (name != other.name){
            return false;
        }
        if (type != "" && other.type != ""){
            return type == other.type;
        }
        return true;
    }
    bool operator!=(Variable const& other) const{
        return !(*this == other);
    }
};

struct Function{
    std::string name;
    std::string returnType;
    std::vector<Variable> parameters;
    bool defined;

    bool operator==(Function const& other) const{
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
        
        bool isVariableDeclared(Variable testVar);
        bool isVariableDefinable(Variable testVar);
        void addVariable(Variable var);

        bool isFunctionDeclared(Function testFunc);
        bool isFunctionDeclarable(Function testFunc);
        bool isFunctionDefinable(Function testFunc);
        void addFunction(Function func);

        void printErrorToken(Token token);

    private:
        std::shared_ptr<AstNode> root;
        std::string filename;
        std::string source;

        std::vector<std::vector<Variable>> variableContexts;
        std::vector<Function> functions;
        bool collapseContexts = false;

        int numLoops = 0;

        bool hasError = false;
};