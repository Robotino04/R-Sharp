#include "R-Sharp/CCodeGenerator.hpp"
#include "R-Sharp/AstNodes.hpp"
#include "R-Sharp/Logging.hpp"

CCodeGenerator::CCodeGenerator(std::shared_ptr<AstNode> root){
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
        Error("Indentation error");
        return;
    }
    indentLevel--;
}
void CCodeGenerator::emit(std::string const& str){
    source += str;
}
void CCodeGenerator::emitIndented(std::string const& str){
    for (int i=0;i<indentLevel;i++){
        source += "    ";
    }
    source += str;
}

void CCodeGenerator::visitAstProgram(AstProgram* node){
    for (auto const& function : node->functions) {
        visit(function);
        emit("\n");
    }
}

void CCodeGenerator::visitAstFunction(AstFunction* node){
    visit(node->returnType);
    emit(" " + node->name);
    visit(node->parameters);
    visit(node->body);
}

void CCodeGenerator::visitAstBlock(AstBlock* node){
    emitIndented("{\n");
    indent();
    for (auto const& statement : node->statements) {
        visit(statement);
        emit("\n");
    }
    dedent();
    emitIndented("}\n");
}

void CCodeGenerator::visitAstReturn(AstReturn* node){

    emitIndented("return ");
    visit(node->value);
    emit(";");
}
void CCodeGenerator::visitAstInteger(AstInteger* node){
    emit(std::to_string(node->value));
}

void CCodeGenerator::visitAstBuiltinType(AstBuiltinType* node){
    for (auto const& modifier : node->modifiers) {
        visit(modifier);
        emit(" ");
    }
    emit(node->name);
}

void CCodeGenerator::visitAstArray(AstArray* node){
    visit(node->type);
    emit("*");
    for (auto const& modifier : node->modifiers) {
        visit(modifier);
        emit(" ");
    }
}

void CCodeGenerator::visitAstTypeModifier(AstTypeModifier* node){
    emit(node->name);
}

void CCodeGenerator::visitAstVariableDeclaration(AstVariableDeclaration* node){
    emitIndented("");
    visit(node->type);
    emit(" " + node->name);
    if (node->value){
        emit(" = ");
        visit(node->value);
    }
    emit(";");
}

void CCodeGenerator::visitAstVariableAccess(AstVariableAccess* node){
    emit(node->name);
}

void CCodeGenerator::visitAstVariableAssignment(AstVariableAssignment* node){
    emit("(" + node->name + " = ");
    visit(node->value);
    emit(")");
}

void CCodeGenerator::visitAstParameterList(AstParameterList* node){
    emit("(");
    for (auto const& parameter : node->parameters) {
        visit(parameter);
        if (parameter != node->parameters.back()) {
            emit(", ");
        }
    }
    emit(")");
}

void CCodeGenerator::visitAstExpressionStatement(AstExpressionStatement* node){
    emitIndented("");
    visit(node->expression);
    emit(";");
}

void CCodeGenerator::visitAstUnary(AstUnary* node){
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
            Fatal("Invalid unary operator ", std::to_string(node->type));
            break;
    }
    visit(node->value);
    emit(")");
}

void CCodeGenerator::visitAstBinary(AstBinary* node){
    emit("(");
    visit(node->left);
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
            Fatal("Invalid binary operator ", std::to_string(node->type));
            break;
    }
    visit(node->right);
    emit(")");
}