#include "R-Sharp/RSIToAssembly.hpp"
#include "R-Sharp/RSI.hpp"
#include "R-Sharp/Architecture.hpp"

#include "R-Sharp/Utils/ContainerTools.hpp"
#include "R-Sharp/Utils/LambdaOverload.hpp"

std::string translateOperand(RSI::Operand const& op, Architecture const& arch, std::string constantPrefix) {
    return std::visit(
        lambda_overload{
            [&](RSI::Constant const& x) { return constantPrefix + std::to_string(x.value); },
            [&](RSI::DynamicConstant const& x) {
                if (x.value == nullptr) {
                    Fatal("Dynamic constrant wasn't resolved.");
                }
                return constantPrefix + std::to_string(*x.value);
            },
            [&](std::shared_ptr<RSI::Reference> x) {
                return std::visit(
                    lambda_overload{
                        [&](RSI::HWRegister reg) -> std::string { return arch.registerTranslation.at(reg); },
                        [](std::monostate) -> std::string { return "(none)"; },
                        [&](RSI::StackSlot slot) -> std::string {
                            return "[" + arch.registerTranslation.at(arch.stackPointerRegister) + "+"
                                 + std::to_string(slot.offset) + "]";
                        },
                    },
                    x->storageLocation
                );
            },
            [](std::shared_ptr<RSI::GlobalReference> x) { return x->name; },
            [](std::shared_ptr<RSI::Label> x) { return x->name; },
            [](std::monostate const&) {
                Fatal("Empty RSI operand used!");
                return std::string();
            },
        },
        op
    );
}

#define ENSURE_RESULT(instr)                                                                                                  \
    do {                                                                                                                      \
        if (std::holds_alternative<std::monostate>(std::get<std::shared_ptr<RSI::Reference>>(instr.result)->storageLocation)) \
            break;                                                                                                            \
    } while (false)

