//
// Created by 7budd on 6/11/2022.
//

#include <string>
#include "Parser.h"

//find helper methods
DefineNode* Parser::getDefine(const std::string& iden) {
    for (DefineNode* node : *defines)
        if (node->getIdentifier() == iden)
            return node;
    return nullptr;
}
FunctionNode *Parser::getFunction(const std::string &iden) {
    for (FunctionNode* node : *funcs)
        if (node->getIdentifier() == iden)
            return node;
    return nullptr;
}
//error logging helper methods
StatementNode *Parser::logError(const std::string& msg) {
    fprintf(stderr, "%s.\n", msg.c_str());
    return nullptr;
}
IncludeNode *Parser::logErrorI(const std::string& msg) {
    logError(msg);
    return nullptr;
}
DefineNode *Parser::logErrorD(const std::string& msg) {
    logError(msg);
    return nullptr;
}
FunctionNode *Parser::logErrorF(const std::string& msg) {
    logError(msg);
    return nullptr;
}
//code parsing helper functions
IfTernaryNode *Parser::parseIf() {
    return nullptr;
}
ForNode *Parser::parseFor() {
    return nullptr;
}
DoWhileNode *Parser::parseDoWhile() {
    return nullptr;
}
StatementNode *Parser::parseOp() {
    return nullptr;
}
StatementNode *Parser::parseMultary() {
    return nullptr;
}
StatementNode *Parser::parseStatement() {
    return nullptr;
}
MultiStatementNode *Parser::parseMultiStatement() {
    //stop at '}', "define", when parsing define and unknown identifier is found
    Location l = lexer->getCurrentToken().Loc;
    auto* statements = new std::vector<StatementNode*>();
    StatementNode *stat;
    while (lexer->getCurrentType() != (TokenType)'}' && lexer->getCurrentType() != TokenType::t_define && (defComp ||
            getDefine(lexer->getCurrentIdentifier()) != nullptr)) {
        stat = parseStatement();
        if (stat == nullptr) break;
        statements->push_back(stat);
    }
    if (statements->empty())
        return nullptr;
    return new MultiStatementNode(statements, l);
}

//public functions
IncludeNode* Parser::parseInclude() {
    if (lexer->getCurrentType() != TokenType::t_include)
        return nullptr;
    if (lexer->getNextType() != TokenType::t_identifier)
        return logErrorI("SyntaxException: Expected file name at " + lexer->getCurrentLocString());
    std::string fname = lexer->getCurrentIdentifier() + '.';
    Location l = lexer->getCurrentToken().Loc;
    if (lexer->getNextType() != (TokenType)'.' || lexer->getNextType() != TokenType::t_identifier)
        return logErrorI("SyntaxException: file name must include extension at " + l.toString());
    fname += lexer->getCurrentIdentifier();
    lexer->getNextToken();
    return new IncludeNode(fname, l);
}
DefineNode* Parser::parseDefine() {
    if (lexer->getCurrentType() != TokenType::t_define) {
        defComp = true;
        return nullptr;
    }
    if (lexer->getNextType() != TokenType::t_identifier)
        return logErrorD("SyntaxException: Expected identifier for define at " + lexer->getCurrentLocString());
    if (getDefine(lexer->getCurrentIdentifier()) != nullptr)
        return logErrorD("MultipleDefinitionException: Define \"" + lexer->getCurrentIdentifier() + "\" at " +
                         lexer->getCurrentLocString() + " is already defined");
    std::string iden = lexer->getCurrentIdentifier();
    Location l = lexer->getCurrentToken().Loc;
    lexer->getNextToken(); //eat identifier
    StatementNode *replacement = parseMultiStatement();
    return new DefineNode(iden, replacement, l);
}
FunctionNode* Parser::parseFunction() {
    if (lexer->getCurrentType() != TokenType::t_identifier)
        return nullptr;
    if (getDefine(lexer->getCurrentIdentifier()) != nullptr)
        return logErrorF("MultipleDefinitionException: Function " + lexer->getCurrentIdentifier() +
                         " overwrites a define with the same name");
    if (getFunction(lexer->getCurrentIdentifier()) != nullptr)
        return logErrorF("MultipleDefinitionException: Function \"" + lexer->getCurrentIdentifier() + "\" at " +
                         lexer->getCurrentLocString() + " is already defined");
    //save identifier token and file position in case code starts with identifier and we need to back up the lexer
    Token iden = lexer->getCurrentToken();
    std::streampos p = lexer->getFilePos();
    if (lexer->getNextType() != (TokenType)'{') {
        //back up lexer to previous identifier token
        lexer->setFilePos(iden, p);
        return nullptr;
    }
    lexer->getNextToken(); //eat '{'
    StatementNode* body = parseMultiStatement();
    if (lexer->getCurrentType() != (TokenType)'}')
        return logErrorF("SyntaxException: Expected \"}\" to close method at" + lexer->getCurrentLocString());
    lexer->getNextToken(); //eat '}'
    return new FunctionNode(iden.Identifier, body, iden.Loc);
}
StatementNode* Parser::parseCode() {
    return parseMultiStatement();
}
