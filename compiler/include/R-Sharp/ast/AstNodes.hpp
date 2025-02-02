#pragma once

#include "R-Sharp/frontend/Token.hpp"
#include "R-Sharp/ast/AstNodesFWD.hpp"
#include "R-Sharp/ast/AstVisitor.hpp"
#include "R-Sharp/backend/RSI_FWD.hpp"

#include <vector>
#include <string>
#include <memory>
#include <variant>
#include <optional>

template <typename... Args>
std::vector<std::shared_ptr<AstNode>> combineChildren(Args... args) {
    std::vector<std::shared_ptr<AstNode>> children;
    (children.push_back(std::dynamic_pointer_cast<AstNode>(args)), ...);
    return children;
}

// Helper macros to make it easier to define AST nodes
#define BASE(NAME)                              \
    NAME() = default;                           \
    NAME(Token const& token) {                  \
        this->token = token;                    \
    }                                           \
    AstNodeType getType() const override {      \
        return AstNodeType::NAME;               \
    }                                           \
    void accept(AstVisitor* visitor) override { \
        visitor->visit(shared_from_this());     \
    }

#define CHILD_INIT(NAME, TYPE, VARIABLE_NAME) \
    NAME(std::shared_ptr<TYPE> child): VARIABLE_NAME(child) {}


#define SINGLE_CHILD(TYPE, VARIABLE_NAME) std::shared_ptr<TYPE> VARIABLE_NAME;

#define GET_SINGLE_CHILDREN(...)                                         \
    std::vector<std::shared_ptr<AstNode>> getChildren() const override { \
        return combineChildren(semanticType, __VA_ARGS__);               \
    }

#define MULTI_CHILD(TYPE, VARIABLE_NAME) std::vector<std::shared_ptr<TYPE>> VARIABLE_NAME;

#define GET_MULTI_CHILD(VARIABLE_NAME)                                                                                \
    std::vector<std::shared_ptr<AstNode>> getChildren() const override {                                              \
        std::vector<std::shared_ptr<AstNode>> internal_children = {std::dynamic_pointer_cast<AstNode>(semanticType)}; \
        for (auto& child : VARIABLE_NAME)                                                                             \
            internal_children.push_back(std::dynamic_pointer_cast<AstNode>(child));                                   \
        return internal_children;                                                                                     \
    }


#define TO_STRING(NAME)                     \
    std::string toString() const override { \
        return #NAME;                       \
    }

