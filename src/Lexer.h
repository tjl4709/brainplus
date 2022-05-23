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
    int Number;
    std::string Identifier;
    Location Loc;
    Token(TokenType t, Location l);
    Token(int n, Location l);
    Token(std::string id, Location l);
    virtual std::string toString();
};

class Lexer {
private:
    Location lexLoc;
    Token *curTok;
    std::ifstream *file;
    int curChar;
    int advance();
public:
    explicit Lexer(const std::string& filename) : lexLoc({1,0}), curTok(nullptr), curChar(' ') {
        file = new std::ifstream(filename);
    }
    bool good() {return file->good();}
    Token getCurrentToken() {return *curTok;}
    Token getNextToken();
};

#endif //BRAINPLUS_LEXER_H
