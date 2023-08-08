#pragma once

#include "R-Sharp/AstVisitor.hpp"

#include <memory>
#include <string>

class CCodeGenerator : public AstVisitor {
    public:
        CCodeGenerator(std::shared_ptr<AstProgram> root);

        std::string generate();

        void visit(std::shared_ptr<AstProgram> node) override;
        void visit(std::shared_ptr<AstParameterList> node) override;

        void visit(std::shared_ptr<AstFunctionDefinition> node) override;

        void visit(std::shared_ptr<AstBlock> node) override;
        void visit(std::shared_ptr<AstReturn> node) override;
        void visit(std::shared_ptr<AstExpressionStatement> node) override;
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
        void visit(std::shared_ptr<AstArrayLiteral> node) override;
        void visit(std::shared_ptr<AstAssignment> node) override;
        void visit(std::shared_ptr<AstConditionalExpression> node) override;
        void visit(std::shared_ptr<AstFunctionCall> node) override;
        void visit(std::shared_ptr<AstAddressOf> node) override;

        void visit(std::shared_ptr<AstVariableAccess> node) override;
        void visit(std::shared_ptr<AstDereference> node) override;
        void visit(std::shared_ptr<AstTypeConversion> node) override;
        void visit(std::shared_ptr<AstArrayAccess> node) override;

        void visit(std::shared_ptr<AstVariableDeclaration> node) override;
        void visit(std::shared_ptr<AstPrimitiveType> node) override;
        void visit(std::shared_ptr<AstPointerType> node) override;
        void visit(std::shared_ptr<AstArrayType> node) override;

    private:
        void indent();
        void dedent();
        // don't indent the next emit
        void blockNextIndentedEmit();
        
        void emit(std::string const& str);
        void emitIndented(std::string const& str);

        static int sizeFromSemanticalType(std::shared_ptr<AstType> type);
        
        


        std::string source_definitions;
        std::string source_declarations;
        std::string* current_source = &source_definitions;
        int indentLevel;
        bool indentedEmitBlocked = false;

        std::string getCTypeFromPreviousNode(std::string name){
            if (middleTypeInformation.length() != 0){
                return leftTypeInformation + " (" + middleTypeInformation + name + ")" + rightTypeInformation;
            }
            else{
                return leftTypeInformation + " " + name + rightTypeInformation;
            }
        }
        void clearTypeInformation(){
            leftTypeInformation = "";
            middleTypeInformation = "";
            rightTypeInformation = "";
        }

        std::string leftTypeInformation = "";
        std::string middleTypeInformation = "";
        std::string rightTypeInformation = "";
};