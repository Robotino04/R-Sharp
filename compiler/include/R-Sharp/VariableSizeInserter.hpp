#pragma once

#include "R-Sharp/AstVisitor.hpp"
#include "R-Sharp/Syscall.hpp"
#include "R-Sharp/Token.hpp"

#include <memory>
#include <functional>
#include <vector>

class VariableSizeInserter : public AstVisitor {
    public:
        VariableSizeInserter(std::shared_ptr<AstProgram> root);

        void insert(std::function<int(std::shared_ptr<AstType>)> typeToSize);

        void visit(std::shared_ptr<AstFunctionDefinition> node) override;
        void visit(std::shared_ptr<AstForLoopDeclaration> node) override;
        void visit(std::shared_ptr<AstBlock> node) override;
        void visit(std::shared_ptr<AstVariableDeclaration> node) override;

    private:
        std::shared_ptr<AstProgram> root;
        std::function<int(std::shared_ptr<AstType>)> typeToSize;

        std::vector<std::shared_ptr<AstBlock>> scopes;
        int stackOffset = 0;
};