//
// Created by 7budd on 6/11/2022.
//

#ifndef BRAINPLUS_PARSER_H
#define BRAINPLUS_PARSER_H


#include "Lexer.h"
#include "ASTNodes.h"

//helper find functions
DefineNode* GetDefine(const std::string& iden, std::vector<DefineNode*> *defines);
DefineNode* GetFunction(const std::string& iden, std::vector<FunctionNode*> *funcs);

class Parser {
    Lexer *lexer;
    std::vector<DefineNode*>* defines;
    std::vector<FunctionNode*>* funcs;
    bool defComp;
    // error logging helper function
    template <typename T = StatementNode>
    static T *logError(const std::string& msg);
    //parsing helper functions
    IfTernaryNode *parseIf();
    ForNode *parseFor();
    DoWhileNode *parseWhile();
    DoWhileNode *parseDo();
    StatementNode *parsePrimary();
    StatementNode *parseMultary(int opPrec, StatementNode *lhs);
    StatementNode *parseStatement();
    StatementNode *parseMultiStatement(bool forceMulti = false);
public:
    explicit Parser(Lexer *l, std::vector<DefineNode*>* d, std::vector<FunctionNode*>* f) :
        lexer(l), defines(d), funcs(f), defComp(false) {
        if (!lexer->good()) throw std::exception(("IOException: " + lexer->getFileName() + " not good").c_str());
    }
    bool good() { return lexer->good(); }
    IncludeNode* parseInclude(std::string dir);
    DefineNode* parseDefine();
    FunctionNode* parseFunction();
    StatementNode* parseCode();
};


#endif //BRAINPLUS_PARSER_H
