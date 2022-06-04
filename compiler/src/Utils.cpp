#include "R-Sharp/Utils.hpp"

std::string escapeString(std::string const& str) {
    std::string result;
    for (char c : str) {
        switch (c) {
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            case '\'': result += "\\'"; break;
            case '\"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            default: result += c; break;
        }
    }
    return result;
}

std::vector<Token> cleanTokens(std::vector<Token> const& tokens) {
    std::vector<Token> result;
    for (auto const& token : tokens) {
        if (token.type != TokenType::Comment) {
            result.push_back(token);
        }
    }
    return result;
}