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
template <typename T>   //default: StatementNode
T *Parser::logError(const std::string& msg) {
    fprintf(stderr, "%s.\n", msg.c_str());
    return nullptr;
}
//code parsing helper functions
IfTernaryNode *Parser::parseIf() {
    //precondition: current token is "if"
    Location l = lexer->getCurrentLocation();
    if (lexer->getNextType() != (TokenType)'(')
        return logError<IfTernaryNode>("SyntaxException: Expected '(' after \"if\" at " + lexer->getCurrentLocString());
    StatementNode *expr = parseStatement(), *body, *elseBody = nullptr;
    if (!expr || !(body = parseMultiStatement()))
        return nullptr;
    if (lexer->getCurrentType() == TokenType::t_else) {
        lexer->getNextToken(); //eat "else"
        if (!(elseBody = parseMultiStatement()))
            return nullptr;
    }
    return new IfTernaryNode(expr, body, elseBody, false, l);
}
ForNode *Parser::parseFor() {
    //precondition: current token is "for"
    Location l = lexer->getCurrentLocation();
    if (lexer->getNextType() != (TokenType)'(')
        return logError<ForNode>("SyntaxException: Expected '(' after \"for\" at " + lexer->getCurrentLocString());
    lexer->getNextToken(); //eat '('
    StatementNode *start = parseMultiStatement(true), *expr, *step, *body;
    if (!start) return nullptr;
    if (lexer->getCurrentType() != (TokenType)';')
        return logError<ForNode>("SyntaxException: Expected ';' after for initializer at " + lexer->getCurrentLocString());
    lexer->getNextToken(); //eat ';'
    if (!(expr = parseStatement())) return nullptr;
    if (lexer->getCurrentType() != (TokenType)';')
        return logError<ForNode>("SyntaxException: Expected ';' after for condition at " + lexer->getCurrentLocString());
    lexer->getNextToken(); //eat ';'
    if (!(step = parseMultiStatement(true))) return nullptr;
    if (lexer->getCurrentType() != (TokenType)')')
        return logError<ForNode>("SyntaxException: Expected ')' after for step at " + lexer->getCurrentLocString());
    lexer->getNextToken(); //eat ')'
    if (!(body = parseMultiStatement())) return nullptr;
    return new ForNode(start, expr, step, body, l);
}
DoWhileNode *Parser::parseWhile() {
    //precondition: current token is "while"
    Location l = lexer->getCurrentLocation();
    if (lexer->getNextType() != (TokenType)'(')
        return logError<DoWhileNode>("SyntaxException: Expected '(' after \"while\" at " + lexer->getCurrentLocString());
    StatementNode *expr = parseStatement(), *body;
    if (!expr || !(body = parseMultiStatement())) return nullptr;
    return new DoWhileNode(expr, body, true, l);
}
DoWhileNode *Parser::parseDo() {
    //precondition: current token is "do"
    Location l = lexer->getCurrentLocation();
    lexer->getNextToken(); //eat "do"
    StatementNode *expr, *body = parseMultiStatement();
    if (!body) return nullptr;
    if (lexer->getCurrentType() != TokenType::t_while)
        return logError<DoWhileNode>("SyntaxException: Expected \"while\" after do loop at " + lexer->getCurrentLocString());
    if (lexer->getNextType() != (TokenType)'(')
        return logError<DoWhileNode>("SyntaxException: Expected '(' after \"while\" at " + lexer->getCurrentLocString());
    if (!(expr = parseStatement())) return nullptr;
    return new DoWhileNode(expr, body, false, l);
}
StatementNode *Parser::parseOp() {
    return nullptr;
}
StatementNode *Parser::parseMultary() {
    return nullptr;
}
StatementNode *Parser::parseStatement() {
    // stop at ')', ';'
    StatementNode *s = nullptr;
    switch (lexer->getCurrentType()) {
        case t_eof:
            return nullptr;
        case t_include: case t_define:
        case t_else:    case t_string:
            return logError("SyntaxException: Unexpected token - " + TypeToString(lexer->getCurrentType()) +
                " at " + lexer->getCurrentLocString());
        case t_if: return parseIf();
        case t_for:return parseFor();
        case t_while: return parseWhile();
        case t_do: return parseDo();
        case t_number:
            s = new NumberNode(lexer->getCurrentToken().Number, lexer->getCurrentLocation());
            break;
        case t_identifier:
            DefineNode *d;
            if ((d = getDefine(lexer->getCurrentIdentifier()))) {
                s = d->getReplacement();
                s->setLocation(lexer->getCurrentLocation());
            } else if (getFunction(lexer->getCurrentIdentifier()))
                s = new CallNode(lexer->getCurrentIdentifier(), lexer->getCurrentLocation());
            else return logError("SyntaxException: Unexpected identifier - \"" + lexer->getCurrentIdentifier() +
                    "\" at " + lexer->getCurrentLocString());
            break;
        default: break;
    }
    if (s) {
        lexer->getNextToken();
        return s;
    }
    if (!(s = parseOp())) return nullptr;
    return parseMultary();
}
StatementNode *Parser::parseMultiStatement(bool forceMulti) {   //default: false
    //stop at '}', EOF, "define", when parsing define and unknown identifier is found
    if (!forceMulti) {
        if (lexer->getCurrentType() != (TokenType) '{')
            return parseStatement();
        else lexer->getNextToken(); //eat '{'
    }
    auto* multi = new MultiStatementNode(lexer->getCurrentLocation());
    StatementNode *stat;
    while (lexer->getCurrentType() != (TokenType)'}' && lexer->getCurrentType() != TokenType::t_eof &&
        lexer->getCurrentType() != TokenType::t_define && (defComp || getDefine(lexer->getCurrentIdentifier()) != nullptr))
    {
        stat = parseStatement();
        if (stat == nullptr) break;
        multi->addStatement(stat);
    }
    if (lexer->getCurrentType() == (TokenType)'}')
        lexer->getNextToken(); //eat '}'
    else if (!forceMulti)
        return logError("SyntaxException: No matching '}' for '{' at " + multi->getLocString());
    if (multi->getNumStatements() == 0) return nullptr;
    if (multi->getNumStatements() == 1) return multi->getStatement(0);
    return multi;
}

