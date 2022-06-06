#include "R-Sharp/AstNodes.hpp"


void AstNode::printTree(std::string prefix, bool isTail) const{
    std::string nodeConnection = isTail ? "└── " : "├── ";
    Print(prefix, nodeConnection, toString());

    auto const& children = getChildren();
    for (int i = 0; i < children.size(); i++) {
        std::string newPrefix = prefix + (isTail ? "    " : "│   ");
        children.at(i)->printTree(newPrefix, i == children.size()-1);
    }
}
std::vector<std::shared_ptr<AstNode>> AstNode::getChildren() const {return {};}
void AstNode::generateCCode(std::string& output){Fatal("Unimplemented Node!"); output += "!!!Unimplemented Node!!!";}


std::vector<std::shared_ptr<AstNode>> AstProgram::getChildren() const {
    std::vector<std::shared_ptr<AstNode>> children;
    children.reserve(functions.size());
    for (int i = 0; i < functions.size(); i++) {
        children.push_back(std::static_pointer_cast<AstNode>(functions.at(i)));
    }
    return children;
}
AstNodeType AstProgram::getType() const {return AstNodeType::PROGRAM;}
std::string AstProgram::toString() const {return "Program";}



std::vector<std::shared_ptr<AstNode>> AstFunction::getChildren() const {
    return {std::static_pointer_cast<AstNode>(parameters), std::static_pointer_cast<AstNode>(returnType), std::static_pointer_cast<AstNode>(body)};
}

AstNodeType AstFunction::getType() const {return AstNodeType::FUNCTION;}
std::string AstFunction::toString() const {return "Function: " + name;}


std::vector<std::shared_ptr<AstNode>> AstBlock::getChildren() const {
    std::vector<std::shared_ptr<AstNode>> children;
    children.reserve(statements.size());
    for (int i = 0; i < statements.size(); i++) {
        children.push_back(std::static_pointer_cast<AstNode>(statements.at(i)));
    }
    return children;
}
AstNodeType AstBlock::getType() const {return AstNodeType::BLOCK;}
std::string AstBlock::toString() const {return "Block";}


std::vector<std::shared_ptr<AstNode>> AstReturn::getChildren() const {
    return {std::static_pointer_cast<AstNode>(value)};
}
AstNodeType AstReturn::getType() const {return AstNodeType::RETURN;}
std::string AstReturn::toString() const {return "Return";}

AstNodeType AstExpressionStatement::getType() const {return AstNodeType::EXPRESSION_STATEMENT;}
std::string AstExpressionStatement::toString() const {return "ExpressionStatement";}
std::vector<std::shared_ptr<AstNode>> AstExpressionStatement::getChildren() const {
    return {std::static_pointer_cast<AstNode>(expression)};
}


std::vector<std::shared_ptr<AstNode>> AstUnary::getChildren() const {
    return {std::static_pointer_cast<AstNode>(value)};
}
AstNodeType AstUnary::getType() const {return AstNodeType::UNARY;}
std::string AstUnary::toString() const {return "Unary " + std::to_string(type);}


std::vector<std::shared_ptr<AstNode>> AstBinary::getChildren() const {
    return {std::static_pointer_cast<AstNode>(left), std::static_pointer_cast<AstNode>(right)};
}
AstNodeType AstBinary::getType() const {return AstNodeType::BINARY;}
std::string AstBinary::toString() const {return "Binary " + std::to_string(type);}


AstNodeType AstInteger::getType() const {return AstNodeType::INTEGER;}
std::string AstInteger::toString() const {return "Int: " + std::to_string(value);}


AstNodeType AstVariableAccess::getType() const {return AstNodeType::VARIABLE_ACCESS;}
std::string AstVariableAccess::toString() const {return "VariableAccess: " + name;}

AstNodeType AstVariableAssignment::getType() const {return AstNodeType::VARIABLE_ASSIGNMENT;}
std::string AstVariableAssignment::toString() const {return "VariableAssignment: " + name;}
std::vector<std::shared_ptr<AstNode>> AstVariableAssignment::getChildren() const {
    return {std::static_pointer_cast<AstNode>(value)};
}

std::vector<std::shared_ptr<AstNode>> AstVariableDeclaration::getChildren() const {
    return {std::static_pointer_cast<AstNode>(type)};
}
AstNodeType AstVariableDeclaration::getType() const {return AstNodeType::VARIABLE_DECLARATION;}
std::string AstVariableDeclaration::toString() const {return "Variable: " + name;}


