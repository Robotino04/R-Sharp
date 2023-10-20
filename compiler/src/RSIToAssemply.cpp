#include "R-Sharp/RSIToAssembly.hpp"
#include "R-Sharp/RSI.hpp"
#include "R-Sharp/Architecture.hpp"

#include "R-Sharp/Utils/ContainerTools.hpp"
#include "R-Sharp/Utils/LambdaOverload.hpp"

std::string translateOperand(RSI::Operand const& op, std::map<RSI::HWRegister, std::string> registerTranslation, std::string constantPrefix){
    return std::visit(lambda_overload{
        [&](RSI::Constant const& x) { return constantPrefix + std::to_string(x.value); },
        [&](std::shared_ptr<RSI::Reference> x){ return x->assignedRegister.has_value() ? registerTranslation.at(x->assignedRegister.value()) : "(none)"; },
        [](std::shared_ptr<RSI::Label> x){ return x->name; },
        [](std::monostate const&){ Fatal("Empty RSI operand used!"); return std::string();},
    }, op);
}

std::string rsiToAarch64(RSI::Function const& function){
    std::string result = "";

    const auto translateOperandAarch64 = [&](RSI::Operand const& op){return translateOperand(op, aarch64.registerTranslation, "#");};

    for (auto instr_it = function.instructions.begin(); instr_it != function.instructions.end(); instr_it++){
        RSI::Instruction const& instr = *instr_it;
        std::optional<std::reference_wrapper<const RSI::Instruction>> next_instr;
        if (instr_it+1 != function.instructions.end()){
            next_instr = *(instr_it+1);
        }

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
                
                // restore callee saved regs
                for (auto reg_it = function.meta.allRegisters.rbegin(); reg_it != function.meta.allRegisters.rend(); reg_it++){
                    if (ContainerTools::contains(aarch64.calleeSavedRegisters, *reg_it)){
                        result += "pop " + aarch64.registerTranslation.at(*reg_it) + "\n";
                    }
                }

                result += "ret\n";
                break;
            case RSI::InstructionType::LOGICAL_NOT:
                if (!std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister.has_value()){ break; }
                result += "cmp " + translateOperandAarch64(instr.op1) + ", 0\n";
                result += "cset " + translateOperandAarch64(instr.result) + ", eq\n";
                break;

            case RSI::InstructionType::NOP:
                break;
            case RSI::InstructionType::DEFINE_LABEL:
                result += std::get<std::shared_ptr<RSI::Label>>(instr.op1)->name + ":\n";
                break;
            case RSI::InstructionType::JUMP:
                result += "b " + std::get<std::shared_ptr<RSI::Label>>(instr.op1)->name + "\n";
                break;
            case RSI::InstructionType::JUMP_IF_ZERO:
                result += "cbz " + translateOperandAarch64(instr.op1) + ", " + std::get<std::shared_ptr<RSI::Label>>(instr.op2)->name + "\n";
                break;
            case RSI::InstructionType::STORE_PARAMETER:
                result += "push " + translateOperandAarch64(instr.op1) + "\n";
                break;
            case RSI::InstructionType::LOAD_PARAMETER:
                break;
            case RSI::InstructionType::CALL:{
                if (!std::holds_alternative<RSI::Constant>(instr.op2))
                    Fatal("call instruction has non constant number of arguments.");

                auto regsToPreserve = next_instr.has_value() ? next_instr.value().get().meta.liveVariablesBefore : std::set<std::shared_ptr<RSI::Reference>>();
                regsToPreserve.erase(std::get<std::shared_ptr<RSI::Reference>>(instr.result));

                // don't save callee saved registers
                for (auto reg_it = regsToPreserve.begin(); reg_it != regsToPreserve.end();){
                    if ((*reg_it)->assignedRegister.has_value() && ContainerTools::contains(aarch64.calleeSavedRegisters, (*reg_it)->assignedRegister.value())){
                        reg_it = regsToPreserve.erase(reg_it);
                    }
                    else if ((*reg_it)->assignedRegister.has_value() && (*reg_it)->assignedRegister.value() == aarch64.returnValueRegister){
                        reg_it = regsToPreserve.erase(reg_it);
                    }
                    else{
                        reg_it++;
                    }
                }

                // save registers
                for (auto reg : regsToPreserve){
                    if (reg->assignedRegister.has_value()){
                        result += "push " + aarch64.registerTranslation.at(reg->assignedRegister.value()) + "\n";
                    }
                }
                
                const std::vector<RSI::HWRegister> usedParameterRegs(aarch64.parameterRegisters.begin(), aarch64.parameterRegisters.begin() + std::get<RSI::Constant>(instr.op2).value);

                constexpr int pushSize = 16;

                std::string tmpReg;
                if (usedParameterRegs.size()){
                    tmpReg = aarch64.registerTranslation.at(aarch64.returnValueRegister);
                    // the return register gets changed anyways, so might as well use it here
                    // yes, it is also parameter register 1 on AArch64, but the registers get loaded in reverse, so it isn't overwritten

                    int stackOffset = regsToPreserve.size() * pushSize;
                    for (auto it = usedParameterRegs.rbegin(); it != usedParameterRegs.rend(); it++){
                        std::string currentParameterRegister = aarch64.registerTranslation.at(*it);
                        result += "ldr " + tmpReg + ", [sp, " + std::to_string(stackOffset) + "]\n";
                        if (currentParameterRegister != tmpReg)
                            result += "mov " + currentParameterRegister + ", " + tmpReg + "\n";
                        stackOffset += pushSize;
                    }
                }
                result += "stp fp, lr, [sp, -16]!\n";
                result += "mov fp, sp\n";
                result += "bl " + std::get<std::shared_ptr<RSI::Label>>(instr.op1)->name + "\n";
                result += "ldp fp, lr, [sp], 16\n";

                // restore registers
                for (auto reg_it = regsToPreserve.rbegin(); reg_it != regsToPreserve.rend(); reg_it++){
                    auto reg = *reg_it;
                    if (reg->assignedRegister.has_value()){
                        result += "pop " + aarch64.registerTranslation.at(reg->assignedRegister.value()) + "\n";
                    }
                }

                // reclaim parameters
                result += "add sp, sp, " + std::to_string(usedParameterRegs.size()*pushSize) + "\n";

                break;
            }
            case RSI::InstructionType::FUNCTION_BEGIN:
                // save callee saved regs
                for (auto reg : function.meta.allRegisters){
                    if (ContainerTools::contains(aarch64.calleeSavedRegisters, reg)){
                        result += "push " + aarch64.registerTranslation.at(reg) + "\n";
                    }
                }
                break;

            default:
                Fatal("Unimplemented RSI instrution for aarch64.");
                break;
        }
    }

    return result;
}