#define TO_STRING_NAME(NAME)                     \
    NAME(std::string const& name): name(name) {} \
    std::string toString() const override {      \
        return std::string(#NAME) + ": " + name; \
    }                                            \
    std::string name;

#define DESTRUCTOR(NAME) virtual ~NAME() = default;


struct SemanticVariableData {
    bool isGlobal = false;
    bool isDefined = false;
    std::weak_ptr<AstType> type;
    std::string name;
    int sizeInBytes = 0;

    // either a global label or the stack offset
    std::variant<std::string, int, RSI::Operand> accessor =
        "Invalid global label. Most likely didn't visit this node.";

    bool operator==(SemanticVariableData other) const {
        return isGlobal == other.isGlobal && type.lock() == other.type.lock();
    }
};

struct SemanticLoopData {
    std::variant<std::string, std::shared_ptr<RSI::Label>> breakLabel;
    std::variant<std::string, std::shared_ptr<RSI::Label>> skipLabel;
    bool hasAdditionalCleanup = false;
};

struct SemanticFunctionData {
    std::string name;
    std::shared_ptr<AstParameterList> parameters;
    std::shared_ptr<AstType> returnType;
    std::shared_ptr<RSI::Label> rsiLabel;
};

// The actual AST nodes
// ----------------------------------| Program |---------------------------------- //
struct AstProgram : public virtual AstNode, public std::enable_shared_from_this<AstProgram> {
    BASE(AstProgram)
    TO_STRING(AstProgram)

    GET_MULTI_CHILD(items)

    MULTI_CHILD(AstProgramItem, items)

    std::shared_ptr<AstBlock> globalScope;
    std::vector<std::shared_ptr<SemanticVariableData>> uninitializedGlobalVariables;
};

struct AstParameterList : public virtual AstNode, public std::enable_shared_from_this<AstParameterList> {
    BASE(AstParameterList)
    TO_STRING(AstParameterList)

    GET_MULTI_CHILD(parameters)

    MULTI_CHILD(AstVariableDeclaration, parameters)

    std::shared_ptr<AstBlock> parameterBlock;
};


// ----------------------------------| Groups |---------------------------------- //
struct AstExpression : public virtual AstNode {
    DESTRUCTOR(AstExpression)
};
struct AstBlockItem : public virtual AstNode {
    DESTRUCTOR(AstBlockItem)
};
struct AstDeclaration : public AstBlockItem {
    DESTRUCTOR(AstDeclaration)
};
struct AstStatement : public AstBlockItem {
    DESTRUCTOR(AstStatement)
};
struct AstErrorNode : public virtual AstNode {
    DESTRUCTOR(AstErrorNode)
    AstErrorNode() = default;
};
struct AstProgramItem : public virtual AstNode {
    DESTRUCTOR(AstProgramItem)
};
struct AstType : public virtual AstNode {
    DESTRUCTOR(AstType)

    virtual bool isErrorType() = 0;
};

// ----------------------------------| Program Items |---------------------------------- //

struct AstFunctionDefinition : public AstProgramItem, public std::enable_shared_from_this<AstFunctionDefinition> {
    BASE(AstFunctionDefinition)
    TO_STRING_NAME(AstFunctionDefinition)

    GET_SINGLE_CHILDREN(parameters, body, tags)

    SINGLE_CHILD(AstBlock, body)
    SINGLE_CHILD(AstParameterList, parameters)
    SINGLE_CHILD(AstTags, tags)

    std::shared_ptr<SemanticFunctionData> functionData;
};


// ----------------------------------| Errors |---------------------------------- //

struct AstErrorStatement : public AstStatement, public AstErrorNode, public std::enable_shared_from_this<AstErrorStatement> {
    BASE(AstErrorStatement)
    TO_STRING_NAME(AstErrorStatement)
};
struct AstErrorProgramItem : public AstProgramItem,
                             public AstErrorNode,
                             public std::enable_shared_from_this<AstErrorProgramItem> {
    BASE(AstErrorProgramItem)
    TO_STRING_NAME(AstErrorProgramItem)
};
struct AstErrorExpression : public AstExpression, public AstErrorNode, public std::enable_shared_from_this<AstErrorExpression> {
    BASE(AstErrorExpression)
    TO_STRING_NAME(AstErrorExpression)
};
// ----------------------------------| Statements |---------------------------------- //
struct AstBlock : public AstStatement, public std::enable_shared_from_this<AstBlock> {
    BASE(AstBlock)
    TO_STRING(AstBlock)

    GET_MULTI_CHILD(items)

    MULTI_CHILD(AstBlockItem, items)

    std::vector<std::shared_ptr<SemanticVariableData>> variables;
    int sizeOfLocalVariables = 0;
    bool hasLoopCurrently = false;
    std::string name;
    // if the block got merged, it doesn't have to clean up the stack
    bool isMerged = false;
};

struct AstReturn : public AstStatement, public std::enable_shared_from_this<AstReturn> {
    BASE(AstReturn)
    TO_STRING(AstReturn)

    CHILD_INIT(AstReturn, AstExpression, value)
    GET_SINGLE_CHILDREN(value)

    SINGLE_CHILD(AstExpression, value)

    std::vector<std::weak_ptr<AstBlock>> containedScopes;
    std::weak_ptr<AstFunctionDefinition> containedFunction;
};

struct AstExpressionStatement : public AstStatement, public std::enable_shared_from_this<AstExpressionStatement> {
    BASE(AstExpressionStatement)
    TO_STRING(AstExpressionStatement)

    CHILD_INIT(AstExpressionStatement, AstExpression, expression)
    GET_SINGLE_CHILDREN(expression)

    SINGLE_CHILD(AstExpression, expression)
};


struct AstConditionalStatement : public AstStatement, public std::enable_shared_from_this<AstConditionalStatement> {
    BASE(AstConditionalStatement)
    TO_STRING(AstConditionalStatement)

    GET_SINGLE_CHILDREN(condition, trueStatement, falseStatement)

    SINGLE_CHILD(AstExpression, condition)
    SINGLE_CHILD(AstStatement, trueStatement)
    SINGLE_CHILD(AstStatement, falseStatement)
};


struct AstWhileLoop : public AstStatement, public std::enable_shared_from_this<AstWhileLoop> {
    BASE(AstWhileLoop)
    TO_STRING(AstWhileLoop)

    GET_SINGLE_CHILDREN(condition, body)

    SINGLE_CHILD(AstExpression, condition)
    SINGLE_CHILD(AstStatement, body)

    std::shared_ptr<SemanticLoopData> loop;
};


struct AstForLoopDeclaration : public AstStatement, public std::enable_shared_from_this<AstForLoopDeclaration> {
    BASE(AstForLoopDeclaration)
    TO_STRING(AstForLoopDeclaration)

    // body first is important because it contains the initialization
    GET_SINGLE_CHILDREN(initialization, condition, increment, body)

    SINGLE_CHILD(AstVariableDeclaration, initialization)
    SINGLE_CHILD(AstExpression, condition)
    SINGLE_CHILD(AstExpression, increment)

    SINGLE_CHILD(AstStatement, body)

    std::shared_ptr<AstBlock> initializationContext;
    std::shared_ptr<SemanticLoopData> loop;
};


struct AstForLoopExpression : public AstStatement, public std::enable_shared_from_this<AstForLoopExpression> {
    BASE(AstForLoopExpression)
    TO_STRING(AstForLoopExpression)

    GET_SINGLE_CHILDREN(variable, condition, increment, body)

    SINGLE_CHILD(AstExpression, variable)
    SINGLE_CHILD(AstExpression, condition)
    SINGLE_CHILD(AstExpression, increment)
    SINGLE_CHILD(AstStatement, body)

    std::shared_ptr<SemanticLoopData> loop;
};


struct AstDoWhileLoop : public AstStatement, public std::enable_shared_from_this<AstDoWhileLoop> {
    BASE(AstDoWhileLoop)
    TO_STRING(AstDoWhileLoop)

    GET_SINGLE_CHILDREN(body, condition)

    SINGLE_CHILD(AstStatement, body)
    SINGLE_CHILD(AstExpression, condition)

    std::shared_ptr<SemanticLoopData> loop;
};


struct AstBreak : public AstStatement, public std::enable_shared_from_this<AstBreak> {
    BASE(AstBreak)
    TO_STRING(AstBreak)

    std::shared_ptr<SemanticLoopData> loop;
    std::vector<std::weak_ptr<AstBlock>> containedScopes;
};


struct AstSkip : public AstStatement, public std::enable_shared_from_this<AstSkip> {
    BASE(AstSkip)
    TO_STRING(AstSkip)

    std::shared_ptr<SemanticLoopData> loop;
    std::vector<std::weak_ptr<AstBlock>> containedScopes;
};


// ----------------------------------| Expressions |---------------------------------- //


struct AstInteger : public AstExpression, public std::enable_shared_from_this<AstInteger> {
    BASE(AstInteger)
    AstInteger(int64_t value): value(value) {}

    std::string toString() const override {
        return "AstInteger: " + std::to_string(value);
    }
    int64_t value;
};
struct AstArrayLiteral : public AstExpression, public std::enable_shared_from_this<AstArrayLiteral> {
    BASE(AstArrayLiteral)

    TO_STRING(AstArrayLiteral)

    GET_MULTI_CHILD(elements)

    MULTI_CHILD(AstExpression, elements)
};

struct AstAssignment : AstExpression, public std::enable_shared_from_this<AstAssignment> {
    BASE(AstAssignment)

    GET_SINGLE_CHILDREN(rvalue, lvalue)
    TO_STRING(AstAssignment)

    SINGLE_CHILD(AstAssignLocation, lvalue)
    SINGLE_CHILD(AstExpression, rvalue)
};

struct AstUnary : public AstExpression, public std::enable_shared_from_this<AstUnary> {
    BASE(AstUnary)
    AstUnary(AstUnaryType op, std::shared_ptr<AstExpression> value): type(op), value(value) {}
    std::string toString() const override {
        switch (type) {
            case AstUnaryType::BinaryNot:  return "AstUnary: BinaryNot";
            case AstUnaryType::LogicalNot: return "AstUnary: LogicalNot";
            case AstUnaryType::Negate:     return "AstUnary: Negate";

            default:                       return "AstUnary: Unknown type";
        }
    }
    GET_SINGLE_CHILDREN(value)

    SINGLE_CHILD(AstExpression, value)
    AstUnaryType type = AstUnaryType::None;
};

struct AstBinary : public AstExpression, public std::enable_shared_from_this<AstBinary> {
    BASE(AstBinary)
    AstBinary(std::shared_ptr<AstExpression> left, AstBinaryType type, std::shared_ptr<AstExpression> right)
        : left(left), type(type), right(right) {}

#define CASE(NAME) \
    case AstBinaryType::NAME: return "AstBinary: " #NAME

    std::string toString() const override {
        switch (type) {
            CASE(Add);
            CASE(Subtract);
            CASE(Multiply);
            CASE(Divide);
            CASE(Equal);
            CASE(NotEqual);
            CASE(GreaterThan);
            CASE(GreaterThanOrEqual);
            CASE(LessThan);
            CASE(LessThanOrEqual);
            CASE(LogicalOr);
            CASE(LogicalAnd);
            CASE(Modulo);

            default: return "AstBinary: Unknown type";
        }
    }

#undef CASE

    GET_SINGLE_CHILDREN(left, right)

    SINGLE_CHILD(AstExpression, left)
    SINGLE_CHILD(AstExpression, right)

    AstBinaryType type = AstBinaryType::None;
};
struct AstConditionalExpression : public AstExpression, public std::enable_shared_from_this<AstConditionalExpression> {
    BASE(AstConditionalExpression)
    TO_STRING(AstConditionalExpression)

    GET_SINGLE_CHILDREN(condition, trueExpression, falseExpression)

    SINGLE_CHILD(AstExpression, condition)
    SINGLE_CHILD(AstExpression, trueExpression)
    SINGLE_CHILD(AstExpression, falseExpression)
};

struct AstEmptyExpression : public AstExpression, public std::enable_shared_from_this<AstEmptyExpression> {
    BASE(AstEmptyExpression)
    TO_STRING(AstEmptyExpression)
};

struct AstFunctionCall : public AstExpression, public std::enable_shared_from_this<AstFunctionCall> {
    BASE(AstFunctionCall)
    TO_STRING_NAME(AstFunctionCall)

    GET_MULTI_CHILD(arguments)

    MULTI_CHILD(AstExpression, arguments)

    std::shared_ptr<SemanticFunctionData> function;
};

struct AstTypeConversion : public AstExpression, public std::enable_shared_from_this<AstTypeConversion> {
    AstTypeConversion(std::shared_ptr<AstExpression> from, std::shared_ptr<AstType> to): value(from) {
        semanticType = to;
    };

    BASE(AstTypeConversion);
    std::string toString() const override {
        return "(" + value->semanticType->toString() + " --> " + semanticType->toString() + ")";
    }

    GET_SINGLE_CHILDREN(value)
    std::shared_ptr<AstExpression> value;
};

struct AstAddressOf : public AstExpression, public std::enable_shared_from_this<AstAddressOf> {
    BASE(AstAddressOf)
    AstAddressOf(std::shared_ptr<AstExpression> operand): operand(operand) {}

    TO_STRING(AstAddressOf)

    GET_SINGLE_CHILDREN(operand)

    SINGLE_CHILD(AstExpression, operand)
};

struct AstVariableAccess : public AstExpression, public std::enable_shared_from_this<AstVariableAccess> {
    BASE(AstVariableAccess)
    TO_STRING_NAME(AstVariableAccess)

    std::shared_ptr<SemanticVariableData> variable;
};

struct AstDereference : public AstExpression, public std::enable_shared_from_this<AstDereference> {
    BASE(AstDereference)
    AstDereference(std::shared_ptr<AstExpression> operand): operand(operand) {}

    TO_STRING(AstDereference)

    GET_SINGLE_CHILDREN(operand)

    SINGLE_CHILD(AstExpression, operand)
};

struct AstArrayAccess : public AstExpression, public std::enable_shared_from_this<AstArrayAccess> {
    BASE(AstArrayAccess)
    TO_STRING(AstArrayAccess)

    GET_SINGLE_CHILDREN(array, index)

    SINGLE_CHILD(AstExpression, array)
    SINGLE_CHILD(AstExpression, index)
};

struct AstAssignLocation : public AstExpression, public std::enable_shared_from_this<AstAssignLocation> {
    BASE(AstAssignLocation)
    TO_STRING(AstAssignLocation)

    GET_SINGLE_CHILDREN(expr)

    SINGLE_CHILD(AstExpression, expr)
};

// ----------------------------------| Declarations |---------------------------------- //
struct AstVariableDeclaration : public AstDeclaration,
                                public AstProgramItem,
                                public std::enable_shared_from_this<AstVariableDeclaration> {
    BASE(AstVariableDeclaration)
    TO_STRING_NAME(AstVariableDeclaration)

    GET_SINGLE_CHILDREN(value)

    SINGLE_CHILD(AstExpression, value)

    std::shared_ptr<SemanticVariableData> variable;
};


RSharpPrimitiveType stringToType(std::string const& str);
std::string typeToString(RSharpPrimitiveType type);

struct AstPrimitiveType : public AstType, public std::enable_shared_from_this<AstPrimitiveType> {
    AstPrimitiveType(RSharpPrimitiveType type): type(type) {};


    BASE(AstPrimitiveType);
    std::string toString() const override {
        return "Primitive Type: " + typeToString(type);
    }

    bool isErrorType() override {
        return type == RSharpPrimitiveType::ErrorType;
    }

    RSharpPrimitiveType type = RSharpPrimitiveType::NONE;
};


struct AstPointerType : public AstType, public std::enable_shared_from_this<AstPointerType> {
    AstPointerType(std::shared_ptr<AstType> subtype): subtype(subtype) {};


    BASE(AstPointerType);
    std::string toString() const override {
        return "Pointer to: " + subtype->toString();
    }


    bool isErrorType() override {
        return subtype->isErrorType();
    }

    GET_SINGLE_CHILDREN(subtype)
    std::shared_ptr<AstType> subtype;
};

struct AstArrayType : public AstType, public std::enable_shared_from_this<AstArrayType> {
    AstArrayType(std::shared_ptr<AstType> subtype): subtype(subtype) {};


    BASE(AstArrayType);
    std::string toString() const override {
        if (size.has_value()) {
            if (size.value()->getType() != AstNodeType::AstInteger) {
                return "Array of unknown size of: " + subtype->toString();
            }
            else {
                return "Array of size " + std::to_string(std::static_pointer_cast<AstInteger>(size.value())->value)
                     + " of: " + subtype->toString();
            }
        }
        else {
            return "Unsized array of: " + subtype->toString();
        }
    }


    bool isErrorType() override {
        return subtype->isErrorType() || (size.has_value() && size.value()->getType() != AstNodeType::AstInteger);
    }

    std::vector<std::shared_ptr<AstNode>> getChildren() const override {
        if (size.has_value()) {
            return combineChildren(subtype, size.value());
        }
        else
            return combineChildren(subtype);
    }

    std::shared_ptr<AstType> subtype;
    std::optional<std::shared_ptr<AstExpression>> size;
};


struct AstTags : public AstNode, public std::enable_shared_from_this<AstTags> {
    BASE(AstTags);

    std::string toString() const override {
        std::string str = "Tags: ";
        for (auto tag : tags) {
            switch (tag) {
                case Value::Extern: str += "extern, "; break;
                default:            str += "[unknown tag]"; break;
            }
        }
        return str.substr(0, str.length() - 2);
    }

    enum class Value {
        Extern,
    };

    std::vector<Value> tags;
};


#undef BASE
#undef GET_SINGLE_CHILDREN
#undef SINGLE_CHILD
#undef GET_MULTI_CHILD
#undef MULTI_CHILD
#undef TO_STRING
#undef TO_STRING_NAME
#undef DESTRUCTOR
#undef TOKEN
