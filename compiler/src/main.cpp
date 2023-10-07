#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "R-Sharp/Logging.hpp"
#include "R-Sharp/Tokenizer.hpp"
#include "R-Sharp/Token.hpp"
#include "R-Sharp/AstNodes.hpp"
#include "R-Sharp/Parser.hpp"
#include "R-Sharp/Utils.hpp"
#include "R-Sharp/ParsingCache.hpp"

#include "R-Sharp/AstPrinter.hpp"
#include "R-Sharp/CCodeGenerator.hpp"
#include "R-Sharp/NASMCodeGenerator.hpp"
#include "R-Sharp/AArch64CodeGenerator.hpp"
#include "R-Sharp/ErrorPrinter.hpp"
#include "R-Sharp/SemanticValidator.hpp"
#include "R-Sharp/RSIGenerator.hpp"
#include "R-Sharp/RSIAnalysis.hpp"
#include "R-Sharp/Utils/LambdaOverload.hpp"

enum class ReturnValue{
    NormalExit = 0,
    UnknownError = 1,
    SyntaxError = 2,
    SemanticError = 3,
    AssemblingError = 4,
};

void printHelp(const char* programName) {
    std::cout << "Usage: " << programName << " [options] [input file]\n";
    std::cout <<
R"(Options:
  -h, --help                Print this help message
  -o, --output <file>       Output file
  -f, --format <format>     Output format (c, nasm, aarch64, rsi)
  --compiler <path>         Use this compiler. Default: "gcc"
  --link <file>             Additionally link <file> into the output. Can be repeated.
  --stdlib <path>           Use the standard library at <path>.

Return values:
  0     Everything OK
  1     Something unknown went wrong
  2     There were syntax errors
  3     There were semantic errors
  4     There were assembling errors
)";
}

enum class OutputFormat {
    C,
    NASM,
    AArch64,
    RSI_NASM,
    RSI_AArch64,
};

enum class NasmRegisters{
    RAX,
    RBX,
    RCX,
    RDX,
    RSI,
    RDI,
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15,

    COUNT,
};

inline const std::vector<RSI::HWRegister> nasmRegisters(static_cast<int>(NasmRegisters::COUNT));
inline const std::map<RSI::HWRegister, std::string> nasmRegisterTranslation = {
    {nasmRegisters.at(static_cast<int>(NasmRegisters::RAX)), "rax"},
    {nasmRegisters.at(static_cast<int>(NasmRegisters::RBX)), "rbx"},
    {nasmRegisters.at(static_cast<int>(NasmRegisters::RCX)), "rcx"},
    {nasmRegisters.at(static_cast<int>(NasmRegisters::RDX)), "rdx"},
    {nasmRegisters.at(static_cast<int>(NasmRegisters::RSI)), "rsi"},
    {nasmRegisters.at(static_cast<int>(NasmRegisters::RDI)), "rdi"},
    {nasmRegisters.at(static_cast<int>(NasmRegisters::R8)), "r8"},
    {nasmRegisters.at(static_cast<int>(NasmRegisters::R9)), "r9"},
    {nasmRegisters.at(static_cast<int>(NasmRegisters::R10)), "r10"},
    {nasmRegisters.at(static_cast<int>(NasmRegisters::R11)), "r11"},
    {nasmRegisters.at(static_cast<int>(NasmRegisters::R12)), "r12"},
    {nasmRegisters.at(static_cast<int>(NasmRegisters::R13)), "r13"},
    {nasmRegisters.at(static_cast<int>(NasmRegisters::R14)), "r14"},
    {nasmRegisters.at(static_cast<int>(NasmRegisters::R15)), "r15"},
};

