#pragma once

#include "R-Sharp/Logging.hpp"
#include "R-Sharp/Token.hpp"
#include "R-Sharp/AstNodesFWD.hpp"
#include "R-Sharp/AstVisitor.hpp"

#include <vector>
#include <string>
#include <memory>

template<typename... Args>
std::vector<std::shared_ptr<AstNode>> combineChildren(Args... args){
    std::vector<std::shared_ptr<AstNode>> children;
    (children.push_back(std::dynamic_pointer_cast<AstNode>(args)), ...);
    return children;
}

// Helper macros to make it easier to define AST nodes
#define BASE(NAME) \
    NAME() = default; \
    NAME(Token const& token) {this->token = token;} \
    AstNodeType getType() const override { return AstNodeType::NAME; } \
    void accept(AstVisitor* visitor) override{ visitor->visit(shared_from_this()); }

#define CHILD_INIT(NAME, TYPE, VARIABLE_NAME) \
    NAME(std::shared_ptr<TYPE> child) : VARIABLE_NAME(child) {}


#define SINGLE_CHILD(TYPE, VARIABLE_NAME) \
    std::shared_ptr<TYPE> VARIABLE_NAME;

#define GET_SINGLE_CHILDREN(...) \
    std::vector<std::shared_ptr<AstNode>> getChildren() const override{ \
        return combineChildren(semanticType, __VA_ARGS__); \
    }

#define MULTI_CHILD(TYPE, VARIABLE_NAME) std::vector<std::shared_ptr<TYPE>> VARIABLE_NAME;

#define GET_MULTI_CHILD(VARIABLE_NAME)\
    std::vector<std::shared_ptr<AstNode>> getChildren() const override{ \
        std::vector<std::shared_ptr<AstNode>> internal_children = {std::dynamic_pointer_cast<AstNode>(semanticType)}; \
        for(auto& child : VARIABLE_NAME) \
            internal_children.push_back(std::dynamic_pointer_cast<AstNode>(child)); \
        return internal_children; \
    } \


#define TO_STRING(NAME) \
    std::string toString() const override{ \
        return #NAME; \
    }

