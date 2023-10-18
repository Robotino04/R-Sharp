#include "R-Sharp/Architecture.hpp"
#include "R-Sharp/RSI.hpp"

#include <numeric>

template<typename T>
bool contains(std::vector<T> const& vec, T const& element){
    return std::find(vec.begin(), vec.end(), element) != vec.end();
}

void Architecture::validate(){
    if (registerTranslation.size() != allRegisters.size()){
        Fatal("Not all registers have a translation");
    }

    for (auto const& reg : calleeSavedRegisters){
        if (!contains(allRegisters, reg)){
            Fatal("Callee saved registers contain unknown registers");
        }
    }

    for (auto const& reg : generalPurposeRegisters){
        if (!contains(allRegisters, reg)){
            Fatal("General purpose registers contain unknown registers");
        }
    }

    for (auto const& reg : parameterRegisters){
        if (!contains(allRegisters, reg)){
            Fatal("Parameter registers contain unknown registers");
        }
    }

    if (!contains(allRegisters, returnValueRegister)){
        Fatal("Return value register is an unknown register");
    }
}

const Architecture x86_64 = [](){
    Architecture arch;
    arch.allRegisters = std::vector<RSI::HWRegister>(static_cast<int>(NasmRegisters::COUNT));
    
    #define REG(NAME) arch.allRegisters.at(static_cast<int>(NasmRegisters::NAME))

    arch.calleeSavedRegisters = {
        REG(RBX),
        REG(RBP),
        REG(RSP),
        REG(R12),
        REG(R13),
        REG(R14),
        REG(R15),
    };
    arch.generalPurposeRegisters = {
        REG(RAX),
        REG(RBX),
        REG(RCX),
        REG(RDX),
        REG(RSI),
        REG(RDI),
        REG(R8),
        REG(R9),
        REG(R10),
        REG(R11),
        REG(R12),
        REG(R13),
        REG(R14),
        REG(R15),
    };
    arch.parameterRegisters = {
        REG(RDI),
        REG(RSI),
        REG(RDX),
        REG(RCX),
        REG(R8),
        REG(R9),
    };
    arch.registerTranslation = {
        {REG(RAX), "rax"},
        {REG(RBX), "rbx"},
        {REG(RCX), "rcx"},
        {REG(RDX), "rdx"},
        {REG(RSI), "rsi"},
        {REG(RDI), "rdi"},
        {REG(RBP), "rbp"},
        {REG(RSP), "rsp"},
        {REG(R8), "r8"},
        {REG(R9), "r9"},
        {REG(R10), "r10"},
        {REG(R11), "r11"},
        {REG(R12), "r12"},
        {REG(R13), "r13"},
        {REG(R14), "r14"},
        {REG(R15), "r15"},
    };
    arch.returnValueRegister = REG(RAX);

    #undef REG

    arch.validate();

    return arch;
}();



const Architecture aarch64 = [](){
    Architecture arch;
    arch.allRegisters = std::vector<RSI::HWRegister>(31);
    
    const auto registerRange = [&arch](int start, int end){
        std::vector<RSI::HWRegister> regs;
        for (int i=start; i<=end; i++){
            regs.push_back(arch.allRegisters.at(i));
        }
        return regs;
    };
    const auto combineRanges = [](std::vector<std::vector<RSI::HWRegister>> const& ranges){
        std::vector<RSI::HWRegister> result;
        result.reserve(
            std::accumulate(
                ranges.begin(), ranges.end(),
                0,
                [](int sizeUntilNow, auto const& range){
                    return range.size() + sizeUntilNow;
                }
            )
        );

        for (auto const& range : ranges){
            result.insert(result.end(), range.begin(), range.end());
        }

        return result;
    };

    arch.calleeSavedRegisters = registerRange(19, 30);
    arch.generalPurposeRegisters = combineRanges({
        registerRange(0, 17),
        registerRange(19, 28)
    });
    arch.parameterRegisters = registerRange(0, 7);
    arch.returnValueRegister = arch.allRegisters.at(0);

    for (int i=0; i<arch.allRegisters.size(); i++){
        arch.registerTranslation.insert({arch.allRegisters.at(i), "x" + std::to_string(i)});
    }

    arch.validate();

    return arch;
}();

