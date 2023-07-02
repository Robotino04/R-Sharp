#include "R-Sharp/NASMCodeGenerator.hpp"
#include "R-Sharp/Logging.hpp"
#include "R-Sharp/AstNodes.hpp"
#include "R-Sharp/Utils.hpp"
#include "R-Sharp/VariableSizeInserter.hpp"

#include <sstream>
#include <map>
#include <tuple>
#include <math.h>

NASMCodeGenerator::NASMCodeGenerator(std::shared_ptr<AstProgram> root, std::string R_SharpSource){
    this->root = root;
    this->R_SharpSource = R_SharpSource;
}

void NASMCodeGenerator::indent(){
    indentLevel++;
}
void NASMCodeGenerator::dedent(){
    if (!indentLevel){
        Fatal("Indentation error");
        return;
    }
    indentLevel--;
}
void NASMCodeGenerator::emit(std::string const& str, NASMCodeGenerator::BinarySection section){
    switch (section){
        case NASMCodeGenerator::BinarySection::Text:
            source_text += str;
            break;
        case NASMCodeGenerator::BinarySection::BSS:
            source_bss += str;
            break;
        case NASMCodeGenerator::BinarySection::Data:
            source_data += str;
            break;
    }
}
void NASMCodeGenerator::emitIndented(std::string const& str, NASMCodeGenerator::BinarySection section){
    for (int i=0;i<indentLevel;i++){
        source_text += "    ";
    }
    switch (section){
        case NASMCodeGenerator::BinarySection::Text:
            source_text += str;
            break;
        case NASMCodeGenerator::BinarySection::BSS:
            source_bss += str;
            break;
        case NASMCodeGenerator::BinarySection::Data:
            source_data += str;
            break;
    }
}

int NASMCodeGenerator::sizeFromSemanticalType(std::shared_ptr<AstType> type){
    static const std::map<RSharpPrimitiveType, int> primitive_sizes = {
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
        default: throw std::runtime_error("Unimplemented type used");
    }
}

std::string NASMCodeGenerator::generate(){
    source_text = "";
    source_data = "";
    source_bss = "";
    indentLevel = 0;
    
    VariableSizeInserter sizeInserter(root);
    sizeInserter.insert(NASMCodeGenerator::sizeFromSemanticalType);

    root->accept(this);

    std::string output = "";
    output += "; NASM code generated by R-Sharp compiler";
    output += "\n\nBITS 64\n";
    output += "section .text\n";
    output += source_text;
    output += "\nsection .data\n";
    output += source_data;
    output += "\nsection .bss\n";
    output += source_bss;



    return output;
}

void NASMCodeGenerator::emitSyscall(Syscall callNr, std::string const& arg1, std::string const& arg2, std::string const& arg3, std::string const& arg4, std::string const& arg5, std::string const& arg6){
    // move the arguments to rdi, rsi, rdx, r10, r8, and r9 respectively
    emitIndented("; Syscall " + syscallToString(callNr) + "(" + std::to_string(static_cast<int>(callNr)) + ")\n");
    if (arg1 != "") emitIndented("mov rdi, " + arg1 + "\n");
    if (arg2 != "") emitIndented("mov rsi, " + arg2 + "\n");
    if (arg3 != "") emitIndented("mov rdx, " + arg3 + "\n");
    if (arg4 != "") emitIndented("mov r10, " + arg4 + "\n");
    if (arg5 != "") emitIndented("mov r8, " + arg5 + "\n");
    if (arg6 != "") emitIndented("mov r9, " + arg6 + "\n");

    emitIndented("mov rax, " + std::to_string(static_cast<int>(callNr)) + "\n");

    emitIndented("syscall\n");
}

std::string NASMCodeGenerator::getUniqueLabel(std::string const& prefix){
    static uint64_t labelCounter = 0;
    return prefix + "_" + std::to_string(labelCounter++);
}
void NASMCodeGenerator::generateFunctionProlouge(){
    emitIndented("; Create stack frame\n");

    // save callee-saved registers
    emitIndented("push rbx\n");
    emitIndented("push rbp\n");
    emitIndented("push r12\n");
    emitIndented("push r13\n");
    emitIndented("push r14\n");
    emitIndented("push r15\n");
    emitIndented("mov rbp, rsp\n");
}

