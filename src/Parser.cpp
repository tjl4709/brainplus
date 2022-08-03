//
// Created by 7budd on 6/11/2022.
//

#include <string>
#include "Parser.h"

//helper finder functions
DefineNode* GetDefine(const std::string& iden, std::vector<DefineNode*> *defines) {
    for (DefineNode* node : *defines)
        if (node->getId() == iden)
            return node;
    return nullptr;
}
DefineNode* GetFunction(const std::string& iden, std::vector<FunctionNode*> *funcs) {
    for (FunctionNode* node : *funcs)
        if (node->getId() == iden)
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
StatementNode *Parser::parsePrimary() {
    StatementNode *s;
    switch (lexer->getCurrentType()) {
        case t_eof: return logError("End of file reached");
        case (TokenType)'(': lexer->getNextToken(); return parseStatement();
        case t_if: return parseIf();
        case t_for:return parseFor();
        case t_while: return parseWhile();
        case t_do: return parseDo();
        case t_number: s = new NumberNode(lexer->getCurrentToken().Number, lexer->getCurrentLocation()); break;
        case t_identifier: {
            DefineNode *d;
            if (!defComp)
                s = new CallNode(lexer->getCurrentIdentifier(), lexer->getCurrentLocation());
            else if ((d = GetDefine(lexer->getCurrentIdentifier(), defines))) {
                s = d->getReplacement();
                s->setLocation(lexer->getCurrentLocation());
            } else if (GetFunction(lexer->getCurrentIdentifier(), funcs))
                s = new CallNode(lexer->getCurrentIdentifier(), lexer->getCurrentLocation());
            else return logError("SyntaxException: Unknown identifier - \"" + lexer->getCurrentIdentifier() +
                "\" at " + lexer->getCurrentLocString());
            break;
        }
        case t_op: {
            Operator op = lexer->getCurrentOp();
            Location l = lexer->getCurrentLocation();
            if (op == Operator::print || op == Operator::read) {
                s = new NullaryOperatorNode(op, l);
            } else if (OpIsValComp(op)) {
                if (lexer->getNextType() != TokenType::t_number && (lexer->getCurrentType() != TokenType::t_op ||
                    !OpIsPtrLookup(lexer->getCurrentOp())))
                    return logError("SyntaxException: Comparative operator at " + l.toString() + " missing right "
                         "hand-side operand");
                auto p = parsePrimary();
                if (!p) return nullptr;
                return new BinaryOperatorNode(op, CurrentValLookup(l), p, l);
            } else if (lexer->getNextType() == TokenType::t_number || op == Operator::bool_not ||
                lexer->getCurrentType() == TokenType::t_op && OpIsPtrLookup(lexer->getCurrentOp())) {
                auto p = parsePrimary();
                if (!p) return nullptr;
                return new UnaryOperatorNode(op, p, l);
            } else if (OpIsPtrLookup(op)) {
                return CurrentValLookup(l);
            } else if (op == Operator::assignment || op == Operator::ptr_assignment || op == Operator::ptr_store) {
                return new UnaryOperatorNode(op, new NumberNode(0, l), l);
            } else return new UnaryOperatorNode(op, new NumberNode(1, l), l);
            break;
        }
        default: return logError("SyntaxException: Unexpected token - " + TypeToString(lexer->getCurrentType()) +
            " at " + lexer->getCurrentLocString());
    }
    lexer->getNextToken();
    return s;
}
StatementNode *Parser::parseMultary(int opPrec, StatementNode *lhs) {
    int curPrec;
    Operator curOp;
    Location l{};
    StatementNode *rhs;
    while (lexer->getCurrentType() == TokenType::t_op) {
        curPrec = OpPrecedence(curOp = lexer->getCurrentOp());
        if (curPrec < opPrec)
            return lhs;
        l = lexer->getCurrentLocation();
        lexer->getNextToken(); //eat operator
        if (!(rhs = parsePrimary()))
            return nullptr;
        if (lexer->getCurrentType() == TokenType::t_op && curPrec < OpPrecedence(lexer->getCurrentOp())
            && !(rhs = parseMultary(curPrec + 1, rhs)))
            return nullptr;
        lhs = new BinaryOperatorNode(curOp, lhs, rhs, l);
    }
    return lhs;
}
StatementNode *Parser::parseStatement() {
    // stop at ')', ';'
    auto p = parsePrimary();
    if (!p) return nullptr;
    if (lexer->getCurrentType() == (TokenType)')') {
        lexer->getNextToken();
        return p;
    }
    return parseMultary(0, p);
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
        lexer->getCurrentType() != TokenType::t_define && (defComp || GetDefine(lexer->getCurrentIdentifier(), defines) != nullptr))
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
    if (GetDefine(lexer->getCurrentIdentifier(), defines) != nullptr)
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
    if (GetDefine(lexer->getCurrentIdentifier(), defines) != nullptr)
        return logError<FunctionNode>("MultipleDefinitionException: Function " + lexer->getCurrentIdentifier() +
                         " overwrites a define with the same name");
    if (GetFunction(lexer->getCurrentIdentifier(), funcs) != nullptr)
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
