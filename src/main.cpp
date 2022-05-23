#include <iostream>
#include "Lexer.h"

Lexer *lexer;

void openFile() {
    std::string file;
    std::cout << "Enter filename:";
    std::cin >> file;
    lexer = new Lexer(file);
}

int main() {
    bool errored;
    openFile();
    while (lexer->good()) {
        errored = false;
        do {
            try {
                while (lexer->getNextToken().Type != TokenType::t_eof) {
                    if (errored) {
                        errored = false;
                        std::cout << '\n';
                    }
                    std::cout << lexer->getCurrentToken().toString() + " ";
                }
            } catch (std::exception &e) {
                std::cout << '\n' << e.what();
                errored = true;
            }
        } while (lexer->getCurrentToken().Type != TokenType::t_eof);
        std::cout << "\n\n";
        openFile();
    }
}
