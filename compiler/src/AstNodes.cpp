#include "R-Sharp/AstNodes.hpp"

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
        case TokenType::Percent:
            return AstBinaryType::Modulo;
        
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


bool operator==(AstVariableDeclaration const& a, AstVariableDeclaration const& b){
    if (a.name.size() && b.name.size() && a.name != b.name){
        return false;
    }
    if (a.semanticType && b.semanticType && *a.semanticType != *b.semanticType)
        return false;
    

    return true;
}
bool operator!=(AstVariableDeclaration const& a, AstVariableDeclaration const& b){
    return !(a==b);
}

bool operator==(AstFunctionDeclaration const& a, AstFunctionDeclaration const& b){
    if (a.name != b.name){
        return false;
    }
    if (a.semanticType && b.semanticType && *a.semanticType != *b.semanticType){
        return false;
    }
    if (a.parameters && b.parameters && *a.parameters != *b.parameters){
        return false;
    }
    return true;
}

bool operator==(AstParameterList const& a, AstParameterList const& b){
    if (a.parameters.size() != b.parameters.size()){
        return false;
    }

    for (int i=0; i<a.parameters.size(); i++){
        if (*a.parameters.at(i) != *b.parameters.at(i)){
            return false;
        }
    }
    return true;
}
bool operator!=(AstParameterList const& a, AstParameterList const& b){
    return !(a==b);
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