inline const std::map<std::pair<std::string, int>, std::string> nasmRegisterSize = {
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

    {{"rsi", 1}, "sil"},
    {{"rsi", 2}, "si"},
    {{"rsi", 4}, "esi"},
    {{"rsi", 8}, "rsi"},

    {{"rdi", 1}, "dil"},
    {{"rdi", 2}, "di"},
    {{"rdi", 4}, "edi"},
    {{"rdi", 8}, "rdi"},

    {{"rsp", 1}, "spl"},
    {{"rsp", 2}, "sp"},
    {{"rsp", 4}, "esp"},
    {{"rsp", 8}, "rsp"},

    {{"rbp", 1}, "bpl"},
    {{"rbp", 2}, "bp"},
    {{"rbp", 4}, "ebp"},
    {{"rbp", 8}, "rbp"},

    {{"r0", 1}, "r0b"},
    {{"r0", 2}, "r0w"},
    {{"r0", 4}, "r0d"},
    {{"r0", 8}, "r0"},


    {{"r1", 1}, "r1b"},
    {{"r1", 2}, "r1w"},
    {{"r1", 4}, "r1d"},
    {{"r1", 8}, "r1"},


    {{"r2", 1}, "r2b"},
    {{"r2", 2}, "r2w"},
    {{"r2", 4}, "r2d"},
    {{"r2", 8}, "r2"},


    {{"r3", 1}, "r3b"},
    {{"r3", 2}, "r3w"},
    {{"r3", 4}, "r3d"},
    {{"r3", 8}, "r3"},


    {{"r4", 1}, "r4b"},
    {{"r4", 2}, "r4w"},
    {{"r4", 4}, "r4d"},
    {{"r4", 8}, "r4"},


    {{"r5", 1}, "r5b"},
    {{"r5", 2}, "r5w"},
    {{"r5", 4}, "r5d"},
    {{"r5", 8}, "r5"},


    {{"r6", 1}, "r6b"},
    {{"r6", 2}, "r6w"},
    {{"r6", 4}, "r6d"},
    {{"r6", 8}, "r6"},


    {{"r7", 1}, "r7b"},
    {{"r7", 2}, "r7w"},
    {{"r7", 4}, "r7d"},
    {{"r7", 8}, "r7"},


    {{"r8", 1}, "r8b"},
    {{"r8", 2}, "r8w"},
    {{"r8", 4}, "r8d"},
    {{"r8", 8}, "r8"},


    {{"r9", 1}, "r9b"},
    {{"r9", 2}, "r9w"},
    {{"r9", 4}, "r9d"},
    {{"r9", 8}, "r9"},


    {{"r10", 1}, "r10b"},
    {{"r10", 2}, "r10w"},
    {{"r10", 4}, "r10d"},
    {{"r10", 8}, "r10"},


    {{"r11", 1}, "r11b"},
    {{"r11", 2}, "r11w"},
    {{"r11", 4}, "r11d"},
    {{"r11", 8}, "r11"},


    {{"r12", 1}, "r12b"},
    {{"r12", 2}, "r12w"},
    {{"r12", 4}, "r12d"},
    {{"r12", 8}, "r12"},


    {{"r13", 1}, "r13b"},
    {{"r13", 2}, "r13w"},
    {{"r13", 4}, "r13d"},
    {{"r13", 8}, "r13"},


    {{"r14", 1}, "r14b"},
    {{"r14", 2}, "r14w"},
    {{"r14", 4}, "r14d"},
    {{"r14", 8}, "r14"},


    {{"r15", 1}, "r15b"},
    {{"r15", 2}, "r15w"},
    {{"r15", 4}, "r15d"},
    {{"r15", 8}, "r15"},
};


inline const std::vector<RSI::HWRegister> aarch64Registers(26);
inline const std::map<RSI::HWRegister, std::string> aarch64RegistersRegisterTranslation = {
    {aarch64Registers.at(0), "x0"},
    {aarch64Registers.at(1), "x1"},
    {aarch64Registers.at(2), "x2"},
    {aarch64Registers.at(3), "x3"},
    {aarch64Registers.at(4), "x4"},
    {aarch64Registers.at(5), "x5"},
    {aarch64Registers.at(6), "x6"},
    {aarch64Registers.at(7), "x7"},
    {aarch64Registers.at(8), "x8"},
    {aarch64Registers.at(9), "x9"},
    {aarch64Registers.at(10), "x10"},
    {aarch64Registers.at(11), "x11"},
    {aarch64Registers.at(12), "x12"},
    {aarch64Registers.at(13), "x13"},
    {aarch64Registers.at(14), "x14"},
    {aarch64Registers.at(15), "x15"},
    {aarch64Registers.at(16), "x16"},
    {aarch64Registers.at(17), "x17"},

    {aarch64Registers.at(18), "x21"},
    {aarch64Registers.at(19), "x22"},
    {aarch64Registers.at(20), "x23"},
    {aarch64Registers.at(21), "x24"},
    {aarch64Registers.at(22), "x25"},
    {aarch64Registers.at(23), "x26"},
    {aarch64Registers.at(24), "x27"},
    {aarch64Registers.at(25), "x28"},
};

