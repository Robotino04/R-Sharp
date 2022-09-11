#include "R-Sharp/AArch64CodeGenerator.hpp"
#include "R-Sharp/Logging.hpp"
#include "R-Sharp/AstNodes.hpp"
#include "R-Sharp/Utils.hpp"

#include <sstream>

AArch64CodeGenerator::AArch64CodeGenerator(std::shared_ptr<AstNode> root, std::string R_SharpSource){
    this->root = root;
    this->R_SharpSource = R_SharpSource;
}

void AArch64CodeGenerator::indent(){
    indentLevel++;
}
void AArch64CodeGenerator::dedent(){
    if (!indentLevel){
        Fatal("Indentation error");
        return;
    }
    indentLevel--;
}
void AArch64CodeGenerator::emit(std::string const& str, AArch64CodeGenerator::BinarySection section){
    switch (section){
        case AArch64CodeGenerator::BinarySection::Text:
            source_text += str;
            break;
        case AArch64CodeGenerator::BinarySection::BSS:
            source_bss += str;
            break;
        case AArch64CodeGenerator::BinarySection::Data:
            source_data += str;
            break;
    }
}
void AArch64CodeGenerator::emitIndented(std::string const& str, AArch64CodeGenerator::BinarySection section){
    for (int i=0;i<indentLevel;i++){
        source_text += "    ";
    }
    switch (section){
        case AArch64CodeGenerator::BinarySection::Text:
            source_text += str;
            break;
        case AArch64CodeGenerator::BinarySection::BSS:
            source_bss += str;
            break;
        case AArch64CodeGenerator::BinarySection::Data:
            source_data += str;
            break;
    }
}

std::string AArch64CodeGenerator::generate(){
    source_text = "";
    source_data = "";
    source_bss = "";
    indentLevel = 0;
    labelCounter = 0;
    externFunctions = {};
    globalVariables = {};
    root->accept(this);

    // collect the uninitialized global variables
    for (auto& var : globalScope.variables){
        if (!var.initialized){
            emit(var.accessStr.substr(1, var.accessStr.size()-2) + ":\n", BinarySection::BSS);
            switch(var.size){
                case 1: emit("    .byte 1\n", BinarySection::BSS); break;
                case 2: emit("    .2byte 1\n", BinarySection::BSS); break;
                case 4: emit("    .4byte 1\n", BinarySection::BSS); break;
                case 8: emit("    .8byte 1\n", BinarySection::BSS); break;
                default: Fatal("Unsupported variable size (" + std::to_string(var.size) + ")");
            }
        }
    }

    std::string output = "";
    output += "// AArch64 code generated by R-Sharp compiler\n\n";
    output += ".text\n";
    output += source_text;
    output += "\n.data\n";
    output += source_data;
    output += "\n.bss\n";
    output += source_bss;


    return output;
}

void AArch64CodeGenerator::emitSyscall(Syscall callNr, std::string const& arg1, std::string const& arg2, std::string const& arg3, std::string const& arg4, std::string const& arg5, std::string const& arg6){
    // move the arguments to rdi, rsi, rdx, r10, r8, and r9 respectively
    emitIndented("// Syscall " + std::to_string(callNr) + "(" + std::to_string(static_cast<int>(callNr)) + ")\n");
    if (arg1 != "") emitIndented("mov rdi, " + arg1 + "\n");
    if (arg2 != "") emitIndented("mov rsi, " + arg2 + "\n");
    if (arg3 != "") emitIndented("mov rdx, " + arg3 + "\n");
    if (arg4 != "") emitIndented("mov r10, " + arg4 + "\n");
    if (arg5 != "") emitIndented("mov r8, " + arg5 + "\n");
    if (arg6 != "") emitIndented("mov r9, " + arg6 + "\n");

    emitIndented("mov x0, " + std::to_string(static_cast<int>(callNr)) + "\n");

    emitIndented("syscall\n");
}
void AArch64CodeGenerator::emitPush(std::string source){
    emitIndented("str " + source + ", [sp, -16]!\n");
}
void AArch64CodeGenerator::emitPop(std::string destination){
    emitIndented("ldr " + destination + ", [sp], 16\n");
}