//public functions
IncludeNode* Parser::parseInclude() {
    if (lexer->getCurrentType() != TokenType::t_include)
        return nullptr;
    if (lexer->getNextType() != TokenType::t_string)
        return logError<IncludeNode>("SyntaxException: Expected file name as a string at " + lexer->getCurrentLocString());
    std::string fname = lexer->getCurrentIdentifier();
    Location l = lexer->getCurrentLocation();
    lexer->getNextToken();
    return new IncludeNode(fname, l);
}
DefineNode* Parser::parseDefine() {
    if (lexer->getCurrentType() != TokenType::t_define) {
        defComp = true;
        return nullptr;
    }
    if (lexer->getNextType() != TokenType::t_identifier)
        return logError<DefineNode>("SyntaxException: Expected identifier for define at " + lexer->getCurrentLocString());
    if (getDefine(lexer->getCurrentIdentifier()) != nullptr)
        return logError<DefineNode>("MultipleDefinitionException: Define \"" + lexer->getCurrentIdentifier() +
            "\" at " + lexer->getCurrentLocString() + " is already defined");
    std::string iden = lexer->getCurrentIdentifier();
    Location l = lexer->getCurrentLocation();
    lexer->getNextToken(); //eat identifier
    StatementNode *replacement = parseMultiStatement(true);
    return new DefineNode(iden, replacement, l);
}
FunctionNode* Parser::parseFunction() {
    if (lexer->getCurrentType() != TokenType::t_identifier)
        return nullptr;
    if (getDefine(lexer->getCurrentIdentifier()) != nullptr)
        return logError<FunctionNode>("MultipleDefinitionException: Function " + lexer->getCurrentIdentifier() +
                         " overwrites a define with the same name");
    if (getFunction(lexer->getCurrentIdentifier()) != nullptr)
        return logError<FunctionNode>("MultipleDefinitionException: Function \"" + lexer->getCurrentIdentifier() +
            "\" at " + lexer->getCurrentLocString() + " is already defined");
    //save identifier token and file position in case code starts with identifier and we need to back up the lexer
    Token iden = lexer->getCurrentToken();
    std::streampos p = lexer->getFilePos();
    if (lexer->getNextType() != (TokenType)'{') {
        //back up lexer to previous identifier token
        lexer->setFilePos(iden, p);
        return nullptr;
    }
    StatementNode* body = parseMultiStatement();
    if (lexer->getCurrentType() != (TokenType)'}')
        return logError<FunctionNode>("SyntaxException: Expected \"}\" to close method at" + lexer->getCurrentLocString());
    lexer->getNextToken(); //eat '}'
    return new FunctionNode(iden.Identifier, body, iden.Loc);
}
StatementNode* Parser::parseCode() {
    return parseMultiStatement(true);
}
