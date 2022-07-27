#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <string_view>
#include "Lexer.h"
#include "Parser.h"
#include "ASTNodes.h"

std::string mainFile;
std::map<IncludeNode*,Parser*> includes;
std::vector<DefineNode*> defines;
std::vector<FunctionNode*> functions;

bool cp_ends_with(char* str, std::string suffix) {
    int str_len = strlen(str), suf_len = suffix.size();
    if (str_len < suf_len) return false;
    for (str_len--, suf_len--; suf_len >= 0; str_len--, suf_len--)
        if (tolower(str[str_len]) != tolower(suffix[suf_len]))
            return false;
    return true;
}
char* cp_to_lower(char* str) {
    for (int i = 0; i < strlen(str); i++)
        if ('A' <= str[i] && str[i] <= 'Z')
            str[i] += 'a' - 'A';
    return str;
}
int exit_msg(const std::string& msg, int code) {
    std::cerr << msg + '\n';
    exit(code);
}


int main(int argc, char *argv[]) {
    // get mainFile and add to includes
    if (argc <= 1) exit_msg("A source file must be specified", 1);
    if (!cp_ends_with(argv[1], ".bp"))
        exit_msg("Brainplus source files must have \"bp\" extension", 2);
    auto *lexer = new Lexer(mainFile = argv[1]);
    if (!lexer->good()) exit_msg("Main file not found", 3);
    includes.insert(std::pair<IncludeNode*, Parser*>(new IncludeNode(mainFile, {0, 0}),
                                                     new Parser(lexer, &defines, &functions)));
    // loop thru includes:
    //   parse include statements. if not in inlcudes and lexer is good, then add to includes
    for (auto it = includes.begin(); it != includes.end(); it++) {
        while (auto inc = it->second->parseInclude()) {
            if (!cp_ends_with((char*)inc->getId().c_str(), ".bp"))
                continue;
            for (auto pair : includes)
                if (pair.first->getId() == inc->getId())
                    continue;
            auto lex = new Lexer(inc->getId());
            if (lex->good())
                includes.insert(std::pair<IncludeNode*, Parser*>(inc, new Parser(lex, &defines, &functions)));
        }
    }
    // loop thru includes:
    //   parse define statements. if define name in defines, throw error, otherwise add to defines
    // loop thru define statements:
    //   if contains reference to itself, throw error, otherwise replace any occurrence in other define statements
    // loop thru includes:
    //   insert defines into parser
    //   parse function definitions. if function name in functions or defines, throw error, otherwise add to functions
    // parse code statements in mainFile
    // codegen functions
    // codegen mainFile code statements
    return 0;
}
