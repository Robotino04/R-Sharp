#pragma once

#include <cstdint>
#include <string>
#include <optional>
#include <variant>
#include <map>
#include <atomic>
#include <set>

#include "R-Sharp/AstNodesFWD.hpp"
#include "R-Sharp/Logging.hpp"

enum class RSIInstructionType{
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

inline const std::map<RSIInstructionType, uint> RSIArgumentsUsed = {
    {RSIInstructionType::NOP, 0},

    {RSIInstructionType::MOVE, 1},
    {RSIInstructionType::RETURN, 1},

    {RSIInstructionType::NEGATE, 1},
    {RSIInstructionType::BINARY_NOT, 1},
    {RSIInstructionType::LOGICAL_NOT, 1},

    {RSIInstructionType::ADD, 2},
    {RSIInstructionType::SUBTRACT, 2},
    {RSIInstructionType::MULTIPLY, 2},
    {RSIInstructionType::DIVIDE, 2},
    {RSIInstructionType::MODULO, 2},

    {RSIInstructionType::EQUAL, 2},
    {RSIInstructionType::NOT_EQUAL, 2},
    {RSIInstructionType::LESS_THAN, 2},
    {RSIInstructionType::LESS_THAN_OR_EQUAL, 2},
    {RSIInstructionType::GREATER_THAN, 2},
    {RSIInstructionType::GREATER_THAN_OR_EQUAL, 2},

    {RSIInstructionType::LOGICAL_AND, 2},
    {RSIInstructionType::LOGICAL_OR, 2},

    {RSIInstructionType::BINARY_AND, 2},
};

inline const std::map<RSIInstructionType, std::string> RSIMnemonic = {
    {RSIInstructionType::NOP, "nop"},

    {RSIInstructionType::MOVE, "mov"},
    {RSIInstructionType::RETURN, "ret"},

    {RSIInstructionType::NEGATE, "neg"},
    {RSIInstructionType::BINARY_NOT, "bnot"},
    {RSIInstructionType::LOGICAL_NOT, "lnot"},

    {RSIInstructionType::ADD, "add"},
    {RSIInstructionType::SUBTRACT, "sub"},
    {RSIInstructionType::MULTIPLY, "mul"},
    {RSIInstructionType::DIVIDE, "div"},
    {RSIInstructionType::MODULO, "mod"},

    {RSIInstructionType::EQUAL, "eq"},
    {RSIInstructionType::NOT_EQUAL, "neq"},
    {RSIInstructionType::LESS_THAN, "lt"},
    {RSIInstructionType::LESS_THAN_OR_EQUAL, "leq"},
    {RSIInstructionType::GREATER_THAN, "gt"},
    {RSIInstructionType::GREATER_THAN_OR_EQUAL, "geq"},

    {RSIInstructionType::LOGICAL_AND, "land"},
    {RSIInstructionType::LOGICAL_OR, "lor"},

    {RSIInstructionType::BINARY_AND, "band"},
};

struct HWRegister{
    HWRegister(): id(highestID++){
    }

    bool operator==(HWRegister const& other) const{
        return this->getID() == other.getID();
    }

    bool operator <(HWRegister const& other) const{
        return this->getID() < other.getID();
    }

    inline uint64_t getID() const{
        return id;
    }

    private:
        uint64_t id;

    private:
        static inline std::atomic<uint64_t> highestID = 0;
};

namespace std{
    template<>
    struct hash<HWRegister>{
        size_t operator() (HWRegister const& reg) const{
            return std::hash<uint64_t>()(reg.getID());
        }
    };
}

struct RSIConstant{
    uint64_t value;
};

struct RSIReference{
    std::string name;
    std::optional<std::shared_ptr<SemanticVariableData>> variable;
    std::optional<HWRegister> assignedRegister;

    bool operator < (RSIReference const& other) const{
        return name < other.name;
    }

    bool operator == (RSIReference const& other) const{
        return name == other.name;
    }
};

using RSIOperand = std::variant<std::monostate, RSIConstant, RSIReference>;


struct RSIInstruction{
    RSIInstructionType type;
    RSIOperand result;
    RSIOperand op1;
    RSIOperand op2;

    struct Metadata{
        std::set<RSIReference> liveVariablesAfter;
    } meta;
};

struct RSIFunction{
    std::string name;
    std::shared_ptr<SemanticFunctionData> function;
    std::vector<RSIInstruction> instructions;
};