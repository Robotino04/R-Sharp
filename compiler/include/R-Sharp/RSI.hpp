#pragma once

#include <cstdint>
#include <string>
#include <optional>
#include <variant>

#include "AstNodesFWD.hpp"

enum class RSIInstructionType{
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

struct RSIConstant{
    uint64_t value;
};

struct RSIReference{
    std::string name;
    std::optional<std::shared_ptr<SemanticVariableData>> variable;
};

using RSIOperand = std::variant<std::monostate, RSIConstant, RSIReference>;


struct RSIInstruction{
    RSIInstructionType type;
    RSIOperand result;
    RSIOperand op1;
    RSIOperand op2;
};

struct RSIFunction{
    std::string name;
    std::shared_ptr<SemanticFunctionData> function;
    std::vector<RSIInstruction> instructions;
};