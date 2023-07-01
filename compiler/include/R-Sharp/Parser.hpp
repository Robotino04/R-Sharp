#pragma once

#include "R-Sharp/AstNodes.hpp"
#include "R-Sharp/Token.hpp"

#include <string>
#include <memory>
#include <vector>
#include <stdexcept>
#include <sstream>

class ParsingError : public std::exception{
public:
    ParsingError(const std::string& message) : message(message) {}
    virtual const char* what() const noexcept override { return message.c_str(); }
private:
    std::string message;
};
namespace {
    template<typename Arg>
    std::string stringify(Arg a){
        std::stringstream ss;
        ss << a;
        return ss.str();
    }
    template<typename Arg, typename... Args>
    std::string stringify(Arg a, Args... args){
        std::stringstream ss;
        ss << a;
        return ss.str() + stringify(args...);
    }
}

class Parser{
    public:
        Parser(std::vector<Token> const& tokens, std::string const& filename);

        std::shared_ptr<AstProgram> parse();

        bool hasErrors() const { return hasError; }
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
        void parserError(Args... args) const{
            throw ParsingError(stringify(filename, ":", getCurrentToken().position.line, ":", getCurrentToken().position.column, ":\t", args...));
        }
        bool hasError = false;
        bool isRecovering = false;

        std::vector<Token> tokens;
        int currentTokenIndex = 0;

        std::string filename;

        std::shared_ptr<AstProgram> program;

        std::shared_ptr<AstProgram> parseProgram();
        std::shared_ptr<AstParameterList> parseParameterList();

        std::shared_ptr<AstProgramItem> parseProgramItem();
        std::shared_ptr<AstFunctionDefinition> parseFunctionDefinition();
        std::shared_ptr<AstVariableDeclaration> parseGlobalVariableDefinition();

        std::shared_ptr<AstStatement> parseStatement();
        std::shared_ptr<AstExpression> parseExpression();
        std::shared_ptr<AstDeclaration> parseDeclaration();
        std::shared_ptr<AstBlockItem> parseBlockItem();

        std::shared_ptr<AstBlock> parseBlock();
        std::shared_ptr<AstReturn> parseReturn();
        std::shared_ptr<AstConditionalStatement> parseConditionalStatement();
        std::shared_ptr<AstForLoopDeclaration> parseForLoopDeclaration();
        std::shared_ptr<AstForLoopExpression> parseForLoopExpression();
        std::shared_ptr<AstWhileLoop> parseWhileLoop();
        std::shared_ptr<AstDoWhileLoop> parseDoWhileLoop();
        std::shared_ptr<AstBreak> parseBreak();
        std::shared_ptr<AstSkip> parseSkip();

        std::shared_ptr<AstExpression> parseConditionalExpression();
        std::shared_ptr<AstExpression> parseLogicalAndExp();
        std::shared_ptr<AstExpression> parseLogicalOrExp();
        std::shared_ptr<AstExpression> parseEqualityExp();
        std::shared_ptr<AstExpression> parseRelationalExp();
        std::shared_ptr<AstExpression> parseAdditiveExp();
        std::shared_ptr<AstExpression> parseTerm();
        std::shared_ptr<AstExpression> parseFactor();
        std::shared_ptr<AstInteger> parseNumber();
        std::shared_ptr<AstAssignment> parseAssignment();
        std::shared_ptr<AstFunctionCall> parseFunctionCall();
        std::shared_ptr<AstAddressOf> parseAstAddressOf();

        std::shared_ptr<AstLValue> parseLValue();
        std::shared_ptr<AstVariableAccess> parseVariableAccess();
        std::shared_ptr<AstDereference> parseDereference();

        std::shared_ptr<AstVariableDeclaration> parseVariableDeclaration();


        std::shared_ptr<AstType> parseType();
        std::shared_ptr<AstTags> parseTags();

        // helpers
        std::shared_ptr<AstStatement> parseForLoop();
        std::shared_ptr<AstExpression> parseOptionalExpression();

        /*
        This class is used to store the current state of the parser.

        If a parsing function is not able to parse the current token, it will
        throw an exception of type ParsingError. The token restorer will
        restore the parser to the state it was in before the parsing function
        was called.
        */
        class TokenRestorer{
            public:
                TokenRestorer(Parser& parser) : parser(parser) {
                    savedTokenIndex = parser.currentTokenIndex;
                }
                ~TokenRestorer(){
                    // only restore the tokens if an exception was thrown
                    if (std::uncaught_exceptions()){
                        parser.currentTokenIndex = savedTokenIndex;
                    }
                }
            private:
                Parser& parser;
                int savedTokenIndex;
                bool released = false;
        };
};
