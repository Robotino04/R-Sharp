#pragma once

#include "R-Sharp/AstNodes.hpp"
#include "R-Sharp/Token.hpp"

#include <string>
#include <memory>
#include <vector>

class Parser{
    public:
        Parser(std::vector<Token> const& tokens, std::string const& filename);

        std::shared_ptr<AstProgram> parse();

    private:
        bool match(TokenType type) const;
        bool match(TokenType type, std::string value) const;
        bool match(std::vector<TokenType> types) const;
        bool matchAny(std::vector<TokenType> types) const;

        bool match(int offset, TokenType type) const;
        bool match(int offset,TokenType type, std::string value) const;
        bool match(int offset,std::vector<TokenType> types) const;
        bool matchAny(int offset,std::vector<TokenType> types) const;

        Token consume();
        Token consume(TokenType type);
        Token consume(TokenType type, std::string value);
        Token consume(std::vector<TokenType> types);
        Token consumeAnyOne(std::vector<TokenType> types);

        bool isAtEnd(int offset = 0) const;

        Token getCurrentToken() const;
        Token getToken(int offset) const;

        template<typename... Args>
        void logError(Args... args) const{
            Error(filename, ":", getCurrentToken().line, ":", getCurrentToken().column, ":\t", args...);
        }

        std::vector<Token> tokens;
        int currentTokenIndex = 0;

        std::string filename;

        std::shared_ptr<AstProgram> program;

        std::shared_ptr<AstProgram> parseProgram();
        std::shared_ptr<AstFunction> parseFunction();
        std::shared_ptr<AstParameterList> parseParameterList();

        std::shared_ptr<AstStatement> parseStatement();
        std::shared_ptr<AstExpression> parseExpression();
        std::shared_ptr<AstType> parseType();
        std::shared_ptr<AstDeclaration> parseDeclaration();
        std::shared_ptr<AstBlockItem> parseBlockItem();

        std::shared_ptr<AstBlock> parseBlock();
        std::shared_ptr<AstReturn> parseReturn();
        std::shared_ptr<AstConditionalStatement> parseConditionalStatement();

        std::shared_ptr<AstExpression> parseConditionalExpression();
        std::shared_ptr<AstExpression> parseLogicalAndExp();
        std::shared_ptr<AstExpression> parseLogicalOrExp();
        std::shared_ptr<AstExpression> parseEqualityExp();
        std::shared_ptr<AstExpression> parseRelationalExp();
        std::shared_ptr<AstExpression> parseAdditiveExp();
        std::shared_ptr<AstExpression> parseTerm();
        std::shared_ptr<AstExpression> parseFactor();
        std::shared_ptr<AstInteger> parseNumber();
        std::shared_ptr<AstVariableAssignment> parseVariableAssignment();
        std::shared_ptr<AstVariableAccess> parseVariableAccess();


        std::shared_ptr<AstBuiltinType> parseBuiltinType();
        std::shared_ptr<AstTypeModifier> parseTypeModifier();
        std::shared_ptr<AstArray> parseArray();

        std::shared_ptr<AstVariableDeclaration> parseVariableDeclaration();
};