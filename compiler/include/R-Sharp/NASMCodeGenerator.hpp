#pragma once

#include "R-Sharp/AstVisitor.hpp"
#include "R-Sharp/Syscall.hpp"
#include "R-Sharp/Token.hpp"

#include <memory>
#include <string>
#include <array>
#include <set>


class NASMCodeGenerator : public AstVisitor {
    public:
        struct StackFrame;
        struct Variable;
        struct LoopInfo;
        struct VariableScope;
        NASMCodeGenerator(std::shared_ptr<AstProgram> root, std::string R_SharpSource);

        std::string generate();

        void visit(std::shared_ptr<AstProgram> node) override;
        void visit(std::shared_ptr<AstParameterList> node) override;

        void visit(std::shared_ptr<AstFunctionDefinition> node) override;

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
        void visit(std::shared_ptr<AstAssignment> node) override;
        void visit(std::shared_ptr<AstConditionalExpression> node) override;
        void visit(std::shared_ptr<AstEmptyExpression> node) override;
        void visit(std::shared_ptr<AstExpressionStatement> node) override;
        void visit(std::shared_ptr<AstFunctionCall> node) override;
        void visit(std::shared_ptr<AstAddressOf> node) override;
        void visit(std::shared_ptr<AstTypeConversion> node) override;

        void visit(std::shared_ptr<AstVariableAccess> node) override;
        void visit(std::shared_ptr<AstDereference> node) override;
        void visit(std::shared_ptr<AstArrayAccess> node) override;
        void visit(std::shared_ptr<AstArrayLiteral> node) override;

        void visit(std::shared_ptr<AstVariableDeclaration> node) override;

    private:
        enum class BinarySection{
            Text,
            BSS,
            Data,
            COUNT
        };

        void indent(BinarySection section = BinarySection::Text);
        void dedent(BinarySection section = BinarySection::Text);
        
        void emit(std::string const& str, BinarySection section = BinarySection::Text);
        void emitIndented(std::string const& str, BinarySection section = BinarySection::Text);

        void emitSyscall(Syscall callNr, std::string const& arg1="", std::string const& arg2="", std::string const& arg3="", std::string const& arg4="", std::string const& arg5="", std::string const& arg6="");

        std::string getUniqueLabel(std::string const& prefix);

        void generateFunctionProlouge();
        void generateFunctionEpilouge();
        void setupLocalVariables(std::shared_ptr<AstBlock> scope);
        void resetStackPointer(std::shared_ptr<AstBlock> scope);

        static int sizeFromSemanticalType(std::shared_ptr<AstType> type);

        void defineGlobalData(std::shared_ptr<AstExpression> node);

        void functionCallPrologue();
        void functionCallEpilogue();


    private:
        std::string sizeToNASMType(int size);
        std::string getRegisterWithSize(std::string reg, int size);

        std::array<std::string, static_cast<size_t>(BinarySection::COUNT)> sources;
        std::array<int, static_cast<size_t>(BinarySection::COUNT)> indentLevels;
        std::set<std::string> externalLabels;

        int stackPassedValueSize = 0;

        std::string R_SharpSource;
};