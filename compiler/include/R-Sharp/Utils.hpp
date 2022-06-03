#pragma once

#include "R-Sharp/Token.hpp"

#include <string>
#include <vector>

std::string escapeString(std::string const& str);

// removes comments and non-code tokens
std::vector<Token> cleanTokens(std::vector<Token> const& tokens);