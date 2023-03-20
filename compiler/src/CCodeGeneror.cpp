#include "R-Sharp/CCodeGenerator.hpp"
#include "R-Sharp/AstNodes.hpp"
#include "R-Sharp/Logging.hpp"

CCodeGenerator::CCodeGenerator(std::shared_ptr<AstProgram> root){
    this->root = root;
}
std::string CCodeGenerator::generate() {
    source = "";
    indentLevel = 0;
    root->accept(this);
    return source;
}

void CCodeGenerator::indent(){
    indentLevel++;
}
void CCodeGenerator::dedent(){
    if (!indentLevel){
        Error("INTERNAL ERROR: Indentation error");
        return;
    }
    indentLevel--;
}
void CCodeGenerator::emit(std::string const& str){
    source += str;
}
void CCodeGenerator::emitIndented(std::string const& str){
    if (indentedEmitBlocked){
        indentedEmitBlocked = false;
    }
    else{
        for (int i=0;i<indentLevel;i++){
            source += "    ";
        }
    }

    source += str;
}
void CCodeGenerator::blockNextIndentedEmit(){
    indentedEmitBlocked = true;
}

void CCodeGenerator::visit(std::shared_ptr<AstProgram> node){
    // stdint.h provides the int64_t type
    emit("#include <stdint.h>\n\n");
    for (auto& function : node->items) {
        function->accept(this);
        emit("\n");
    }
}
void CCodeGenerator::visit(std::shared_ptr<AstParameterList> node){
    emit("(");
    for (auto const& parameter : node->parameters) {
        parameter->semanticType->accept(this);
        emit(" " + parameter->name);

        if (parameter != node->parameters.back()) {
            emit(", ");
        }
    }
    emit(")");
}

// program items
void CCodeGenerator::visit(std::shared_ptr<AstFunctionDeclaration> node){
    node->semanticType->accept(this);
    emit(" " + node->name);
    node->parameters->accept(this);

    if (node->body){
        node->body->accept(this);
    }
    else{
        emit(";\n");
    }

}

// Statements
void CCodeGenerator::visit(std::shared_ptr<AstBlock> node){
    emitIndented("{\n");
    indent();
    for (auto const& statement : node->items) {
        statement->accept(this);
        emit("\n");
    }
    dedent();
    emitIndented("}\n");
}
void CCodeGenerator::visit(std::shared_ptr<AstReturn> node){
    emitIndented("return ");
    node->value->accept(this);
    emit(";");
}
void CCodeGenerator::visit(std::shared_ptr<AstExpressionStatement> node){
    emitIndented("");
    node->expression->accept(this);
    emit(";");
}
void CCodeGenerator::visit(std::shared_ptr<AstConditionalStatement> node){
    emitIndented("if (");
    node->condition->accept(this);
    emit(") ");
    blockNextIndentedEmit();
    node->trueStatement->accept(this);
    if (node->falseStatement){
        emit("\n");
        emitIndented("else ");
        blockNextIndentedEmit();
        node->falseStatement->accept(this);
    }
}
void CCodeGenerator::visit(std::shared_ptr<AstForLoopDeclaration> node){
    emitIndented("for (");
    blockNextIndentedEmit();
    node->initialization->accept(this);
    emit(" ");
    node->condition->accept(this);
    emit("; ");
    node->increment->accept(this);
    emit(") ");

    emit("{\n");
    indent();
    node->body->accept(this);
    dedent();
    emit("\n");
    emitIndented("}");
}
void CCodeGenerator::visit(std::shared_ptr<AstForLoopExpression> node){
    emitIndented("for (");
    node->variable->accept(this);
    emit("; ");
    node->condition->accept(this);
    emit("; ");
    node->increment->accept(this);
    emit(") ");
    if (node->body->getType() == AstNodeType::AstBlock)
        node->body->accept(this);
    else{
        emit("{\n");
        indent();
        node->body->accept(this);
        dedent();
        emit("\n}");
    }
}
void CCodeGenerator::visit(std::shared_ptr<AstWhileLoop> node){
    emitIndented("while (");
    node->condition->accept(this);
    emit(") ");
    if (node->body->getType() == AstNodeType::AstBlock)
        node->body->accept(this);
    else{
        emit("{\n");
        indent();
        node->body->accept(this);
        dedent();
        emit("\n}");
    }
}
void CCodeGenerator::visit(std::shared_ptr<AstDoWhileLoop> node){
    emitIndented("do ");
    if (node->body->getType() == AstNodeType::AstBlock)
        node->body->accept(this);
    else{
        emit("{\n");
        indent();
        node->body->accept(this);
        dedent();
        emit("\n}");
    }
    emit("\n");
    emitIndented("while (");
    node->condition->accept(this);
    emit(");");
}
void CCodeGenerator::visit(std::shared_ptr<AstBreak> node){
    emitIndented("break;");
}
void CCodeGenerator::visit(std::shared_ptr<AstSkip> node){
    emitIndented("continue;");
}
void CCodeGenerator::visit(std::shared_ptr<AstErrorStatement> node){
    emitIndented("// Error(\"" + node->name + "\");");
}

