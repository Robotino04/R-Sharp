#pragma once

#include "R-Sharp/AstVisitor.hpp"
#include "R-Sharp/Syscall.hpp"
#include "R-Sharp/Token.hpp"

#include <memory>
#include <string>


class NASMCodeGenerator : public AstVisitor {
    public:
        struct StackFrame;
        struct Variable;
        struct LoopInfo;
        struct VariableScope;
        NASMCodeGenerator(std::shared_ptr<AstNode> root, std::string R_SharpSource);

        std::string generate();

        void visit(AstProgram* node) override;
        // void visit(AstParameterList* node) override;

        void visit(AstFunction* node) override;
        // void visit(AstFunctionDeclaration* node) override;

        void visit(AstBlock* node) override;
        void visit(AstReturn* node) override;
        // void visit(AstExpressionStatement* node) override;
        void visit(AstConditionalStatement* node) override;
        void visit(AstForLoopDeclaration* node) override;
        void visit(AstForLoopExpression* node) override;
        void visit(AstWhileLoop* node) override;
        void visit(AstDoWhileLoop* node) override;
        void visit(AstBreak* node) override;
        void visit(AstSkip* node) override;
        // void visit(AstErrorStatement* node) override;

        void visit(AstUnary* node) override;
        void visit(AstBinary* node) override;
        void visit(AstInteger* node) override;
        void visit(AstVariableAccess* node) override;
        void visit(AstVariableAssignment* node) override;
        void visit(AstConditionalExpression* node) override;
        void visit(AstEmptyExpression* node) override;
        // void visit(AstFunctionCall* node) override;

        // void visit(AstBuiltinType* node) override;
        // void visit(AstTypeModifier* node) override;
        // void visit(AstArray* node) override;

        void visit(AstVariableDeclaration* node) override;

    private:
        void indent();
        void dedent();
        
        void emit(std::string const& str);
        void emitIndented(std::string const& str);

        void emitSyscall(Syscall callNr, std::string const& arg1="", std::string const& arg2="", std::string const& arg3="", std::string const& arg4="", std::string const& arg5="", std::string const& arg6="");

        std::string getUniqueLabel(std::string const& prefix);
        int labelCounter = 0;

        Variable addVariable(AstVariableDeclaration* node);
        Variable getVariable(std::string const& name);
        void pushStackFrame();
        void popStackFrame(bool codeOnly = false);
        void pushVariableScope();
        void popVariableScope();
        void popVariableScope(VariableScope const& scope);

        StackFrame& getCurrentStackFrame();
        VariableScope& getCurrentVariableScope();
        int getCurrentScopeSize();
        int getScopeSize(VariableScope const& scope);


        int stackOffset = 0;
        std::vector<StackFrame> stackFrames;

        std::string sizeToNASMType(int size);

        std::string source;
        int indentLevel;
    
    public:
        struct Variable{
            std::string name;
            std::string type;
            int size;
            int stackOffset;
        };
        struct VariableScope{
            std::vector<Variable> variables;
            bool hasLoop = false;
        };
        struct LoopInfo {
            std::string skipLabel;
            std::string breakLabel;
        };
        struct StackFrame {
            std::vector<VariableScope> variableScopes;
            std::vector<LoopInfo> loopInfo;
        };
        void printErrorToken(Token token);

        std::string R_SharpSource;
};