std::string translateOperand(RSI::Operand const& op, std::map<RSI::HWRegister, std::string> registerTranslation, std::string constantPrefix){
    return std::visit(lambda_overload{
        [&](RSI::Constant const& x) { return constantPrefix + std::to_string(x.value); },
        [&](std::shared_ptr<RSI::Reference> x){ return registerTranslation.at(x->assignedRegister.value()); },
        [](std::monostate const&){ Fatal("Empty RSI operand used!"); return std::string();},
    }, op);
}

std::string rsiToAarch64(RSI::Function const& function){
    std::string result = "";

    const auto translateOperandAarch64 = [&](RSI::Operand const& op){return translateOperand(op, aarch64RegistersRegisterTranslation, "#");};

    for (auto const& instr : function.instructions){
        switch(instr.type){
            case RSI::InstructionType::ADD:
                if (!std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister.has_value()){ break; }
                result += "add " + translateOperandAarch64(instr.result) + ", " + translateOperandAarch64(instr.op1) + ", " + translateOperandAarch64(instr.op2) + "\n";
                break;
            case RSI::InstructionType::SUBTRACT:
                if (!std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister.has_value()){ break; }
                result += "sub " + translateOperandAarch64(instr.result) + ", " + translateOperandAarch64(instr.op1) + ", " + translateOperandAarch64(instr.op2) + "\n";
                break;
            case RSI::InstructionType::MULTIPLY:
                if (!std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister.has_value()){ break; }
                result += "mul " + translateOperandAarch64(instr.result) + ", " + translateOperandAarch64(instr.op1) + ", " + translateOperandAarch64(instr.op2) + "\n";
                break;
            case RSI::InstructionType::DIVIDE:
                if (!std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister.has_value()){ break; }
                result += "sdiv " + translateOperandAarch64(instr.result) + ", " + translateOperandAarch64(instr.op1) + ", " + translateOperandAarch64(instr.op2) + "\n";
                break;
            case RSI::InstructionType::NEGATE:
                if (!std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister.has_value()){ break; }
                result += "neg " + translateOperandAarch64(instr.result) + ", " + translateOperandAarch64(instr.op1) + "\n";
                break;
            case RSI::InstructionType::BINARY_NOT:
                if (!std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister.has_value()){ break; }
                result += "mvn " + translateOperandAarch64(instr.result) + ", " + translateOperandAarch64(instr.op1) + "\n";
                break;
            case RSI::InstructionType::EQUAL:
                if (!std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister.has_value()){ break; }
                result += "cmp " + translateOperandAarch64(instr.op1) + ", " + translateOperandAarch64(instr.op2) + "\n";
                result += "cset " + translateOperandAarch64(instr.result) + ", eq\n";
                break;
            case RSI::InstructionType::NOT_EQUAL:
                if (!std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister.has_value()){ break; }
                result += "cmp " + translateOperandAarch64(instr.op1) + ", " + translateOperandAarch64(instr.op2) + "\n";
                result += "cset " + translateOperandAarch64(instr.result) + ", ne\n";
                break;
            case RSI::InstructionType::LESS_THAN:
                if (!std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister.has_value()){ break; }
                result += "cmp " + translateOperandAarch64(instr.op1) + ", " + translateOperandAarch64(instr.op2) + "\n";
                result += "cset " + translateOperandAarch64(instr.result) + ", lt\n";
                break;
            case RSI::InstructionType::LESS_THAN_OR_EQUAL:
                if (!std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister.has_value()){ break; }
                result += "cmp " + translateOperandAarch64(instr.op1) + ", " + translateOperandAarch64(instr.op2) + "\n";
                result += "cset " + translateOperandAarch64(instr.result) + ", le\n";
                break;
            case RSI::InstructionType::GREATER_THAN:
                if (!std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister.has_value()){ break; }
                result += "cmp " + translateOperandAarch64(instr.op1) + ", " + translateOperandAarch64(instr.op2) + "\n";
                result += "cset " + translateOperandAarch64(instr.result) + ", gt\n";
                break;
            case RSI::InstructionType::GREATER_THAN_OR_EQUAL:
                if (!std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister.has_value()){ break; }
                result += "cmp " + translateOperandAarch64(instr.op1) + ", " + translateOperandAarch64(instr.op2) + "\n";
                result += "cset " + translateOperandAarch64(instr.result) + ", ge\n";
                break;
            case RSI::InstructionType::MOVE:
                if (!std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister.has_value()){ break; }
                if (std::holds_alternative<RSI::Constant>(instr.op1)){
                    uint64_t valueCopy = static_cast<uint64_t>(std::get<RSI::Constant>(instr.op1).value);
                    if (valueCopy == 0){
                        result += "movz " + translateOperandAarch64(instr.result) + ", 0\n";
                    }
                    else{
                        for (int shiftAmount = 0; valueCopy >> shiftAmount && shiftAmount < 64; shiftAmount += 16){
                            if (shiftAmount == 0){
                                result += "movz " + translateOperandAarch64(instr.result) + ", " + std::to_string((valueCopy >> shiftAmount) & 0xFFFF) + "\n";
                            }
                            else{
                                result += "movk " + translateOperandAarch64(instr.result) + ", " + std::to_string((valueCopy >> shiftAmount) & 0xFFFF) + ", lsl " + std::to_string(shiftAmount) + "\n";
                            }
                        }
                    }
                }
                else{
                    result += "mov " + translateOperandAarch64(instr.result) + ", " + translateOperandAarch64(instr.op1) + "\n";
                }
                break;
            case RSI::InstructionType::RETURN:
                result += "mov x0, " + translateOperandAarch64(instr.op1) + "\n";
                result += "ret\n";
                break;
            case RSI::InstructionType::LOGICAL_NOT:
                if (!std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister.has_value()){ break; }
                result += "cmp " + translateOperandAarch64(instr.op1) + ", 0\n";
                result += "cset " + translateOperandAarch64(instr.result) + ", eq\n";
                break;

            case RSI::InstructionType::NOP:
                break;

            default:
                Fatal("Unimplemented RSI instrution for aarch64.");
                break;
        }
    }

    return result;
}


