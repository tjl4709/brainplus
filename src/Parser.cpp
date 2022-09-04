//
// Created by 7budd on 6/11/2022.
//

#include <string>
#include <utility>
#include "Parser.h"

//helper finder functions
DefineNode* GetDefine(const std::string& iden, std::vector<DefineNode*> *defines) {
    for (DefineNode* node : *defines)
        if (node->getId() == iden)
            return node;
    return nullptr;
}
FunctionNode* GetFunction(const std::string& iden, std::vector<FunctionNode*> *funcs) {
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
void Parser::checkForDefine() {
    DefineNode *d;
    if (lexer->getCurrentType() == TokenType::t_identifier &&
        (d = GetDefine(lexer->getCurrentIdentifier(), defines)))
        lexer->setReplacement(d->getReplacements());
}
IfTernaryNode *Parser::parseIf() {
    //precondition: current token is "if"
    Location l = lexer->getCurrentLocation();
    lexer->getNextToken();
    checkForDefine();
    if (lexer->getCurrentType() != (TokenType)'(')
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
    lexer->getNextToken();
    checkForDefine();
    if (lexer->getCurrentType() != (TokenType)'(')
        return logError<ForNode>("SyntaxException: Expected '(' after \"for\" at " + lexer->getCurrentLocString());
    lexer->getNextToken(); //eat '('
    StatementNode *start = parseMultiStatement(true), *expr, *step, *body;
    if (!start) return nullptr;
    if (lexer->getCurrentType() != (TokenType)';')
        return logError<ForNode>("SyntaxException: Expected ';' after for initializer at " + lexer->getCurrentLocString());
    lexer->getNextToken(); //eat ';'
    if (!(expr = parseStatement())) return nullptr;
    checkForDefine();
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
    lexer->getNextToken();
    checkForDefine();
    if (lexer->getCurrentType() != (TokenType)'(')
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
    lexer->getNextToken();
    checkForDefine();
    if (lexer->getCurrentType() != (TokenType)'(')
        return logError<DoWhileNode>("SyntaxException: Expected '(' after \"while\" at " + lexer->getCurrentLocString());
    if (!(expr = parseStatement())) return nullptr;
    return new DoWhileNode(expr, body, false, l);
}

StatementNode *Parser::parsePrimary(int parenDepth) {
    StatementNode *s;
    switch (lexer->getCurrentType()) {
        case t_eof: return logError("End of file reached");
        case (TokenType)'(': lexer->getNextToken(); return parseStatement(parenDepth + 1);
        case t_if: return parseIf();
        case t_for:return parseFor();
        case t_while: return parseWhile();
        case t_do: return parseDo();
        case t_number: s = new NumberNode(lexer->getCurrentToken()->Number, lexer->getCurrentLocation()); break;
        case t_identifier: {
            DefineNode *d;
            if ((d = GetDefine(lexer->getCurrentIdentifier(), defines))) {
                lexer->setReplacement(d->getReplacements());
                return parsePrimary(parenDepth);
            } else if (GetFunction(lexer->getCurrentIdentifier(), funcs))
                s = new CallNode(lexer->getCurrentIdentifier(), lexer->getCurrentLocation());
            else return logError("SyntaxException: Unknown identifier - \"" + lexer->getCurrentIdentifier() +
                "\" at " + lexer->getCurrentLocString());
            break;
        }
        case t_op: {
            Operator op = lexer->getCurrentOp();
            Location l = lexer->getCurrentLocation();
            lexer->getNextToken();
            checkForDefine();
            if (op == Operator::print || op == Operator::read) {
                s = new NullaryOperatorNode(op, l);
            } else if (EnumOps::OpIsValComp(op)) {
                if (lexer->getCurrentType() != TokenType::t_number && (lexer->getCurrentType() != TokenType::t_op ||
                    !EnumOps::OpIsPtrLookup(lexer->getCurrentOp())))
                    return logError("SyntaxException: Comparative operator at " + l.toString() + " missing right "
                         "hand-side operand");
                auto p = parsePrimary(parenDepth);
                if (!p) return nullptr;
                return new BinaryOperatorNode(op, NodeOps::CurrentValLookup(l), p, l);
            } else if (lexer->getCurrentType() == TokenType::t_number || op == Operator::bool_not ||
                lexer->getCurrentType() == TokenType::t_op && EnumOps::OpIsPtrLookup(lexer->getCurrentOp())) {
                auto p = parsePrimary(parenDepth);
                if (!p) return nullptr;
                return new UnaryOperatorNode(op, p, l);
            } else if (EnumOps::OpIsPtrLookup(op)) {
                return NodeOps::CurrentValLookup(l);
            } else if (op == Operator::assignment || op == Operator::ptr_assignment || op == Operator::ptr_store) {
                return new UnaryOperatorNode(op, new NumberNode(0, l), l);
            } else return new UnaryOperatorNode(op, new NumberNode(1, l), l);
            break;
        }
        default: return logError("SyntaxException: Unexpected token - " + EnumOps::TypeToString(lexer->getCurrentType()) +
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
    checkForDefine();
    while (lexer->getCurrentType() == TokenType::t_op && EnumOps::OpIsMultary(lexer->getCurrentOp())) {
        curPrec = EnumOps::OpPrecedence(curOp = lexer->getCurrentOp());
        if (curPrec < opPrec)
            return lhs;
        l = lexer->getCurrentLocation();
        lexer->getNextToken(); //eat operator
        if (!(rhs = parsePrimary(0)))
            return nullptr;
        checkForDefine();
        if (lexer->getCurrentType() == TokenType::t_op && curPrec < EnumOps::OpPrecedence(lexer->getCurrentOp())
            && !(rhs = parseMultary(curPrec + 1, rhs)))
            return nullptr;
        lhs = new BinaryOperatorNode(curOp, lhs, rhs, l);
    }
    return lhs;
}
StatementNode *Parser::parseStatement(int parenDepth) {
    // stop at ')', ';'
    auto p = parsePrimary(parenDepth);
    if (!p) return nullptr;
    checkForDefine();
    if (lexer->getCurrentType() == (TokenType)')') {
        if (parenDepth > 0) {
            lexer->getNextToken();
            parenDepth--;
        }
        return p;
    }
    p = parseMultary(0, p);
    if (lexer->getCurrentType() == (TokenType)')' && parenDepth > 0) {
        lexer->getNextToken(); //eat ')'
        parenDepth--;
    }
    return p;
}
StatementNode *Parser::parseMultiStatement(bool forceMulti) {   //default: false
    //stop at ';', ')', '}', EOF
    if (!forceMulti) {
        checkForDefine();
        if (lexer->getCurrentType() != (TokenType) '{')
            return parseStatement();
        lexer->getNextToken(); //eat '{'
    }
    auto* multi = new MultiStatementNode(lexer->getCurrentLocation());
    StatementNode *stat;
    checkForDefine();
    while (lexer->getCurrentType() != (TokenType)';' && lexer->getCurrentType() != (TokenType)')' &&
           lexer->getCurrentType() != (TokenType)'}' && lexer->getCurrentType() != TokenType::t_eof)
    {
        stat = parseStatement();
        checkForDefine();
        if (stat == nullptr) break;
        multi->addStatement(stat);
    }
    if (lexer->getCurrentType() == (TokenType)'}')
        lexer->getNextToken(); //eat '}'
    else if (!forceMulti)
        return logError("SyntaxException: No matching '}' for '{' at " + multi->getLocString());
    StatementNode *s;
    if (multi->getNumStatements() == 0)
        s = nullptr;
    else if (multi->getNumStatements() == 1) {
        s = multi->getStatement(0);
        multi->removeStatement(0);
    } else return multi;
    delete multi;
    return s;
}

//public functions
IncludeNode* Parser::parseInclude(std::string dir) {
    if (lexer->getCurrentType() != TokenType::t_include)
        return nullptr;
    if (lexer->getNextType() != TokenType::t_string)
        return logError<IncludeNode>("SyntaxException: Expected file name as a string at " + lexer->getCurrentLocString());
    std::string fname = lexer->getCurrentIdentifier().find(':') == -1 ? dir : "";
    if (dir[dir.size()-1] == '\\' && lexer->getCurrentIdentifier()[0] == '\\')
        fname = fname.erase(fname.size()-1, 1);
    else if (dir[dir.size()-1] != '\\' && lexer->getCurrentIdentifier()[0] != '\\')
        fname += '\\';
    fname += lexer->getCurrentIdentifier();
    Location l = lexer->getCurrentLocation();
    lexer->getNextToken();
    return new IncludeNode(fname, l);
}
DefineNode* Parser::parseDefine() {
    if (lexer->getCurrentType() != TokenType::t_define) {
        if (lexer->getCurrentType() == TokenType::t_enddef)
            lexer->getNextToken();
        return nullptr;
    }
    if (lexer->getNextType() != TokenType::t_identifier)
        return logError<DefineNode>("SyntaxException: Expected identifier for define at " + lexer->getCurrentLocString());
    if (GetDefine(lexer->getCurrentIdentifier(), defines) != nullptr)
        return logError<DefineNode>("MultipleDefinitionException: Define \"" + lexer->getCurrentIdentifier() +
            "\" at " + lexer->getCurrentLocString() + " is already defined");
    std::string iden = lexer->getCurrentIdentifier();
    Location l = lexer->getCurrentLocation();
    auto rep = new std::vector<Token*>();
    while (lexer->getNextType() != TokenType::t_define && lexer->getCurrentType() != TokenType::t_enddef)
        if (lexer->getCurrentType() == TokenType::t_identifier && lexer->getCurrentIdentifier() == iden)
            return logError<DefineNode>("RecursiveDefineException: Define \"" + iden + "\" contains a reference to itself");
        else
            rep->push_back(lexer->getCurrentToken());
    return new DefineNode(iden, rep, l);
}
FunctionNode* Parser::parseFunction() {
    if (lexer->getCurrentType() != TokenType::t_identifier)
        return nullptr;
    //save identifier token in case code starts with identifier and we need to back up the lexer
    auto *iden = new Token(lexer->getCurrentToken());
    checkForDefine();
    if (lexer->getNextType() != (TokenType)'{') {
        //back up lexer to previous identifier token
        lexer->rollback(iden);
        return nullptr;
    }
    if (GetDefine(lexer->getCurrentIdentifier(), defines) != nullptr) {
        delete iden;
        return logError<FunctionNode>("MultipleDefinitionException: Function \"" + lexer->getCurrentIdentifier() +
                                      "\" overwrites a define with the same name");
    }
    if (GetFunction(lexer->getCurrentIdentifier(), funcs) != nullptr) {
        delete iden;
        return logError<FunctionNode>("MultipleDefinitionException: Function \"" + lexer->getCurrentIdentifier() +
                                      "\" at " + lexer->getCurrentLocString() + " is previously defined");
    }
    StatementNode* body = parseMultiStatement();
    auto *func = new FunctionNode(iden->Identifier, body, iden->Loc);
    delete iden;
    return func;
}
StatementNode* Parser::parseCode() {
    return parseMultiStatement(true);
}
