#include "R-Sharp/ast/AstNodes.hpp"
#include "R-Sharp/Logging.hpp"

AstUnaryType toUnaryOperator(TokenType type) {
    switch (type) {
        case TokenType::Minus: return AstUnaryType::Negate;
        case TokenType::Bang:  return AstUnaryType::LogicalNot;
        case TokenType::Tilde: return AstUnaryType::BinaryNot;
        default:               Fatal("Invalid unary operator");
    }
}

AstBinaryType toBinaryOperator(TokenType type) {
    switch (type) {
        case TokenType::Plus:             return AstBinaryType::Add;
        case TokenType::Minus:            return AstBinaryType::Subtract;
        case TokenType::Star:             return AstBinaryType::Multiply;
        case TokenType::Slash:            return AstBinaryType::Divide;
        case TokenType::Percent:          return AstBinaryType::Modulo;

        case TokenType::EqualEqual:       return AstBinaryType::Equal;
        case TokenType::NotEqual:         return AstBinaryType::NotEqual;
        case TokenType::LessThan:         return AstBinaryType::LessThan;
        case TokenType::GreaterThan:      return AstBinaryType::GreaterThan;
        case TokenType::LessThanEqual:    return AstBinaryType::LessThanOrEqual;
        case TokenType::GreaterThanEqual: return AstBinaryType::GreaterThanOrEqual;

        case TokenType::DoubleAmpersand:  return AstBinaryType::LogicalAnd;
        case TokenType::DoublePipe:       return AstBinaryType::LogicalOr;

        default:                          Fatal("Invalid binary operator");
    }
}


bool operator==(AstVariableDeclaration const& a, AstVariableDeclaration const& b) {
    if (a.name.size() && b.name.size() && a.name != b.name) {
        return false;
    }
    if (a.semanticType != b.semanticType) return false;


    return true;
}
bool operator!=(AstVariableDeclaration const& a, AstVariableDeclaration const& b) {
    return !(a == b);
}

bool operator==(AstFunctionDefinition const& a, AstFunctionDefinition const& b) {
    if (a.name != b.name) {
        return false;
    }
    if (a.semanticType != b.semanticType) {
        return false;
    }
    if (a.parameters != b.parameters) {
        return false;
    }
    return true;
}

bool operator==(AstParameterList const& a, AstParameterList const& b) {
    if (a.parameters.size() != b.parameters.size()) {
        return false;
    }

    for (int i = 0; i < a.parameters.size(); i++) {
        if (*a.parameters.at(i) != *b.parameters.at(i)) {
            return false;
        }
    }
    return true;
}
bool operator!=(AstParameterList const& a, AstParameterList const& b) {
    return !(a == b);
}