bool isRegister(RSI::Operand const& op, NasmRegisters reg){
    return std::get<std::shared_ptr<RSI::Reference>>(op)->assignedRegister == x86_64.allRegisters.at(static_cast<int>(reg));
}   

std::string rsiToNasm(RSI::Function const& function){
    std::string result = "";

    const auto translateOperandNasm = [&](RSI::Operand const& op){return translateOperand(op, x86_64.registerTranslation, "");};

    for (auto instr_it = function.instructions.begin(); instr_it != function.instructions.end(); instr_it++){
        RSI::Instruction const& instr = *instr_it;
        std::optional<std::reference_wrapper<const RSI::Instruction>> next_instr;
        if (instr_it+1 != function.instructions.end()){
            next_instr = *(instr_it+1);
        }

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
                if (isRegister(instr.op2, NasmRegisters::RAX)){
                    result += "imul " + translateOperandNasm(instr.op1) + "\n";
                }
                else{
                    result += "mov rax, " + translateOperandNasm(instr.op1) + "\n";
                    result += "imul " + translateOperandNasm(instr.op2) + "\n";
                }

                result += "mov " + translateOperandNasm(instr.result) + ", rax\n";

                if (!isRegister(instr.result, NasmRegisters::RDX))
                    result += "pop rdx\n";
                else
                    result += "add rsp, 8\n";

                if (!isRegister(instr.result, NasmRegisters::RAX))
                    result += "pop rax\n";
                else
                    result += "add rsp, 8\n";

                break;
            case RSI::InstructionType::DIVIDE:
                if (!std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister.has_value()){ break; }

                if (!isRegister(instr.result, NasmRegisters::RAX)){
                    Fatal("The result of a division is not in RAX. Divisions may not have been isolated correctly.");
                }
                if (!isRegister(instr.op1, NasmRegisters::RAX)){
                    Fatal("The first operand of a division is not in RAX. Divisions may not have been isolated correctly.");
                }

                result += "push rdx\n";
                result += "cqo\n";
                result += "idiv " + translateOperandNasm(instr.op2) + "\n";
                result += "pop rdx\n";

                break;
            case RSI::InstructionType::MODULO:
                if (!std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister.has_value()){ break; }
                
                if (!isRegister(instr.result, NasmRegisters::RAX))
                    result += "push rax\n";
                if (!isRegister(instr.result, NasmRegisters::RDX))
                    result += "push rdx\n";

                result += "mov rax, " + translateOperandNasm(instr.op1) + "\n";
                result += "cqo\n";
                result += "idiv " + translateOperandNasm(instr.op2) + "\n";
                result += "mov " + translateOperandNasm(instr.result) + ", rdx\n";

                if (!isRegister(instr.result, NasmRegisters::RDX))
                    result += "pop rdx\n";

                if (!isRegister(instr.result, NasmRegisters::RAX))
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

                // restore callee saved regs
                for (auto reg_it = function.meta.allRegisters.rbegin(); reg_it != function.meta.allRegisters.rend(); reg_it++){
                    if (ContainerTools::contains(x86_64.calleeSavedRegisters, *reg_it)){
                        result += "pop " + x86_64.registerTranslation.at(*reg_it) + "\n";
                    }
                }

                result += "ret\n";
                break;
            case RSI::InstructionType::LOGICAL_NOT:
                result += "cmp " + translateOperandNasm(instr.op1) + ", 0\n";
                result += "mov " + translateOperandNasm(instr.result) + ", 0\n";
                result += "sete " + nasmRegisterSize.at(std::make_pair(translateOperandNasm(instr.result), 1)) + "\n";
                break;

            case RSI::InstructionType::NOP:
                break;
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
            case RSI::InstructionType::LOAD_PARAMETER:
                break;
            case RSI::InstructionType::CALL:{
                if (!std::holds_alternative<RSI::Constant>(instr.op2))
                    Fatal("call instruction has non constant number of arguments.");

                auto regsToPreserve = next_instr.has_value() ? next_instr.value().get().meta.liveVariablesBefore : std::set<std::shared_ptr<RSI::Reference>>();
                regsToPreserve.erase(std::get<std::shared_ptr<RSI::Reference>>(instr.result));

                // don't save callee saved registers
                for (auto reg_it = regsToPreserve.begin(); reg_it != regsToPreserve.end();){
                    if ((*reg_it)->assignedRegister.has_value() && ContainerTools::contains(x86_64.calleeSavedRegisters, (*reg_it)->assignedRegister.value())){
                        reg_it = regsToPreserve.erase(reg_it);
                    }
                    else{
                        reg_it++;
                    }
                }

                // save registers
                for (auto reg : regsToPreserve){
                    if (reg->assignedRegister.has_value()){
                        result += "push " + x86_64.registerTranslation.at(reg->assignedRegister.value()) + "\n";
                    }
                }
                
                const std::vector<RSI::HWRegister> usedParameterRegs(x86_64.parameterRegisters.begin(), x86_64.parameterRegisters.begin() + std::get<RSI::Constant>(instr.op2).value);

                constexpr int pushSize = 8;

                std::string tmpReg;
                if (usedParameterRegs.size()){
                    tmpReg = x86_64.registerTranslation.at(x86_64.returnValueRegister);
                    // the return register gets changed anyways, so might as well use it here

                    int stackOffset = regsToPreserve.size() * pushSize;
                    for (auto it = usedParameterRegs.rbegin(); it != usedParameterRegs.rend(); it++){
                        std::string currentParameterRegister = x86_64.registerTranslation.at(*it);
                        result += "mov " + tmpReg + ", [rsp+" + std::to_string(stackOffset) + "]\n";
                        if (currentParameterRegister != tmpReg)
                            result += "mov " + currentParameterRegister + ", " + tmpReg + "\n";
                        stackOffset += pushSize;
                    }
                }
                result += "call " + std::get<std::shared_ptr<RSI::Label>>(instr.op1)->name + "\n";


                // restore registers
                for (auto reg_it = regsToPreserve.rbegin(); reg_it != regsToPreserve.rend(); reg_it++){
                    auto reg = *reg_it;
                    if (reg->assignedRegister.has_value()){
                        result += "pop " + x86_64.registerTranslation.at(reg->assignedRegister.value()) + "\n";
                    }
                }

                // reclaim parameters
                result += "add rsp, " + std::to_string(usedParameterRegs.size()*pushSize) + "\n";

                break;
            }
            case RSI::InstructionType::FUNCTION_BEGIN:
                // save callee saved regs
                for (auto reg : function.meta.allRegisters){
                    if (ContainerTools::contains(x86_64.calleeSavedRegisters, reg)){
                        result += "push " + x86_64.registerTranslation.at(reg) + "\n";
                    }
                }
                break;

            default:
                Fatal("Unimplemented RSI instruction for nasm.");
                break;
        }
    }

    return result;
}