//
// Created by 7budd on 6/11/2022.
//

#include <string>
#include "Parser.h"

StatementNode* Parser::getDefine(const std::string& iden) {
    for (DefineNode* node : *defines)
        if (node->getIdentifier() == iden)
            return node->getReplacement();
    return nullptr;
}


IncludeNode* Parser::parseInclude() {
    if (lexer->getCurrentTokenType() != TokenType::t_include)
        return nullptr;
    if (lexer->getNextTokenType() != TokenType::t_identifier)
        throw std::exception(("SyntaxException: Expected file name at " + lexer->getCurrentLocString()).c_str());
    std::string fname = lexer->getCurrentToken().Identifier + '.';
    Location l = lexer->getCurrentToken().Loc;
    if (lexer->getNextTokenType() != (TokenType)'.' || lexer->getNextTokenType() != TokenType::t_identifier)
        throw std::exception(("SyntaxException: file name must include extension at " + l.toString()).c_str());
    fname += lexer->getCurrentToken().Identifier;
    lexer->getNextToken();
    return new IncludeNode(fname, l);
}

DefineNode* Parser::parseDefine() {
    if (lexer->getCurrentTokenType() != TokenType::t_define)
        return nullptr;
    if (lexer->getNextTokenType() != TokenType::t_identifier)
        throw std::exception(("SyntaxException: Expected identifier for define at " + lexer->getCurrentLocString()).c_str());
    if (getDefine(lexer->getCurrentToken().Identifier) != nullptr)
        throw std::exception(("MultipleDefinitionException: \"" + lexer->getCurrentToken().Identifier + "\" at " +
            lexer->getCurrentLocString() + " is already defined").c_str());
    std::string iden = lexer->getCurrentToken().Identifier;
    Location l = lexer->getCurrentToken().Loc;
    lexer->getNextToken(); //eat identifier
    StatementNode *replacement; //TODO parse define code
    return new DefineNode(iden, replacement, l);
}

FunctionNode* Parser::parseFunction() {
    if (lexer->getCurrentTokenType() != TokenType::t_identifier)
        return nullptr;
    if (getDefine(lexer->getCurrentToken().Identifier) != nullptr)
        throw std::exception(("MultipleDefinitionException: Function " + lexer->getCurrentToken().Identifier +
            " overwrites a define with the same name").c_str());
    //TODO check if function with the same name already exists
    //save identifier token and file position in case code starts with identifier and we need to back up the lexer
    Token iden = lexer->getCurrentToken();
    std::streampos p = lexer->getFilePos();
    if (lexer->getNextTokenType() != (TokenType)'{') {
        //back up lexer to previous identifier token
        lexer->setFilePos(iden, p);
        return nullptr;
    }
    lexer->getNextToken(); //eat '{'
    StatementNode* body; //TODO parse function code
    if (lexer->getCurrentTokenType() != (TokenType)'}')
        throw std::exception(("SyntaxException: Expected \"}\" to close method at" + lexer->getCurrentLocString()).c_str());
    lexer->getNextToken(); //eat '}'
    return new FunctionNode(iden.Identifier, body, iden.Loc);
}

StatementNode* Parser::parseCode() {
    return nullptr; //TODO parse code
}
