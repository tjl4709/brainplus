//
// Created by 7budd on 6/11/2022.
//

#ifndef BRAINPLUS_PARSER_H
#define BRAINPLUS_PARSER_H


#include "Lexer.h"
#include "ASTNodes.h"

//helper find functions
DefineNode* GetDefine(const std::string& iden, std::vector<DefineNode*> *defines);
FunctionNode* GetFunction(const std::string& iden, std::vector<FunctionNode*> *funcs);

class Parser {
    Lexer *lexer;
    std::vector<DefineNode*>* defines;
    std::vector<FunctionNode*>* funcs;
    bool funcComp;
    // error logging helper function
    template <typename T = StatementNode>
    static T *logError(const std::string& msg);
    //parsing helper functions
    void checkForDefine();
    IfTernaryNode *parseIf();
    IfTernaryNode *parseTernary(StatementNode *expr);
    ForNode *parseFor();
    DoWhileNode *parseWhile();
    DoWhileNode *parseDo();

    StatementNode *parseOp(int parenDepth);
    StatementNode *parsePrimary(int parenDepth);
    StatementNode *parseMultary(int opPrec, StatementNode *lhs);
    StatementNode *parseStatement(int parenDepth = 0);
    StatementNode *parseMultiStatement(bool forceMulti = false);
public:
    explicit Parser(Lexer *l, std::vector<DefineNode*>* d, std::vector<FunctionNode*>* f) :
        lexer(l), defines(d), funcs(f), funcComp(false) {
        if (!lexer->good()) throw std::exception(("IOException: " + lexer->getFileName() + " not good").c_str());
    }
    ~Parser() { delete lexer; }

    bool good() { return lexer->good(); }
    IncludeNode* parseInclude(std::string dir);
    DefineNode* parseDefine();
    FunctionNode* parseFunction();
    StatementNode* parseCode();
};


#endif //BRAINPLUS_PARSER_H