std::string rsiToAarch64(RSI::Function const& function) {
    std::string result = "";

    const auto translateOperandAarch64 = [&](RSI::Operand const& op) {
        return translateOperand(op, aarch64, "#");
    };

    for (auto instr_it = function.instructions.begin(); instr_it != function.instructions.end(); instr_it++) {
        RSI::Instruction const& instr = *instr_it;
        std::optional<std::reference_wrapper<const RSI::Instruction>> next_instr;
        if (instr_it + 1 != function.instructions.end()) {
            next_instr = *(instr_it + 1);
        }

        switch (instr.type) {
            case RSI::InstructionType::ADD:
                ENSURE_RESULT(instr);

                result += "add " + translateOperandAarch64(instr.result) + ", "
                        + translateOperandAarch64(instr.op1) + ", " + translateOperandAarch64(instr.op2)
                        + "\n";
                break;
            case RSI::InstructionType::SUBTRACT:
                ENSURE_RESULT(instr);

                result += "sub " + translateOperandAarch64(instr.result) + ", "
                        + translateOperandAarch64(instr.op1) + ", " + translateOperandAarch64(instr.op2)
                        + "\n";
                break;
            case RSI::InstructionType::MULTIPLY:
                ENSURE_RESULT(instr);

                result += "mul " + translateOperandAarch64(instr.result) + ", "
                        + translateOperandAarch64(instr.op1) + ", " + translateOperandAarch64(instr.op2)
                        + "\n";
                break;
            case RSI::InstructionType::DIVIDE:
                ENSURE_RESULT(instr);

                result += "sdiv " + translateOperandAarch64(instr.result) + ", "
                        + translateOperandAarch64(instr.op1) + ", " + translateOperandAarch64(instr.op2)
                        + "\n";
                break;
            case RSI::InstructionType::NEGATE:
                ENSURE_RESULT(instr);

                result += "neg " + translateOperandAarch64(instr.result) + ", "
                        + translateOperandAarch64(instr.op1) + "\n";
                break;
            case RSI::InstructionType::BINARY_NOT:
                ENSURE_RESULT(instr);

                result += "mvn " + translateOperandAarch64(instr.result) + ", "
                        + translateOperandAarch64(instr.op1) + "\n";
                break;
            case RSI::InstructionType::EQUAL:
                ENSURE_RESULT(instr);

                result += "cmp " + translateOperandAarch64(instr.op1) + ", "
                        + translateOperandAarch64(instr.op2) + "\n";
                result += "cset " + translateOperandAarch64(instr.result) + ", eq\n";
                break;
            case RSI::InstructionType::NOT_EQUAL:
                ENSURE_RESULT(instr);

                result += "cmp " + translateOperandAarch64(instr.op1) + ", "
                        + translateOperandAarch64(instr.op2) + "\n";
                result += "cset " + translateOperandAarch64(instr.result) + ", ne\n";
                break;
            case RSI::InstructionType::LESS_THAN:
                ENSURE_RESULT(instr);

                result += "cmp " + translateOperandAarch64(instr.op1) + ", "
                        + translateOperandAarch64(instr.op2) + "\n";
                result += "cset " + translateOperandAarch64(instr.result) + ", lt\n";
                break;
            case RSI::InstructionType::LESS_THAN_OR_EQUAL:
                ENSURE_RESULT(instr);

                result += "cmp " + translateOperandAarch64(instr.op1) + ", "
                        + translateOperandAarch64(instr.op2) + "\n";
                result += "cset " + translateOperandAarch64(instr.result) + ", le\n";
                break;
            case RSI::InstructionType::GREATER_THAN:
                ENSURE_RESULT(instr);

                result += "cmp " + translateOperandAarch64(instr.op1) + ", "
                        + translateOperandAarch64(instr.op2) + "\n";
                result += "cset " + translateOperandAarch64(instr.result) + ", gt\n";
                break;
            case RSI::InstructionType::GREATER_THAN_OR_EQUAL:
                ENSURE_RESULT(instr);

                result += "cmp " + translateOperandAarch64(instr.op1) + ", "
                        + translateOperandAarch64(instr.op2) + "\n";
                result += "cset " + translateOperandAarch64(instr.result) + ", ge\n";
                break;
            case RSI::InstructionType::MOVE:
                if (std::holds_alternative<RSI::Constant>(instr.op1)) {
                    uint64_t valueCopy = static_cast<uint64_t>(std::get<RSI::Constant>(instr.op1).value);
                    if (valueCopy == 0) {
                        result += "movz " + translateOperandAarch64(instr.result) + ", 0\n";
                    }
                    else {
                        for (int shiftAmount = 0; valueCopy >> shiftAmount && shiftAmount < 64; shiftAmount += 16) {
                            if (shiftAmount == 0) {
                                result += "movz " + translateOperandAarch64(instr.result) + ", "
                                        + std::to_string((valueCopy >> shiftAmount) & 0xFFFF) + "\n";
                            }
                            else {
                                result += "movk " + translateOperandAarch64(instr.result) + ", "
                                        + std::to_string((valueCopy >> shiftAmount) & 0xFFFF) + ", lsl "
                                        + std::to_string(shiftAmount) + "\n";
                            }
                        }
                    }
                }
                else if (std::holds_alternative<std::shared_ptr<RSI::Reference>>(instr.result) && std::holds_alternative<std::shared_ptr<RSI::GlobalReference>>(instr.op1)){
                    result += "ldr " + translateOperandAarch64(instr.result)
                            + ", =" + translateOperandAarch64(instr.op1) + "\n";
                }
                else if (std::holds_alternative<std::shared_ptr<RSI::GlobalReference>>(instr.result) && std::holds_alternative<std::shared_ptr<RSI::Reference>>(instr.op1)){
                    Fatal("Invalid move instruction. Use STORE_MEMORY to assign to global.");
                }
                else {
                    result += "mov " + translateOperandAarch64(instr.result) + ", "
                            + translateOperandAarch64(instr.op1) + "\n";
                }
                break;
            case RSI::InstructionType::STORE_MEMORY:
                result += "str " + translateOperandAarch64(instr.op2) + ", ["
                        + translateOperandAarch64(instr.op1) + "]\n";
                break;
            case RSI::InstructionType::LOAD_MEMORY:
                result += "ldr " + translateOperandAarch64(instr.result) + ", ["
                        + translateOperandAarch64(instr.op1) + "]\n";
                break;
            case RSI::InstructionType::RETURN:
                result += "mov x0, " + translateOperandAarch64(instr.op1) + "\n";

                result += "add sp, sp, " + std::to_string(function.meta.maxStackUsage) + "\n";

                // restore callee saved regs
                for (auto reg_it = function.meta.allRegisters.rbegin(); reg_it != function.meta.allRegisters.rend();
                     reg_it++) {
                    if (ContainerTools::contains(aarch64.calleeSavedRegisters, *reg_it)) {
                        result += "pop " + aarch64.registerTranslation.at(*reg_it) + "\n";
                    }
                }

                result += "ret\n";
                break;
            case RSI::InstructionType::LOGICAL_NOT:
                ENSURE_RESULT(instr);
                result += "cmp " + translateOperandAarch64(instr.op1) + ", 0\n";
                result += "cset " + translateOperandAarch64(instr.result) + ", eq\n";
                break;

            case RSI::InstructionType::NOP: break;
            case RSI::InstructionType::DEFINE_LABEL:
                result += std::get<std::shared_ptr<RSI::Label>>(instr.op1)->name + ":\n";
                break;
            case RSI::InstructionType::JUMP:
                result += "b " + std::get<std::shared_ptr<RSI::Label>>(instr.op1)->name + "\n";
                break;
            case RSI::InstructionType::JUMP_IF_ZERO:
                result += "cbz " + translateOperandAarch64(instr.op1) + ", "
                        + std::get<std::shared_ptr<RSI::Label>>(instr.op2)->name + "\n";
                break;
            case RSI::InstructionType::STORE_PARAMETER:
                result += "push " + translateOperandAarch64(instr.op1) + "\n";
                break;
            case RSI::InstructionType::LOAD_PARAMETER: break;
            case RSI::InstructionType::CALL:           {
                if (!std::holds_alternative<RSI::Constant>(instr.op2))
                    Fatal("call instruction has non constant number of arguments.");

                auto regsToPreserve = next_instr.has_value() ? next_instr.value().get().meta.liveVariablesBefore
                                                             : std::set<std::shared_ptr<RSI::Reference>>();
                regsToPreserve.erase(std::get<std::shared_ptr<RSI::Reference>>(instr.result));

                // don't save callee saved registers
                for (auto reg_it = regsToPreserve.begin(); reg_it != regsToPreserve.end();) {
                    if (std::holds_alternative<RSI::HWRegister>((*reg_it)->storageLocation)) {
                        auto hwreg = std::get<RSI::HWRegister>((*reg_it)->storageLocation);
                        if (ContainerTools::contains(aarch64.calleeSavedRegisters, hwreg)
                            || hwreg == aarch64.returnValueRegister) {
                            reg_it = regsToPreserve.erase(reg_it);
                            continue;
                        }
                    }

                    reg_it++;
                }

                // save registers
                for (auto reg : regsToPreserve) {
                    if (std::holds_alternative<RSI::HWRegister>(reg->storageLocation)) {
                        result += "push " + aarch64.registerTranslation.at(std::get<RSI::HWRegister>(reg->storageLocation))
                                + "\n";
                    }
                }

                const std::vector<RSI::HWRegister> usedParameterRegs(
                    aarch64.parameterRegisters.begin(),
                    aarch64.parameterRegisters.begin() + std::get<RSI::Constant>(instr.op2).value
                );

                constexpr int pushSize = 16;

                if (usedParameterRegs.size()) {
                    int stackOffset = regsToPreserve.size() * pushSize;
                    for (auto it = usedParameterRegs.rbegin(); it != usedParameterRegs.rend(); it++) {
                        std::string currentParameterRegister = aarch64.registerTranslation.at(*it);
                        result += "ldr " + currentParameterRegister + ", [sp, " + std::to_string(stackOffset)
                                + "]\n";
                        stackOffset += pushSize;
                    }
                }
                result += "stp fp, lr, [sp, -16]!\n";
                result += "mov fp, sp\n";
                result += "bl " + std::get<std::shared_ptr<RSI::Label>>(instr.op1)->name + "\n";
                result += "ldp fp, lr, [sp], 16\n";

                // restore registers
                for (auto reg_it = regsToPreserve.rbegin(); reg_it != regsToPreserve.rend(); reg_it++) {
                    auto reg = *reg_it;
                    if (std::holds_alternative<RSI::HWRegister>(reg->storageLocation)) {
                        result += "pop " + aarch64.registerTranslation.at(std::get<RSI::HWRegister>(reg->storageLocation))
                                + "\n";
                    }
                }

                // reclaim parameters
                result += "add sp, sp, " + std::to_string(usedParameterRegs.size() * pushSize) + "\n";

                break;
            }
            case RSI::InstructionType::FUNCTION_BEGIN:
                // save callee saved regs
                for (auto reg : function.meta.allRegisters) {
                    if (ContainerTools::contains(aarch64.calleeSavedRegisters, reg)) {
                        result += "push " + aarch64.registerTranslation.at(reg) + "\n";
                    }
                }
                result += "sub sp, sp, " + std::to_string(function.meta.maxStackUsage) + "\n";

                break;
            case RSI::InstructionType::SET_LIVE: break;
            default:
                Fatal("Unimplemented RSI instruction for aarch64. (", RSI::mnemonics.at(instr.type), ")");
                break;
        }
    }

    return result;
}

