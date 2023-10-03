#pragma once

#include "R-Sharp/AstVisitor.hpp"
#include "R-Sharp/Syscall.hpp"
#include "R-Sharp/Token.hpp"
#include "R-Sharp/Logging.hpp"
#include "R-Sharp/RSI.hpp"

#include <memory>
#include <string>
#include <set>
#include <array>


class RSIGenerator : public AstVisitor {
    public:
        RSIGenerator(std::shared_ptr<AstProgram> root, std::string R_SharpSource);

        std::vector<RSI::Function> generate();

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

        static int sizeFromSemanticalType(std::shared_ptr<AstType> type);


    private:
        void emit(RSI::Instruction instr){
            functions.back().instructions.push_back(instr);
            lastResult = instr.result;
        }

        std::string getUniqueLabel(std::string const& prefix);
        
        void resetStackPointer(std::shared_ptr<AstBlock> scope){};
        void setupLocalVariables(std::shared_ptr<AstBlock> scope);

        void defineGlobalData(std::shared_ptr<AstExpression> node);

        enum ValueType {
            Value,
            Address,
        };
        
        void expectValueType(ValueType valueType){
            if (expectedValueType != valueType){
                Fatal("NASM Generator: Invalid intermediate value type expected!");
            }
        }

    private:
        std::vector<RSI::Function> functions;
        std::set<std::string> externalLabels;

        int stackPassedValueSize = 0;
        int arrayAccessFinalSize = 0;
        ValueType expectedValueType = ValueType::Value;

        RSI::Operand lastResult;

        std::string R_SharpSource;
};