void NASMCodeGenerator::generateFunctionEpilouge(){
    emitIndented("; Destroy stack frame\n");
    
    // restore callee-saved registers
    emitIndented("pop r15\n");
    emitIndented("pop r14\n");
    emitIndented("pop r13\n");
    emitIndented("pop r12\n");
    emitIndented("pop rbp\n");
    emitIndented("pop rbx\n");
}
void NASMCodeGenerator::resetStackPointer(std::shared_ptr<AstBlock> scope){
    emitIndented("; Restore stack pointer to before this scope (" + scope->name + ")\n");
    emitIndented("add rsp, " + std::to_string(scope->sizeOfLocalVariables) + "\n");
}

std::string NASMCodeGenerator::sizeToNASMType(int size){
    if (size == 8) return "qword";
    else if (size == 4) return "dword";
    else if (size == 2) return "word";
    else if (size == 1) return "byte";
    else{
        Fatal("Invalid size ", size);
        return "";
    }
}

std::string NASMCodeGenerator::getRegisterWithSize(std::string reg, int size){
    const std::map<std::pair<std::string, int>, std::string> map = {
        {{"rax", 1}, "al"},
        {{"rax", 2}, "ax"},
        {{"rax", 4}, "eax"},
        {{"rax", 8}, "rax"},

        {{"rbx", 1}, "bl"},
        {{"rbx", 2}, "bx"},
        {{"rbx", 4}, "ebx"},
        {{"rbx", 8}, "rbx"},

        {{"rcx", 1}, "cl"},
        {{"rcx", 2}, "cx"},
        {{"rcx", 4}, "ecx"},
        {{"rcx", 8}, "rcx"},

        {{"rdx", 1}, "dl"},
        {{"rdx", 2}, "dx"},
        {{"rdx", 4}, "edx"},
        {{"rdx", 8}, "rdx"},
    };

    return map.at({reg, size});
}

// program
void NASMCodeGenerator::visit(std::shared_ptr<AstProgram> node){
    node->globalScope->accept(this);
    
    for (auto var : node->globalScope->variables){
        var->accessor = getUniqueLabel(var->name);
    }

    for (auto const& child : node->getChildren()){
        if (!child) continue;
        if (child->getType() == AstNodeType::AstFunctionDefinition){
            child->accept(this);
        }
        else if (child->getType() == AstNodeType::AstVariableDeclaration){
            child->accept(this);
        }
        else{
            Fatal("Invalid node type in program");
        }
    }

    // uninitialized global variables
    for (auto var : node->uninitializedGlobalVariables){
        emit(std::get<std::string>(var->accessor) + ":\n", BinarySection::BSS);
        emit("    resb " + std::to_string(var->sizeInBytes) + "\n", BinarySection::BSS);
    }
}
void NASMCodeGenerator::visit(std::shared_ptr<AstParameterList> node){
    // generate access strings
    node->parameterBlock->accept(this);

    int argumentNumber = 0;
    for (auto  child : node->parameters){
        child->accept(this);

        emitIndented("; Load argument " + std::to_string(argumentNumber) + "\n");
        emitIndented("mov " + sizeToNASMType(child->variable->sizeInBytes) + " [rbp -" + std::to_string(std::get<int>(child->variable->accessor)) + "], ");

        switch(argumentNumber){
            case 0: emit("rdi\n"); break;
            case 1: emit("rsi\n"); break;
            case 2: emit("rdx\n"); break;
            case 3: emit("rcx\n"); break;
            case 4: emit("r8\n"); break;
            case 5: emit("r9\n"); break;
        }

        argumentNumber++;
    }
}

// definitions
void NASMCodeGenerator::visit(std::shared_ptr<AstFunctionDefinition> node){
    if(std::find(node->tags->tags.begin(), node->tags->tags.end(), AstTags::Value::Extern) == node->tags->tags.end()){
        emitIndented("; Function " + node->name + "\n\n");
        emitIndented("global " + node->functionData->name + "\n");
        emitIndented(node->functionData->name + ":\n");
        indent();
        generateFunctionProlouge();

        node->parameters->accept(this);
        node->body->accept(this);

        emitIndented("; fallback if the function has no return\n");
        generateFunctionEpilouge();
        emitIndented("mov rax, 0\n");
        emitIndented("ret\n");
        dedent();
    }
    else{
        emitIndented("extern " + node->functionData->name + "\n");
    }
}