std::string AArch64CodeGenerator::getUniqueLabel(std::string const& prefix){
    return prefix + "_" + std::to_string(labelCounter++);
}

AArch64CodeGenerator::Variable AArch64CodeGenerator::addVariable(AstVariableDeclaration* node){
    Variable var;
    var.name = node->name;
    var.type = node->semanticType;
    
    // if (node->type->getType() == AstNodeType::AstBuiltinType)
    //     var.type = std::static_pointer_cast<AstBuiltinType>(node->type)->name;
    // else{
    //     Error("Only builtin types are supported");
    //     printErrorToken(node->token, R_SharpSource);
    //     exit(1);
    // }
    if (!var.type){
        Error("INTERNAL ERROR: AstVariableDeclaration has no semanticType");
        printErrorToken(node->token, R_SharpSource);
        exit(1);
    }
    if (var.type->type == RSharpType::I64){
        var.size = 8;
        var.stackSize = 16;
    }
    else if (var.type->type == RSharpType::I32){
        var.size = 4;
        var.stackSize = 16;
    }
    else{
        Error("Unsupported type Nr.", static_cast<int>(var.type->type));
        printErrorToken(node->token, R_SharpSource);
        exit(1);
    }

    if (node->isGlobal){
        // test if the variable is already declared
        if (std::find(globalScope.variables.begin(), globalScope.variables.end(), var) != globalScope.variables.end()){
            auto match = std::find(globalScope.variables.begin(), globalScope.variables.end(), var);
            if (match->initialized){
                Error("Variable " + var.name + " is already defined");
                printErrorToken(node->token, R_SharpSource);
                exit(1);
            }
            else{
                match->initialized = true;
                return *match;
            }
        }

        globalVariables.push_back(node->name);
        var.accessStr = "[" + getUniqueLabel(node->name) + "]";
        var.initialized = node->value != nullptr;
        globalScope.variables.push_back(var);
    }
    else{
        stackOffset += var.stackSize;
        var.accessStr = "[fp, -" + std::to_string(stackOffset) + "]";
        getCurrentVariableScope().variables.push_back(var);
    }
    return var;
}
AArch64CodeGenerator::Variable AArch64CodeGenerator::getVariable(std::string const& name){
    for (auto scope = getCurrentStackFrame().variableScopes.rbegin(); scope != getCurrentStackFrame().variableScopes.rend(); scope++){
        for (auto const& var : scope->variables){
            if (var.name == name) return var;
        }
    }
    // if we didn't find it in the current scope, it must be global
    for (auto const& var : globalScope.variables){
        if (var.name == name) return var;
    }

    Fatal("Variable " + name + " not found");
    return Variable();
}
void AArch64CodeGenerator::pushStackFrame(){
    emitIndented("// Create stack frame\n");

    // save callee-saved registers
    emitIndented("mov fp, sp\n");

    stackFrames.emplace_back();
    stackFrames.back().variableScopes.emplace_back();
    stackOffset = 0;
}
int AArch64CodeGenerator::getCurrentScopeSize(){
    return getScopeSize(getCurrentVariableScope());
}
int AArch64CodeGenerator::getScopeSize(VariableScope const& scope){
    int size = 0;
    for (auto& var : scope.variables){
        size += var.stackSize;
    }
    return size;
}

void AArch64CodeGenerator::popStackFrame(bool codeOnly){
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
    emitIndented("// Destroy stack frame\n");
    
    // restore callee-saved registers
    if (!codeOnly)
        stackFrames.pop_back();
}
void AArch64CodeGenerator::pushVariableScope(){
    stackFrames.back().variableScopes.emplace_back();
}
void AArch64CodeGenerator::popVariableScope(){
    popVariableScope(getCurrentVariableScope());
    stackOffset -= getCurrentScopeSize();
    getCurrentStackFrame().variableScopes.pop_back();
}
void AArch64CodeGenerator::popVariableScope(VariableScope const& scope){
    emitIndented("add sp, sp, " + std::to_string(getScopeSize(scope)) + "\n");
}
AArch64CodeGenerator::StackFrame& AArch64CodeGenerator::getCurrentStackFrame(){
    return stackFrames.back();
}
AArch64CodeGenerator::VariableScope& AArch64CodeGenerator::getCurrentVariableScope(){
    return stackFrames.back().variableScopes.back();
}

