//
// Created by 7budd on 6/11/2022.
//

#ifndef BRAINPLUS_PARSER_H
#define BRAINPLUS_PARSER_H


#include "Lexer.h"
#include "ASTNodes.h"

class Parser {
    Lexer *lexer;
    // parser helper functions
public:
    explicit Parser(Lexer *l) : lexer(l) {
        if (!lexer->good()) throw std::exception(("IOException: " + lexer->getFileName() + " not good").c_str());
    }
    IncludeNode parseInclude();
    DefineNode parseDefine();
    FunctionNode parseFunction();
    StatementNode parseCode();
};


#endif //BRAINPLUS_PARSER_H
