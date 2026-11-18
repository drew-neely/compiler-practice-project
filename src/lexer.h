#pragma once

#include <string>
#include <vector>
#include <format>

using namespace std;

enum class LexTokenType {
    ValInteger, ValIdentifier,
    KwInput, KwIf, KwElse, KwWhile, kwReturn,
    OpAssign, OpAdd, OpSub, OpMul, OpDiv, OpMod,
    OpLT, OpGT, OpLTEQ, OpGTEQ, OpEQ,
    SmLParen, SmRParen, SmLBrace, SmRBrace, SmSemi,
    SmEOF
};

struct LexToken {
    LexTokenType type;
    int line;
    int intVal;
    string strVal;

    LexToken(LexTokenType type, int line) : type(type), line(line) {}
    LexToken(LexTokenType type, int line, int intVal) : type(type), line(line), intVal(intVal) {}
    LexToken(LexTokenType type, int line, string strVal) : type(type), line(line), strVal(strVal) {}

    string toString() {
        string typeStr;
        switch (type) {
            case LexTokenType::ValInteger:    typeStr = "ValInteger"; break;
            case LexTokenType::ValIdentifier: typeStr = "ValIdentifier"; break;
            case LexTokenType::KwInput:       typeStr = "KwInput"; break;
            case LexTokenType::KwIf:          typeStr = "KwIf"; break;
            case LexTokenType::KwElse:        typeStr = "KwElse"; break;
            case LexTokenType::KwWhile:       typeStr = "KwWhile"; break;
            case LexTokenType::kwReturn:      typeStr = "kwReturn"; break;
            case LexTokenType::OpAssign:      typeStr = "OpAssign"; break;
            case LexTokenType::OpAdd:         typeStr = "OpAdd"; break;
            case LexTokenType::OpSub:         typeStr = "OpSub"; break;
            case LexTokenType::OpMul:         typeStr = "OpMul"; break;
            case LexTokenType::OpDiv:         typeStr = "OpDiv"; break;
            case LexTokenType::OpMod:         typeStr = "OpMod"; break;
            case LexTokenType::OpLT:          typeStr = "OpLT"; break;
            case LexTokenType::OpGT:          typeStr = "OpGT"; break;
            case LexTokenType::OpLTEQ:        typeStr = "OpLTEQ"; break;
            case LexTokenType::OpGTEQ:        typeStr = "OpGTEQ"; break;
            case LexTokenType::OpEQ:          typeStr = "OPEQ"; break;
            case LexTokenType::SmLParen:      typeStr = "SmLParen"; break;
            case LexTokenType::SmRParen:      typeStr = "SmRParen"; break;
            case LexTokenType::SmLBrace:      typeStr = "SmLBrace"; break;
            case LexTokenType::SmRBrace:      typeStr = "SmRBrace"; break;
            case LexTokenType::SmSemi:        typeStr = "SmSemi"; break;
            case LexTokenType::SmEOF:         typeStr = "SmEOF"; break;
            default:                       typeStr = "Unknown"; break;
        }
        if(type == LexTokenType::ValInteger) {
            return format("<{} {}> line {}", typeStr, intVal, line);
        } else if(type == LexTokenType::ValIdentifier) {
            return format("<{} {}> line {}", typeStr, strVal, line);
        } else {
            return format("<{}> line {}", typeStr, line);
        }
    }
};

vector<LexToken> lex(string& program);