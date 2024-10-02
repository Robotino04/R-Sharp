#include "R-Sharp/Utils.hpp"
#include "R-Sharp/frontend/Utils.hpp"
#include "R-Sharp/Logging.hpp"

#include "ANSI/ANSI.hpp"

#include <sstream>

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
            default:   result += c; break;
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

void printErrorToken(Token token, std::string const& source) {
    int start = token.position.startPos;
    int end = token.position.endPos;

    std::string src = source;
    src.replace(start, end - start, ANSI::set4BitColor(ANSI::Red) + src.substr(start, end - start) + ANSI::reset());

    // print the error and 3 lines above it
    std::stringstream ss;
    int line = 1;
    int column = 1;
    int pos = 0;

    for (char c : src) {
        if (line >= token.position.line - 3 && line <= token.position.line) {
            if (column == 1) {
                ss << line;
                // print some spacing to align the code
                for (int i = 0; i < std::to_string(token.position.line).length() - std::to_string(line).length(); i++) {
                    ss << " ";
                }
                ss << "| ";
            }
            if (line == token.position.line && c == '\n')
                break;
            ss << c;
        }
        pos++;
        if (c == '\n') {
            line++;
            column = 1;
        }
        else {
            column++;
        }
    }

    int prefixLen = (std::to_string(token.position.line) + "| ").length();

    ss << "\n"
       << ANSI::set4BitColor(ANSI::Red)                           // enable red text
       << std::string(prefixLen + token.position.column - 1, ' ') // print spaces before the error
       << "^";
    try {
        ss << std::string(end - start - 1, '~'); // underline the error
    }
    catch (std::length_error) {
    }

    ss << ANSI::reset(); // disable red text
    Print(ss.str());
}
