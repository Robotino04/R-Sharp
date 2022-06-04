#include "R-Sharp/Token.hpp"
#include "R-Sharp/Utils.hpp"

namespace std{
    string to_string(Token const& token){
        return "Token(" + to_string(token.type) + ", \"" + escapeString(token.value) + "\")";
    }

    string to_string(TokenType const& type){
        switch(type){
            case TokenType::None: return "None";
            case TokenType::ID: return "ID";
            case TokenType::Number: return "Number";
            case TokenType::Typename: return "Typename";
            case TokenType::TypeModifier: return "TypeModifier";
            case TokenType::Semicolon: return "Semicolon";
            case TokenType::Colon: return "Colon";
            case TokenType::Comma: return "Comma";
            case TokenType::LeftParen: return "OpenParenthesis";
            case TokenType::RightParen: return "CloseParenthesis";
            case TokenType::LeftBracket: return "OpenBracket";
            case TokenType::RightBracket: return "CloseBracket";
            case TokenType::LeftBrace: return "OpenBrace";
            case TokenType::RightBrace: return "CloseBrace";
            case TokenType::Star: return "Star";
            case TokenType::Comment: return "Comment";
            case TokenType::EndOfFile: return "EOF";
            case TokenType::Minus: return "Minus";
            case TokenType::ExclamationPoint: return "ExclamationPoint";
            case TokenType::Tilde: return "Tilde";

            case TokenType::Return: return "Return";
            default: return "Unknown";
        }
    }

    ostream& operator<<(ostream& os, Token const& token){
        return os << to_string(token);
    }

    ostream& operator<<(ostream& os, TokenType const& type){
        return os << to_string(type);
    }
}