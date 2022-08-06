//
// Created by 7budd on 5/23/2022.
//
#include "Lexer.h"

Token::Token(TokenType t, Location l) : Type(t), Loc(l), Number(0), Op(Operator::null) {}
Token::Token(int n, Location l) : Token(TokenType::t_number, l) { Number = n; }
Token::Token(Operator op, Location l) : Token(TokenType::t_op, l) { Op = op; }
Token::Token(std::string id, bool isIden, Location l) : Token(isIden ? TokenType::t_identifier : TokenType::t_string, l)
    { Identifier = std::move(id); }
Token::Token(std::string id, Location l) : Token(std::move(id), true, l) {}

void Token::copy(const Token& tok) {
    Loc = tok.Loc;
    Type = tok.Type;
    Identifier = Type == TokenType::t_identifier || Type == TokenType::t_string ? tok.Identifier : "";
    Number = Type == TokenType::t_number ? tok.Number : 0;
    Op = Type == TokenType::t_op ? tok.Op : Operator::null;
}
std::string Token::toString() const {
    switch (Type) {
        case t_eof: return "EOF";
        case t_include: return "include";
        case t_define: return "define";
        case t_if: return "if";
        case t_else: return "else";
        case t_for: return "for";
        case t_while: return "while";
        case t_do: return "do";
        case t_number: return "N:" + std::to_string(Number);
        case t_identifier: return "ID:" + Identifier;
        case t_string: return "S:" + Identifier;
        case t_op: return "OP:" + EnumOps::OpToStr(Op);
        default: return {0, (char)Type};
    }
}


