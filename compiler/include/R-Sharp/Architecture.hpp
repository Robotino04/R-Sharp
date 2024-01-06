#pragma once

#include "R-Sharp/RSI.hpp"

#include <map>
#include <set>
#include <tuple>
#include <string>
#include <vector>

struct Architecture {
    std::vector<RSI::HWRegister> allRegisters;
    std::vector<RSI::HWRegister> generalPurposeRegisters;
    std::vector<RSI::HWRegister> parameterRegisters;
    std::vector<RSI::HWRegister> calleeSavedRegisters;
    RSI::HWRegister returnValueRegister;
    RSI::HWRegister stackPointerRegister;

    std::map<RSI::HWRegister, std::string> registerTranslation;

    void validate();
};

enum class OutputArchitecture {
    x86_64,
    AArch64,
};

inline const std::set<OutputArchitecture> allArchitectureTypes = {OutputArchitecture::x86_64, OutputArchitecture::AArch64};

extern const Architecture x86_64;
extern const Architecture aarch64;

enum class NasmRegisters {
    RAX,
    RBX,
    RCX,
    RDX,
    RSI,
    RDI,
    RBP,
    RSP,
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
extern const std::map<std::pair<std::string, int>, std::string> nasmRegisterSize;
