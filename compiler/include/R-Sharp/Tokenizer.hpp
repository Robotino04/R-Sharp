#pragma once

#include "R-Sharp/Token.hpp"
#include "R-Sharp/Logging.hpp"

#include <string>
#include <vector>

class Tokenizer{
    public:
        Tokenizer(std::string const& filename);

        std::vector<Token> tokenize();

    private:
        bool isDone() const;
        Token nextToken();

        char getCurrentChar() const;
        char getNextChar() const;
        char advance();

        template<typename... Args>
        void logError(Args... args){
            Error(filename, ":", line, ":", column, ":\t", args...);
            numErrors++;
        }

        std::string advanceUntil(std::string str);
    
    private:
        const std::string filename;
        std::string source;
        size_t currentPosition;

        int line = 1;
        int column = 1;

        int numErrors = 0;
};