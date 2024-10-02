#pragma once

#include "R-Sharp/ast/AstVisitor.hpp"

#include <memory>
#include <string>
#include <stack>

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
    std::string getIndentation();

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

    std::string getCTypeFromPreviousNode(std::string name) {
        if (middleTypeInformation.length() != 0) {
            return leftTypeInformation + " (" + middleTypeInformation + name + ")" + rightTypeInformation;
        }
        else {
            return leftTypeInformation + " " + name + rightTypeInformation;
        }
    }
    void clearTypeInformation() {
        leftTypeInformation = "";
        middleTypeInformation = "";
        rightTypeInformation = "";
    }

    std::string leftTypeInformation = "";
    std::string middleTypeInformation = "";
    std::string rightTypeInformation = "";

    std::stack<std::string> prefixStatements;
    uint64_t uniqueVarID = 0;

    class PrefixStatementContext {
    public:
        PrefixStatementContext(CCodeGenerator* parent) {
            this->parent = parent;
            temporary_source = std::make_shared<std::string>("");
            last_source = parent->current_source;
            parent->current_source = temporary_source.get();
            parent->prefixStatements.push("");
            applied = false;
        }
        ~PrefixStatementContext() {
            if (!applied) {
                apply();
            }
        }

        // insert the topmost prefix statement into the output
        void apply() {
            auto top = parent->prefixStatements.top();
            parent->prefixStatements.pop();
            if (top.length() != 0) {
                *last_source += top + "\n" + *parent->current_source;
            }
            else {
                *last_source += *parent->current_source;
            }
            parent->current_source = last_source;

            applied = true;
        }

        // insert the topmost prefix statement into the second highest one
        void applyToParent() {
            auto top = parent->prefixStatements.top();
            parent->prefixStatements.pop();
            if (top.length() != 0) {
                parent->prefixStatements.top() = top + parent->prefixStatements.top();
            }
            parent->current_source = last_source;

            applied = true;
        }

    private:
        CCodeGenerator* parent;
        std::shared_ptr<std::string> temporary_source;
        std::string* last_source;
        bool applied = false;
    };

    std::string getUniqueVarName(std::string const& prefix) {
        return prefix + "_r_sharp_internal_" + std::to_string(uniqueVarID++);
    }
};