std::vector<std::shared_ptr<AstNode>> AstBuiltinType::getChildren() const {
    std::vector<std::shared_ptr<AstNode>> children;
    children.reserve(modifiers.size());
    for (int i = 0; i < modifiers.size(); i++) {
        children.push_back(std::static_pointer_cast<AstNode>(modifiers.at(i)));
    }
    return children;
}
AstNodeType AstBuiltinType::getType() const {return AstNodeType::BUILTIN_TYPE;}
std::string AstBuiltinType::toString() const {return "Builtin Type: " + name;}


AstNodeType AstTypeModifier::getType() const {return AstNodeType::TYPE_MODIFIER;}
std::string AstTypeModifier::toString() const {return "TypeModifier: " + name;}


std::vector<std::shared_ptr<AstNode>> AstParameterList::getChildren() const {
    std::vector<std::shared_ptr<AstNode>> children;
    children.reserve(parameters.size());
    for (int i = 0; i < parameters.size(); i++) {
        children.push_back(std::static_pointer_cast<AstNode>(parameters.at(i)));
    }
    return children;
}
AstNodeType AstParameterList::getType() const {return AstNodeType::PARAMETER_LIST;}
std::string AstParameterList::toString() const {return "Parameters";}


std::vector<std::shared_ptr<AstNode>> AstArray::getChildren() const {
    return {std::static_pointer_cast<AstNode>(type)};
}
AstNodeType AstArray::getType() const {return AstNodeType::ARRAY;}
std::string AstArray::toString() const {return "Array";}


AstUnary::Type toUnaryOperator(TokenType type){
    switch (type) {
        case TokenType::Minus:
            return AstUnary::Type::Negate;
        case TokenType::Bang:
            return AstUnary::Type::LogicalNot;
        case TokenType::Tilde:
            return AstUnary::Type::BinaryNot;
        default:
            Fatal("Invalid unary operator");
            return AstUnary::Type::None;
    }
}

AstBinary::Type toBinaryOperator(TokenType type){
    switch (type) {
        case TokenType::Plus:
            return AstBinary::Type::Add;
        case TokenType::Minus:
            return AstBinary::Type::Subtract;
        case TokenType::Star:
            return AstBinary::Type::Multiply;
        case TokenType::Slash:
            return AstBinary::Type::Divide;
        
        case TokenType::EqualEqual:
            return AstBinary::Type::Equal;
        case TokenType::NotEqual:
            return AstBinary::Type::NotEqual;
        case TokenType::LessThan:
            return AstBinary::Type::LessThan;
        case TokenType::GreaterThan:
            return AstBinary::Type::GreaterThan;
        case TokenType::LessThanEqual:
            return AstBinary::Type::LessThanOrEqual;
        case TokenType::GreaterThanEqual:
            return AstBinary::Type::GreaterThanOrEqual;
        
        case TokenType::DoubleAmpersand:
            return AstBinary::Type::LogicalAnd;
        case TokenType::DoublePipe:
            return AstBinary::Type::LogicalOr;

        default:
            Fatal("Invalid binary operator");
            return AstBinary::Type::None;
    }
}

namespace std{
    std::string to_string(AstUnary::Type type){
        switch (type) {
            case AstUnary::Type::Negate: return "Negate";
            case AstUnary::Type::LogicalNot: return "LogicalNot";
            case AstUnary::Type::BinaryNot: return "BinaryNot";
            default: return "Unknown";
        }
    }
    std::string to_string(AstBinary::Type type){
        switch (type) {
            case AstBinary::Type::Add: return "Add";
            case AstBinary::Type::Subtract: return "Subtract";
            case AstBinary::Type::Multiply: return "Multiply";
            case AstBinary::Type::Divide: return "Divide";

            case AstBinary::Type::Equal: return "Equal";
            case AstBinary::Type::NotEqual: return "NotEqual";
            case AstBinary::Type::LessThan: return "LessThan";
            case AstBinary::Type::GreaterThan: return "GreaterThan";
            case AstBinary::Type::LessThanOrEqual: return "LessThanOrEqual";
            case AstBinary::Type::GreaterThanOrEqual: return "GreaterThanOrEqual";

            case AstBinary::Type::LogicalAnd: return "LogicalAnd";
            case AstBinary::Type::LogicalOr: return "LogicalOr";

            default: return "Unknown";
        }
    }
}