#pragma once

#include <vector>

#include "R-Sharp/backend/RSI_FWD.hpp"
#include "Architecture.hpp"

namespace RSI {

void analyzeLiveVariables(Function& function, Architecture const& architecture);
void assignRegistersGraphColoring(Function& func, Architecture const& architecture);
void enumerateRegisters(Function& func, Architecture const& architecture);

void replaceModWithDivMulSub(
    RSI::Instruction& instr, std::vector<RSI::Instruction>& beforeInstructions, std::vector<RSI::Instruction>& afterInstructions
);
void makeTwoOperandCompatible(
    RSI::Instruction& instr, std::vector<RSI::Instruction>& beforeInstructions, std::vector<RSI::Instruction>& afterInstructions
);
void moveConstantsToReferences(
    RSI::Instruction& instr, std::vector<RSI::Instruction>& beforeInstructions, std::vector<RSI::Instruction>& afterInstructions
);
void seperateDivReferences(
    RSI::Instruction& instr, std::vector<RSI::Instruction>& beforeInstructions, std::vector<RSI::Instruction>& afterInstructions
);
void seperateCallResults(
    Architecture const& architecture,
    RSI::Instruction& instr,
    std::vector<RSI::Instruction>& beforeInstructions,
    std::vector<RSI::Instruction>& afterInstructions
);
void separateLoadParameters(
    Architecture const& architecture,
    RSI::Instruction& instr,
    std::vector<RSI::Instruction>& beforeInstructions,
    std::vector<RSI::Instruction>& afterInstructions
);
void resolveAddressOf(
    Architecture const& architecture,
    RSI::Instruction& instr,
    std::vector<RSI::Instruction>& beforeInstructions,
    std::vector<RSI::Instruction>& afterInstructions
);
void separateGlobalReferences(
    RSI::Instruction& instr, std::vector<RSI::Instruction>& beforeInstructions, std::vector<RSI::Instruction>& afterInstructions
);
void globalReferenceToMemoryAccess(
    RSI::Instruction& instr, std::vector<RSI::Instruction>& beforeInstructions, std::vector<RSI::Instruction>& afterInstructions
);
void separateStackVariables(
    RSI::Instruction& instr, std::vector<RSI::Instruction>& beforeInstructions, std::vector<RSI::Instruction>& afterInstructions
);


bool makeTwoOperandCompatible_prefilter(RSI::Instruction const& instr);

}