bool isRegister(RSI::Operand const& op, NasmRegisters reg) {
    auto const& loc = std::get<std::shared_ptr<RSI::Reference>>(op)->storageLocation;

    return std::holds_alternative<RSI::HWRegister>(loc)
        && std::get<RSI::HWRegister>(loc) == x86_64.allRegisters.at(static_cast<int>(reg));
}

std::string rsiToNasm(RSI::Function const& function) {
    std::string result = "";

    const auto translateOperandNasm = [&](RSI::Operand const& op) { return translateOperand(op, x86_64, ""); };

    for (auto instr_it = function.instructions.begin(); instr_it != function.instructions.end(); instr_it++) {
        RSI::Instruction const& instr = *instr_it;
        std::optional<std::reference_wrapper<const RSI::Instruction>> next_instr;
        if (instr_it + 1 != function.instructions.end()) {
            next_instr = *(instr_it + 1);
        }

        try {
            if (!std::holds_alternative<std::monostate>(instr.op2)
                && std::get<std::shared_ptr<RSI::Reference>>(instr.result)
                       != std::get<std::shared_ptr<RSI::Reference>>(instr.op1)) {
                Fatal("RSI instruction is not nasm compatible. (result and op1 are different)");
            }
        }
        catch (std::bad_variant_access) {
        }

        switch (instr.type) {
            case RSI::InstructionType::ADD:
                ENSURE_RESULT(instr);
                result += "add " + translateOperandNasm(instr.result) + ", " + translateOperandNasm(instr.op2)
                        + "\n";
                break;
            case RSI::InstructionType::SUBTRACT:
                ENSURE_RESULT(instr);
                result += "sub " + translateOperandNasm(instr.result) + ", " + translateOperandNasm(instr.op2)
                        + "\n";
                break;
            case RSI::InstructionType::MULTIPLY:
                ENSURE_RESULT(instr);
                result += "push rax\n";
                result += "push rdx\n";
                if (isRegister(instr.op2, NasmRegisters::RAX)) {
                    result += "imul " + translateOperandNasm(instr.op1) + "\n";
                }
                else {
                    result += "mov rax, " + translateOperandNasm(instr.op1) + "\n";
                    result += "imul " + translateOperandNasm(instr.op2) + "\n";
                }

                result += "mov " + translateOperandNasm(instr.result) + ", rax\n";

                if (!isRegister(instr.result, NasmRegisters::RDX)) {
                    result += "pop rdx\n";
                }
                else
                    result += "add rsp, 8\n";

                if (!isRegister(instr.result, NasmRegisters::RAX)) {
                    result += "pop rax\n";
                }
                else
                    result += "add rsp, 8\n";

                break;
            case RSI::InstructionType::DIVIDE:
                ENSURE_RESULT(instr);

                if (!isRegister(instr.result, NasmRegisters::RAX)) {
                    Fatal(
                        "The result of a division is not in RAX. Divisions may not have been isolated "
                        "correctly."
                    );
                }
                if (!isRegister(instr.op1, NasmRegisters::RAX)) {
                    Fatal(
                        "The first operand of a division is not in RAX. Divisions may not have been isolated "
                        "correctly."
                    );
                }

                result += "push rdx\n";
                result += "cqo\n";
                result += "idiv " + translateOperandNasm(instr.op2) + "\n";
                result += "pop rdx\n";

                break;
            case RSI::InstructionType::MODULO:
                ENSURE_RESULT(instr);

                if (!isRegister(instr.result, NasmRegisters::RAX)) result += "push rax\n";
                if (!isRegister(instr.result, NasmRegisters::RDX)) result += "push rdx\n";

                result += "mov rax, " + translateOperandNasm(instr.op1) + "\n";
                result += "cqo\n";
                result += "idiv " + translateOperandNasm(instr.op2) + "\n";
                result += "mov " + translateOperandNasm(instr.result) + ", rdx\n";

                if (!isRegister(instr.result, NasmRegisters::RDX)) result += "pop rdx\n";

                if (!isRegister(instr.result, NasmRegisters::RAX)) result += "pop rax\n";

                break;
            case RSI::InstructionType::NEGATE:
                ENSURE_RESULT(instr);
                result += "neg " + translateOperandNasm(instr.result) + "\n";
                break;
            case RSI::InstructionType::BINARY_NOT:
                ENSURE_RESULT(instr);
                result += "not " + translateOperandNasm(instr.result) + "\n";
                break;

            case RSI::InstructionType::EQUAL:
                ENSURE_RESULT(instr);
                result += "cmp " + translateOperandNasm(instr.op1) + ", " + translateOperandNasm(instr.op2)
                        + "\n";
                result += "sete " + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 1))
                        + "\n";
                result += "movzx "
                        + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 4)) + ", "
                        + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 1)) + "\n";
                break;
            case RSI::InstructionType::NOT_EQUAL:
                ENSURE_RESULT(instr);
                result += "cmp " + translateOperandNasm(instr.op1) + ", " + translateOperandNasm(instr.op2)
                        + "\n";
                result += "setne "
                        + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 1)) + "\n";
                result += "movzx "
                        + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 4)) + ", "
                        + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 1)) + "\n";
                break;
            case RSI::InstructionType::LESS_THAN:
                ENSURE_RESULT(instr);
                result += "cmp " + translateOperandNasm(instr.op1) + ", " + translateOperandNasm(instr.op2)
                        + "\n";
                result += "setl " + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 1))
                        + "\n";
                result += "movzx "
                        + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 4)) + ", "
                        + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 1)) + "\n";
                break;
            case RSI::InstructionType::LESS_THAN_OR_EQUAL:
                ENSURE_RESULT(instr);
                result += "cmp " + translateOperandNasm(instr.op1) + ", " + translateOperandNasm(instr.op2)
                        + "\n";
                result += "setle "
                        + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 1)) + "\n";
                result += "movzx "
                        + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 4)) + ", "
                        + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 1)) + "\n";
                break;
            case RSI::InstructionType::GREATER_THAN:
                ENSURE_RESULT(instr);
                result += "cmp " + translateOperandNasm(instr.op1) + ", " + translateOperandNasm(instr.op2)
                        + "\n";
                result += "setg " + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 1))
                        + "\n";
                result += "movzx "
                        + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 4)) + ", "
                        + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 1)) + "\n";
                break;
            case RSI::InstructionType::GREATER_THAN_OR_EQUAL:
                ENSURE_RESULT(instr);
                result += "cmp " + translateOperandNasm(instr.op1) + ", " + translateOperandNasm(instr.op2)
                        + "\n";
                result += "setge "
                        + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 1)) + "\n";
                result += "movzx "
                        + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 4)) + ", "
                        + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 1)) + "\n";
                break;
            case RSI::InstructionType::MOVE:
                result += "mov QWORD ";
                if (std::holds_alternative<std::shared_ptr<RSI::Reference>>(instr.result))
                    result += translateOperandNasm(instr.result);
                else if (std::holds_alternative<std::shared_ptr<RSI::GlobalReference>>(instr.result))
                    result += "[" + translateOperandNasm(instr.result) + "]";
                else
                    Fatal("Unknown type of result used for move instruction");
                result += ", ";
                if (std::holds_alternative<std::shared_ptr<RSI::Reference>>(instr.op1))
                    result += translateOperandNasm(instr.op1);
                else if (std::holds_alternative<RSI::Constant>(instr.op1))
                    result += translateOperandNasm(instr.op1);
                else if (std::holds_alternative<RSI::DynamicConstant>(instr.op1))
                    result += translateOperandNasm(instr.op1);
                else if (std::holds_alternative<std::shared_ptr<RSI::GlobalReference>>(instr.op1))
                    result += "[" + translateOperandNasm(instr.op1) + "]";
                else
                    Fatal("Unknown type of operand used for move instruction");
                result += "\n";
                break;
            case RSI::InstructionType::STORE_MEMORY:
                result += "mov QWORD [" + translateOperandNasm(instr.op1) + "], "
                        + translateOperandNasm(instr.op2) + "\n";
                break;
            case RSI::InstructionType::LOAD_MEMORY:
                result += "mov QWORD " + translateOperandNasm(instr.result) + ", ["
                        + translateOperandNasm(instr.op1) + "]\n";
                break;
            case RSI::InstructionType::RETURN:
                result += "mov rax, " + translateOperandNasm(instr.op1) + "\n";

                result += "add rsp, " + std::to_string(function.meta.maxStackUsage) + "\n";

                // restore callee saved regs
                for (auto reg_it = function.meta.allRegisters.rbegin(); reg_it != function.meta.allRegisters.rend();
                     reg_it++) {
                    if (ContainerTools::contains(x86_64.calleeSavedRegisters, *reg_it)) {
                        result += "pop " + x86_64.registerTranslation.at(*reg_it) + "\n";
                    }
                }

                result += "ret\n";
                break;
            case RSI::InstructionType::LOGICAL_NOT:
                result += "cmp " + translateOperandNasm(instr.op1) + ", 0\n";
                result += "mov " + translateOperandNasm(instr.result) + ", 0\n";
                result += "sete " + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 1))
                        + "\n";
                break;

            case RSI::InstructionType::NOP: break;
            case RSI::InstructionType::DEFINE_LABEL:
                result += std::get<std::shared_ptr<RSI::Label>>(instr.op1)->name + ":\n";
                break;
            case RSI::InstructionType::JUMP:
                result += "jmp " + std::get<std::shared_ptr<RSI::Label>>(instr.op1)->name + "\n";
                break;
            case RSI::InstructionType::JUMP_IF_ZERO:
                result += "cmp " + translateOperandNasm(instr.op1) + ", 0\n";
                result += "je " + std::get<std::shared_ptr<RSI::Label>>(instr.op2)->name + "\n";
                break;
            case RSI::InstructionType::STORE_PARAMETER:
                result += "push " + translateOperandNasm(instr.op1) + "\n";
                break;
            case RSI::InstructionType::LOAD_PARAMETER: {
                break;
            }
            case RSI::InstructionType::CALL: {
                if (!std::holds_alternative<RSI::Constant>(instr.op2))
                    Fatal("call instruction has non constant number of arguments.");

                auto regsToPreserve = next_instr.has_value() ? next_instr.value().get().meta.liveVariablesBefore
                                                             : std::set<std::shared_ptr<RSI::Reference>>();
                regsToPreserve.erase(std::get<std::shared_ptr<RSI::Reference>>(instr.result));

                // don't save callee saved registers
                for (auto reg_it = regsToPreserve.begin(); reg_it != regsToPreserve.end();) {
                    if (std::holds_alternative<RSI::HWRegister>((*reg_it)->storageLocation)
                        && ContainerTools::contains(
                            x86_64.calleeSavedRegisters, std::get<RSI::HWRegister>((*reg_it)->storageLocation)
                        )) {
                        reg_it = regsToPreserve.erase(reg_it);
                    }
                    else {
                        reg_it++;
                    }
                }

                // save registers
                for (auto reg : regsToPreserve) {
                    if (std::holds_alternative<RSI::HWRegister>(reg->storageLocation)) {
                        result += "push " + x86_64.registerTranslation.at(std::get<RSI::HWRegister>(reg->storageLocation))
                                + "\n";
                    }
                }

                const std::vector<RSI::HWRegister> usedParameterRegs(
                    x86_64.parameterRegisters.begin(),
                    x86_64.parameterRegisters.begin() + std::get<RSI::Constant>(instr.op2).value
                );

                constexpr int pushSize = 8;

                if (usedParameterRegs.size()) {
                    // the return register gets changed anyways, so might as well use it here

                    int stackOffset = regsToPreserve.size() * pushSize;
                    for (auto it = usedParameterRegs.rbegin(); it != usedParameterRegs.rend(); it++) {
                        std::string currentParameterRegister = x86_64.registerTranslation.at(*it);
                        result += "mov " + currentParameterRegister + ", [rsp+" + std::to_string(stackOffset)
                                + "]\n";
                        stackOffset += pushSize;
                    }
                }
                result += "call " + std::get<std::shared_ptr<RSI::Label>>(instr.op1)->name + "\n";


                // restore registers
                for (auto reg_it = regsToPreserve.rbegin(); reg_it != regsToPreserve.rend(); reg_it++) {
                    auto reg = *reg_it;
                    if (std::holds_alternative<RSI::HWRegister>(reg->storageLocation)) {
                        result += "pop " + x86_64.registerTranslation.at(std::get<RSI::HWRegister>(reg->storageLocation))
                                + "\n";
                    }
                }

                // reclaim parameters
                result += "add rsp, " + std::to_string(usedParameterRegs.size() * pushSize) + "\n";

                break;
            }
            case RSI::InstructionType::FUNCTION_BEGIN:
                // save callee saved regs
                for (auto reg : function.meta.allRegisters) {
                    if (ContainerTools::contains(x86_64.calleeSavedRegisters, reg)) {
                        result += "push " + x86_64.registerTranslation.at(reg) + "\n";
                    }
                }
                result += "sub rsp, " + std::to_string(function.meta.maxStackUsage) + "\n";
                break;
            case RSI::InstructionType::SET_LIVE: break;

            default:
                Fatal("Unimplemented RSI instruction for nasm. (", RSI::mnemonics.at(instr.type), ")");
                break;
        }
    }

    return result;
}