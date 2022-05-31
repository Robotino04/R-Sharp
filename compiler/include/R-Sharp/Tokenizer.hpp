#pragma once

#include "R-Sharp/Token.hpp"

#include <string>
#include <vector>

class Tokenizer{
    public:
        Tokenizer(std::string const& source);

        std::vector<Token> tokenize();

    private:
        bool isDone() const;
        Token nextToken();

        char getCurrentChar() const;
        char getNextChar() const;
        char advance();

        std::string advanceUntil(std::string str);
    
    private:
        std::string source;
        size_t currentPosition;
};