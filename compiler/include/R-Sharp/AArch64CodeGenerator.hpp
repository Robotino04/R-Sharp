#pragma once

#include "R-Sharp/AstVisitor.hpp"
#include "R-Sharp/Syscall.hpp"
#include "R-Sharp/Token.hpp"

#include <memory>
#include <string>


class AArch64CodeGenerator : public AstVisitor {
    public:
        AArch64CodeGenerator(std::shared_ptr<AstProgram> root, std::string R_SharpSource);

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
        void visit(std::shared_ptr<AstVariableAccess> node) override;
        void visit(std::shared_ptr<AstVariableAssignment> node) override;
        void visit(std::shared_ptr<AstConditionalExpression> node) override;
        void visit(std::shared_ptr<AstEmptyExpression> node) override;
        void visit(std::shared_ptr<AstFunctionCall> node) override;

        void visit(std::shared_ptr<AstVariableDeclaration> node) override;

        static int sizeFromSemanticalType(std::shared_ptr<AstType> type);


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

        std::string getUniqueLabel(std::string const& prefix);
        
        void generateFunctionProlouge();
        void generateFunctionEpilouge();
        void resetStackPointer(std::shared_ptr<AstBlock> scope);

    private:
        int stackOffset = 0;

        std::string source_text;
        std::string source_data;
        std::string source_bss;
        int indentLevel;

        std::string R_SharpSource;
};