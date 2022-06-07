#include "R-Sharp/AstNodes.hpp"


void AstNode::printTree(std::string prefix, bool isTail) const{
    std::string nodeConnection = isTail ? "└── " : "├── ";
    Print(prefix, nodeConnection, toString());

    auto const& children = getChildren();
    for (int i = 0; i < children.size(); i++) {
        if (!children.at(i)) continue;
        std::string newPrefix = prefix + (isTail ? "    " : "│   ");
        children.at(i)->printTree(newPrefix, i == children.size()-1);
    }
}
void AstNode::generateCCode(std::string& output){Fatal("Unimplemented Node!"); output += "!!!Unimplemented Node!!!";}

AstUnaryType toUnaryOperator(TokenType type){
    switch (type) {
        case TokenType::Minus:
            return AstUnaryType::Negate;
        case TokenType::Bang:
            return AstUnaryType::LogicalNot;
        case TokenType::Tilde:
            return AstUnaryType::BinaryNot;
        default:
            Fatal("Invalid unary operator");
            return AstUnaryType::None;
    }
}

AstBinaryType toBinaryOperator(TokenType type){
    switch (type) {
        case TokenType::Plus:
            return AstBinaryType::Add;
        case TokenType::Minus:
            return AstBinaryType::Subtract;
        case TokenType::Star:
            return AstBinaryType::Multiply;
        case TokenType::Slash:
            return AstBinaryType::Divide;
        
        case TokenType::EqualEqual:
            return AstBinaryType::Equal;
        case TokenType::NotEqual:
            return AstBinaryType::NotEqual;
        case TokenType::LessThan:
            return AstBinaryType::LessThan;
        case TokenType::GreaterThan:
            return AstBinaryType::GreaterThan;
        case TokenType::LessThanEqual:
            return AstBinaryType::LessThanOrEqual;
        case TokenType::GreaterThanEqual:
            return AstBinaryType::GreaterThanOrEqual;
        
        case TokenType::DoubleAmpersand:
            return AstBinaryType::LogicalAnd;
        case TokenType::DoublePipe:
            return AstBinaryType::LogicalOr;

        default:
            Fatal("Invalid binary operator");
            return AstBinaryType::None;
    }
}

namespace std{
    std::string to_string(AstUnaryType type){
        switch (type) {
            case AstUnaryType::Negate: return "Negate";
            case AstUnaryType::LogicalNot: return "LogicalNot";
            case AstUnaryType::BinaryNot: return "BinaryNot";
            default: return "Unknown";
        }
    }
    std::string to_string(AstBinaryType type){
        switch (type) {
            case AstBinaryType::Add: return "Add";
            case AstBinaryType::Subtract: return "Subtract";
            case AstBinaryType::Multiply: return "Multiply";
            case AstBinaryType::Divide: return "Divide";

            case AstBinaryType::Equal: return "Equal";
            case AstBinaryType::NotEqual: return "NotEqual";
            case AstBinaryType::LessThan: return "LessThan";
            case AstBinaryType::GreaterThan: return "GreaterThan";
            case AstBinaryType::LessThanOrEqual: return "LessThanOrEqual";
            case AstBinaryType::GreaterThanOrEqual: return "GreaterThanOrEqual";

            case AstBinaryType::LogicalAnd: return "LogicalAnd";
            case AstBinaryType::LogicalOr: return "LogicalOr";

            default: return "Unknown";
        }
    }
}