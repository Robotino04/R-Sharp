#include "R-Sharp/RSITools.hpp"
#include "R-Sharp/RSI.hpp"

#include "R-Sharp/Utils/LambdaOverload.hpp"

namespace RSI{

std::string stringify_operand(Operand const& op, std::map<HWRegister, std::string> const& registerTranslation){
    return std::visit(lambda_overload{
        [](Constant const& x) { return std::to_string(x.value); },
        [&](std::shared_ptr<RSI::Reference> x){ return x->name + "(" + (x->assignedRegister.has_value() ? registerTranslation.at(x->assignedRegister.value()) : "None") + ")"; },
        [](std::shared_ptr<RSI::Label> x){ return x->name; },
        [](std::monostate const&){ return std::string("[none]");},
    }, op);
}

std::string stringify_function(RSI::Function const& function, std::map<HWRegister, std::string> const& registerTranslation){
    const uint maxLiveVariableStringSize = 55;
    std::string result = "";
    for (auto instr : function.instructions){
        std::string prefix;
        prefix += "[";
        bool isFirst = true;
        for (auto liveVar : instr.meta.liveVariablesBefore){
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
        else if (instr.type == RSI::InstructionType::DEFINE_LABEL){
            result += " " + stringify_operand(instr.op1, registerTranslation) + "\n";
            continue;
        }
        else if (instr.type == RSI::InstructionType::JUMP){
            result += " -> " + stringify_operand(instr.op1, registerTranslation) + "\n";
            continue;
        }
        else if (instr.type == RSI::InstructionType::JUMP_IF_ZERO){
            result += " " + stringify_operand(instr.op1, registerTranslation) + " -> " + stringify_operand(instr.op2, registerTranslation) + "\n";
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

}