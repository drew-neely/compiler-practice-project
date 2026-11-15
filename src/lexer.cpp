#include <print>
#include <string>
#include <fstream>
#include <vector>
#include <stdexcept>

#include "lexer.h"

using namespace std;

vector<LexToken> lex(string& program) {
    vector<LexToken> tokens;

    string word;
    int line = 1;

    for (size_t cursor = 0; cursor < program.size(); cursor++)
    {
        char c = program[cursor];
        char nc = cursor + 1 < program.size() ? program[cursor + 1] : 0;
        if(c == '\n') {
            line++;
        }
        if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_') {
            word += c;
            if(nc == 0 || !((nc >= 'a' && nc <= 'z') || (nc >= 'A' && nc <= 'Z') || (nc >= '0' && nc <= '9') || nc == '_')) {
                // word is finished
                bool isInt = 1;
                for(size_t i = 0; i < word.size(); i++) {
                    if(!(word[0] >= '0' && word[0] <= '9')) {
                        isInt = false;
                        break;
                    }
                }
                if(isInt) {
                    int intVal;
                    try {
                        intVal = stoi(word);
                    } catch (out_of_range& exception) {
                        throw out_of_range("Integer constant is out of range");
                    } catch (invalid_argument& exception) {
                        throw logic_error("Unexpected error when parsing integer");
                    }
                    tokens.push_back(LexToken(LexTokenType::ValInteger, line, intVal));
                } else if(word[0] >= '0' && word[0] <= '9') {
                    throw invalid_argument("Identifier cannot start with a digit");
                } else {
                    if(word == "if") {
                        tokens.push_back(LexToken(LexTokenType::KwIf, line));
                    } else if(word == "else") {
                        tokens.push_back(LexToken(LexTokenType::KwElse, line));
                    } else if(word == "while") {
                        tokens.push_back(LexToken(LexTokenType::KwWhile, line));
                    } else if(word == "return") {
                        tokens.push_back(LexToken(LexTokenType::kwReturn, line)); 
                    } else {
                        tokens.push_back(LexToken(LexTokenType::ValIdentifier, line, word));
                    }
                }
                word = "";
            }
            continue;
        } else if(c == '=' && nc != '=') {
            tokens.push_back(LexToken(LexTokenType::OpAssign, line)); 
        } else if(c == '=' && nc == '=') {
            tokens.push_back(LexToken(LexTokenType::OpEQ, line));
            cursor++;
        } else if(c == '+') {
            tokens.push_back(LexToken(LexTokenType::OpAdd, line));
        } else if(c == '-') {
            tokens.push_back(LexToken(LexTokenType::OpSub, line));
        } else if(c == '*') {
            tokens.push_back(LexToken(LexTokenType::OpMul, line));
        } else if(c == '/') {
            tokens.push_back(LexToken(LexTokenType::OpDiv, line));
        } else if(c == '%') {
            tokens.push_back(LexToken(LexTokenType::OpMod, line));
        } else if(c == '<' && nc != '=') {
            tokens.push_back(LexToken(LexTokenType::OpLT, line));
        } else if(c == '<' && nc == '=') {
            tokens.push_back(LexToken(LexTokenType::OpLTEQ, line));
            cursor++;
        } else if(c == '>' && nc != '=') {
            tokens.push_back(LexToken(LexTokenType::OpGT, line));
        } else if(c == '>' && nc == '=') {
            tokens.push_back(LexToken(LexTokenType::OpGTEQ, line));
            cursor++;
        } else if(c == '(') {
            tokens.push_back(LexToken(LexTokenType::SmLParen, line));
        } else if(c == ')') {
            tokens.push_back(LexToken(LexTokenType::SmRParen, line));
        } else if(c == '{') {
            tokens.push_back(LexToken(LexTokenType::SmLBrace, line));
        } else if(c == '}') {
            tokens.push_back(LexToken(LexTokenType::SmRBrace, line));
        } else if(c == ';') {
            tokens.push_back(LexToken(LexTokenType::SmSemi, line));
        } else if(c == ' ' || c == '\t' || c == '\n') {
            // pass
        } else {
            throw invalid_argument(std::string("Unexpected symbol \"") + c + "\"");
        }
    }
    tokens.push_back(LexToken(LexTokenType::SmEOF, line));

    return tokens;    
}
