#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <print>

#include "lexer.h"

using namespace std;

enum class ASTNodeType {
    None,
    ValInt, ValIdentifier,
    StmtInput, StmtAssign, StmtIf, StmtWhile, StmtReturn,
    OpAdd, OpSub, OpMul, OpDiv, OpMod,
    OpLT, OpGT, OpLTEQ, OpGTEQ, OpEQ,
};

struct ASTNode {
    ASTNodeType type = ASTNodeType::None;
    ASTNode* cond = nullptr;
    ASTNode* lhs = nullptr;
    ASTNode* rhs = nullptr;
    ASTNode* next = nullptr;
    int line = 0;
    int intVal = 0;
    string strVal;

    ASTNode() {}

    inline static vector<ASTNode*> nodes;
    inline static vector<uint64_t> freeList;
    
    static ASTNode* allocNode() {
        for(size_t i = 0; i < freeList.size(); i++) {
            int freeIdx = countr_one(freeList[i]);
            if(freeIdx != 64) {
                freeList[i] |= uint64_t{1} << freeIdx;
                ASTNode* node = nodes[i] + freeIdx;
                construct_at(node);
                return node;
            }
        }
        // allocate new chunk and mark first spot allocated
        ASTNode* node = (ASTNode*) malloc(sizeof(ASTNode) * 64);
        nodes.push_back(node);
        freeList.push_back(uint64_t{1});
        construct_at(node);
        return node;
    }

    static void freeNode(ASTNode* node) {
        for(size_t i = 0; i < nodes.size(); i++) {
            if(node >= nodes[i] && node < nodes[i] + 64) {
                int idx = node - nodes[i];
                std::destroy_at(node);
                freeList[i] &= ~(uint64_t{1} << idx);
                if(freeList[i] == 0) {
                    // dealloc chunk
                    free(nodes[i]);
                    nodes[i] = nodes.back();
                    freeList[i] = freeList.back();
                    nodes.pop_back();
                    freeList.pop_back();
                }
                return;
            }
        }
        throw logic_error("Attempting to free an ASTNode at an unrecognized address");
    }

    bool isVal() {
        return type == ASTNodeType::ValIdentifier ||
               type == ASTNodeType::ValInt;
    }

    bool isStmt() {
        return type == ASTNodeType::StmtInput ||
               type == ASTNodeType::StmtAssign ||
               type == ASTNodeType::StmtIf ||
               type == ASTNodeType::StmtReturn ||
               type == ASTNodeType::StmtWhile;
    }

    bool isOp() {
        return type == ASTNodeType::OpAdd ||
               type == ASTNodeType::OpSub ||
               type == ASTNodeType::OpMul ||
               type == ASTNodeType::OpDiv ||
               type == ASTNodeType::OpMod ||
               type == ASTNodeType::OpLT ||
               type == ASTNodeType::OpGT ||
               type == ASTNodeType::OpLTEQ ||
               type == ASTNodeType::OpGTEQ ||
               type == ASTNodeType::OpEQ;
    }

    string toString(int depth = 0) {
        string ident(depth, '\t');
        string nodeStr;
        switch(type) {
            case ASTNodeType::ValInt :
                nodeStr = to_string(intVal);
                break;
            case ASTNodeType::ValIdentifier :
                nodeStr = strVal;
                break;
            case ASTNodeType::StmtInput:
                nodeStr = "input";
                break;
            case ASTNodeType::StmtAssign :
                nodeStr = "assign";
                break;
            case ASTNodeType::StmtIf :
                nodeStr = "if";
                break;
            case ASTNodeType::StmtWhile :
                nodeStr = "while";
                break;
            case ASTNodeType::StmtReturn :
                nodeStr = "return";
                break;
            case ASTNodeType::OpAdd :
                nodeStr = "+";
                break;
            case ASTNodeType::OpSub :
                nodeStr = "-";
                break;
            case ASTNodeType::OpMul :
                nodeStr = "*";
                break;
            case ASTNodeType::OpDiv :
                nodeStr = "/";
                break;
            case ASTNodeType::OpMod :
                nodeStr = "%";
                break;
            case ASTNodeType::OpLT :
                nodeStr = "<";
                break;
            case ASTNodeType::OpGT :
                nodeStr = ">";
                break;
            case ASTNodeType::OpLTEQ :
                nodeStr = "<=";
                break;
            case ASTNodeType::OpGTEQ :
                nodeStr = ">=";
                break;
            case ASTNodeType::OpEQ :
                nodeStr = "==";
                break;
            case ASTNodeType::None :
                nodeStr = "NONE";
        }
        string res = ident + nodeStr;
        if(cond) res += "\n" + cond->toString(depth + 1);
        if(lhs)  res += "\n" + lhs ->toString(depth + 1);
        if(rhs)  res += "\n" + rhs ->toString(depth + 1);
        if(next) res += "\n" + next->toString(depth);
        
        return res;
    }
};

ASTNode* parse(vector<LexToken>& tokens);