const std::map<std::pair<std::string, int>, std::string> nasmRegisterSize = {
    {{"rax", 1}, "al"},
    {{"rax", 2}, "ax"},
    {{"rax", 4}, "eax"},
    {{"rax", 8}, "rax"},

    {{"rbx", 1}, "bl"},
    {{"rbx", 2}, "bx"},
    {{"rbx", 4}, "ebx"},
    {{"rbx", 8}, "rbx"},

    {{"rcx", 1}, "cl"},
    {{"rcx", 2}, "cx"},
    {{"rcx", 4}, "ecx"},
    {{"rcx", 8}, "rcx"},

    {{"rdx", 1}, "dl"},
    {{"rdx", 2}, "dx"},
    {{"rdx", 4}, "edx"},
    {{"rdx", 8}, "rdx"},

    {{"rsi", 1}, "sil"},
    {{"rsi", 2}, "si"},
    {{"rsi", 4}, "esi"},
    {{"rsi", 8}, "rsi"},

    {{"rdi", 1}, "dil"},
    {{"rdi", 2}, "di"},
    {{"rdi", 4}, "edi"},
    {{"rdi", 8}, "rdi"},

    {{"rsp", 1}, "spl"},
    {{"rsp", 2}, "sp"},
    {{"rsp", 4}, "esp"},
    {{"rsp", 8}, "rsp"},

    {{"rbp", 1}, "bpl"},
    {{"rbp", 2}, "bp"},
    {{"rbp", 4}, "ebp"},
    {{"rbp", 8}, "rbp"},

    {{"r0", 1}, "r0b"},
    {{"r0", 2}, "r0w"},
    {{"r0", 4}, "r0d"},
    {{"r0", 8}, "r0"},


    {{"r1", 1}, "r1b"},
    {{"r1", 2}, "r1w"},
    {{"r1", 4}, "r1d"},
    {{"r1", 8}, "r1"},


    {{"r2", 1}, "r2b"},
    {{"r2", 2}, "r2w"},
    {{"r2", 4}, "r2d"},
    {{"r2", 8}, "r2"},


    {{"r3", 1}, "r3b"},
    {{"r3", 2}, "r3w"},
    {{"r3", 4}, "r3d"},
    {{"r3", 8}, "r3"},


    {{"r4", 1}, "r4b"},
    {{"r4", 2}, "r4w"},
    {{"r4", 4}, "r4d"},
    {{"r4", 8}, "r4"},


    {{"r5", 1}, "r5b"},
    {{"r5", 2}, "r5w"},
    {{"r5", 4}, "r5d"},
    {{"r5", 8}, "r5"},


    {{"r6", 1}, "r6b"},
    {{"r6", 2}, "r6w"},
    {{"r6", 4}, "r6d"},
    {{"r6", 8}, "r6"},


    {{"r7", 1}, "r7b"},
    {{"r7", 2}, "r7w"},
    {{"r7", 4}, "r7d"},
    {{"r7", 8}, "r7"},


    {{"r8", 1}, "r8b"},
    {{"r8", 2}, "r8w"},
    {{"r8", 4}, "r8d"},
    {{"r8", 8}, "r8"},


    {{"r9", 1}, "r9b"},
    {{"r9", 2}, "r9w"},
    {{"r9", 4}, "r9d"},
    {{"r9", 8}, "r9"},


    {{"r10", 1}, "r10b"},
    {{"r10", 2}, "r10w"},
    {{"r10", 4}, "r10d"},
    {{"r10", 8}, "r10"},


    {{"r11", 1}, "r11b"},
    {{"r11", 2}, "r11w"},
    {{"r11", 4}, "r11d"},
    {{"r11", 8}, "r11"},


    {{"r12", 1}, "r12b"},
    {{"r12", 2}, "r12w"},
    {{"r12", 4}, "r12d"},
    {{"r12", 8}, "r12"},


    {{"r13", 1}, "r13b"},
    {{"r13", 2}, "r13w"},
    {{"r13", 4}, "r13d"},
    {{"r13", 8}, "r13"},


    {{"r14", 1}, "r14b"},
    {{"r14", 2}, "r14w"},
    {{"r14", 4}, "r14d"},
    {{"r14", 8}, "r14"},


    {{"r15", 1}, "r15b"},
    {{"r15", 2}, "r15w"},
    {{"r15", 4}, "r15d"},
    {{"r15", 8}, "r15"},
};