#include "R-Sharp/NASMCodeGenerator.hpp"
#include "R-Sharp/Logging.hpp"
#include "R-Sharp/AstNodes.hpp"
#include "R-Sharp/Utils.hpp"

#include <sstream>

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

std::string NASMCodeGenerator::generate(){
    source_text = "";
    source_data = "";
    source_bss = "";
    indentLevel = 0;
    root->accept(this);

    // collect the uninitialized global variables
    for (auto var : root->globalScope->variables){
        if (!var->isDefined){
            emit(var->accessStr.substr(1, var->accessStr.size()-2) + ":\n", BinarySection::BSS);
            switch(var->sizeInBytes){
                case 1: emit("    resb 1\n", BinarySection::BSS); break;
                case 2: emit("    resw 1\n", BinarySection::BSS); break;
                case 4: emit("    resd 1\n", BinarySection::BSS); break;
                case 8: emit("    resq 1\n", BinarySection::BSS); break;
                default: Fatal("Unsupported variable size (" + std::to_string(var->sizeInBytes) + "b)");
            }
        }
    }

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

    stackOffset = 0;
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

// program
void NASMCodeGenerator::visit(std::shared_ptr<AstProgram> node){
    node->globalScope->accept(this);

    for (auto const& child : node->getChildren()){
        if (!child) continue;
        if (child->getType() == AstNodeType::AstFunctionDeclaration){
            child->accept(this);
        }
        else if (child->getType() == AstNodeType::AstVariableDeclaration){
            child->accept(this);
        }
        else{
            Fatal("Invalid node type in program");
        }
    }

    // emit extern functions
    for (auto func : node->items){
        if (func->getType() == AstNodeType::AstFunctionDeclaration){
            if (!std::dynamic_pointer_cast<AstFunctionDeclaration>(func)->function->isDefined)
                emitIndented("extern " + std::dynamic_pointer_cast<AstFunctionDeclaration>(func)->function->accessString + "\n");
        }
    }

    // uninitialized global variables
    for (auto var : node->uninitializedGlobalVariables){
        emit(var->accessStr.substr(1, var->accessStr.size()-2) + ":\n", BinarySection::BSS);
        emit("    resb" + std::to_string(var->sizeInBytes) + "\n", BinarySection::BSS);
    }
}
void NASMCodeGenerator::visit(std::shared_ptr<AstParameterList> node){
    node->parameterBlock->accept(this);

    int argumentNumber = 0;
    for (auto  child : node->parameters){
        child->accept(this);

        emitIndented("; Load argument " + std::to_string(argumentNumber) + "\n");
        emitIndented("mov " + sizeToNASMType(child->variable->sizeInBytes) + " " + child->variable->accessStr + ", ");

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
void NASMCodeGenerator::visit(std::shared_ptr<AstFunctionDeclaration> node){
    node->function->accessString = node->function->name;
    if (node->body){
        emitIndented("; Function " + node->name + "\n\n");
        emitIndented("global " + node->function->accessString + "\n");
        emitIndented(node->function->accessString + ":\n");
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
}

// statements
void NASMCodeGenerator::visit(std::shared_ptr<AstBlock> node){
    emitIndented("; Block begin (" + node->name + ")\n");
    indent();
    int scopeSize = 0;
    for (auto var : node->variables){

        if (var->isGlobal){
            var->accessStr = "[" + getUniqueLabel(var->name) +"]";
        }
        else{
            stackOffset += var->sizeInBytes;
            scopeSize += var->sizeInBytes;
            var->accessStr = "[rbp - " + std::to_string(stackOffset) +"]";
        }
    }

    for (auto const& child : node->getChildren()){
        if (child) child->accept(this);
    }
    // don't change stack pointer if it wasn't modified
    if (scopeSize) stackOffset -= scopeSize;
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
    std::string else_label = getUniqueLabel("else");
    std::string end_label = getUniqueLabel("end");

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
    std::string start_label = getUniqueLabel("start");
    std::string end_label = getUniqueLabel("end");
    std::string increment_label = getUniqueLabel("increment");

    node->loop->skipAccessString = increment_label;
    node->loop->breakAccessString = end_label;

    // manually generate the access string for the counter variable
    stackOffset += node->initialization->variable->sizeInBytes;
    node->initialization->variable->accessStr = "[rbp - " + std::to_string(stackOffset) +"]";

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
    std::string start_label = getUniqueLabel("start");
    std::string end_label = getUniqueLabel("end");
    std::string increment_label = getUniqueLabel("increment");

    node->loop->skipAccessString = increment_label;
    node->loop->breakAccessString = end_label;

    emitIndented("; For loop\n");
    emitIndented("; Initialization\n");
    node->variable->accept(this);

    emitIndented("; For loop\n");
    emitIndented(start_label + ":\n");
    indent();
    emitIndented("; Condition\n");
    node->condition->accept(this);
    emitIndented("cmp rax, 0\n");
    emitIndented("je " + end_label + "\n");
    emitIndented("; Body\n");
    node->body->accept(this);
    dedent();
    emitIndented("; Increment\n");
    emitIndented(increment_label + ":\n");
    indent();
    node->increment->accept(this);
    emitIndented("jmp " + start_label + "\n");
    dedent();
    emitIndented(end_label + ":\n");
}
void NASMCodeGenerator::visit(std::shared_ptr<AstWhileLoop> node){
    std::string start_label = getUniqueLabel("start");
    std::string end_label = getUniqueLabel("end");

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
    std::string start_label = getUniqueLabel("start");
    std::string end_label = getUniqueLabel("end");

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
            emitIndented("cqo\n");
            emitIndented("idiv rbx\n");
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
            std::string clause2 = getUniqueLabel("second_expression");
            std::string end = getUniqueLabel("end");
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
            std::string clause2 = getUniqueLabel("second_expression");
            std::string end = getUniqueLabel("end");
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
void NASMCodeGenerator::visit(std::shared_ptr<AstVariableAccess> node){
    emitIndented("; Variable Access(" + node->name + ")\n");
    emitIndented("mov " + sizeToNASMType(node->variable->sizeInBytes) + " rax, " + node->variable->accessStr + "\n");
}
void NASMCodeGenerator::visit(std::shared_ptr<AstVariableAssignment> node){
    node->value->accept(this);
    emitIndented("mov " + sizeToNASMType(node->variable->sizeInBytes) + " " + node->variable->accessStr + ", rax\n");
}
void NASMCodeGenerator::visit(std::shared_ptr<AstConditionalExpression> node){
    std::string true_clause = getUniqueLabel("true_expression");
    std::string false_clause = getUniqueLabel("false_expression");
    std::string end = getUniqueLabel("end");
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
    emitIndented("call " + node->function->accessString + "\n");

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
        // remove the brackets from the name
        std::string label = node->variable->accessStr.substr(1, node->variable->accessStr.size()-2);

        emit("    global " + label + "\n", BinarySection::Data);
        emit("    " + label + ": ", BinarySection::Data);
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
            emitIndented("push " + sizeToNASMType(node->variable->sizeInBytes) + " rax\n");
        }
        else{
            emitIndented("push " + sizeToNASMType(node->variable->sizeInBytes) + " 0\n");
        }
    }
}