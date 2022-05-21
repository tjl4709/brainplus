#include <iostream>
#include "Lexer.h"

Lexer *lexer;

void openFile() {
    std::string file;
    std::cout << "Enter filename: ";
    std::cin >> file;
    lexer = new Lexer(file);
}

int main() {
    openFile();
    while (lexer->good()) {
        try {
            while (lexer->getNextToken().Type != TokenType::t_eof) {
                std::cout << lexer->getCurrentToken().toString() + " ";
            }
        } catch (std::exception &e) {
            std::cout << '\n' << e.what();
        }
        std::cout << '\n';
        openFile();
    }
}
