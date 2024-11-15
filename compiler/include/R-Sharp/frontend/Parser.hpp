#pragma once

#include "R-Sharp/ast/AstNodes.hpp"
#include "R-Sharp/frontend/Token.hpp"
#include "R-Sharp/frontend/ParsingCache.hpp"
#include "R-Sharp/Utils/ScopeGuard.hpp"

#include <string>
#include <memory>
#include <vector>
#include <sstream>

class ParsingError : public std::exception {
public:
    ParsingError(const std::string& message): message(message) {}
    virtual const char* what() const noexcept override {
        return message.c_str();
    }

private:
    std::string message;
};
namespace {
template <typename Arg>
std::string stringify(Arg a) {
    std::stringstream ss;
    ss << a;
    return ss.str();
}
template <typename Arg, typename... Args>
std::string stringify(Arg a, Args... args) {
    std::stringstream ss;
    ss << a;
    return ss.str() + stringify(args...);
}
}

class Parser {
public:
    Parser(std::vector<Token> const& tokens, std::string const& filename, std::string const& importSearchPath, ParsingCache& cache);

    std::shared_ptr<AstProgram> parse();

    bool hasErrors() const {
        return hasError;
    }

private:
    bool match(TokenType type) const;
    bool match(TokenType type, std::string value) const;
    bool match(std::vector<TokenType> types) const;
    bool matchAny(std::vector<TokenType> types) const;

    bool match(int offset, TokenType type) const;
    bool match(int offset, TokenType type, std::string value) const;
    bool match(int offset, std::vector<TokenType> types) const;
    bool matchAny(int offset, std::vector<TokenType> types) const;

    Token consume();
    Token consume(TokenType type);
    Token consume(TokenType type, std::string value);
    Token consume(std::vector<TokenType> types);
    Token consumeAnyOne(std::vector<TokenType> types);

    bool isAtEnd(int offset = 0) const;

    Token getCurrentToken() const;
    Token getToken(int offset) const;

    template <typename... Args>
    void parserError(Args... args) const {
        throw ParsingError(stringify(
            filename, ":", getCurrentToken().position.line, ":", getCurrentToken().position.column, ":\t", args...
        ));
    }
    bool hasError = false;
    bool isRecovering = false;

    std::vector<Token> tokens;
    int currentTokenIndex = 0;

    std::string filename;
    std::string importSearchPath;
    ParsingCache& cache;

    std::shared_ptr<AstProgram> program;

    std::shared_ptr<AstProgram> parseProgram();
    std::shared_ptr<AstParameterList> parseParameterList();

    std::shared_ptr<AstProgramItem> parseProgramItem();
    std::shared_ptr<AstFunctionDefinition> parseFunctionDefinition();
    std::shared_ptr<AstVariableDeclaration> parseGlobalVariableDefinition();
    std::vector<std::shared_ptr<AstProgramItem>> parseImportStatement();

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
    std::shared_ptr<AstExpression> parseMultiplicativeExp();
    std::shared_ptr<AstExpression> parsePrefixExp();
    std::shared_ptr<AstExpression> parsePostfixExp();
    std::shared_ptr<AstExpression> parsePrimaryExp();


    std::shared_ptr<AstAssignLocation> parseAssignmentLocation();
    std::shared_ptr<AstAssignment> parseAssignment();

    std::shared_ptr<AstExpression> parseNumber();
    std::shared_ptr<AstFunctionCall> parseFunctionCall();
    std::shared_ptr<AstInteger> parseCharacterLiteral();
    std::shared_ptr<AstArrayLiteral> parseArrayLiteral();
    std::shared_ptr<AstArrayLiteral> parseStringLiteral();

    std::shared_ptr<AstVariableDeclaration> parseVariableDeclaration();

    std::shared_ptr<AstArrayType> parseArrayType();
    std::shared_ptr<AstType> parseType();
    std::shared_ptr<AstTags> parseTags();

    // helpers
    std::shared_ptr<AstStatement> parseForLoop();
    std::shared_ptr<AstExpression> parseOptionalExpression();

    /*
    A checkpoint is used to store the current state of the parser.

    If a parsing function is not able to parse the current
    token, it will throw an exception of type ParsingError.
    The checkpoint will restore the parser to the state it
    was in when the checkpoint was created.
    */
    [[nodiscard]] ScopeGuard getTokenCheckpoint() {
        const int savedTokenIndex = currentTokenIndex;
        return ScopeGuard([this, savedTokenIndex]() {
            // only restore the tokens if an exception was thrown
            if (std::uncaught_exceptions()) {
                this->currentTokenIndex = savedTokenIndex;
            }
        });
    }
};
