#include "R-Sharp/Architecture.hpp"
#include "R-Sharp/RSI.hpp"

const std::map<RSI::HWRegister, std::string> x86_64RegisterTranslation = {
    {x86_64Registers.at(static_cast<int>(NasmRegisters::RAX)), "rax"},
    {x86_64Registers.at(static_cast<int>(NasmRegisters::RBX)), "rbx"},
    {x86_64Registers.at(static_cast<int>(NasmRegisters::RCX)), "rcx"},
    {x86_64Registers.at(static_cast<int>(NasmRegisters::RDX)), "rdx"},
    {x86_64Registers.at(static_cast<int>(NasmRegisters::RSI)), "rsi"},
    {x86_64Registers.at(static_cast<int>(NasmRegisters::RDI)), "rdi"},
    {x86_64Registers.at(static_cast<int>(NasmRegisters::R8)), "r8"},
    {x86_64Registers.at(static_cast<int>(NasmRegisters::R9)), "r9"},
    {x86_64Registers.at(static_cast<int>(NasmRegisters::R10)), "r10"},
    {x86_64Registers.at(static_cast<int>(NasmRegisters::R11)), "r11"},
    {x86_64Registers.at(static_cast<int>(NasmRegisters::R12)), "r12"},
    {x86_64Registers.at(static_cast<int>(NasmRegisters::R13)), "r13"},
    {x86_64Registers.at(static_cast<int>(NasmRegisters::R14)), "r14"},
    {x86_64Registers.at(static_cast<int>(NasmRegisters::R15)), "r15"},
};

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

const std::map<RSI::HWRegister, std::string> aarch64RegistersRegisterTranslation = {
    {aarch64Registers.at(0), "x0"},
    {aarch64Registers.at(1), "x1"},
    {aarch64Registers.at(2), "x2"},
    {aarch64Registers.at(3), "x3"},
    {aarch64Registers.at(4), "x4"},
    {aarch64Registers.at(5), "x5"},
    {aarch64Registers.at(6), "x6"},
    {aarch64Registers.at(7), "x7"},
    {aarch64Registers.at(8), "x8"},
    {aarch64Registers.at(9), "x9"},
    {aarch64Registers.at(10), "x10"},
    {aarch64Registers.at(11), "x11"},
    {aarch64Registers.at(12), "x12"},
    {aarch64Registers.at(13), "x13"},
    {aarch64Registers.at(14), "x14"},
    {aarch64Registers.at(15), "x15"},
    {aarch64Registers.at(16), "x16"},
    {aarch64Registers.at(17), "x17"},

    {aarch64Registers.at(18), "x21"},
    {aarch64Registers.at(19), "x22"},
    {aarch64Registers.at(20), "x23"},
    {aarch64Registers.at(21), "x24"},
    {aarch64Registers.at(22), "x25"},
    {aarch64Registers.at(23), "x26"},
    {aarch64Registers.at(24), "x27"},
    {aarch64Registers.at(25), "x28"},
};