std::string AArch64CodeGenerator::sizeToAArch64Type(int size){
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
void AArch64CodeGenerator::visit(AstProgram* node){
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
    for (auto const& func : externFunctions){
        emitIndented("extern " + func + "\n");
    }
}
void AArch64CodeGenerator::visit(AstParameterList* node){
    int argumentNumber = 0;
    for (auto const& child : node->parameters){
        child->accept(this);
        Variable var = getVariable(child->name);

        emitIndented("// Load argument " + std::to_string(argumentNumber) + "\n");
        emitIndented("mov " + sizeToAArch64Type(var.size) + " " + var.accessStr + ", ");

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
void AArch64CodeGenerator::visit(AstFunctionDeclaration* node){

    if (node->body){
        emitIndented("// Function " + node->name + "\n\n");
        emitIndented(".global " + node->name + "\n");
        emitIndented(node->name + ":\n");
        indent();
        pushStackFrame();

        node->parameters->accept(this);
        node->body->accept(this);

        popStackFrame();
        emitIndented("mov x0, 0\n");
        emitIndented("ret\n");
        dedent();

        // remove this function from externFunctions
        for (auto it = externFunctions.begin(); it != externFunctions.end(); it++){
            if (*it == node->name){
                externFunctions.erase(it);
                break;
            }
        }
    }
    else{
        // add function to externFunctions if it is not already there
        if (std::find(externFunctions.begin(), externFunctions.end(), node->name) == externFunctions.end()){
            externFunctions.push_back(node->name);
        }
    }
}


// statements
void AArch64CodeGenerator::visit(AstBlock* node){
    pushVariableScope();
    for (auto const& child : node->getChildren()){
        if (child) child->accept(this);
    }
    popVariableScope();
}
void AArch64CodeGenerator::visit(AstReturn* node){
    node->value->accept(this);
    popStackFrame(true);
    emitIndented("ret\n");
}
void AArch64CodeGenerator::visit(AstConditionalStatement* node){
    std::string else_label = getUniqueLabel("else");
    std::string end_label = getUniqueLabel("end");

    node->condition->accept(this);
    emitIndented("// If statement\n");
    emitIndented("cbz x0, " + else_label + "\n");
    indent();
    node->trueStatement->accept(this);
    emitIndented("b " + end_label + "\n");
    dedent();
    emitIndented(else_label + ":\n");
    indent();
    if (node->falseStatement) node->falseStatement->accept(this);
    dedent();
    emitIndented(end_label + ":\n");
}
void AArch64CodeGenerator::visit(AstForLoopDeclaration* node){
    std::string start_label = getUniqueLabel("start");
    std::string end_label = getUniqueLabel("end");
    std::string increment_label = getUniqueLabel("increment");

    getCurrentStackFrame().loopInfo.push_back({increment_label, end_label});

    pushVariableScope();
    getCurrentVariableScope().hasLoop = true;
    
    emitIndented("// For loop\n");
    emitIndented("// Initialization\n");
    node->variable->accept(this);

    emitIndented("// For loop\n");
    emitIndented(start_label + ":\n");
    indent();
    emitIndented("// Condition\n");
    node->condition->accept(this);
    emitIndented("cmp x0, 0\n");
    emitIndented("je " + end_label + "\n");
    emitIndented("// Body\n");
    node->body->accept(this);
    dedent();
    emitIndented("// Increment\n");
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
void AArch64CodeGenerator::visit(AstForLoopExpression* node){
    std::string start_label = getUniqueLabel("start");
    std::string end_label = getUniqueLabel("end");
    std::string increment_label = getUniqueLabel("increment");

    getCurrentStackFrame().loopInfo.push_back({increment_label, end_label});
    getCurrentVariableScope().hasLoop = true;

    emitIndented("// For loop\n");
    emitIndented("// Initialization\n");
    node->variable->accept(this);

    emitIndented("// For loop\n");
    emitIndented(start_label + ":\n");
    indent();
    emitIndented("// Condition\n");
    node->condition->accept(this);
    emitIndented("cmp x0, 0\n");
    emitIndented("je " + end_label + "\n");
    emitIndented("// Body\n");
    node->body->accept(this);
    dedent();
    emitIndented("// Increment\n");
    emitIndented(increment_label + ":\n");
    indent();
    node->increment->accept(this);
    emitIndented("jmp " + start_label + "\n");
    dedent();
    emitIndented(end_label + ":\n");

    getCurrentVariableScope().hasLoop = false;
    getCurrentStackFrame().loopInfo.pop_back();
}
void AArch64CodeGenerator::visit(AstWhileLoop* node){
    std::string start_label = getUniqueLabel("start");
    std::string end_label = getUniqueLabel("end");

    getCurrentStackFrame().loopInfo.push_back({start_label, end_label});
    getCurrentVariableScope().hasLoop = true;

    emitIndented("// While loop\n");
    emitIndented(start_label + ":\n");
    indent();
    node->condition->accept(this);
    emitIndented("cmp x0, 0\n");
    emitIndented("je " + end_label + "\n");
    emitIndented("// Body\n");
    node->body->accept(this);
    emitIndented("jmp " + start_label + "\n");
    dedent();
    emitIndented(end_label + ":\n");

    getCurrentVariableScope().hasLoop = false;
    getCurrentStackFrame().loopInfo.pop_back();
}
void AArch64CodeGenerator::visit(AstDoWhileLoop* node){
    std::string start_label = getUniqueLabel("start");
    std::string end_label = getUniqueLabel("end");

    getCurrentStackFrame().loopInfo.push_back({start_label, end_label});
    getCurrentVariableScope().hasLoop = true;

    emitIndented("// Do loop\n");
    emitIndented(start_label + ":\n");
    indent();
    emitIndented("// Body\n");
    node->body->accept(this);
    node->condition->accept(this);
    emitIndented("cmp x0, 0\n");
    emitIndented("jne " + start_label + "\n");
    dedent();
    emitIndented(end_label + ":\n");

    getCurrentVariableScope().hasLoop = false;
    getCurrentStackFrame().loopInfo.pop_back();

}
void AArch64CodeGenerator::visit(AstBreak* node){
    if (getCurrentStackFrame().loopInfo.empty()){
        Error("AArch64 Generator: Break statement outside of loop!");
    }

    for (auto varScope = getCurrentStackFrame().variableScopes.rbegin(); varScope != getCurrentStackFrame().variableScopes.rend(); ++varScope){
        if (varScope->hasLoop){
            break;
        }
        else{
            popVariableScope(*varScope);
        }
    }

    emitIndented("// Break\n");
    emitIndented("jmp " + getCurrentStackFrame().loopInfo.back().breakLabel + "\n");
}
void AArch64CodeGenerator::visit(AstSkip* node){
    if (getCurrentStackFrame().loopInfo.empty()){
        Error("AArch64 Generator: Skip statement outside of loop!");
    }
    for (auto varScope = getCurrentStackFrame().variableScopes.rbegin(); varScope != getCurrentStackFrame().variableScopes.rend(); ++varScope){
        if (varScope->hasLoop){
            break;
        }
        else{
            popVariableScope(*varScope);
        }
    }

    emitIndented("// Skip\n");
    emitIndented("jmp " + getCurrentStackFrame().loopInfo.back().skipLabel + "\n");
}


// expressions
void AArch64CodeGenerator::visit(AstUnary* node){
    node->value->accept(this);
    switch (node->type){
        case AstUnaryType::Negate:
            emitIndented("neg x0, x0\n");
            break;
        case AstUnaryType::BinaryNot:
            emitIndented("mvn x0, x0\n");
            break;
        case AstUnaryType::LogicalNot:
            emitIndented("cmp x0, 0\n");
            emitIndented("cset x0, eq\n");
            break;
        default:
            Error("AArch64 Generator: Unary operator not implemented!");
            printErrorToken(node->token, R_SharpSource);
            exit(1);
            break;
    }
}
void AArch64CodeGenerator::visit(AstBinary* node){
    node->left->accept(this);

    // logical and and or will short circuit, so the right side is not evaluated until necessary
    if (!(node->type == AstBinaryType::LogicalOr || node->type == AstBinaryType::LogicalAnd)){
        emitPush("x0");
        node->right->accept(this);
        emitIndented("mov x1, x0\n");
        emitPop("x0");
    }
    switch (node->type){
        case AstBinaryType::Add:
            emitIndented("// Add\n");
            emitIndented("add x0, x0, x1\n");
            break;
        case AstBinaryType::Subtract:
            emitIndented("// Subtract\n");
            emitIndented("sub x0, x0, x1\n");
            break;
        case AstBinaryType::Multiply:
            emitIndented("// Multiply\n");
            emitIndented("mul x0, x0, x1\n");
            break;
        case AstBinaryType::Divide:
            emitIndented("// Divide\n");
            emitIndented("sdiv x0, x0, x1\n");
            break;
        case AstBinaryType::Modulo:
            emitIndented("// Modulo\n");
            emitIndented("sdiv x2, x0, x1\n");
            emitIndented("msub x0, x2, x1, x0\n");
            break;

        case AstBinaryType::Equal:
            emitIndented("// Equal\n");
            emitIndented("cmp x0, x1\n");
            emitIndented("cset x0, eq\n");
            break;
        case AstBinaryType::NotEqual:
            emitIndented("// Not Equal\n");
            emitIndented("cmp x0, x1\n");
            emitIndented("cset x0, ne\n");
            break;
        case AstBinaryType::LessThan:
            emitIndented("// Less Than\n");
            emitIndented("cmp x0, x1\n");
            emitIndented("cset x0, lt\n");
            break;
        case AstBinaryType::LessThanOrEqual:
            emitIndented("// Less Than Or Equal\n");
            emitIndented("cmp x0, x1\n");
            emitIndented("cset x0, le\n");
            break;
        case AstBinaryType::GreaterThan:
            emitIndented("// Greater Than\n");
            emitIndented("cmp x0, x1\n");
            emitIndented("cset x0, gt\n");
            break;
        case AstBinaryType::GreaterThanOrEqual:
            emitIndented("// Greater Than Or Equal\n");
            emitIndented("cmp x0, x1\n");
            emitIndented("cset x0, ge\n");
            break;

        case AstBinaryType::LogicalAnd:{
            emitIndented("// Logical And\n");
            std::string end = getUniqueLabel("end");
            emitIndented("cbz x0, " + end + "\n");

            // evaluate right side
            node->right->accept(this);
            emitIndented("cmp x0, 0\n");
            emitIndented("cset x0, ne\n");
            emitIndented(end + ":\n");
            break;
        }

        case AstBinaryType::LogicalOr:{
            emitIndented("// Logical Or\n");
            std::string clause2 = getUniqueLabel("second_expression");
            std::string end = getUniqueLabel("end");
            emitIndented("cbz x0, " + clause2 + "\n");
            emitIndented("mov x0, 1\n");
            emitIndented("b " + end + "\n");
            emitIndented(clause2 + ":\n"); indent();

            // evaluate right side
            node->right->accept(this);
            emitIndented("cmp x0, 0\n");
            emitIndented("cset x0, ne\n");
            dedent();
            emitIndented(end + ":\n");
            break;
        }
        default:
            Error("AArch64 Generator: Binary operator not implemented!");
            printErrorToken(node->token, R_SharpSource);
            exit(1);
            break;
    }
}
void AArch64CodeGenerator::visit(AstInteger* node){
    emitIndented("// Integer " + std::to_string(node->value) + "\n");
    emitIndented("mov x0, " + std::to_string(node->value) + "\n");
}
void AArch64CodeGenerator::visit(AstVariableAccess* node){
    emitIndented("// Variable Access(" + node->name + ")\n");
    Variable var = getVariable(node->name);
    emitIndented("ldr x0, " + var.accessStr + "\n");
}
void AArch64CodeGenerator::visit(AstVariableAssignment* node){
    Variable var = getVariable(node->name);
    node->value->accept(this);
    emitIndented("str x0, " + var.accessStr + "\n");
}
void AArch64CodeGenerator::visit(AstConditionalExpression* node){
    std::string true_clause = getUniqueLabel("true_expression");
    std::string false_clause = getUniqueLabel("false_expression");
    std::string end = getUniqueLabel("end");
    node->condition->accept(this);
    emitIndented("// Conditional Expression\n");
    emitIndented("cbz x0, " + false_clause + "\n");
    emitIndented(true_clause + ":\n"); indent();
    node->trueExpression->accept(this);
    emitIndented("b " + end + "\n");
    dedent();
    emitIndented(false_clause + ":\n"); indent();
    node->falseExpression->accept(this);
    dedent();
    emitIndented(end + ":\n");
}
void AArch64CodeGenerator::visit(AstEmptyExpression* node){
    emitIndented("// Empty Expression\n");
    emitIndented("mov x0, 1\n");
}
void AArch64CodeGenerator::visit(AstFunctionCall* node){
    // rdi, rsi, rdx, rcx, r8, and r9 are used for parameters
    // more than 6 parameters are not supported yet

    if (node->arguments.size() > 6){
        Error("AArch64 Generator: More than 6 parameters are not supported yet!");
        printErrorToken(node->token, R_SharpSource);
        exit(1);
    }
    // save registers
    emitIndented("// Prepare for function call (" + node->name + ")\n");
    int argCount = node->arguments.size();
    for (int i = 0; i < argCount; i++){
        switch(i){
            case 0: emitPush("rdi"); break;
            case 1: emitPush("rsi"); break;
            case 2: emitPush("rdx"); break;
            case 3: emitPush("rcx"); break;
            case 4: emitPush("r8"); break;
            case 5: emitPush("r9"); break;
        }
    }

    // evaluate arguments
    for (auto arg : node->arguments){
        arg->accept(this);
        emitPush("x0");
    }
    // move arguments to registers
    for (int i = argCount - 1; i >= 0; i--){
        switch(i){
            case 0: emitPop("rdi"); break;
            case 1: emitPop("rsi"); break;
            case 2: emitPop("rdx"); break;
            case 3: emitPop("rcx"); break;
            case 4: emitPop("r8"); break;
            case 5: emitPop("r9"); break;
        }
    }

    emitIndented("// Function Call (" + node->name + ")\n");
    emitIndented("call " + node->name + "\n");

    emitIndented("// Restore after function call (" + node->name + ")\n");
    // restore registers
    for (int i = argCount - 1; i >= 0; i--){
        switch(i){
            case 0: emitPop("rdi"); break;
            case 1: emitPop("rsi"); break;
            case 2: emitPop("rdx"); break;
            case 3: emitPop("rcx"); break;
            case 4: emitPop("r8"); break;
            case 5: emitPop("r9"); break;
        }
    }
}


// declarations
void AArch64CodeGenerator::visit(AstVariableDeclaration* node){
    Variable var = addVariable(node);
    if (node->isGlobal){
        if (!node->value){
            return;
        }
        if (node->value->getType() != AstNodeType::AstInteger){
            Error("AArch64 Generator: Global variable must be of type integer!");
            printErrorToken(node->token, R_SharpSource);
            exit(1);
        }
        auto intNode = std::static_pointer_cast<AstInteger>(node->value);
        emit("    ; Global Variable (" + node->name + ")\n", BinarySection::Data);
        // remove the brackets from the name
        std::string label = var.accessStr.substr(1, var.accessStr.size()-2);
        emit("    global " + label + "\n", BinarySection::Data);
        emit(label + ":\n", BinarySection::Data);
        switch(var.size){
            case 1: emit("    db " + std::to_string(intNode->value) + "\n", BinarySection::Data); break;
            case 2: emit("    dw " + std::to_string(intNode->value) + "\n", BinarySection::Data); break;
            case 4: emit("    dd " + std::to_string(intNode->value) + "\n", BinarySection::Data); break;
            case 8: emit("    dq " + std::to_string(intNode->value) + "\n", BinarySection::Data); break;
            default:
                Error("AArch64 Generator: Global variable size not supported!");
                printErrorToken(node->token, R_SharpSource);
                exit(1);
                break;
        }
    }
    else{
        emitIndented("// Variable (" + node->name + ")\n");
        if (node->value){
            node->value->accept(this);
            emitIndented("str x0, [sp, -" + std::to_string(var.stackSize) + "]!\n");
        }
        else{
            emitIndented("mov x0, 0\n");
            emitIndented("str x0, [sp, -" + std::to_string(var.stackSize) + "]!\n");
        }
    }
}