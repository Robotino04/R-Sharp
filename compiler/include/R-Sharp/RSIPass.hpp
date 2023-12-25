#pragma once

#include "R-Sharp/RSI_FWD.hpp"
#include "R-Sharp/Architecture.hpp"

#include <functional>
#include <string>
#include <set>

struct RSIPass {
    std::string humanHeader;
    std::set<OutputArchitecture> architectures;
    std::set<RSI::InstructionType> positiveInstructionTypes;
    std::set<RSI::InstructionType> negativeInstructionTypes;


    std::function<bool(RSI::Instruction const&)> prefilter = [](auto const&) { return true; };

    std::function<void(RSI::Instruction&, std::vector<RSI::Instruction>&, std::vector<RSI::Instruction>&)> perInstructionFunction;

    bool isFunctionWide = false;
    std::function<void(RSI::Function&, Architecture const&)> perFunctionFunction;

    void operator()(RSI::Function& function, OutputArchitecture arch) const;
    void operator()(std::vector<RSI::Function>& functions, OutputArchitecture arch) const;
};