// statements
void NASMCodeGenerator::visit(std::shared_ptr<AstBlock> node){
    emitIndented("; Block begin (" + node->name + ")\n");
    indent();

    for (auto child : node->getChildren()){
        if (child) child->accept(this);
    }

    // don't change stack pointer if it wasn't modified
    if (!node->isMerged) resetStackPointer(node);
    dedent();
    emitIndented("; Block end (" + node->name + ")\n");
}
void NASMCodeGenerator::visit(std::shared_ptr<AstReturn> node){
    node->value->accept(this);
    for (auto scope = node->containedScopes.rbegin(); scope != node->containedScopes.rend(); ++scope){
        if (scope->expired()){
            Fatal("INTERNAL ERROR: std::weak_ptr expired during code generation.");
        }
        else{
            resetStackPointer(scope->lock());
        }
    }
    generateFunctionEpilouge();
    emitIndented("ret\n");
}
void NASMCodeGenerator::visit(std::shared_ptr<AstConditionalStatement> node){
    std::string else_label = "." + getUniqueLabel("else");
    std::string end_label = "." + getUniqueLabel("end");

    node->condition->accept(this);
    emitIndented("; If statement\n");
    emitIndented("cmp rax, 0\n");
    emitIndented("je " + else_label + "\n");
    indent();
    node->trueStatement->accept(this);
    emitIndented("jmp " + end_label + "\n");
    dedent();
    emitIndented(else_label + ":\n");
    indent();
    if (node->falseStatement) node->falseStatement->accept(this);
    dedent();
    emitIndented(end_label + ":\n");
}
void NASMCodeGenerator::visit(std::shared_ptr<AstForLoopDeclaration> node){
    std::string start_label = "." + getUniqueLabel("start");
    std::string end_label = "." + getUniqueLabel("end");
    std::string increment_label = "." + getUniqueLabel("increment");

    node->loop->skipAccessString = increment_label;
    node->loop->breakAccessString = end_label;

    emitIndented("; For loop\n");
    emitIndented("; For loop initialization\n");
    node->initialization->accept(this);

    emitIndented(start_label + ":\n");
    indent();
    emitIndented("; For loop condition\n");
    node->condition->accept(this);
    emitIndented("cmp rax, 0\n");
    emitIndented("je " + end_label + "\n");

    emitIndented("; For loop body\n");
    node->body->accept(this);
    dedent();
    emitIndented("; For loop increment\n");
    emitIndented(increment_label + ":\n");
    indent();
    node->increment->accept(this);
    emitIndented("jmp " + start_label + "\n");
    dedent();
    emitIndented(end_label + ":\n");

    // manually restore the stack pointer
    resetStackPointer(node->initializationContext);
    emitIndented("; For loop end\n");
}
void NASMCodeGenerator::visit(std::shared_ptr<AstForLoopExpression> node){
    std::string start_label = "." + getUniqueLabel("start");
    std::string end_label = "." + getUniqueLabel("end");
    std::string increment_label = "." + getUniqueLabel("increment");

    node->loop->skipAccessString = increment_label;
    node->loop->breakAccessString = end_label;

    emitIndented("; For loop\n");
    emitIndented("; For loop initialization\n");
    node->variable->accept(this);

    emitIndented("; For loop\n");
    emitIndented(start_label + ":\n");
    indent();
    emitIndented("; For loop condition\n");
    node->condition->accept(this);
    emitIndented("cmp rax, 0\n");
    emitIndented("je " + end_label + "\n");

    emitIndented("; For loop body\n");
    node->body->accept(this);
    dedent();
    emitIndented("; For loop increment\n");
    emitIndented(increment_label + ":\n");
    indent();
    node->increment->accept(this);
    emitIndented("jmp " + start_label + "\n");

    dedent();
    emitIndented(end_label + ":\n");
    emitIndented("; For loop end\n");
}
void NASMCodeGenerator::visit(std::shared_ptr<AstWhileLoop> node){
    std::string start_label = "." + getUniqueLabel("start");
    std::string end_label = "." + getUniqueLabel("end");

    node->loop->skipAccessString = start_label;
    node->loop->breakAccessString = end_label;

    emitIndented("; While loop\n");
    emitIndented(start_label + ":\n");
    indent();
    node->condition->accept(this);
    emitIndented("cmp rax, 0\n");
    emitIndented("je " + end_label + "\n");
    emitIndented("; Body\n");
    node->body->accept(this);
    emitIndented("jmp " + start_label + "\n");
    dedent();
    emitIndented(end_label + ":\n");
}
void NASMCodeGenerator::visit(std::shared_ptr<AstDoWhileLoop> node){
    std::string start_label = "." + getUniqueLabel("start");
    std::string end_label = "." + getUniqueLabel("end");

    node->loop->skipAccessString = start_label;
    node->loop->breakAccessString = end_label;

    emitIndented("; Do loop\n");
    emitIndented(start_label + ":\n");
    indent();
    emitIndented("; Body\n");
    node->body->accept(this);
    node->condition->accept(this);
    emitIndented("cmp rax, 0\n");
    emitIndented("jne " + start_label + "\n");
    dedent();
    emitIndented(end_label + ":\n");

}
void NASMCodeGenerator::visit(std::shared_ptr<AstBreak> node){
    for (auto varScope = node->containedScopes.rbegin(); varScope != node->containedScopes.rend(); ++varScope){
        if (varScope->expired()){
            Fatal("INTERNAL ERROR: std::weak_ptr expired during code generation.");
        }
        else{
            resetStackPointer(varScope->lock());
        }
    }

    emitIndented("; Break\n");
    emitIndented("jmp " + node->loop->breakAccessString + "\n");
}
void NASMCodeGenerator::visit(std::shared_ptr<AstSkip> node){
    for (auto varScope = node->containedScopes.rbegin(); varScope != node->containedScopes.rend(); ++varScope){
        if (varScope->expired()){
            Fatal("INTERNAL ERROR: std::weak_ptr expired during code generation.");
        }
        else{
            resetStackPointer(varScope->lock());
        }
    }

    emitIndented("; Skip\n");
    emitIndented("jmp " + node->loop->skipAccessString + "\n");
}


