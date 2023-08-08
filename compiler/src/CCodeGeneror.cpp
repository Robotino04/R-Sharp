#include "R-Sharp/CCodeGenerator.hpp"
#include "R-Sharp/AstNodes.hpp"
#include "R-Sharp/Logging.hpp"

#include <map>

CCodeGenerator::CCodeGenerator(std::shared_ptr<AstProgram> root){
    this->root = root;
}
std::string CCodeGenerator::generate() {
    source_declarations = "";
    source_definitions = "";
    current_source = &source_definitions;
    indentLevel = 0;
    root->accept(this);
    return *current_source;
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
    *current_source += str;
}
void CCodeGenerator::emitIndented(std::string const& str){
    if (indentedEmitBlocked){
        indentedEmitBlocked = false;
    }
    else{
        for (int i=0;i<indentLevel;i++){
            *current_source += "    ";
        }
    }

    *current_source += str;
}
void CCodeGenerator::blockNextIndentedEmit(){
    indentedEmitBlocked = true;
}

int CCodeGenerator::sizeFromSemanticalType(std::shared_ptr<AstType> type){
    static const std::map<RSharpPrimitiveType, int> primitive_sizes = {
        {RSharpPrimitiveType::C_void, 1}, // should only be used for pointer arithmetic
        {RSharpPrimitiveType::I8, 1},
        {RSharpPrimitiveType::I16, 2},
        {RSharpPrimitiveType::I32, 4},
        {RSharpPrimitiveType::I64, 8},
    };

    switch(type->getType()){
        case AstNodeType::AstPrimitiveType:{
            return primitive_sizes.at(std::static_pointer_cast<AstPrimitiveType>(type)->type);
        }
        case AstNodeType::AstPointerType:{
            return 8;
        }
        case AstNodeType::AstArrayType:{
            auto array = std::static_pointer_cast<AstArrayType>(type);
            if (!array->size.has_value()){
                throw std::runtime_error("Array without size during code generation.");
            }
            if (array->size.value()->getType() != AstNodeType::AstInteger){
                throw std::runtime_error("Array with non constant size during code generation.");
            }
            return sizeFromSemanticalType(array->subtype) * std::static_pointer_cast<AstInteger>(array->size.value())->value;
        }
        default: throw std::runtime_error("Unimplemented type used");
    }
}

void CCodeGenerator::visit(std::shared_ptr<AstProgram> node){
    for (auto& function : node->items) {
        function->accept(this);
        emit("\n");
    }
    *current_source = 
R"(#include <stdint.h>


static void zero_memory_r_sharp_internal(void* address, uint64_t len){
    extern void* memset(void*, int, int64_t);
    memset(address, 0, len);
}

// ----------Declarations----------
)"
    + source_declarations + "\n"
    "// -----------Definitions-----------\n"
    + source_definitions;
}
void CCodeGenerator::visit(std::shared_ptr<AstParameterList> node){
    emit("(");
    for (auto const& parameter : node->parameters) {
        clearTypeInformation();
        parameter->semanticType->accept(this);
        emit(getCTypeFromPreviousNode(parameter->name));

        if (parameter != node->parameters.back()) {
            emit(", ");
        }
    }
    emit(")");
}

