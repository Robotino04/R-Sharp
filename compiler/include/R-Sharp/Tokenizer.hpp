#pragma once

#include "R-Sharp/Token.hpp"
#include "R-Sharp/Logging.hpp"

#include <string>
#include <vector>

class Tokenizer{
    public:
        Tokenizer(std::string const& filename);

        std::vector<Token> tokenize();
        std::string getSource() const { return source; };

    private:
        Token nextToken();

        bool match(char c) const;
        bool match(std::string str) const;
        bool matchAny(std::string str) const;

        bool match(int offset, char c) const;
        bool match(int offset, std::string str) const;
        bool matchAny(int offset, std::string str) const;

        char consume();
        char consume(char c);
        std::string consume(std::string str);
        std::string consumeAny(std::string str);
        char consumeAnyOne(std::string str);
        std::string consumeUntil(std::string str);

        bool isAtEnd(int offset=0) const;

        char getCurrentChar() const;
        char getChar(int offset) const;

        template<typename... Args>
        void logError(Args... args){
            Error(filename, ":", line, ":", column, ":\t", args...);
        }

        std::string source;
        size_t currentPosition;
        int line = 1;
        int column = 1;

        const std::string filename;
};