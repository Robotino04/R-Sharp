#pragma once

#include <string>
#include <map>

#include "R-Sharp/RSI.hpp"

namespace RSI{

std::string stringify_operand(Operand const& op, std::map<HWRegister, std::string> const& registerTranslation);

std::string stringify_function(Function const& function, std::map<HWRegister, std::string> const& registerTranslation);

void analyzeLiveVariables(Function& function);

void assignRegistersLinearScan(Function& func, std::vector<HWRegister> const& allRegisters);
void assignRegistersGraphColoring(Function& func, std::vector<HWRegister> const& allRegisters);

void makeTwoOperandCompatible(Function& func);
void replaceModWithDivMulSub(Function& func);
void moveConstantsToReferences(Function& func);
void nasm_seperateDivReferences(Function& func, RSI::HWRegister rax);

}