
#include <print>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

#include "lexer.h"
#include "parser.h"

using namespace std;


int main(int argc, char *argv[]) {
    if(argc != 2) {
        println(stderr, "Error: Wrong number of arguments - expected 1 arg <filename>");
        return 1;
    }
    ifstream file(argv[1]);
    if (!file.is_open()) {
        std::println(stderr, "Error: Could not open file '{}'", argv[1]);
        return 1;
    }

    ostringstream buffer;
    buffer << file.rdbuf();
    string programStr = buffer.str();

    vector<LexToken> tokens = lex(programStr);

    for(size_t i = 0; i < tokens.size(); i++) {
        println("{}", tokens[i].toString());
    }

    ASTNode* ast = parse(tokens);

    println("{}", ast->toString());
    
}