// expressions
void NASMCodeGenerator::visit(std::shared_ptr<AstUnary> node){
    node->value->accept(this);
    switch (node->type){
        case AstUnaryType::Negate:
            emitIndented("neg rax\n");
            break;
        case AstUnaryType::BinaryNot:
            emitIndented("not rax\n");
            break;
        case AstUnaryType::LogicalNot:
            emitIndented("cmp rax, 0\n");
            emitIndented("mov rax, 0\n");
            emitIndented("sete al\n");
            break;
        default:
            Error("NASM Generator: Unary operator not implemented!");
            printErrorToken(node->token, R_SharpSource);
            exit(1);
            break;
    }
}
void NASMCodeGenerator::visit(std::shared_ptr<AstBinary> node){
    node->left->accept(this);

    // logical and and or will short circuit, so the right side is not evaluated until necessary
    if (!(node->type == AstBinaryType::LogicalAnd || node->type == AstBinaryType::LogicalOr)){
        emitIndented("push rax\n");
        node->right->accept(this);
        emitIndented("mov rbx, rax\n");
        emitIndented("pop rax\n");
    }
    switch (node->type){
        case AstBinaryType::Add:
            emitIndented("; Add\n");
            emitIndented("add rax, rbx\n");
            break;
        case AstBinaryType::Subtract:
            emitIndented("; Subtract\n");
            emitIndented("sub rax, rbx\n");
            break;
        case AstBinaryType::Multiply:
            emitIndented("; Multiply\n");
            emitIndented("imul rax, rbx\n");
            break;
        case AstBinaryType::Divide:
            emitIndented("; Divide\n");
            if (sizeFromSemanticalType(node->left->semanticType) == 1)
                emitIndented("cbw    ; sign extend from 8-bit to 16-bit\n");
            if (sizeFromSemanticalType(node->left->semanticType) == 2)
                emitIndented("cwd    ; sign extend from 16-bit to 32-bit\n");
            if (sizeFromSemanticalType(node->left->semanticType) == 4)
                emitIndented("cdq    ; sign extend from 32-bit to 64-bit\n");
            if (sizeFromSemanticalType(node->left->semanticType) == 8)
                emitIndented("cqo    ; sign extend from 64-bit to 128-bit\n");

            emitIndented("idiv " + getRegisterWithSize("rbx", sizeFromSemanticalType(node->left->semanticType)) + "\n");
            break;
        case AstBinaryType::Modulo:
            emitIndented("; Modulo\n");
            emitIndented("cqo\n");
            emitIndented("idiv rbx\n");
            emitIndented("mov rax, rdx\n");
            break;

        case AstBinaryType::Equal:
            emitIndented("; Equal\n");
            emitIndented("cmp rax, rbx\n");
            emitIndented("mov rax, 0\n");
            emitIndented("sete al\n");
            break;
        case AstBinaryType::NotEqual:
            emitIndented("; Not Equal\n");
            emitIndented("cmp rax, rbx\n");
            emitIndented("mov rax, 0\n");
            emitIndented("setne al\n");
            break;
        case AstBinaryType::LessThan:
            emitIndented("; Less Than\n");
            emitIndented("cmp rax, rbx\n");
            emitIndented("mov rax, 0\n");
            emitIndented("setl al\n");
            break;
        case AstBinaryType::LessThanOrEqual:
            emitIndented("; Less Than Or Equal\n");
            emitIndented("cmp rax, rbx\n");
            emitIndented("mov rax, 0\n");
            emitIndented("setle al\n");
            break;
        case AstBinaryType::GreaterThan:
            emitIndented("; Greater Than\n");
            emitIndented("cmp rax, rbx\n");
            emitIndented("mov rax, 0\n");
            emitIndented("setg al\n");
            break;
        case AstBinaryType::GreaterThanOrEqual:
            emitIndented("; Greater Than Or Equal\n");
            emitIndented("cmp rax, rbx\n");
            emitIndented("mov rax, 0\n");
            emitIndented("setge al\n");
            break;

        case AstBinaryType::LogicalAnd:{
            emitIndented("; Logical And\n");
            std::string clause2 = "." + getUniqueLabel("second_expression");
            std::string end = "." + getUniqueLabel("end");
            emitIndented("cmp rax, 0\n");
            emitIndented("jne " + clause2 + "\n");
            emitIndented("jmp " + end + "\n");
            emitIndented(clause2 + ":\n"); indent();

            // evaluate right side
            node->right->accept(this);
            emitIndented("cmp rax, 0\n");
            emitIndented("mov rax, 0\n");
            emitIndented("setne al\n");
            dedent();
            emitIndented(end + ":\n");
            break;
        }

        case AstBinaryType::LogicalOr:{
            emitIndented("; Logical Or\n");
            std::string clause2 = "." + getUniqueLabel("second_expression");
            std::string end = "." + getUniqueLabel("end");
            emitIndented("cmp rax, 0\n");
            emitIndented("je " + clause2 + "\n");
            emitIndented("mov rax, 1\n");
            emitIndented("jmp " + end + "\n");
            emitIndented(clause2 + ":\n"); indent();

            // evaluate right side
            node->right->accept(this);
            emitIndented("cmp rax, 0\n");
            emitIndented("mov rax, 0\n");
            emitIndented("setne al\n");
            dedent();
            emitIndented(end + ":\n");
            break;
        }
        default:
            Error("NASM Generator: Binary operator not implemented!");
            printErrorToken(node->token, R_SharpSource);
            exit(1);
            break;
    }
}
void NASMCodeGenerator::visit(std::shared_ptr<AstInteger> node){
    emitIndented("; Integer " + std::to_string(node->value) + "\n");
    emitIndented("mov rax, " + std::to_string(node->value) + "\n");
}
void NASMCodeGenerator::visit(std::shared_ptr<AstAssignment> node){
    node->rvalue->accept(this);
    if (node->lvalue->getType() == AstNodeType::AstVariableAccess){
        auto var = std::static_pointer_cast<AstVariableAccess>(node->lvalue);
        emitIndented("; Variable assignment (" + var->name + ")\n");
        if (var->variable->isGlobal){
            emitIndented("mov " + sizeToNASMType(var->variable->sizeInBytes) + " [" + std::get<std::string>(var->variable->accessor) + "], " + getRegisterWithSize("rax", var->variable->sizeInBytes) + "\n");
        }
        else{
            emitIndented("mov " + sizeToNASMType(var->variable->sizeInBytes) + " [rbp - " + std::to_string(std::get<int>(var->variable->accessor)) + "], " + getRegisterWithSize("rax", var->variable->sizeInBytes) + "\n");
        }
    }
    else if(node->lvalue->getType() == AstNodeType::AstDereference){
        auto deref = std::static_pointer_cast<AstDereference>(node->lvalue);
        auto size = sizeFromSemanticalType(deref->semanticType);
        emitIndented("; Dereference Assignment\n");
        emitIndented("push rax\n");

        // put the address to store to into rax
        deref->operand->accept(this);

        emitIndented("mov rbx, rax\n");
        emitIndented("pop rax\n");
        emitIndented("mov " + sizeToNASMType(size) + " [rbx], " + getRegisterWithSize("rax", size) + "\n");
    }
    else{
        Error("Unimplemented type of lvalue.");
        printErrorToken(node->lvalue->token, R_SharpSource);
        exit(1);
    }
}
void NASMCodeGenerator::visit(std::shared_ptr<AstConditionalExpression> node){
    std::string true_clause = "." + getUniqueLabel("true_expression");
    std::string false_clause = "." + getUniqueLabel("false_expression");
    std::string end = "." + getUniqueLabel("end");
    node->condition->accept(this);
    emitIndented("; Conditional Expression\n");
    emitIndented("cmp rax, 0\n");
    emitIndented("je " + false_clause + "\n");
    emitIndented(true_clause + ":\n"); indent();
    node->trueExpression->accept(this);
    emitIndented("jmp " + end + "\n");
    dedent();
    emitIndented(false_clause + ":\n"); indent();
    node->falseExpression->accept(this);
    dedent();
    emitIndented(end + ":\n");
}
void NASMCodeGenerator::visit(std::shared_ptr<AstEmptyExpression> node){
    emitIndented("; Empty Expression\n");
    emitIndented("mov rax, 1\n");
}
void NASMCodeGenerator::visit(std::shared_ptr<AstFunctionCall> node){
    // rdi, rsi, rdx, rcx, r8, and r9 are used for parameters
    // more than 6 parameters are not supported yet

    if (node->arguments.size() > 6){
        Error("NASM Generator: More than 6 parameters are not supported yet!");
        printErrorToken(node->token, R_SharpSource);
        exit(1);
    }
    // save registers
    emitIndented("; Prepare for function call (" + node->name + ")\n");
    int argCount = node->arguments.size();
    for (int i = 0; i < argCount; i++){
        switch(i){
            case 0: emitIndented("push rdi\n"); break;
            case 1: emitIndented("push rsi\n"); break;
            case 2: emitIndented("push rdx\n"); break;
            case 3: emitIndented("push rcx\n"); break;
            case 4: emitIndented("push r8\n"); break;
            case 5: emitIndented("push r9\n"); break;
        }
    }

    // evaluate arguments
    for (auto arg : node->arguments){
        arg->accept(this);
        emitIndented("push rax\n");
    }
    // move arguments to registers
    for (int i = argCount - 1; i >= 0; i--){
        switch(i){
            case 0: emitIndented("pop rdi\n"); break;
            case 1: emitIndented("pop rsi\n"); break;
            case 2: emitIndented("pop rdx\n"); break;
            case 3: emitIndented("pop rcx\n"); break;
            case 4: emitIndented("pop r8\n"); break;
            case 5: emitIndented("pop r9\n"); break;
        }
    }

    emitIndented("; Function Call (" + node->name + ")\n");
    emitIndented("call " + node->function->name + "\n");

    emitIndented("; Restore after function call (" + node->name + ")\n");
    // restore registers
    for (int i = argCount - 1; i >= 0; i--){
        switch(i){
            case 0: emitIndented("pop rdi\n"); break;
            case 1: emitIndented("pop rsi\n"); break;
            case 2: emitIndented("pop rdx\n"); break;
            case 3: emitIndented("pop rcx\n"); break;
            case 4: emitIndented("pop r8\n"); break;
            case 5: emitIndented("pop r9\n"); break;
        }
    }
}
void NASMCodeGenerator::visit(std::shared_ptr<AstAddressOf> node){
    if (node->operand->variable->isGlobal)
        emitIndented("lea rax, [" + std::get<std::string>(node->operand->variable->accessor) + "]\n");
    else
        emitIndented("lea rax, [rbp - " + std::to_string(std::get<int>(node->operand->variable->accessor)) + "]\n");
}
void NASMCodeGenerator::visit(std::shared_ptr<AstTypeConversion> node){
    int originSize = sizeFromSemanticalType(node->value->semanticType);
    int targetSize = sizeFromSemanticalType(node->semanticType);
    node->value->accept(this);
    if (targetSize > originSize){
        emitIndented("; Convert from " + std::to_string(originSize) + " bytes to " + std::to_string(targetSize) + " bytes.\n");
        emitIndented("movsx ");
        emit(getRegisterWithSize("rax", targetSize) + ", ");
        emit(getRegisterWithSize("rax", originSize) + "\n");
    }
    else if (targetSize == originSize);
    else{
        emitIndented("; explicit and to detect invalid upcasts later (" + std::to_string(originSize) + " Bytes --> " + std::to_string(targetSize) + " Bytes)\n");
        emitIndented("mov rbx, " + std::to_string(uint64_t((__uint128_t(1) << __uint128_t(targetSize*8))-1)) + "\n");
        emitIndented("and rax, rbx\n");
    }
}


