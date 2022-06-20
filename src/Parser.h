//
// Created by 7budd on 6/11/2022.
//

#ifndef BRAINPLUS_PARSER_H
#define BRAINPLUS_PARSER_H


#include "Lexer.h"
#include "ASTNodes.h"

class Parser {
    Lexer *lexer;
    std::vector<DefineNode*>* defines;
    // parser helper functions
    StatementNode* getDefine(const std::string& iden);
public:
    explicit Parser(Lexer *l, std::vector<DefineNode*>* d) : lexer(l), defines(d) {
        if (!lexer->good()) throw std::exception(("IOException: " + lexer->getFileName() + " not good").c_str());
    }
    IncludeNode* parseInclude();
    DefineNode* parseDefine();
    FunctionNode* parseFunction();
    StatementNode* parseCode();
};


#endif //BRAINPLUS_PARSER_H
