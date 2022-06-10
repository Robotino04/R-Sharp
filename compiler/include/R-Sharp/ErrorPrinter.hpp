#pragma once

#include "R-Sharp/AstNodes.hpp"
#include "R-Sharp/AstVisitor.hpp"
#include "R-Sharp/Logging.hpp"

class ErrorPrinter : public AstVisitor {
    public:
        ErrorPrinter(std::shared_ptr<AstNode> root, std::string const& filename, std::string const& source):
            root(root), filename(filename), source(source) {}

        void print(){
            root->accept(this);
        }

        void printErrorToken(AstErrorNode* node){
            int start = node->token.position.startPos;
            int end = node->token.position.endPos+1;

            std::string src = source;
            src.replace(start, end - start, "\033[31m" + src.substr(start, end - start) + "\033[0m");

            // print the error and 3 lines above it
            std::stringstream ss;
            int line = 1;
            int column = 1;
            int pos = 0;

            for (char c : src) {
                if (line >= node->token.position.line - 3 && line <= node->token.position.line) {
                    if (column == 1){
                        ss << line << "| ";
                    }
                    if (line == node->token.position.line && c == '\n') break;
                    ss << c;
                }
                pos++;
                if (c == '\n') {
                    line++;
                    column = 1;
                } else {
                    column++;
                }
            }

            int prefixLen = (std::to_string(node->token.position.line) + "| ").length();

            ss << "\n\033[31m" // enable red text
                << std::string(prefixLen + node->token.position.column - 1, ' ') // print spaces before the error
                << "^";
            try {
                ss << std::string(node->token.position.endPos - node->token.position.startPos - 1, '~'); // underline the error
            }
            catch(std::length_error){}

            ss << "\033[0m"; // disable red text
            Print(ss.str());
        }

        void visit(AstErrorStatement* node) {
            Error(node->name);
            printErrorToken(node);
        };
        void visit(AstErrorFunction* node) {
            Error(node->name);
            printErrorToken(node);
        };
    
    private:
        std::string filename;
        std::string source;
        std::shared_ptr<AstNode> root;
};