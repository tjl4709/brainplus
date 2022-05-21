//
// Created by 7budd on 5/20/2022.
//
#ifndef BRAINPLUS_LEXER_H
#define BRAINPLUS_LEXER_H

#include <string>
#include <utility>
#include <fstream>


struct Location {
    int Line, Col;
    std::string toString() const {
        return "l:" + std::to_string(Line) + " c:" + std::to_string(Col);
    }
};

enum TokenType {
    t_eof       =  -1,

    //keywords
    t_include   =  -2,
    t_define    =  -3,
    t_if        =  -4,
    t_else      =  -5,
    t_for       =  -6,
    t_while     =  -7,
    t_do        =  -8,

    //primary
    t_number    =  -9,
    t_identifier= -10
};
class Token {
public:
    TokenType Type;
    Location Loc;
    Token(TokenType t, Location l) : Type(t), Loc(l) {}
    virtual std::string toString() {
        switch (Type) {
            case t_eof: return "EOF";
            case t_include: return "include";
            case t_define: return "define";
            case t_if: return "if";
            case t_else: return "else";
            case t_for: return "for";
            case t_while: return "while";
            case t_do: return "do";
            case t_number: return "NUMBER";
            case t_identifier: return "IDENTIFIER";
            default: return {0, (char)Type};
        }
    }
};
class IdentifierToken : public Token {
public:
    std::string Identifier;
    IdentifierToken(std::string id, Location l) : Token(TokenType::t_identifier, l), Identifier(std::move(id)) {}
    std::string toString() override { return "ID:" + Identifier; }
};
class NumberToken : public Token {
public:
    int Number;
    NumberToken(int n, Location l) : Token(TokenType::t_number, l), Number(n) {}
    std::string toString() override { return "N:" + std::to_string(Number); }
};

class Lexer {
private:
    Location lexLoc;
    Token *curTok;
    std::ifstream *file;
    int advance() {
        int c = file->get();
        if (c == '\r')
            c = file->get();
        if (c == '\n') {
            lexLoc.Line++;
            lexLoc.Col = 0;
        } else
            lexLoc.Col++;
        return c;
    }
public:
    explicit Lexer(const std::string& filename) : lexLoc({0,0}), curTok(nullptr) {
        file = new std::ifstream(filename);
    }
    bool good() {return file->good();}
    Token getCurrentToken() {return *curTok;}
    Token getNextToken() {
        int c;
        while (isspace(c = advance()));
        Location curLoc = lexLoc;
        TokenType tt;

        if (isalpha(c)) {
            std::string str(1,c);
            while(isalnum(c = advance()))
                str += (char)c;

            if (str == "include") tt = TokenType::t_include;
            else if (str == "define") tt = TokenType::t_define;
            else if (str == "if") tt = TokenType::t_if;
            else if (str == "else") tt = TokenType::t_else;
            else if (str == "for") tt = TokenType::t_for;
            else if (str == "while") tt = TokenType::t_while;
            else if (str == "do") tt = TokenType::t_do;
            else return *(curTok = new IdentifierToken(str, curLoc));
        } else if (isdigit(c)) {
            int n = 0;
            do {
                n = n*10 + c;
            } while (isdigit(c = advance()));
            return *(curTok = new NumberToken(n, curLoc));
        } else if (c == '/') {
            c = advance();
            if (c == '/') {
                while ((c = advance()) != '\n');
                return getNextToken();
            } else if (c == '*') {
                c = advance();
                do {
                    while (c != '*' && c != EOF)
                        c = advance();
                    if (c == EOF)
                        throw std::exception(("SyntaxException: Multiline comment not closed at "+curLoc.toString()).c_str());
                    c = advance();
                } while (c != '/');
                return getNextToken();
            } else throw std::exception(("SyntaxException: Unexpected '/' at " + curLoc.toString()).c_str());
        } else if (c == EOF) tt = TokenType::t_eof;
        else tt = (TokenType)c;

        return *(curTok = new Token(tt, curLoc));
    }
};

#endif //BRAINPLUS_LEXER_H
