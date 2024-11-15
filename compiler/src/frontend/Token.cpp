#include "R-Sharp/frontend/Token.hpp"
#include "R-Sharp/frontend/Utils.hpp"
#include "R-Sharp/Logging.hpp"

std::string Token::toString() const {
    return "Token(" + tokenTypeToString(type) + ", \"" + escapeString(value) + "\")";
}

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::None:             return "None";
        case TokenType::EndOfFile:        return "EOF";

        case TokenType::ID:               return "ID";
        case TokenType::Comment:          return "Comment";
        case TokenType::Number:           return "Number";
        case TokenType::Typename:         return "Typename";


        case TokenType::At:               return "At";
        case TokenType::DoubleColon:      return "DoubleColon";

        case TokenType::Semicolon:        return "Semicolon";
        case TokenType::Colon:            return "Colon";
        case TokenType::Comma:            return "Comma";

        case TokenType::LeftParen:        return "OpenParenthesis";
        case TokenType::RightParen:       return "CloseParenthesis";
        case TokenType::LeftBracket:      return "OpenBracket";
        case TokenType::RightBracket:     return "CloseBracket";
        case TokenType::LeftBrace:        return "OpenBrace";
        case TokenType::RightBrace:       return "CloseBrace";

        case TokenType::ExclamationPoint: return "ExclamationPoint";
        case TokenType::Tilde:            return "Tilde";
        case TokenType::QuestionMark:     return "QuestionMark";

        case TokenType::Plus:             return "Plus";
        case TokenType::Minus:            return "Minus";
        case TokenType::Star:             return "Star";
        case TokenType::Slash:            return "Slash";
        case TokenType::Percent:          return "Percent";

        case TokenType::DoubleAmpersand:  return "DoubleAmpersand";
        case TokenType::DoublePipe:       return "DoublePipe";
        case TokenType::EqualEqual:       return "EqualEqual";
        case TokenType::NotEqual:         return "NotEqual";
        case TokenType::LessThan:         return "LessThan";
        case TokenType::LessThanEqual:    return "LessThanEqual";
        case TokenType::GreaterThan:      return "GreaterThan";
        case TokenType::GreaterThanEqual: return "GreaterThanEqual";

        case TokenType::Return:           return "Return";

        case TokenType::If:               return "If";
        case TokenType::Elif:             return "Elif";
        case TokenType::Else:             return "Else";

        case TokenType::While:            return "While";
        case TokenType::For:              return "For";
        case TokenType::Do:               return "Do";
        case TokenType::Break:            return "Break";
        case TokenType::Skip:             return "Skip";

        case TokenType::Assign:           return "Assign";
        case TokenType::DollarSign:       return "DollarSign";

        case TokenType::CharacterLiteral: return "CharacterLiteral";
        case TokenType::StringLiteral:    return "StringLiteral";

        default:                          Fatal("Unknown token type: ", static_cast<int>(type));
    }
}
