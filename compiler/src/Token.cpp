#include "R-Sharp/Token.hpp"
#include "R-Sharp/Utils.hpp"

namespace std{
    string to_string(Token const& token){
        return "Token(" + to_string(token.type) + ", \"" + escapeString(token.value) + "\")";
    }

    string to_string(TokenType const& type){
        switch(type){
            case TokenType_None: return "None";
            case TokenType_ID: return "ID";
            case TokenType_Number: return "Number";
            case TokenType_Typename: return "Typename";
            case TokenType_TypeModifier: return "TypeModifier";
            case TokenType_Semicolon: return "Semicolon";
            case TokenType_Colon: return "Colon";
            case TokenType_Comma: return "Comma";
            case TokenType_OpenParenthesis: return "OpenParenthesis";
            case TokenType_CloseParenthesis: return "CloseParenthesis";
            case TokenType_OpenBracket: return "OpenBracket";
            case TokenType_CloseBracket: return "CloseBracket";
            case TokenType_OpenBrace: return "OpenBrace";
            case TokenType_CloseBrace: return "CloseBrace";
            case TokenType_Star: return "Star";
            case TokenType_Comment: return "Comment";
            case TokenType_MultilineComment: return "MultilineComment";
            case TokenType_EndOfFile: return "EOF";

            case TokenType_Return: return "Return";
            default: return "Unknown";
        }
    }

    ostream& operator<<(ostream& os, Token const& token){
        return os << to_string(token);
    }
}