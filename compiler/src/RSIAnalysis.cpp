#include "R-Sharp/RSIAnalysis.hpp"

#include "R-Sharp/Utils/LambdaOverload.hpp"

namespace RSI{

std::string stringify_rsi_operand(RSIOperand const& op){
    return std::visit(lambda_overload{
        [](RSIConstant const& x) { return std::to_string(x.value); },
        [](RSIReference const& x){ return x.name + "(" + (x.assignedRegister.has_value() ? std::to_string(x.assignedRegister.value().getID()) : "None") + ")"; },
        [](std::monostate const&){ Fatal("Empty RSI operand used!"); return std::string();},
    }, op);
}

std::string stringify_rsi(RSIFunction const& function){
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
            prefix += stringify_rsi_operand(liveVar);
        }
        prefix += "]  ";
        while (prefix.length() < maxLiveVariableStringSize) prefix += " ";
        result += prefix;

        result += RSIMnemonic.at(instr.type);
        
        if (instr.type == RSIInstructionType::RETURN){
            result += " " + stringify_rsi_operand(instr.op1) + "\n";
            continue;
        }
        
        switch(RSIArgumentsUsed.at(instr.type)){
            case 0:
                result += "\n";
                break;
            case 1:
                result += " " + stringify_rsi_operand(instr.result) + ", " + stringify_rsi_operand(instr.op1) + "\n";
                break;
            case 2:
                result += " " + stringify_rsi_operand(instr.result) + ", " + stringify_rsi_operand(instr.op1) + ", " + stringify_rsi_operand(instr.op2) + "\n";
                break;
            default:
                Fatal("Unimplemented number of arguments used in RSI.");
                break;
        }
    }
    return result;
}

void analyzeLiveRSIVariables(RSIFunction& function){
    for (auto it = function.instructions.rbegin(); it != function.instructions.rend(); it++){
        auto& instr = *it;
        std::optional<std::reference_wrapper<RSIInstruction>> prevInstruction;
        if (it+1 != function.instructions.rend()){
            prevInstruction = *(it+1);
        }
        
        if (prevInstruction.has_value())
            prevInstruction.value().get().meta.liveVariablesAfter = instr.meta.liveVariablesAfter;

        if (prevInstruction.has_value() && std::holds_alternative<RSIReference>(instr.result)){
            prevInstruction.value().get().meta.liveVariablesAfter.erase(std::get<RSIReference>(instr.result));
        }

        if (prevInstruction.has_value() && std::holds_alternative<RSIReference>(instr.op1)) {
            prevInstruction.value().get().meta.liveVariablesAfter.insert(std::get<RSIReference>(instr.op1));
        }
        if (prevInstruction.has_value() && std::holds_alternative<RSIReference>(instr.op2)) {
            prevInstruction.value().get().meta.liveVariablesAfter.insert(std::get<RSIReference>(instr.op2));
        }
    }
}

}