#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <print>
#include <map>

#include "parser.h"

using namespace std;

enum class CFGOperandType {None, Reg, Imm};

struct CFGOperand {
    CFGOperandType type = CFGOperandType::None;
    int value = -1;

    CFGOperand() {}
    CFGOperand(CFGOperandType type, int value) : type(type), value(value) {}

    string toString() {
        switch(type) {
            case CFGOperandType::Reg  : return string("r") + to_string(value);
            case CFGOperandType::Imm  : return to_string(value);
            case CFGOperandType::None : return "NONE";
        }
    }
};

enum class CFGInstrType {None, Copy, Add, Sub, Mul, Div, Mod, LT, 
                         GT, LTEQ, GTEQ, EQ};

struct CFGInstr {
    CFGInstrType type = CFGInstrType::None;
    int dest = -1;
    CFGOperand lhs = CFGOperand();
    CFGOperand rhs = CFGOperand();
    int line = -1;

    CFGInstr(CFGInstrType type, int dest, CFGOperand lhs, CFGOperand rhs, int line) :
        type(type), dest(dest), lhs(lhs), rhs(rhs), line(line) {}
    CFGInstr(CFGInstrType type, int dest, CFGOperand lhs, int line) :
        type(type), dest(dest), lhs(lhs), line(line) {}
    
    string toString() {
        string opStr;
        switch(type) {
            case CFGInstrType::Copy : opStr = ""    ; break;
            case CFGInstrType::Add  : opStr = "+"   ; break;
            case CFGInstrType::Sub  : opStr = "-"   ; break;
            case CFGInstrType::Mul  : opStr = "*"   ; break;
            case CFGInstrType::Div  : opStr = "/"   ; break;
            case CFGInstrType::Mod  : opStr = "%"   ; break;
            case CFGInstrType::LT   : opStr = "<"   ; break;
            case CFGInstrType::GT   : opStr = ">"   ; break;
            case CFGInstrType::LTEQ : opStr = "<="  ; break;
            case CFGInstrType::GTEQ : opStr = ">="  ; break;
            case CFGInstrType::EQ   : opStr = "=="  ; break;
            case CFGInstrType::None : opStr = "NONE"; break;
        }
        string res = "r";
        res += to_string(dest);
        res += " = ";
        res += lhs.toString();
        if(type != CFGInstrType::Copy) {
            res += " ";
            res += opStr;
            res += " ";
            res += rhs.toString();
        }
        res += ";";
        return res;
    }
};

enum class CFGTerminatorType {None, Jump, Branch, Return};

struct CFGTerminator {
    CFGTerminatorType type = CFGTerminatorType::None;
    CFGOperand value = CFGOperand(); // Jump: unused, Branch: condition, Return: return value

    CFGTerminator() {}
    CFGTerminator(CFGTerminatorType type, CFGOperand value) : type(type), value(value) {}
    CFGTerminator(CFGTerminatorType type) : type(type) {}

    string toString() {
        switch(type) {
            case CFGTerminatorType::Jump   : return string("Jump");
            case CFGTerminatorType::Branch : return string("If ") + value.toString();
            case CFGTerminatorType::Return : return string("Return ") + value.toString();
            case CFGTerminatorType::None : return string("NONE");
        }
    }
};

struct CFGBlock {
    int id;
    vector<CFGInstr> instrs;
    CFGTerminator terminator = CFGTerminator();
    
    vector<int> succs; // Jump : [target], Branch : [<true>, <false>] , Return : []
    vector<int> preds;

    CFGBlock(int id) : id(id) {}

    void addInstr(CFGInstr instr) {
        instrs.push_back(instr);
    }

    string toString() {
        string nameStr = string("B") + to_string(id);
        string instrsStr = "";
        for(size_t i = 0; i < instrs.size(); i++) {
            instrsStr += "\t" + instrs[i].toString() + ((i < instrs.size() - 1)? "\n" : "");
        }
        string termString = terminator.toString();
        string succsStr = "[";
        for(size_t i = 0; i < succs.size(); i++) {
            succsStr += string("B") + to_string(succs[i]) + ((i < succs.size() - 1)? ", " : "");
        }
        succsStr += "]";
        return nameStr + ":\n" + instrsStr + "\n\t" + termString + "\n\t" + succsStr;
    }
};

struct CFGSymbolTable {
    int regCount = 0;
    map<string, int> symbols = {}; // maps to register value

    int getReg() {
        return regCount++;
    }

    int getReg(string symbol) {
        if (symbols.find(symbol) != symbols.end()) {
            return symbols[symbol];
        } else {
            return -1;
        }
    }

    void updateSymbol(string symbol, int reg) {
        if (symbols.find(symbol) != symbols.end()) {
            throw logic_error("Attempting to update already existing symbol");
        } else {
            symbols[symbol] = reg;
        }
    }

};

struct CFG {
    vector<CFGBlock> blocks; // root is always the first block in blocks
    CFGSymbolTable symbolTable = CFGSymbolTable();

    string toString() {
        string res = "";
        for(size_t i = 0; i < blocks.size(); i++) {
            res += blocks[i].toString() + "\n";
        }
        return res;
    }
};

CFG* lower(ASTNode* ast);