void NASMCodeGenerator::visit(std::shared_ptr<AstVariableAccess> node){
    auto size = NASMCodeGenerator::sizeFromSemanticalType(node->semanticType);
    emitIndented("; Variable Access(" + node->name + ")\n");
    if (node->variable->sizeInBytes != 8 && node->variable->sizeInBytes != 4){
        // non 32-bit and 64-bit operations don't clear the remaining bits
        emitIndented("xor eax, eax\n");
    }
    if (node->variable->isGlobal)
        emitIndented("mov " + sizeToNASMType(size) + " " + getRegisterWithSize("rax", size) + ", [" + std::get<std::string>(node->variable->accessor) + "]\n");
    else
        emitIndented("mov " + sizeToNASMType(size) + " " + getRegisterWithSize("rax", size) + ", [rbp - " + std::to_string(std::get<int>(node->variable->accessor)) + "]\n");
}
void NASMCodeGenerator::visit(std::shared_ptr<AstDereference> node){
    node->operand->accept(this);
    auto size = NASMCodeGenerator::sizeFromSemanticalType(node->semanticType);
    emitIndented("; Dereference\n");
    emitIndented("mov " + sizeToNASMType(size) + " " + getRegisterWithSize("rax", size) + ", [rax]\n");
}


// declarations
void NASMCodeGenerator::visit(std::shared_ptr<AstVariableDeclaration> node){
    if (node->variable->isGlobal){
        if (!node->value){
            return;
        }
        if (node->value->getType() != AstNodeType::AstInteger){
            Error("NASM Generator: Global variable must be of type integer!");
            printErrorToken(node->token, R_SharpSource);
            exit(1);
        }
        auto intNode = std::dynamic_pointer_cast<AstInteger>(node->value);
        emit("    ; Global Variable (" + node->name + ")\n", BinarySection::Data);
        emit("    global " + std::get<std::string>(node->variable->accessor) + "\n", BinarySection::Data);
        emit("    " + std::get<std::string>(node->variable->accessor) + ": ", BinarySection::Data);
        switch(node->variable->sizeInBytes){
            case 1: emit("db " + std::to_string(intNode->value) + "\n", BinarySection::Data); break;
            case 2: emit("dw " + std::to_string(intNode->value) + "\n", BinarySection::Data); break;
            case 4: emit("dd " + std::to_string(intNode->value) + "\n", BinarySection::Data); break;
            case 8: emit("dq " + std::to_string(intNode->value) + "\n", BinarySection::Data); break;
            default:
                Error("NASM Generator: Global variable size not supported!");
                printErrorToken(node->token, R_SharpSource);
                exit(1);
                break;
        }
    }
    else{
        emitIndented("; Variable (" + node->name + ")\n");
        if (node->value){
            node->value->accept(this);
            emitIndented("sub rsp, " + std::to_string(node->variable->sizeInBytes) + "\n");
            emitIndented("mov [rbp - " + std::to_string(std::get<int>(node->variable->accessor)) + "], " + getRegisterWithSize("rax", node->variable->sizeInBytes) + "\n");
        }
        else{
            emitIndented("sub rsp, " + std::to_string(node->variable->sizeInBytes) + "\n");
            emitIndented("mov " + sizeToNASMType(node->variable->sizeInBytes) + " [rbp - " + std::to_string(std::get<int>(node->variable->accessor)) + "], 0\n");
        }
    }
}