#pragma once

#include <cstdint>
#include <string>
#include <optional>
#include <variant>
#include <map>
#include <atomic>
#include <set>

#include "R-Sharp/RSI_FWD.hpp"
#include "R-Sharp/AstNodes.hpp"
#include "R-Sharp/Logging.hpp"

namespace RSI {

inline const std::map<InstructionType, uint> numArgumentsUsed = {
    {InstructionType::NOP,                   0},

    {InstructionType::MOVE,                  1},
    {InstructionType::RETURN,                1},

    {InstructionType::NEGATE,                1},
    {InstructionType::BINARY_NOT,            1},
    {InstructionType::LOGICAL_NOT,           1},

    {InstructionType::ADD,                   2},
    {InstructionType::SUBTRACT,              2},
    {InstructionType::MULTIPLY,              2},
    {InstructionType::DIVIDE,                2},
    {InstructionType::MODULO,                2},

    {InstructionType::EQUAL,                 2},
    {InstructionType::NOT_EQUAL,             2},
    {InstructionType::LESS_THAN,             2},
    {InstructionType::LESS_THAN_OR_EQUAL,    2},
    {InstructionType::GREATER_THAN,          2},
    {InstructionType::GREATER_THAN_OR_EQUAL, 2},

    {InstructionType::LOGICAL_AND,           2},
    {InstructionType::LOGICAL_OR,            2},

    {InstructionType::BINARY_AND,            2},

    {InstructionType::JUMP,                  1},
    {InstructionType::JUMP_IF_ZERO,          2},
    {InstructionType::DEFINE_LABEL,          1},

    {InstructionType::STORE_PARAMETER,       1},
    {InstructionType::LOAD_PARAMETER,        1},
    {InstructionType::CALL,                  2},

    {InstructionType::FUNCTION_BEGIN,        0},

    {InstructionType::STORE_MEMORY,          2},
    {InstructionType::LOAD_MEMORY,           1},

    {InstructionType::ADDRESS_OF,            1},
    {InstructionType::SET_LIVE,              0},
};

inline const std::map<InstructionType, std::string> mnemonics = {
    {InstructionType::NOP,                   "nop" },

    {InstructionType::MOVE,                  "mov" },
    {InstructionType::RETURN,                "ret" },

    {InstructionType::NEGATE,                "neg" },
    {InstructionType::BINARY_NOT,            "bnot"},
    {InstructionType::LOGICAL_NOT,           "lnot"},

    {InstructionType::ADD,                   "add" },
    {InstructionType::SUBTRACT,              "sub" },
    {InstructionType::MULTIPLY,              "mul" },
    {InstructionType::DIVIDE,                "div" },
    {InstructionType::MODULO,                "mod" },

    {InstructionType::EQUAL,                 "eq"  },
    {InstructionType::NOT_EQUAL,             "neq" },
    {InstructionType::LESS_THAN,             "lt"  },
    {InstructionType::LESS_THAN_OR_EQUAL,    "leq" },
    {InstructionType::GREATER_THAN,          "gt"  },
    {InstructionType::GREATER_THAN_OR_EQUAL, "geq" },

    {InstructionType::LOGICAL_AND,           "land"},
    {InstructionType::LOGICAL_OR,            "lor" },

    {InstructionType::BINARY_AND,            "band"},

    {InstructionType::JUMP,                  "jmp" },
    {InstructionType::JUMP_IF_ZERO,          "jmpz"},
    {InstructionType::DEFINE_LABEL,          "defl"},

    {InstructionType::STORE_PARAMETER,       "spar"},
    {InstructionType::LOAD_PARAMETER,        "lpar"},
    {InstructionType::CALL,                  "call"},

    {InstructionType::FUNCTION_BEGIN,        "fbeg"},

    {InstructionType::STORE_MEMORY,          "smem"},
    {InstructionType::LOAD_MEMORY,           "lmem"},

    {InstructionType::ADDRESS_OF,            "adof"},
    {InstructionType::SET_LIVE,              "setl"},
};

struct HWRegister {
    HWRegister(): id(highestID++) {}

    constexpr bool operator==(HWRegister const& other) const {
        return this->getID() == other.getID();
    }
    constexpr bool operator!=(HWRegister const& other) const {
        return !(*this == other);
    }

    constexpr bool operator<(HWRegister const& other) const {
        return this->getID() < other.getID();
    }

    constexpr inline uint64_t getID() const {
        return id;
    }

private:
    uint64_t id;

private:
    static inline std::atomic<uint64_t> highestID = 0;
};

struct StackSlot {
    uint64_t offset = 0;
};


struct Reference {
    std::string name;
    std::optional<std::shared_ptr<SemanticVariableData>> variable;
    StorageLocation storageLocation;

    bool operator<(Reference const& other) const {
        return name < other.name;
    }

    bool operator==(Reference const& other) const {
        return name == other.name;
    }
    bool operator!=(Reference const& other) const {
        return !(*this == other);
    }
};

struct GlobalReference {
    std::string name;
    std::optional<std::shared_ptr<SemanticVariableData>> variable;

    bool operator<(GlobalReference const& other) const {
        return name < other.name;
    }

    bool operator==(GlobalReference const& other) const {
        return name == other.name;
    }
    bool operator!=(GlobalReference const& other) const {
        return !(*this == other);
    }
};
struct Label {
    std::string name;

    bool operator<(Label const& other) const {
        return name < other.name;
    }

    bool operator==(Label const& other) const {
        return name == other.name;
    }
    bool operator!=(Label const& other) const {
        return !(*this == other);
    }
};


struct Instruction {
    InstructionType type;
    Operand result;
    Operand op1;
    Operand op2;

    struct Metadata {
        std::set<std::shared_ptr<Reference>> liveVariablesBefore = {};
    } meta;
};

struct Function {
    std::string name;
    std::shared_ptr<SemanticFunctionData> function;
    std::vector<Instruction> instructions;

    struct Metadata {
        std::set<std::shared_ptr<Reference>> allReferences = {};
        std::set<HWRegister> allRegisters = {};
        uint64_t maxStackUsage = 0;
    } meta;
};

struct TranslationUnit {
    std::vector<RSI::Function> functions;
    std::vector<std::shared_ptr<RSI::Label>> externLabels;
    std::vector<std::pair<std::shared_ptr<RSI::GlobalReference>, RSI::Constant>> initializedGlobalVariables;
    std::vector<std::shared_ptr<RSI::GlobalReference>> uninitializedGlobalVariables;
};

}


namespace std {
template <>
struct hash<RSI::HWRegister> {
    size_t operator()(RSI::HWRegister const& reg) const {
        return std::hash<uint64_t>()(reg.getID());
    }
};
}