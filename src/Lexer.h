//
// Created by 7budd on 5/20/2022.
//
#ifndef BRAINPLUS_LEXER_H
#define BRAINPLUS_LEXER_H

#include <string>
#include <utility>
#include <fstream>
#include <vector>
#include "enums.h"

struct Location {
    int Line, Col;
    std::string toString() const {
        return "l:" + std::to_string(Line) + " c:" + std::to_string(Col);
    }
};

class Token {
public:
    TokenType Type;
    int Number;
    Operator Op;
    std::string Identifier;
    Location Loc;
    Token(){}   //*only temporary for Lexer constructor. DO NOT USE
    Token(TokenType t, Location l);
    Token(int n, Location l);
    Token(Operator op, Location l);
    Token(std::string id, bool isIden, Location l);
    Token(std::string id, Location l);
    void copy(const Token& tok);
    std::string toString() const;
};

class Lexer {
private:
    Location lexLoc;
    Token curTok;
    std::string fname;
    std::vector<Token> *defRep;
    std::ifstream *file;
    int curChar;
    int advance();
    Operator parseOp();
public:
    explicit Lexer(const std::string& filename) : lexLoc({1,0}), curChar(' ') {
        defRep = new std::vector<Token>();
        fname = filename;
        file = new std::ifstream(filename);
        getNextToken();
    }

    Token getCurrentToken() {return curTok;}
    TokenType getCurrentType() const {return curTok.Type;}
    std::string getCurrentIdentifier() const {return curTok.Identifier;}
    Operator getCurrentOp() const {return curTok.Op;}
    Location getCurrentLocation() const {return curTok.Loc;}
    std::string getCurrentLocString() const {return curTok.Loc.toString();}

    Token getNextToken();
    TokenType getNextType() {return getNextToken().Type;}

    bool good() {return file->good();}
    std::string getFileName() {return fname;}
    std::streampos getFilePos() {return file->tellg();}
    std::istream& setFilePos(const Token& tok, std::streampos pos) {
        curTok.copy(tok);
        return file->seekg(pos);
    }
    void setReplacement(std::vector<Token> *rep) {
        for (unsigned int i = rep->size();i > 1;)
            defRep->push_back(rep->at(--i));
        curTok = rep->front();
        curTok.Loc = lexLoc;
    }
};

#endif //BRAINPLUS_LEXER_H