// program items
void CCodeGenerator::visit(std::shared_ptr<AstFunctionDefinition> node){
    current_source = &source_declarations;

    if(std::find(node->tags->tags.begin(), node->tags->tags.end(), AstTags::Value::Extern) != node->tags->tags.end()){
        emit("extern ");
    }
    
    clearTypeInformation();
    node->semanticType->accept(this);
    auto returnType = getCTypeFromPreviousNode("");
    emit(returnType + " " + node->name);
    node->parameters->accept(this);
    emit(";\n");

    current_source = &source_definitions;
    if(std::find(node->tags->tags.begin(), node->tags->tags.end(), AstTags::Value::Extern) == node->tags->tags.end()){
        emit(returnType + " " + node->name);
        node->parameters->accept(this);

        node->body->accept(this);
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

    if (node->expression->getType() == AstNodeType::AstArrayLiteral){
        emit("((");
        clearTypeInformation();
        node->expression->semanticType->accept(this);
        emit(getCTypeFromPreviousNode(""));
        emit(")");
        node->expression->accept(this);
        emit(")");
    }
    else{
        node->expression->accept(this);
    }

    emit(";");
}
void CCodeGenerator::visit(std::shared_ptr<AstConditionalStatement> node){
    emitIndented("if (");
    node->condition->accept(this);
    emit(") {");
    blockNextIndentedEmit();
    node->trueStatement->accept(this);
    emitIndented("}");
    if (node->falseStatement){
        emit("\n");
        emitIndented("else {");
        blockNextIndentedEmit();
        node->falseStatement->accept(this);
        emitIndented("}");
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

// Expressions
void CCodeGenerator::visit(std::shared_ptr<AstInteger> node){
    emit(std::to_string(node->value));
}
void CCodeGenerator::visit(std::shared_ptr<AstArrayLiteral> node) {
    node->semanticType->accept(this);
    emit("{");
    bool isFirstIteration = true;
    for (auto element : node->elements){
        if (!isFirstIteration) emit(",");
        element->accept(this);
        isFirstIteration = false;
    }
    emit("}");
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
void CCodeGenerator::visit(std::shared_ptr<AstAssignment> node){
    emit("(");
    node->lvalue->accept(this);
    emit(" = ");
    node->rvalue->accept(this);
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
void CCodeGenerator::visit(std::shared_ptr<AstAddressOf> node) {
    emit("(&");
    node->operand->accept(this);
    emit(")");
}


// Declarations
void CCodeGenerator::visit(std::shared_ptr<AstVariableDeclaration> node){
    clearTypeInformation();
    node->semanticType->accept(this);
    emitIndented(getCTypeFromPreviousNode(node->name));
    if (node->value){
        emit(" = ");
        node->value->accept(this);
    }
    else{
        if (!node->variable->isGlobal){
            emit("; zero_memory_r_sharp_internal(&" + node->name + ", " + std::to_string(sizeFromSemanticalType(node->semanticType)) + ")");
        }
        else{
            // the varible will live in the BSS section that is zero from the beginning
        }
    }
    emit(";");
}

void CCodeGenerator::visit(std::shared_ptr<AstVariableAccess> node){
    emit(node->name);
}
void CCodeGenerator::visit(std::shared_ptr<AstDereference> node){
    emit("*");
    node->operand->accept(this);
}
void CCodeGenerator::visit(std::shared_ptr<AstArrayAccess> node){
    if (node->array->getType() == AstNodeType::AstArrayLiteral){
        emit("((");
        clearTypeInformation();
        node->array->semanticType->accept(this);
        emit(getCTypeFromPreviousNode(""));
        emit(")");
        node->array->accept(this);
        emit(")[");
        node->index->accept(this);
        emit("]");
    } 
    else{
        emit("(");
        node->array->accept(this);
        emit(")[");
        node->index->accept(this);
        emit("]");
    }
}
void CCodeGenerator::visit(std::shared_ptr<AstTypeConversion> node){
    emit("((");
    clearTypeInformation();
    node->semanticType->accept(this);
    emit(getCTypeFromPreviousNode(""));
    emit(")(");
    node->value->accept(this);
    emit("))");
}


void CCodeGenerator::visit(std::shared_ptr<AstPrimitiveType> node){
    switch (node->type){
        case RSharpPrimitiveType::I8: leftTypeInformation += "int8_t"; break;
        case RSharpPrimitiveType::I16: leftTypeInformation += "int16_t"; break;
        case RSharpPrimitiveType::I32: leftTypeInformation += "int32_t"; break;
        case RSharpPrimitiveType::I64: leftTypeInformation += "int64_t"; break;
        case RSharpPrimitiveType::C_void: leftTypeInformation += "void"; break;

        default:
            Fatal("Unimplemented type Nr.", static_cast<int>(node->type));
            break;
    }
}
void CCodeGenerator::visit(std::shared_ptr<AstPointerType> node){
    node->subtype->accept(this);
    if (middleTypeInformation.length() != 0){
        middleTypeInformation =+ "*";
        return;
    }

    if (rightTypeInformation.length() != 0){
        middleTypeInformation =+ "*";
        return;
    }

    leftTypeInformation += "*";
}

void CCodeGenerator::visit(std::shared_ptr<AstArrayType> node) {
    node->subtype->accept(this);
    
    std::string insertedTypeInfo = "";
    insertedTypeInfo += "[";
    if (node->size.has_value()){
        insertedTypeInfo += std::to_string(std::dynamic_pointer_cast<AstInteger>(node->size.value())->value);
    }
    insertedTypeInfo += "]";

    rightTypeInformation = insertedTypeInfo + rightTypeInformation;
}