int Lexer::advance() {
    curChar = file->get();
    if (curChar == '\r')
        curChar = file->get();
    if (curChar == '\n') {
        lexLoc.Line++;
        lexLoc.Col = 0;
    } else
        lexLoc.Col++;
    return curChar;
}
Operator Lexer::parseOp() {
    Operator op = Operator::null;
    switch (curChar) {
        case '.': op = Operator::print; break;
        case ',': op = Operator::read; break;
        case '+': op = Operator::addition; break;
        case '-': op = Operator::subtraction; break;
        case '*': op = Operator::multiplication; break;
        case '/': op = Operator::division; break;
        case '#': op = Operator::ptr_store; break;
        case '=':
            if (advance() == '=') {
                op = Operator::equalTo;
                break;
            } else return Operator::assignment;
        case '!':
            if (advance() == '!')
                op = Operator::bool_not;
            else if (curChar == '=')
                op = Operator::notEqual;
            else return Operator::bit_not;
            break;
        case '&':
            if (advance() == '&') {
                op = Operator::bool_and;
                break;
            } else return Operator::bit_and;
        case '|':
            if (advance() == '|') {
                op = Operator::bool_or;
                break;
            } else return Operator::bit_or;
        case '^':
            if (advance() == '^') {
                op = Operator::bool_xor;
                break;
            } else return Operator::bit_xor;
        case '<':
            if (advance() == '=') {
                op = Operator::lessOrEqual;
                break;
            } else return Operator::lessThan;
        case '>':
            if (advance() == '=') {
                op = Operator::greaterOrEqual;
                break;
            } else return Operator::greaterThan;
        case '@': { //ptr operators
            advance();
            std::streampos pos = file->tellg();
            switch(parseOp()) {
                case Operator::addition:       return Operator::ptr_addition;
                case Operator::subtraction:    return Operator::ptr_subtraction;
                case Operator::multiplication: return Operator::ptr_multiplication;
                case Operator::division:       return Operator::ptr_division;
                case Operator::assignment:     return Operator::ptr_assignment;
                case Operator::lessThan:       return Operator::ptr_lessThan;
                case Operator::lessOrEqual:    return Operator::ptr_lessOrEqual;
                case Operator::greaterThan:    return Operator::ptr_greaterThan;
                case Operator::greaterOrEqual: return Operator::ptr_greaterOrEqual;
                case Operator::equalTo:        return Operator::ptr_equalTo;
                case Operator::notEqual:       return Operator::ptr_notEqual;
                case Operator::bit_not:        return Operator::ptr_not;
                case Operator::bit_and:        return Operator::ptr_and;
                case Operator::bit_or:         return Operator::ptr_or;
                case Operator::bit_xor:        return Operator::ptr_xor;
                case Operator::ptr_store:
                    if (curChar == '#') {
                        op = Operator::ptr_lookupRelDown;
                        break;
                    } else return Operator::ptr_lookupRelUp;
                default:
                    file->seekg(pos); //reset position in case non-ptr op parsed
                    return Operator::ptr_lookup;
            }
            break;
        }
        default: return Operator::null;
    }
    advance();
    return op;
}
Token Lexer::getNextToken() {
    while (isspace(curChar))
        advance();
    Location curLoc = lexLoc;
    TokenType tt;
    Operator op;

    if (isalpha(curChar)) {
        std::string str(1, (char)curChar);
        while(isalnum(advance()))
            str += (char)curChar;

        if (str == "include") tt = TokenType::t_include;
        else if (str == "define") tt = TokenType::t_define;
        else if (str == "if") tt = TokenType::t_if;
        else if (str == "else") tt = TokenType::t_else;
        else if (str == "for") tt = TokenType::t_for;
        else if (str == "while") tt = TokenType::t_while;
        else if (str == "do") tt = TokenType::t_do;
        else return *(curTok = new Token(str, curLoc));
    } else if (isdigit(curChar)) {
        int n = 0;
        do {
            n = n * 10 + curChar - '0';
        } while (isdigit(advance()));
        if (n == 0 && (curChar == 'x' || curChar == 'X')) {
            int t;
            while (advance()) {
                if (isdigit(curChar))
                    t = curChar - '0';
                else if (curChar >= 'a' && curChar <= 'f')
                    t = curChar - 'a' + 10;
                else if (curChar >= 'A' && curChar <= 'F')
                    t = curChar - 'A' + 10;
                else break;
                n = (n << 4) + t;
            }
        }
        return *(curTok = new Token(n, curLoc));
    } else if (curChar == '\'') {
        if (advance() == '\'') {
            advance();
            throw std::exception(("SyntaxException: Empty "
                                  "character constant at " + curLoc.toString()).c_str());
        }
        if (curChar == '\\')
            switch (advance()) {
                case 'a': curChar = '\a'; break;
                case 'b': curChar = '\b'; break;
                case 'f': curChar = '\f'; break;
                case 'n': curChar = '\n'; break;
                case 'r': curChar = '\r'; break;
                case 't': curChar = '\t'; break;
                case 'v': curChar = '\v'; break;
                case '1': curChar = '\1'; break;
                case '2': curChar = '\2'; break;
                case '3': curChar = '\3'; break;
                case '4': curChar = '\4'; break;
                case '5': curChar = '\5'; break;
                case '6': curChar = '\6'; break;
                case '7': curChar = '\7'; break;
            }
        curTok = new Token(curChar, curLoc);
        if (advance() != '\'') {
            while (advance() != '\'' && curChar != EOF);
            if (curChar == EOF)
                throw std::exception(("SyntaxException: Character "
                                      "constant not closed at " + curLoc.toString()).c_str());
            advance();
            throw std::exception(("SyntaxException: Multi-char "
                                  "character constant at " + curLoc.toString()).c_str());
        }
        advance();
        return *curTok;
    } else if (curChar == '/') {
        advance();
        if (curChar == '/') {
            while (advance() != '\n' && curChar != EOF);
            return getNextToken();
        } else if (curChar == '*') {
            advance();
            do {
                while (curChar != '*' && curChar != EOF)
                    advance();
                if (curChar == EOF)
                    throw std::exception(("SyntaxException: Multiline comment not closed at "+curLoc.toString()).c_str());
                advance();
            } while (curChar != '/');
            advance(); //eat '/'
            return getNextToken();
        } else return *(curTok = new Token(Operator::division, curLoc));
    } else if (curChar == '\"') {
        std::string str;
        while (advance() != '\"' && curChar != EOF)
            str += (char)curChar;
        if (curChar == EOF)
            throw std::exception(("SyntaxException: String literal never closed at "+curLoc.toString()).c_str());
        advance(); //eat ending quote
        return *(curTok = new Token(str, false, curLoc));
    } else if (curChar == EOF) tt = TokenType::t_eof;
    else if ((op = parseOp()) != Operator::null) {
        return *(curTok = new Token(op, curLoc));
    } else tt = (TokenType)curChar;

    advance();
    return *(curTok = new Token(tt, curLoc));
}
