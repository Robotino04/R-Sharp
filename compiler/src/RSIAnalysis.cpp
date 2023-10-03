#include "R-Sharp/RSIAnalysis.hpp"

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
    const uint maxLiveVariableStringSize = 35;
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

}