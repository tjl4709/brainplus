//
// Created by 7budd on 5/23/2022.
//
#include "Lexer.h"

Token::Token(TokenType t, Location l) : Type(t), Loc(l), Number(0) {}
Token::Token(int n, Location l) : Type(TokenType::t_number), Loc(l), Number(n) {}
Token::Token(std::string id, Location l) : Type(TokenType::t_identifier), Loc(l), Number(0), Identifier(std::move(id)) {}

std::string Token::toString() {
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
Token Lexer::getNextToken() {
    while (isspace(curChar))
        advance();
    Location curLoc = lexLoc;
    TokenType tt;

    if (isalpha(curChar)) {
        std::string str(1, curChar);
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
        } else throw std::exception(("SyntaxException: Unexpected '/' at " + curLoc.toString()).c_str());
    } else if (curChar == EOF) tt = TokenType::t_eof;
    else tt = (TokenType)curChar;

    advance();
    return *(curTok = new Token(tt, curLoc));
}
