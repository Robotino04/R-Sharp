#pragma once

#include <vector>

#include "R-Sharp/RSI_FWD.hpp"
#include "Architecture.hpp"

namespace RSI{

void analyzeLiveVariables(Function& function, Architecture const& architecture);
void assignRegistersGraphColoring(Function& func, Architecture const& architecture);
void enumerateRegisters(Function& func, Architecture const& architecture);

void replaceModWithDivMulSub(RSI::Instruction& instr, std::vector<RSI::Instruction>& beforeInstructions, std::vector<RSI::Instruction>& afterInstructions);
void makeTwoOperandCompatible(RSI::Instruction& instr, std::vector<RSI::Instruction>& beforeInstructions, std::vector<RSI::Instruction>& afterInstructions);
void moveConstantsToReferences(RSI::Instruction& instr, std::vector<RSI::Instruction>& beforeInstructions, std::vector<RSI::Instruction>& afterInstructions);
void seperateDivReferences(RSI::Instruction& instr, std::vector<RSI::Instruction>& beforeInstructions, std::vector<RSI::Instruction>& afterInstructions);
void seperateCallResults(Architecture const& architecture, RSI::Instruction& instr, std::vector<RSI::Instruction>& beforeInstructions, std::vector<RSI::Instruction>& afterInstructions);
void seperateLoadParameters(Architecture const& architecture, RSI::Instruction& instr, std::vector<RSI::Instruction>& beforeInstructions, std::vector<RSI::Instruction>& afterInstructions);
void seperateGlobalReferences(RSI::Instruction& instr, std::vector<RSI::Instruction>& beforeInstructions, std::vector<RSI::Instruction>& afterInstructions);


bool makeTwoOperandCompatible_prefilter(RSI::Instruction const& instr);

}