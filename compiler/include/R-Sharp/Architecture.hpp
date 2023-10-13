#pragma once

#include "R-Sharp/RSI_FWD.hpp"

#include <map>
#include <set>
#include <tuple>
#include <string>
#include <vector>

enum class OutputArchitecture{
    x86_64,
    AArch64,
};

inline const std::set<OutputArchitecture> allArchitectures = {OutputArchitecture::x86_64, OutputArchitecture::AArch64};

enum class NasmRegisters{
    RAX,
    RBX,
    RCX,
    RDX,
    RSI,
    RDI,
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15,

    COUNT,
};

inline const std::vector<RSI::HWRegister> x86_64Registers(static_cast<int>(NasmRegisters::COUNT));
extern const std::map<RSI::HWRegister, std::string> x86_64RegisterTranslation;

extern const std::map<std::pair<std::string, int>, std::string> nasmRegisterSize;


inline const std::vector<RSI::HWRegister> aarch64Registers(26);
extern const std::map<RSI::HWRegister, std::string> aarch64RegistersRegisterTranslation;