#define TO_STRING_NAME(NAME) \
    NAME(std::string const& name): name(name){} \
    std::string toString() const override{ \
        return std::string(#NAME) + ": " + name; \
    }\
    std::string name;

#define DESTRUCTOR(NAME)\
    virtual ~NAME() = default;


struct SemanticVariableData{
    bool isGlobal = false;
    bool isDefined = false;
    RSharpType type;
    std::string name;
    int sizeInBytes = 0;
    std::string accessStr;

    bool operator ==(SemanticVariableData other) const{
        return isGlobal == other.isGlobal && type == type;
    }
};

struct SemanticLoopData{
    std::string breakAccessString;
    std::string skipAccessString;
};

struct SemanticFunctionData{
    std::string name;
    std::string accessString;
    bool isDefined = false;
    std::shared_ptr<AstParameterList> parameters;
    RSharpType returnType;
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
struct AstExpression : public virtual AstNode{
    DESTRUCTOR(AstExpression)
};
struct AstBlockItem : public virtual AstNode{
    DESTRUCTOR(AstBlockItem)
};
struct AstDeclaration : public AstBlockItem{
    DESTRUCTOR(AstDeclaration)
};
struct AstStatement : public AstBlockItem{
    DESTRUCTOR(AstStatement)
};
struct AstErrorNode : public virtual AstNode{
    DESTRUCTOR(AstErrorNode)
    AstErrorNode() = default;
};
struct AstProgramItem : public virtual AstNode{
    DESTRUCTOR(AstProgramItem)
};

// ----------------------------------| Program Items |---------------------------------- //

struct AstFunctionDeclaration : public AstProgramItem, public std::enable_shared_from_this<AstFunctionDeclaration> {
    BASE(AstFunctionDeclaration)
    TO_STRING_NAME(AstFunctionDeclaration)

    GET_SINGLE_CHILDREN(parameters, body)

    SINGLE_CHILD(AstBlock, body)
    SINGLE_CHILD(AstParameterList, parameters)

    std::shared_ptr<SemanticFunctionData> function;
};


// ----------------------------------| Errors |---------------------------------- //

struct AstErrorStatement : public AstStatement, public AstErrorNode, public std::enable_shared_from_this<AstErrorStatement> {
    BASE(AstErrorStatement)
    TO_STRING_NAME(AstErrorStatement)
};
struct AstErrorProgramItem : public AstProgramItem, public AstErrorNode, public std::enable_shared_from_this<AstErrorProgramItem> {
    BASE(AstErrorProgramItem)
    TO_STRING_NAME(AstErrorProgramItem)
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
struct AstVariableAccess : public AstExpression, public std::enable_shared_from_this<AstVariableAccess> {
    BASE(AstVariableAccess)
    TO_STRING_NAME(AstVariableAccess)

    std::shared_ptr<SemanticVariableData> variable;
};

struct AstInteger : public AstExpression, public std::enable_shared_from_this<AstInteger> {
    BASE(AstInteger)
    AstInteger(int64_t value): value(value){}
    
    std::string toString() const override{
        return "AstInteger: " + std::to_string(value);
    }
    int64_t value;
};

struct AstVariableAssignment : AstExpression, public std::enable_shared_from_this<AstVariableAssignment>{
    BASE(AstVariableAssignment)
    TO_STRING_NAME(AstVariableAssignment)

    CHILD_INIT(AstVariableAssignment, AstExpression, value)
    GET_SINGLE_CHILDREN(value)

    SINGLE_CHILD(AstExpression, value)

    std::shared_ptr<SemanticVariableData> variable;
};

struct AstUnary : public AstExpression, public std::enable_shared_from_this<AstUnary> {
    BASE(AstUnary)
    AstUnary(AstUnaryType op, std::shared_ptr<AstExpression> value): type(op), value(value){}
    std::string toString() const override{
        switch (type){
            case AstUnaryType::BinaryNot: return "AstUnary: BinaryNot";
            case AstUnaryType::LogicalNot: return "AstUnary: LogicalNot";
            case AstUnaryType::Negate: return "AstUnary: Negate";

            default: return "AstUnary: Unknown type";
        }
    }
    GET_SINGLE_CHILDREN(value)

    SINGLE_CHILD(AstExpression, value)
    AstUnaryType type = AstUnaryType::None;
};

struct AstBinary : public AstExpression, public std::enable_shared_from_this<AstBinary> {
    BASE(AstBinary)
    AstBinary(std::shared_ptr<AstExpression> left, AstBinaryType type, std::shared_ptr<AstExpression> right)
        : left(left), type(type), right(right){}
    std::string toString() const override{
        switch (type){
            #define CASE(NAME) case AstBinaryType::NAME: return "AstBinary: "#NAME
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
            #undef CASE
        }
    }

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

// ----------------------------------| Declarations |---------------------------------- //
struct AstVariableDeclaration : public AstDeclaration, public AstProgramItem, public std::enable_shared_from_this<AstVariableDeclaration> {
    BASE(AstVariableDeclaration)
    TO_STRING_NAME(AstVariableDeclaration)

    GET_SINGLE_CHILDREN(value)

    SINGLE_CHILD(AstExpression, value)
    
    std::shared_ptr<SemanticVariableData> variable;
};


RSharpType stringToType(std::string const& str);
std::string typeToString(RSharpType type);

struct AstType : public AstNode, public std::enable_shared_from_this<AstType>{
    AstType(RSharpType type);
    AstType(RSharpType type, std::shared_ptr<AstType> subtype);


    BASE(AstType);
    std::string toString() const override{
        return "Semantic Type: " + typeToString(type);
    }
    
    RSharpType type = RSharpType::NONE;
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