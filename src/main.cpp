#include <iostream>
#include <vector>
#include <map>
#include <string>
#include "Parser.h"
#ifdef _WINDOWS
#include <direct.h>
#define getCurDir _getcwd
#else
#include <unistd.h>
#define getCurDir getcwd
#endif

std::string mainFile;
std::map<IncludeNode*,Parser*> *includes;
std::vector<DefineNode*> defines;
std::vector<FunctionNode*> functions;
StatementNode *code;

bool cp_ends_with(char* str, std::string suffix) {
    unsigned int str_len = strlen(str), suf_len = suffix.size();
    if (str_len < suf_len) return false;
    for (; suf_len > 0; )
        if (tolower(str[--str_len]) != tolower(suffix[--suf_len]))
            return false;
    return true;
}
char* cp_to_lower(char* str) {
    for (int i = 0; i < strlen(str); i++)
        if ('A' <= str[i] && str[i] <= 'Z')
            str[i] += 'a' - 'A';
    return str;
}
std::string join(std::vector<std::string> elems, const std::string& delim) {
    std::string str;
    if (!elems.empty()) {
        str = elems[0];
        for (auto i = elems.begin()+1; i < elems.end(); i++)
            str += delim + *i;
    }
    return std::move(str);
}
int exit_msg(const std::string& msg, int code) {
    for (auto func : functions)
        delete func;
    for (auto def : defines)
        delete def;
    for (auto inc : *includes) {
        delete inc.first;
        delete inc.second;
    }
    delete includes;
    std::cerr << msg + '\n';
    exit(code);
}

void parseIncludes() {
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
            else delete lex;
        }
    }
    /*TEST 1: Includes*
    std::cout << "Included files:";
    for (auto inc : *includes)
        std::cout << ' ' << inc.first->getFname();
    std::cout << '\n';
    /*END TEST 1*/
}
void parseDefines() {
    // loop thru includes:
    //   parse define statements. if define name in defines, throw error, otherwise add to defines
    for (auto inc : *includes)
        while (auto def = inc.second->parseDefine())
            defines.push_back(def);
    /*TEST 2.1: Recursive Defines*
    std::cout << "Defines::\n";
    for (auto def : defines)
        std::cout << def->to_string() + '\n';
    /*END TEST 2.1*/

    // loop thru define statements:
    //   if contains reference to itself, throw error, otherwise replace any occurrence (call node) in other define statements
    auto defNames = new std::vector<std::string>();
    DefineNode *d;
    for (auto def : defines) {
        defNames->clear();
        defNames->push_back(def->getId());
        for (int i = 0; i < def->getNumReplacements(); i++)
            if (def->getReplacement(i)->Type == TokenType::t_identifier) {
                if (def->getReplacement(i)->Identifier == def->getId())
                    exit_msg("RecursiveDefineException: Some or all of the following defines create a cycle - " +
                             join(*defNames, ", "), 4);
                if ((d = GetDefine(def->getReplacement(i)->Identifier, &defines))) {
                    defNames->push_back(d->getId());
                    def->setReplacement(d->getReplacements(), i--);
                    break;
                }
            }
    }
    delete defNames;
    /*TEST 2.2: Cyclic Defines*
    std::cout << "\nDefines::\n";
    for (auto def : defines)
        std::cout << def->to_string() + '\n';
    /*END TEST 2.2*/
}
void checkForIdentifier(std::string id, Location l) {
    if (!GetDefine(id, &defines) && !GetFunction(id, &functions))
        exit_msg("UnknownIdentifierException: Unknown identifier \"" + id + "\" at " + l.toString(), 5);
}
void checkForUnknownIds() {
    // loop thru defines and functions:
    //   check for any remaining call nodes that are unidentified
    for (auto def : defines)
        for (int i = 0; i < def->getNumReplacements(); i++)
            if (def->getReplacement(i)->Type == TokenType::t_identifier)
                checkForIdentifier(def->getReplacement(i)->Identifier, def->getReplacement(i)->Loc);
    for (auto func : functions)
        if (func->getBody()->getType() == NodeType::MultiStatement) {
            auto *m = (MultiStatementNode*)func->getBody();
            for (int i = 0; i < m->getNumStatements(); i++)
                if (m->getStatement(i)->getType() == NodeType::Call)
                    checkForIdentifier(((CallNode *) m->getStatement(i))->getId(), m->getStatement(i)->getLoc());
        } else if (func->getBody()->getType() == NodeType::Call)
            checkForIdentifier(((CallNode *) func->getBody())->getId(), func->getBody()->getLoc());
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
        std::string dir = getCurDir(tmp, FILENAME_MAX);
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
    //add other specified files to include?

    parseIncludes();
    parseDefines();

    // loop thru includes:
    //   parse function definitions. if function name in functions or defines, throw error, otherwise add to functions
    for (auto inc : *includes)
        while (auto func = inc.second->parseFunction())
            functions.push_back(func);
    /*TEST 3: Function Definitions*
    std::cout << "Function::\n";
    for (auto func : functions)
        std::cout << func->to_string() + '\n';
    /*END TEST 3*/

    checkForUnknownIds();

    // parse code statements in mainFile
    // codegen functions
    for (auto inc : *includes)
        if (inc.first->getId() == mainFile)
            code = inc.second->parseCode();
    /*TEST 4: Code*/
    std::cout << "Code:\n" + code->to_string();
    /*END TEST 4*/

    // codegen mainFile code statements
    // delete AST
    return 0;
}
