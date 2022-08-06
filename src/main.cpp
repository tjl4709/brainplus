#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <direct.h>
#include "Parser.h"

std::string mainFile;
std::map<IncludeNode*,Parser*> *includes;
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
    //create variables
    includes = new std::map<IncludeNode*,Parser*>();

    // get mainFile and add to includes
    if (argc <= 1) exit_msg("A source file must be specified", 1);
    if (!cp_ends_with(argv[1], ".bp"))
        exit_msg("Brainplus source files must have \"bp\" extension", 2);
    mainFile = argv[1];
    if (mainFile.find(':') == -1) {
        char* tmp = (char*)malloc(FILENAME_MAX);
        std::string dir = _getcwd(tmp, FILENAME_MAX);
        free(tmp);
        if (dir[dir.size() - 1] == '\\' && mainFile[0] == '\\')
            mainFile = dir + mainFile.substr(1);
        else if (dir[dir.size() - 1] != '\\' && mainFile[0] != '\\')
            mainFile = dir + '\\' + mainFile;
    }
    auto *lexer = new Lexer(mainFile = argv[1]);
    if (!lexer->good()) exit_msg("Main file not found", 3);
    includes->insert(std::pair<IncludeNode*, Parser*>(new IncludeNode(mainFile, {0, 0}),
                                                     new Parser(lexer, &defines, &functions)));
    //add other included files to include?

    // loop thru includes:
    //   parse include statements. if not in inlcudes and lexer is good, then add to includes
    for (auto it = includes->begin(); it != includes->end(); it++) {
        while (auto inc = it->second->parseInclude(it->first->getDir())) {
            bool keep = true;
            if (!cp_ends_with((char*)inc->getId().c_str(), ".bp"))
                continue;
            for (auto pair : *includes)
                if (pair.first->getFname() == inc->getFname()) {
                    keep = false;
                    break;
                }
            if (!keep) continue;
            auto lex = new Lexer(inc->getId());
            if (lex->good())
                includes->insert(std::pair<IncludeNode*, Parser*>(inc, new Parser(lex, &defines, &functions)));
        }
    }
    /*TEST 1: Includes*/
    std::cout << "Included files:";
    for (auto inc : *includes)
        std::cout << ' ' << inc.first->getFname();
    std::cout << '\n';
    /*END TEST 1*/

    // loop thru includes:
    //   parse define statements. if define name in defines, throw error, otherwise add to defines
    for (auto inc : *includes)
        while (auto def = inc.second->parseDefine())
            defines.push_back(def);
    // loop thru define statements:
    //   if contains reference to itself, throw error, otherwise replace any occurrence (call node) in other define statements
    for (auto def : defines) {
        if (def->getReplacement()->getType() == NodeType::MultiStatement) {
            auto stmts = (MultiStatementNode *)def->getReplacement();
            for (int i = 0; i < stmts->getNumStatements(); i++)
                if (stmts->getStatement(i)->getType() == NodeType::Call &&
                    ((CallNode *)stmts->getStatement(i))->getId() == def->getId())
                    exit_msg("Recursive Defines are not allowed (" + def->getId() + ")", 4);
        } else if (def->getReplacement()->getType() == NodeType::Call &&
                   ((CallNode *)def->getReplacement())->getId() == def->getId())
            exit_msg("Recursive Defines are not allowed (" + def->getId() + ")", 4);
        for (auto d : defines)
            if (def != d) {
                if (d->getReplacement()->getType() == NodeType::MultiStatement) {
                    auto stmts = (MultiStatementNode *) d->getReplacement();
                    for (int i = 0; i < stmts->getNumStatements(); i++)
                        if (stmts->getStatement(i)->getType() == NodeType::Call &&
                            ((CallNode *) stmts->getStatement(i))->getId() == def->getId()) {
                            stmts->removeStatement(i);
                            stmts->insertStatement(def->getReplacement(), i);
                        }
                } else if (d->getReplacement()->getType() == NodeType::Call &&
                           ((CallNode *) d->getReplacement())->getId() == def->getId())
                    d->setReplacement(def->getReplacement());
            }
    }
    // loop thru includes:
    //   parse function definitions. if function name in functions or defines, throw error, otherwise add to functions
    // loop thru defines:
    //   check for any remaining call nodes that are unidentified
    // parse code statements in mainFile
    // codegen functions
    // codegen mainFile code statements
    return 0;
}
