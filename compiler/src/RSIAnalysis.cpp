#include "R-Sharp/RSIAnalysis.hpp"
#include "R-Sharp/RSIGenerator.hpp"

#include "R-Sharp/Utils/LambdaOverload.hpp"

namespace RSI{

std::string stringify_operand(Operand const& op, std::map<HWRegister, std::string> const& registerTranslation){
    return std::visit(lambda_overload{
        [](Constant const& x) { return std::to_string(x.value); },
        [&](std::shared_ptr<RSI::Reference> x){ return x->name + "(" + (x->assignedRegister.has_value() ? registerTranslation.at(x->assignedRegister.value()) : "None") + ")"; },
        [](std::monostate const&){ Fatal("Empty RSI operand used!"); return std::string();},
    }, op);
}

std::string stringify_function(RSI::Function const& function, std::map<HWRegister, std::string> const& registerTranslation){
    const uint maxLiveVariableStringSize = 55;
    std::string result = "";
    for (auto instr : function.instructions){
        std::string prefix;
        prefix += "[";
        bool isFirst = true;
        for (auto liveVar : instr.meta.liveVariablesAfter){
            if (!isFirst){
                prefix += ", ";
            }
            isFirst = false;
            prefix += stringify_operand(liveVar, registerTranslation);
        }
        prefix += "]  ";
        while (prefix.length() < maxLiveVariableStringSize) prefix += " ";
        result += prefix;

        result += mnemonics.at(instr.type);
        
        if (instr.type == RSI::InstructionType::RETURN){
            result += " " + stringify_operand(instr.op1, registerTranslation) + "\n";
            continue;
        }
        
        switch(numArgumentsUsed.at(instr.type)){
            case 0:
                result += "\n";
                break;
            case 1:
                result += " " + stringify_operand(instr.result, registerTranslation) + ", " + stringify_operand(instr.op1, registerTranslation) + "\n";
                break;
            case 2:
                result += " " + stringify_operand(instr.result, registerTranslation) + ", " + stringify_operand(instr.op1, registerTranslation) + ", " + stringify_operand(instr.op2, registerTranslation) + "\n";
                break;
            default:
                Fatal("Unimplemented number of arguments used in RSI.");
                break;
        }
    }
    return result;
}

void analyzeLiveVariables(RSI::Function& function){
    for (auto it = function.instructions.rbegin(); it != function.instructions.rend(); it++){
        auto& instr = *it;
        std::optional<std::reference_wrapper<RSI::Instruction>> prevInstruction;
        if (it+1 != function.instructions.rend()){
            prevInstruction = *(it+1);
        }
        
        if (prevInstruction.has_value())
            prevInstruction.value().get().meta.liveVariablesAfter = instr.meta.liveVariablesAfter;

        if (prevInstruction.has_value() && std::holds_alternative<std::shared_ptr<RSI::Reference>>(instr.result)){
            prevInstruction.value().get().meta.liveVariablesAfter.erase(std::get<std::shared_ptr<RSI::Reference>>(instr.result));
        }

        if (prevInstruction.has_value() && std::holds_alternative<std::shared_ptr<RSI::Reference>>(instr.op1)) {
            prevInstruction.value().get().meta.liveVariablesAfter.insert(std::get<std::shared_ptr<RSI::Reference>>(instr.op1));
        }
        if (prevInstruction.has_value() && std::holds_alternative<std::shared_ptr<RSI::Reference>>(instr.op2)) {
            prevInstruction.value().get().meta.liveVariablesAfter.insert(std::get<std::shared_ptr<RSI::Reference>>(instr.op2));
        }
    }
}

void assignRegistersLinearScan(Function& func, std::vector<HWRegister> const& allRegisters){
    for (auto& instr : func.instructions){
        std::vector<HWRegister> unusedRegisters = allRegisters;
        for (auto& var : instr.meta.liveVariablesAfter){
            if (var->assignedRegister.has_value()){
                unusedRegisters.erase(std::remove(unusedRegisters.begin(), unusedRegisters.end(), var->assignedRegister.value()), unusedRegisters.end());
            }
        }

        for (auto& var : instr.meta.liveVariablesAfter){
            if (!var->assignedRegister.has_value()){
                var->assignedRegister = unusedRegisters.front();
                unusedRegisters.erase(unusedRegisters.begin());
            }
        }
    }
}

void makeTwoOperandCompatible(Function& func){
    for (int i=0; i<func.instructions.size(); i++){
        auto& instr = func.instructions.at(i);
        if (std::holds_alternative<std::shared_ptr<Reference>>(instr.result) && std::holds_alternative<std::shared_ptr<Reference>>(instr.op1) && !std::holds_alternative<std::monostate>(instr.op2)){
            if (std::get<std::shared_ptr<Reference>>(instr.result) != std::get<std::shared_ptr<Reference>>(instr.op1)){
                Instruction move{
                    .type = InstructionType::MOVE,
                    .result = instr.result,
                    .op1 = instr.op1,
                };
                instr.op1 = instr.result;
                func.instructions.insert(func.instructions.begin()+i, move);
            }
        }
    }
}

void replaceModWithDivMulSub(Function& func){
    for (int i=0; i<func.instructions.size(); i++){
        auto& instr = func.instructions.at(i);
        if (instr.type == InstructionType::MODULO){
            instr.type = InstructionType::DIVIDE;

            auto finalResult = instr.result;
            instr.result = std::make_shared<Reference>(Reference{.name = RSIGenerator::getUniqueLabel("tmp")});

            Instruction mult{
                .type = InstructionType::MULTIPLY,
                .result = std::make_shared<Reference>(Reference{.name = RSIGenerator::getUniqueLabel("tmp")}),
                .op1 = instr.result,
                .op2 = instr.op2,
            };
            Instruction sub{
                .type = InstructionType::SUBTRACT,
                .result = finalResult,
                .op1 = instr.op1,
                .op2 = mult.result,
            };
            
            
            func.instructions.insert(func.instructions.begin()+i+1, mult);
            func.instructions.insert(func.instructions.begin()+i+2, sub);
        }
    }
}

}