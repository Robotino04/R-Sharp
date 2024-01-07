#pragma once

#include <string>
#include <variant>
#include <map>
#include <memory>

namespace RSI {

enum class InstructionType {
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

    JUMP,
    JUMP_IF_ZERO,
    DEFINE_LABEL,

    STORE_PARAMETER,
    LOAD_PARAMETER,
    CALL,

    FUNCTION_BEGIN,

    STORE_MEMORY,
    LOAD_MEMORY,

    ADDRESS_OF,
    SET_LIVE,
};

extern const std::map<InstructionType, uint> numArgumentsUsed;
extern const std::map<InstructionType, std::string> mnemonics;

struct HWRegister;
struct Reference;
struct Label;
struct GlobalReference;
struct StackSlot;

struct Constant {
    uint64_t value;

    bool operator==(Constant const& other) const {
        return this->value == other.value;
    }
    bool operator!=(Constant const& other) const {
        return !(*this == other);
    }
};
struct DynamicConstant {
    uint64_t* value = nullptr;

    bool operator==(DynamicConstant const& other) const {
        return this->value == other.value;
    }
    bool operator!=(DynamicConstant const& other) const {
        return !(*this == other);
    }
};

using Operand = std::variant<std::monostate, Constant, DynamicConstant, std::shared_ptr<Reference>, std::shared_ptr<Label>, std::shared_ptr<GlobalReference>>;
using StorageLocation = std::variant<std::monostate, HWRegister, StackSlot>;


struct Instruction;
struct Function;

struct TranslationUnit;

}


namespace std {
template <>
struct hash<RSI::HWRegister>;
}