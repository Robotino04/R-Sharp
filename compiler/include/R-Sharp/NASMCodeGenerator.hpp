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

        void visit(std::shared_ptr<AstProgram> node) override;
        void visit(std::shared_ptr<AstParameterList> node) override;

        void visit(std::shared_ptr<AstFunctionDeclaration> node) override;

        void visit(std::shared_ptr<AstBlock> node) override;
        void visit(std::shared_ptr<AstReturn> node) override;
        void visit(std::shared_ptr<AstConditionalStatement> node) override;
        void visit(std::shared_ptr<AstForLoopDeclaration> node) override;
        void visit(std::shared_ptr<AstForLoopExpression> node) override;
        void visit(std::shared_ptr<AstWhileLoop> node) override;
        void visit(std::shared_ptr<AstDoWhileLoop> node) override;
        void visit(std::shared_ptr<AstBreak> node) override;
        void visit(std::shared_ptr<AstSkip> node) override;

        void visit(std::shared_ptr<AstUnary> node) override;
        void visit(std::shared_ptr<AstBinary> node) override;
        void visit(std::shared_ptr<AstInteger> node) override;
        void visit(std::shared_ptr<AstVariableAccess> node) override;
        void visit(std::shared_ptr<AstVariableAssignment> node) override;
        void visit(std::shared_ptr<AstConditionalExpression> node) override;
        void visit(std::shared_ptr<AstEmptyExpression> node) override;
        void visit(std::shared_ptr<AstFunctionCall> node) override;

        // void visit(std::shared_ptr<AstBuiltinType> node) override;

        void visit(std::shared_ptr<AstVariableDeclaration> node) override;

    private:
        enum class BinarySection{
            Text,
            BSS,
            Data,
        };

        void indent();
        void dedent();
        
        void emit(std::string const& str, BinarySection section = BinarySection::Text);
        void emitIndented(std::string const& str, BinarySection section = BinarySection::Text);

        void emitSyscall(Syscall callNr, std::string const& arg1="", std::string const& arg2="", std::string const& arg3="", std::string const& arg4="", std::string const& arg5="", std::string const& arg6="");

        std::string getUniqueLabel(std::string const& prefix);
        int labelCounter = 0;

        void pushStackFrame();
        void popStackFrame(bool codeOnly = false);
        void popVariableScope(std::shared_ptr<AstBlock> scope);

        StackFrame& getCurrentStackFrame();
        std::shared_ptr<AstBlock> getCurrentVariableScope();

    
    public:
        struct Variable{
            std::string name;
            std::shared_ptr<AstType> type;
            std::string accessStr;
            int size;
            bool initialized = true;

            bool operator==(Variable const& other) const{
                return name == other.name && *type == *other.type && size == other.size;
            }
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
            std::vector<std::shared_ptr<AstBlock>> variableScopes;
            std::vector<LoopInfo> loopInfo;
        };

    private:
        int stackOffset = 0;
        std::vector<StackFrame> stackFrames;
        VariableScope globalScope;

        std::string sizeToNASMType(int size);

        std::vector<std::string> externFunctions;
        std::vector<std::string> globalVariables;

        std::string source_text;
        std::string source_data;
        std::string source_bss;
        int indentLevel;

        std::string R_SharpSource;
};