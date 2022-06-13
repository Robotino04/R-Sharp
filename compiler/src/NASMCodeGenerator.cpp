#include "R-Sharp/NASMCodeGenerator.hpp"
#include "R-Sharp/Logging.hpp"
#include "R-Sharp/AstNodes.hpp"

#include <sstream>

NASMCodeGenerator::NASMCodeGenerator(std::shared_ptr<AstNode> root, std::string R_SharpSource){
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
void NASMCodeGenerator::emit(std::string const& str){
    source += str;
}
void NASMCodeGenerator::emitIndented(std::string const& str){
    for (int i=0;i<indentLevel;i++){
        source += "    ";
    }
    source += str;
}

std::string NASMCodeGenerator::generate(){
    source = "";
    indentLevel = 0;
    labelCounter = 0;
    root->accept(this);
    return source;
}

void NASMCodeGenerator::emitSyscall(Syscall callNr, std::string const& arg1, std::string const& arg2, std::string const& arg3, std::string const& arg4, std::string const& arg5, std::string const& arg6){
    // move the arguments to rdi, rsi, rdx, r10, r8, and r9 respectively
    emitIndented("; Syscall " + std::to_string(callNr) + "(" + std::to_string(static_cast<int>(callNr)) + ")\n");
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
    return prefix + "_" + std::to_string(labelCounter++);
}

NASMCodeGenerator::Variable NASMCodeGenerator::addVariable(AstVariableDeclaration* node){
    Variable var;
    var.name = node->name;
    if (node->type->getType() == AstNodeType::AstBuiltinType)
        var.type = std::static_pointer_cast<AstBuiltinType>(node->type)->name;
    else{
        Error("Only builtin types are supported");
        printErrorToken(node->token);
        exit(1);
    }
    if (var.type == "i64"){
        var.size = 8;
    }
    else if (var.type == "i32"){
        var.size = 4;
    }
    else{
        Fatal("Only i64 and i32 are supported");
    }
    stackOffset += var.size;
    var.stackOffset = stackOffset;
    getCurrentVariableScope().variables.push_back(var);
    return var;
}
NASMCodeGenerator::Variable NASMCodeGenerator::getVariable(std::string const& name){
    for (auto scope = getCurrentStackFrame().variableScopes.rbegin(); scope != getCurrentStackFrame().variableScopes.rend(); scope++){
        for (auto const& var : scope->variables){
            if (var.name == name) return var;
        }
    }
    Fatal("Variable " + name + " not found");
    return Variable();
}
void NASMCodeGenerator::pushStackFrame(){
    emitIndented("; Create stack frame\n");

    // save callee-saved registers
    emitIndented("push rbx\n");
    emitIndented("push rbp\n");
    emitIndented("push r12\n");
    emitIndented("push r13\n");
    emitIndented("push r14\n");
    emitIndented("push r15\n");
    emitIndented("mov rbp, rsp\n");

    stackFrames.emplace_back();
    stackFrames.back().variableScopes.emplace_back();
    stackOffset = 0;
}
int NASMCodeGenerator::getCurrentScopeSize(){
    return getScopeSize(getCurrentVariableScope());
}
int NASMCodeGenerator::getScopeSize(VariableScope const& scope){
    int size = 0;
    for (auto& var : scope.variables){
        size += var.size;
    }
    return size;
}

void NASMCodeGenerator::popStackFrame(bool codeOnly){
    if (!codeOnly){
        while (getCurrentStackFrame().variableScopes.size()){
            popVariableScope();
        }
    }
    else{
        for (auto varScope = stackFrames.back().variableScopes.rbegin(); varScope != stackFrames.back().variableScopes.rend(); varScope++){
            popVariableScope(*varScope);
        }
    }
    emitIndented("; Destroy stack frame\n");
    
    // restore callee-saved registers
    emitIndented("pop r15\n");
    emitIndented("pop r14\n");
    emitIndented("pop r13\n");
    emitIndented("pop r12\n");
    emitIndented("pop rbp\n");
    emitIndented("pop rbx\n");
    if (!codeOnly)
        stackFrames.pop_back();
}
void NASMCodeGenerator::pushVariableScope(){
    stackFrames.back().variableScopes.emplace_back();
}
void NASMCodeGenerator::popVariableScope(){
    popVariableScope(getCurrentVariableScope());
    stackOffset -= getCurrentScopeSize();
    getCurrentStackFrame().variableScopes.pop_back();
}
void NASMCodeGenerator::popVariableScope(VariableScope const& scope){
    emitIndented("add rsp, " + std::to_string(getScopeSize(scope)) + "\n");
}
NASMCodeGenerator::StackFrame& NASMCodeGenerator::getCurrentStackFrame(){
    return stackFrames.back();
}
NASMCodeGenerator::VariableScope& NASMCodeGenerator::getCurrentVariableScope(){
    return stackFrames.back().variableScopes.back();
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
void NASMCodeGenerator::visit(AstProgram* node){
    emit("; NASM code generated by R-Sharp compiler");
    emit("\n\nBITS 64\n");
    emit("global _start\n");
    emit("section .text\n");
    for (auto const& child : node->getChildren()){
        if (child->getType() == AstNodeType::AstFunction){
            child->accept(this);
        }
        else{
            Error("NASM Generator: Only functions are implemented!");
            if (child->getType() == AstNodeType::AstVariableDeclaration)
                printErrorToken(std::dynamic_pointer_cast<AstVariableDeclaration>(child)->token);
            exit(1);
        }
    }
    emit("_start:\n");
    indent();
    emitIndented("call main\n");
    emitSyscall(Syscall::exit, "rax");
    dedent();
}

// definitions
void NASMCodeGenerator::visit(AstFunction* node){
    emitIndented("; Function " + node->name + "\n\n");
    emitIndented("global " + node->name + "\n");
    emitIndented(node->name + ":\n");
    indent();
    pushStackFrame();

    node->parameters->accept(this);
    node->body->accept(this);

    popStackFrame();
    emitIndented("mov rax, 0\n");
    emitIndented("ret\n");
    dedent();
}


// statements
void NASMCodeGenerator::visit(AstBlock* node){
    pushVariableScope();
    for (auto const& child : node->getChildren()){
        child->accept(this);
    }
    popVariableScope();
}
void NASMCodeGenerator::visit(AstReturn* node){
    node->value->accept(this);
    popStackFrame(true);
    emitIndented("ret\n");
}
void NASMCodeGenerator::visit(AstConditionalStatement* node){
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
void NASMCodeGenerator::visit(AstForLoopDeclaration* node){
    std::string start_label = getUniqueLabel("start");
    std::string end_label = getUniqueLabel("end");
    std::string increment_label = getUniqueLabel("increment");

    getCurrentStackFrame().loopInfo.push_back({increment_label, end_label});

    pushVariableScope();
    getCurrentVariableScope().hasLoop = true;
    
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

    getCurrentVariableScope().hasLoop = false;
    popVariableScope();

    getCurrentStackFrame().loopInfo.pop_back();
}
void NASMCodeGenerator::visit(AstForLoopExpression* node){
    std::string start_label = getUniqueLabel("start");
    std::string end_label = getUniqueLabel("end");
    std::string increment_label = getUniqueLabel("increment");

    getCurrentStackFrame().loopInfo.push_back({increment_label, end_label});
    getCurrentVariableScope().hasLoop = true;

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

    getCurrentVariableScope().hasLoop = false;
    getCurrentStackFrame().loopInfo.pop_back();
}
void NASMCodeGenerator::visit(AstWhileLoop* node){
    std::string start_label = getUniqueLabel("start");
    std::string end_label = getUniqueLabel("end");

    getCurrentStackFrame().loopInfo.push_back({start_label, end_label});
    getCurrentVariableScope().hasLoop = true;

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

    getCurrentVariableScope().hasLoop = false;
    getCurrentStackFrame().loopInfo.pop_back();
}
void NASMCodeGenerator::visit(AstDoWhileLoop* node){
    std::string start_label = getUniqueLabel("start");
    std::string end_label = getUniqueLabel("end");

    getCurrentStackFrame().loopInfo.push_back({start_label, end_label});
    getCurrentVariableScope().hasLoop = true;

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

    getCurrentVariableScope().hasLoop = false;
    getCurrentStackFrame().loopInfo.pop_back();

}
void NASMCodeGenerator::visit(AstBreak* node){
    if (getCurrentStackFrame().loopInfo.empty()){
        Error("NASM Generator: Break statement outside of loop!");
    }

    for (auto varScope = getCurrentStackFrame().variableScopes.rbegin(); varScope != getCurrentStackFrame().variableScopes.rend(); ++varScope){
        if (varScope->hasLoop){
            break;
        }
        else{
            popVariableScope(*varScope);
        }
    }

    emitIndented("; Break\n");
    emitIndented("jmp " + getCurrentStackFrame().loopInfo.back().breakLabel + "\n");
}
void NASMCodeGenerator::visit(AstSkip* node){
    if (getCurrentStackFrame().loopInfo.empty()){
        Error("NASM Generator: Skip statement outside of loop!");
    }
    for (auto varScope = getCurrentStackFrame().variableScopes.rbegin(); varScope != getCurrentStackFrame().variableScopes.rend(); ++varScope){
        if (varScope->hasLoop){
            break;
        }
        else{
            popVariableScope(*varScope);
        }
    }

    emitIndented("; Skip\n");
    emitIndented("jmp " + getCurrentStackFrame().loopInfo.back().skipLabel + "\n");
}


// expressions
void NASMCodeGenerator::visit(AstUnary* node){
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
        default:
            Error("NASM Generator: Unary operator not implemented!");
            printErrorToken(node->token);
            exit(1);
            break;
    }
}
void NASMCodeGenerator::visit(AstBinary* node){
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
            printErrorToken(node->token);
            exit(1);
            break;
    }
}
void NASMCodeGenerator::visit(AstInteger* node){
    emitIndented("mov rax, " + std::to_string(node->value) + "\n");
}
void NASMCodeGenerator::visit(AstVariableAccess* node){
    Variable var = getVariable(node->name);
    emitIndented("mov " + sizeToNASMType(var.size) + " rax, [rbp - " + std::to_string(var.stackOffset) + "]\n");
}
void NASMCodeGenerator::visit(AstVariableAssignment* node){
    Variable var = getVariable(node->name);
    node->value->accept(this);
    emitIndented("mov " + sizeToNASMType(var.size) + " [rbp - " + std::to_string(var.stackOffset) + "], rax\n");
}
void NASMCodeGenerator::visit(AstConditionalExpression* node){
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
void NASMCodeGenerator::visit(AstEmptyExpression* node){
    emitIndented("; Empty Expression\n");
    emitIndented("mov rax, 1\n");
}


// declarations
void NASMCodeGenerator::visit(AstVariableDeclaration* node){
    emitIndented("; Variable (" + node->name + ")\n");
    Variable var = addVariable(node);
    if (node->value){
        node->value->accept(this);
        emitIndented("push " + sizeToNASMType(var.size) + " rax\n");
    }
    else{
        emitIndented("push " + sizeToNASMType(var.size) + " 0\n");
    }
}


void NASMCodeGenerator::printErrorToken(Token token){
    int start = token.position.startPos;
    int end = token.position.endPos;

    std::string src = R_SharpSource;
    src.replace(start, end - start, "\033[31m" + src.substr(start, end - start) + "\033[0m");

    // print the error and 3 lines above it
    std::stringstream ss;
    int line = 1;
    int column = 1;
    int pos = 0;

    for (char c : src) {
        if (line >= token.position.line - 3 && line <= token.position.line) {
            if (column == 1){
                ss << line << "| ";
            }
            if (line == token.position.line && c == '\n') break;
            ss << c;
        }
        pos++;
        if (c == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
    }

    int prefixLen = (std::to_string(token.position.line) + "| ").length();

    ss << "\n\033[31m" // enable red text
        << std::string(prefixLen + token.position.column - 1, ' ') // print spaces before the error
        << "^";
    try {
        ss << std::string(end - start - 1, '~'); // underline the error
    }
    catch(std::length_error){}

    ss << "\033[0m"; // disable red text
    Print(ss.str());
}