std::string rsiToNasm(RSI::Function const& function){
    std::string result = "";

    const auto translateOperandNasm = [&](RSI::Operand const& op){return translateOperand(op, nasmRegisterTranslation, "");};

    for (auto const& instr : function.instructions){
        try{
            if (!std::holds_alternative<std::monostate>(instr.op2) && std::get<std::shared_ptr<RSI::Reference>>(instr.result) != std::get<std::shared_ptr<RSI::Reference>>(instr.op1)){
                Fatal("RSI instruction is not nasm compatible. (result and op1 are different)");
            }
        }
        catch(std::bad_variant_access){}

        switch(instr.type){
            case RSI::InstructionType::ADD:
                if (!std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister.has_value()){ break; }
                result += "add " + translateOperandNasm(instr.result) + ", " + translateOperandNasm(instr.op2) + "\n";
                break;
            case RSI::InstructionType::SUBTRACT:
                if (!std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister.has_value()){ break; }
                result += "sub " + translateOperandNasm(instr.result) + ", " + translateOperandNasm(instr.op2) + "\n";
                break;
            case RSI::InstructionType::MULTIPLY:
                if (!std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister.has_value()){ break; }
                result += "push rax\n";
                result += "push rdx\n";
                result += "mov rax, " + translateOperandNasm(instr.op1) + "\n";
                result += "imul " + translateOperandNasm(instr.op2) + "\n";
                result += "mov " + translateOperandNasm(instr.result) + ", rax\n";

                if (std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister != nasmRegisters.at(static_cast<int>(NasmRegisters::RDX)))
                    result += "pop rdx\n";
                else
                    result += "add rsp, 8\n";

                if (std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister != nasmRegisters.at(static_cast<int>(NasmRegisters::RAX)))
                    result += "pop rax\n";
                else
                    result += "add rsp, 8\n";

                break;
            case RSI::InstructionType::DIVIDE:
                if (!std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister.has_value()){ break; }
                
                if (std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister != nasmRegisters.at(static_cast<int>(NasmRegisters::RAX)))
                    result += "push rax\n";
                if (std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister != nasmRegisters.at(static_cast<int>(NasmRegisters::RDX)))
                    result += "push rdx\n";

                result += "mov rax, " + translateOperandNasm(instr.op1) + "\n";
                result += "cqo\n";
                result += "idiv " + translateOperandNasm(instr.op2) + "\n";
                result += "mov " + translateOperandNasm(instr.result) + ", rax\n";

                if (std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister != nasmRegisters.at(static_cast<int>(NasmRegisters::RDX)))
                    result += "pop rdx\n";

                if (std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister != nasmRegisters.at(static_cast<int>(NasmRegisters::RAX)))
                    result += "pop rax\n";

                break;
            case RSI::InstructionType::MODULO:
                if (!std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister.has_value()){ break; }
                
                if (std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister != nasmRegisters.at(static_cast<int>(NasmRegisters::RAX)))
                    result += "push rax\n";
                if (std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister != nasmRegisters.at(static_cast<int>(NasmRegisters::RDX)))
                    result += "push rdx\n";

                result += "mov rax, " + translateOperandNasm(instr.op1) + "\n";
                result += "cqo\n";
                result += "idiv " + translateOperandNasm(instr.op2) + "\n";
                result += "mov " + translateOperandNasm(instr.result) + ", rdx\n";

                if (std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister != nasmRegisters.at(static_cast<int>(NasmRegisters::RDX)))
                    result += "pop rdx\n";

                if (std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister != nasmRegisters.at(static_cast<int>(NasmRegisters::RAX)))
                    result += "pop rax\n";

                break;
            case RSI::InstructionType::NEGATE:
                if (!std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister.has_value()){ break; }
                result += "neg " + translateOperandNasm(instr.result) + "\n";
                break;
            case RSI::InstructionType::BINARY_NOT:
                if (!std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister.has_value()){ break; }
                result += "not " + translateOperandNasm(instr.result) + "\n";
                break;
            
            case RSI::InstructionType::EQUAL:
                if (!std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister.has_value()){ break; }
                result += "cmp " + translateOperandNasm(instr.op1) + ", " + translateOperandNasm(instr.op2) + "\n";
                result += "sete " + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 1)) + "\n";
                result += "movzx " + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 4)) + ", " + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 1)) + "\n";
                break;
            case RSI::InstructionType::NOT_EQUAL:
                if (!std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister.has_value()){ break; }
                result += "cmp " + translateOperandNasm(instr.op1) + ", " + translateOperandNasm(instr.op2) + "\n";
                result += "setne " + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 1)) + "\n";
                result += "movzx " + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 4)) + ", " + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 1)) + "\n";
                break;
            case RSI::InstructionType::LESS_THAN:
                if (!std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister.has_value()){ break; }
                result += "cmp " + translateOperandNasm(instr.op1) + ", " + translateOperandNasm(instr.op2) + "\n";
                result += "setl " + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 1)) + "\n";
                result += "movzx " + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 4)) + ", " + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 1)) + "\n";
                break;
            case RSI::InstructionType::LESS_THAN_OR_EQUAL:
                if (!std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister.has_value()){ break; }
                result += "cmp " + translateOperandNasm(instr.op1) + ", " + translateOperandNasm(instr.op2) + "\n";
                result += "setle " + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 1)) + "\n";
                result += "movzx " + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 4)) + ", " + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 1)) + "\n";
                break;
            case RSI::InstructionType::GREATER_THAN:
                if (!std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister.has_value()){ break; }
                result += "cmp " + translateOperandNasm(instr.op1) + ", " + translateOperandNasm(instr.op2) + "\n";
                result += "setg " + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 1)) + "\n";
                result += "movzx " + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 4)) + ", " + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 1)) + "\n";
                break;
            case RSI::InstructionType::GREATER_THAN_OR_EQUAL:
                if (!std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister.has_value()){ break; }
                result += "cmp " + translateOperandNasm(instr.op1) + ", " + translateOperandNasm(instr.op2) + "\n";
                result += "setge " + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 1)) + "\n";
                result += "movzx " + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 4)) + ", " + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 1)) + "\n";
                break;
            case RSI::InstructionType::MOVE:
                if (!std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister.has_value()){ break; }
                result += "mov " + translateOperandNasm(instr.result) + ", " + translateOperandNasm(instr.op1) + "\n";
                break;
            case RSI::InstructionType::RETURN:
                result += "mov rax, " + translateOperandNasm(instr.op1) + "\n";
                result += "ret\n";
                break;
            case RSI::InstructionType::LOGICAL_NOT:
                result += "cmp " + translateOperandNasm(instr.op1) + ", 0\n";
                result += "mov " + translateOperandNasm(instr.result) + ", 0\n";
                result += "sete " + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 1)) + "\n";
                break;

            case RSI::InstructionType::NOP:
                break;

            default:
                Fatal("Unimplemented RSI instrution for nasm.");
                break;
        }
    }

    return result;
}