// Expressions
void CCodeGenerator::visit(std::shared_ptr<AstInteger> node){
    emit(std::to_string(node->value));
}
void CCodeGenerator::visit(std::shared_ptr<AstUnary> node){
    emit("(");
    switch (node->type) {
        case AstUnaryType::Negate:
            emit("-");
            break;
        case AstUnaryType::LogicalNot:
            emit("!");
            break;
        case AstUnaryType::BinaryNot:
            emit("~");
            break;
        default:
            Fatal("Invalid unary operator Nr. ", static_cast<int>(node->type));
            break;
    }
    node->value->accept(this);
    emit(")");
}
void CCodeGenerator::visit(std::shared_ptr<AstBinary> node){
    emit("(");
    node->left->accept(this);
    switch (node->type){
        case AstBinaryType::Add:
            emit(" + ");
            break;
        case AstBinaryType::Subtract:
            emit(" - ");
            break;
        case AstBinaryType::Multiply:
            emit(" * ");
            break;
        case AstBinaryType::Divide:
            emit(" / ");
            break;
        case AstBinaryType::Modulo:
            emit(" % ");
            break;

        case AstBinaryType::Equal:
            emit(" == ");
            break;
        case AstBinaryType::NotEqual:
            emit(" != ");
            break;
        case AstBinaryType::LessThan:
            emit(" < ");
            break;
        case AstBinaryType::LessThanOrEqual:
            emit(" <= ");
            break;
        case AstBinaryType::GreaterThan:
            emit(" > ");
            break;
        case AstBinaryType::GreaterThanOrEqual:
            emit(" >= ");
            break;

        case AstBinaryType::LogicalAnd:
            emit(" && ");
            break;
        case AstBinaryType::LogicalOr:
            emit(" || ");
            break;

        default:
            Fatal("Invalid binary operator Nr. ", static_cast<int>(node->type));
            break;
    }
    node->right->accept(this);
    emit(")");
}
void CCodeGenerator::visit(std::shared_ptr<AstVariableAccess> node){
    emit(node->name);
}
void CCodeGenerator::visit(std::shared_ptr<AstVariableAssignment> node){
    emit("(" + node->name + " = ");
    node->value->accept(this);
    emit(")");
}
void CCodeGenerator::visit(std::shared_ptr<AstConditionalExpression> node){
    emit("(");
    node->condition->accept(this);
    emit(" ? ");
    node->trueExpression->accept(this);
    emit(" : ");
    node->falseExpression->accept(this);
    emit(")");
}
void CCodeGenerator::visit(std::shared_ptr<AstFunctionCall> node){
    emit(node->name + "(");
    for (int i = 0; i < node->arguments.size(); i++){
        node->arguments[i]->accept(this);
        if (i < node->arguments.size() - 1)
            emit(", ");
    }
    emit(")");
}
// Declarations
void CCodeGenerator::visit(std::shared_ptr<AstVariableDeclaration> node){
    emitIndented("");
    node->semanticType->accept(this);
    emit(" " + node->name);
    if (node->value){
        emit(" = ");
        node->value->accept(this);
    }
    emit(";");
}
void CCodeGenerator::visit(std::shared_ptr<AstType> node){
    switch (node->type){
        case RSharpType::I32: emit("int32_t"); break;
        case RSharpType::I64: emit("int64_t"); break;

        default:
            Fatal("Unimplemented type Nr.", static_cast<int>(node->type));
            break;
    }
}