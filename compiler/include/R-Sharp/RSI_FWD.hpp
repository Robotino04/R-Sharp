#pragma once

#include <string>
#include <variant>
#include <map>
#include <memory>

namespace RSI{

enum class InstructionType{
    NOP,

    MOVE,
    RETURN,

    NEGATE,
    BINARY_NOT,
    LOGICAL_NOT,


    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    MODULO,
    
    EQUAL,
    NOT_EQUAL,
    LESS_THAN,
    LESS_THAN_OR_EQUAL,
    GREATER_THAN,
    GREATER_THAN_OR_EQUAL,

    LOGICAL_AND,
    LOGICAL_OR,

    BINARY_AND,
};

extern const std::map<InstructionType, uint> numArgumentsUsed;
extern const std::map<InstructionType, std::string> mnemonics;

struct HWRegister;
struct Constant;
struct Reference;

using Operand = std::variant<std::monostate, Constant, std::shared_ptr<Reference>>;


struct Instruction;
struct Function;

}


namespace std{
    template<>
    struct hash<RSI::HWRegister>;
}