int main(int argc, const char** argv) {
    std::string inputFilename;
    std::string outputFilename = "a.out";
    OutputFormat outputFormat = OutputFormat::C;
    std::string compiler = "gcc";
    std::vector<std::string> additionalyLinkedFiles;
    std::string stdlibIncludePath = std::filesystem::path(argv[0]).replace_filename("stdlib/");

    if (argc < 2) {
        printHelp(argv[0]);
        return static_cast<int>(ReturnValue::UnknownError);
    }

    for (int i=1; i<argc; i++){
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            printHelp(argv[0]);
            return static_cast<int>(ReturnValue::NormalExit);
        }
        else if (arg == "-o" || arg == "--output") {
            if (i+1 < argc) {
                outputFilename = argv[i+1];
                i++;
            } else {
                Error("Missing output file");
                return static_cast<int>(ReturnValue::UnknownError);
            }
        }
        else if (arg == "--link") {
            if (i+1 < argc) {
                additionalyLinkedFiles.push_back(argv[i+1]);
                i++;
            } else {
                Error("Missing file to link");
                return static_cast<int>(ReturnValue::UnknownError);
            }
        }
        else if (arg == "-f" || arg == "--format") {
            if (i+1 < argc) {
                std::string format = argv[i+1];
                if (format == "c") {
                    outputFormat = OutputFormat::C;
                }
                else if (format == "nasm") {
                    outputFormat = OutputFormat::NASM;
                }
                else if (format == "aarch64") {
                    outputFormat = OutputFormat::AArch64;
                }
                else if (format == "rsi_nasm") {
                    outputFormat = OutputFormat::RSI_NASM;
                }
                else if (format == "rsi_aarch64") {
                    outputFormat = OutputFormat::RSI_AArch64;
                }
                else {
                    Error("Unknown output format \"" + format + "\"");
                    return static_cast<int>(ReturnValue::UnknownError);
                }
                i++;
            } else {
                Error("Missing output format");
                return static_cast<int>(ReturnValue::UnknownError);
            }
        }
        else if (arg == "--compiler"){
            if (i+1 < argc) {
                compiler = argv[++i];
            } else {
                Error("Missing compiler path");
                return static_cast<int>(ReturnValue::UnknownError);
            }
        }
        else if (arg == "--stdlib"){
            if (i+1 < argc) {
                stdlibIncludePath = argv[++i];
            } else {
                Error("Missing standard library path");
                return static_cast<int>(ReturnValue::UnknownError);
            }
        }
        else {
            // test if it is a filename
            if (std::filesystem::exists(arg)) {
                inputFilename = arg;
            } else {
                Error("Invalid argument: ", arg);
                return static_cast<int>(ReturnValue::UnknownError);
            }
        }
    }

    std::vector<Token> tokens;
    std::shared_ptr<AstProgram> ast;
    std::string outputSource;
    std::string R_Sharp_Source;

    Print("--------------| Tokenizing |--------------");
    {
        Tokenizer tokenizer(inputFilename);
        tokens = tokenizer.tokenize();
        R_Sharp_Source = tokenizer.getSource();

        for (auto const& token : tokens){
            Print(token.toString());
        }

        tokens = cleanTokens(tokens);
    }

    Print("--------------| Parsing |--------------");
    {
        ParsingCache cache;
        Parser parser = Parser(tokens, inputFilename, stdlibIncludePath, cache);
        ast = parser.parse();

        Print("--------------| Syntax Errors |--------------");
        if (parser.hasErrors()) {
            ErrorPrinter printer(ast, inputFilename, R_Sharp_Source);
            printer.print();
            Error("Parsing errors.");
            return static_cast<int>(ReturnValue::SyntaxError);
        }
        else{
            Print("No errors"); 
        }
    }
    Print("--------------| Raw AST |--------------");
    {
        AstPrinter printer(ast);
        printer.print();
    }
    
    Print("--------------| Semantic Errors |--------------");
    {
        SemanticValidator validator(ast, inputFilename, R_Sharp_Source);
        validator.validate();

        if (validator.hasErrors()) {
            Error("Semantic errors.");
            return static_cast<int>(ReturnValue::SemanticError);
        }
        else{
            Print("No errors"); 
        }
    }
    Print("--------------| Typed AST |--------------");
    {
        AstPrinter printer(ast);
        printer.print();
    }


    std::vector<RSI::Function> ir;
    Print("--------------| Generated code |--------------");
    {
        switch(outputFormat) {
            case OutputFormat::C:
                outputSource = CCodeGenerator(ast).generate();
                break;
            case OutputFormat::NASM:
                outputSource = NASMCodeGenerator(ast, R_Sharp_Source).generate();
                break;
            case OutputFormat::AArch64:
                outputSource = AArch64CodeGenerator(ast, R_Sharp_Source).generate();
                break;
            case OutputFormat::RSI_NASM:
            case OutputFormat::RSI_AArch64:
                ir = RSIGenerator(ast, R_Sharp_Source).generate();
                break;
        }
        if (outputSource.length())
            Print(outputSource);
        else{
            auto const& registerTranslation = outputFormat == OutputFormat::RSI_NASM ? nasmRegisterTranslation : aarch64RegistersRegisterTranslation;
            auto const& allRegisters = outputFormat == OutputFormat::RSI_NASM ? nasmRegisters : aarch64Registers;

            Print("--------------| Raw RSI |--------------");
            for (auto& func : ir){
                Print("; Function \"", func.name, "\"");
                Print(RSI::stringify_function(func, registerTranslation));
            }
            if (outputFormat == OutputFormat::RSI_NASM){
                Print("--------------| Two Operand Compatibility |--------------");
                for (auto& func : ir){
                    Print("; Function \"", func.name, "\"");
                    RSI::makeTwoOperandCompatible(func);
                    Print(RSI::stringify_function(func, registerTranslation));
                }
            }
            if (outputFormat == OutputFormat::RSI_AArch64){
                Print("--------------| Replace modulo with div, mul, sub |--------------");
                for (auto& func : ir){
                    Print("; Function \"", func.name, "\"");
                    RSI::replaceModWithDivMulSub(func);
                    Print(RSI::stringify_function(func, registerTranslation));
                }
            }
            Print("--------------| Liveness analysis |--------------");
            for (auto& func : ir){
                RSI::analyzeLiveVariables(func);
                Print("; Function \"", func.name, "\"");
                Print(RSI::stringify_function(func, registerTranslation));
            }
            {
                bool hasError = false;
                for (auto const& func : ir){
                    if (func.instructions.at(0).meta.liveVariablesAfter.size() != 0){
                        Error("Function \"", func.name, "\" requires live variables before main code. This probably means some transformation is incorrect.");
                        hasError = true;
                    }
                }
                if (hasError){
                    return static_cast<int>(ReturnValue::UnknownError);
                }
            }
            Print("--------------| Linear scan register assignment |--------------");
            for (auto& func : ir){
                RSI::assignRegistersLinearScan(ir.at(0), allRegisters);
                Print("; Function \"", func.name, "\"");
                Print(RSI::stringify_function(func, registerTranslation));
            }
            Print("--------------| RSI to assembly |--------------");
            if (outputFormat == OutputFormat::RSI_NASM){
                outputSource =
R"(; NASM code generated by R-Sharp compiler (using RSI)

BITS 64
section .text

)";
                for (auto& func : ir){
                    outputSource += "global " + func.name + "\n" + func.name + ":\n";
                    outputSource += rsiToNasm(func) + "\n";
                }
                outputFormat = OutputFormat::NASM;
            }
            else{
                outputSource =
R"(// Aarch64 code generated by R-Sharp compiler (using RSI)

.text

)";
                for (auto& func : ir){
                    outputSource += ".global " + func.name + "\n" + func.name + ":\n";
                    outputSource += rsiToAarch64(func) + "\n";
                }
                outputFormat = OutputFormat::AArch64;
            }
            Print(outputSource);
        }
    }
    std::string temporaryFile = outputFilename;
    switch(outputFormat) {
        case OutputFormat::C:
            temporaryFile += ".c";
            break;
        case OutputFormat::NASM:
            temporaryFile += ".asm";
            break;
        case OutputFormat::AArch64:
            temporaryFile += ".S";
            break;
        default:
            Error("Unknown output format");
            return static_cast<int>(ReturnValue::UnknownError);
            break;
    }

    Print("Writing to file: ", temporaryFile);
    std::ofstream outputFile(temporaryFile);
    if (outputFile.is_open()) {
        outputFile << outputSource;
        outputFile.close();
    } else {
        Error("Could not open file: ", temporaryFile);
        return static_cast<int>(ReturnValue::UnknownError);
    }

    std::string additionalyLinkedFiles_str;
    for (auto file : additionalyLinkedFiles){
        additionalyLinkedFiles_str += file + " ";
    }

    const std::string gccArgumentsCompile = "-g -Werror -Wall -Wno-unused-variable -Wno-unused-value -Wno-unused-but-set-variable -Wno-unused-function";
    const std::string gccArgumentsLink = "-g -Werror -Wall -no-pie ";
    const std::string nasmArgumentsCompile = "-g -w+error -w+all";

    switch(outputFormat) {
        case OutputFormat::C:{
            Print("--------------| Compiling using gcc |--------------");
            std::string command = compiler + " " + gccArgumentsCompile + " " + temporaryFile + " " + additionalyLinkedFiles_str + " -o " + outputFilename;
            Print("Executing: ", command);
            int success = !system(command.c_str());
            if (success)
                Print("Compilation successful.");
            else{
                Error("Compilation failed.");
                return static_cast<int>(ReturnValue::AssemblingError);
            }
            break;
        }
        case OutputFormat::AArch64:{
            Print("--------------| Compiling using gcc |--------------");
            std::string command = compiler + " " + gccArgumentsCompile + " " + temporaryFile + " " + additionalyLinkedFiles_str + " -o " + outputFilename;
            Print("Executing: ", command);
            int success = !system(command.c_str());
            if (success)
                Print("Compilation successful.");
            else{
                Error("Compilation failed.");
                return static_cast<int>(ReturnValue::AssemblingError);
            }
            break;
        }
        case OutputFormat::NASM:{
            Print("--------------| Assembling using nasm |--------------");
            std::string command = "nasm " + nasmArgumentsCompile + " -f elf64 " + temporaryFile + " -o " + outputFilename + ".o";
            Print("Executing: ", command);
            int success = !system(command.c_str());
            if (success)
                Print("Assembling successful.");
            else{
                Error("Assembling failed.");
                return static_cast<int>(ReturnValue::AssemblingError);
            }

            Print("--------------| Linking using gcc |--------------");
            command = compiler + " " + gccArgumentsLink + " " + outputFilename + ".o " + additionalyLinkedFiles_str + " -o " + outputFilename;
            Print("Executing: ", command);
            success = !system(command.c_str());
            if (success)
                Print("Linking successful.");
            else{
                Error("Linking failed.");
                return static_cast<int>(ReturnValue::AssemblingError);
            }
            
            break;
        }
        default:
            Error("Unsupported output format");
            return static_cast<int>(ReturnValue::UnknownError);
            break;
    }

    return static_cast<int>(ReturnValue::NormalExit);
}