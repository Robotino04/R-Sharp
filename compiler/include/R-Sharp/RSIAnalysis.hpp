#pragma once

#include <vector>

#include "R-Sharp/RSI_FWD.hpp"
#include "Architecture.hpp"

namespace RSI{

void analyzeLiveVariables(Function& function, OutputArchitecture arch);

void assignRegistersLinearScan(Function& func, OutputArchitecture arch);
void assignRegistersGraphColoring(Function& func, OutputArchitecture arch);

void replaceModWithDivMulSub(RSI::Instruction& instr, std::vector<RSI::Instruction>& beforeInstructions, std::vector<RSI::Instruction>& afterInstructions);
void makeTwoOperandCompatible(RSI::Instruction& instr, std::vector<RSI::Instruction>& beforeInstructions, std::vector<RSI::Instruction>& afterInstructions);
void moveConstantsToReferences(RSI::Instruction& instr, std::vector<RSI::Instruction>& beforeInstructions, std::vector<RSI::Instruction>& afterInstructions);
void seperateDivReferences(RSI::Instruction& instr, std::vector<RSI::Instruction>& beforeInstructions, std::vector<RSI::Instruction>& afterInstructions);

bool makeTwoOperandCompatible_prefilter(RSI::Instruction const& instr);

}