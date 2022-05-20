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
};
class IdentifierToken : public Token {
public:
    std::string Identifier;
    IdentifierToken(std::string id, Location l) : Token(TokenType::t_identifier, l), Identifier(std::move(id)) {}
};
class NumberToken : public Token {
public:
    int Number;
    NumberToken(int n, Location l) : Token(TokenType::t_number, l), Number(n) {}
};

class Lexer {
private:
    Location lexLoc;
    Token *curTok;
    std::ifstream file;
    int advance() {
        int c = file.get();
        if (c == '\r')
            c = file.get();
        if (c == '\n') {
            lexLoc.Line++;
            lexLoc.Col = 0;
        } else
            lexLoc.Col++;
        return c;
    }
public:
    Token getCurrentToken() {return *curTok;}
    Token getNextToken() {
        int c;
        while (isspace(c = advance()));
        Location curLoc = lexLoc;
        TokenType tt;

        if (isalpha(c)) {
            std::string str = std::to_string(c);
            while(isalnum(c = advance()))
                str += std::to_string(c);

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
        } 

        return *curTok;
    }
};


#endif //BRAINPLUS_LEXER_H
