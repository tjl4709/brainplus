#include <iostream>
#include <vector>
#include <map>
#include "Lexer.h"
#include "Parser.h"
#include "ASTNodes.h"

std::string mainFile;
std::map<IncludeNode*,Parser*> includes;
std::vector<DefineNode*> defines;
std::vector<FunctionNode*> functions;

int main() {
    // get mainFile and add to includes
    // loop thru includes:
    //   parse include statements. if not in inlcudes and lexer is good, then add to includes
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
}
