#include <string>
#include <fstream>
#include <vector>
#include <print>

#include "cfg.h"
#include "parser.h"

ASTNode* iterParams(CFG* cfg, ASTNode* ast);
ASTNode* step(CFG* cfg, int& blockId, ASTNode* ast);
CFGOperand lowerExpr(CFG* cfg, int& blockId, ASTNode* ast);

CFG* lower(ASTNode* ast) {
    CFG* cfg = new CFG();
    ast = iterParams(cfg, ast);
    cfg->blocks.emplace_back(0);
    int blockId = 0;
    while(ast) {
        ast = step(cfg, blockId, ast);
    }
    return cfg;
}

ASTNode* iterParams(CFG* cfg, ASTNode* ast) {
    if(ast && ast->type == ASTNodeType::StmtInput) {
        ASTNode* listPtr = ast->lhs;
        while(listPtr) {
            if(listPtr->type == ASTNodeType::ValIdentifier) {
                cfg->symbolTable.newParam(listPtr->strVal);
            } else {
                throw invalid_argument("Unexpected node in input parameter list");
            }
            listPtr = listPtr->next;
        }
        return ast->next;
    }
    return ast;
}

ASTNode* step(CFG* cfg, int& blockId, ASTNode* ast) {
    if(ast == nullptr) {
        throw logic_error("step passed a nullptr");
    }
    if(!ast->isStmt()) {
        throw logic_error(string() + "Expected a statement at line: " + to_string(ast->line)); // Todo: better error message
    }
    if(ast->type == ASTNodeType::StmtAssign) {
        if(ast->lhs->type != ASTNodeType::ValIdentifier) {
            throw logic_error("Expected an identifier on lhs of assignment");
        }
        CFGOperand rhs = lowerExpr(cfg, blockId, ast->rhs);
        if(rhs.type == CFGOperandType::Reg || rhs.type == CFGOperandType::Imm) {
            int reg = cfg->symbolTable.getReg(ast->lhs->strVal);
            if(reg < 0) { // symbol is not defined
                reg = cfg->symbolTable.newSymbol(ast->lhs->strVal);
            }
            cfg->blocks[blockId].addInstr(CFGInstr(CFGInstrType::Copy, reg, rhs, ast->line));
        } else {
            throw logic_error("Rhs operand of assignment has invalid type");
        }
        return ast->next;
    } else if(ast->type == ASTNodeType::StmtIf) {
        CFGOperand cond = lowerExpr(cfg, blockId, ast->cond);
        cfg->blocks[blockId].terminator = CFGTerminator(CFGTerminatorType::Branch, cond);
        int thenBlockId = cfg->blocks.size();
        cfg->blocks.emplace_back(thenBlockId);

        cfg->blocks[blockId].succs.push_back(thenBlockId);
        cfg->blocks[thenBlockId].preds.push_back(blockId);

        ASTNode* blockAstPtr = ast->lhs;
        int thenBlockIdPtr = thenBlockId;
        while(blockAstPtr) {
            blockAstPtr = step(cfg, thenBlockIdPtr, blockAstPtr);
        }
        
        int elseBlockIdPtr = -1;
        if(ast->rhs) {
            int elseBlockId = cfg->blocks.size();
            cfg->blocks.emplace_back(elseBlockId);

            cfg->blocks[blockId].succs.push_back(elseBlockId);
            cfg->blocks[elseBlockId].preds.push_back(blockId);

            ASTNode* blockAstPtr = ast->rhs;
            elseBlockIdPtr = elseBlockId;
            while(blockAstPtr) {
                blockAstPtr = step(cfg, elseBlockIdPtr, blockAstPtr);
            }
        }

        int joinBlockId = cfg->blocks.size();
        cfg->blocks.emplace_back(joinBlockId);

        if(cfg->blocks[thenBlockIdPtr].terminator.type != CFGTerminatorType::Return) {
            if(cfg->blocks[thenBlockIdPtr].terminator.type != CFGTerminatorType::None) {
                throw logic_error("Unexpected terminator for body of then block");
            }
            cfg->blocks[thenBlockIdPtr].succs.push_back(joinBlockId);
            cfg->blocks[joinBlockId].preds.push_back(thenBlockIdPtr);
            cfg->blocks[thenBlockIdPtr].terminator = CFGTerminator(CFGTerminatorType::Jump);
        }
        if(elseBlockIdPtr >= 0) {
            if(cfg->blocks[elseBlockIdPtr].terminator.type != CFGTerminatorType::Return) {
                if(cfg->blocks[elseBlockIdPtr].terminator.type != CFGTerminatorType::None) {
                    throw logic_error("Unexpected terminator for body of else block");
                }
                cfg->blocks[elseBlockIdPtr].succs.push_back(joinBlockId);
                cfg->blocks[joinBlockId].preds.push_back(elseBlockIdPtr);
                cfg->blocks[elseBlockIdPtr].terminator = CFGTerminator(CFGTerminatorType::Jump);
            }
        } else {
            cfg->blocks[blockId].succs.push_back(joinBlockId);
            cfg->blocks[joinBlockId].preds.push_back(blockId);
        }

        blockId = joinBlockId;
        return ast->next;
    } else if(ast->type == ASTNodeType::StmtWhile) { // while
        int condBlockId = cfg->blocks.size();
        cfg->blocks.emplace_back(condBlockId);
        CFGOperand cond = lowerExpr(cfg, condBlockId, ast->cond);
        cfg->blocks[blockId].terminator = CFGTerminator(CFGTerminatorType::Jump);
        cfg->blocks[blockId].succs.push_back(condBlockId);
        cfg->blocks[condBlockId].preds.push_back(blockId);
        cfg->blocks[condBlockId].terminator = CFGTerminator(CFGTerminatorType::Branch, cond);

        int bodyBlockId = cfg->blocks.size();
        cfg->blocks.emplace_back(bodyBlockId);

        cfg->blocks[condBlockId].succs.push_back(bodyBlockId);
        cfg->blocks[bodyBlockId].preds.push_back(condBlockId);
        
        ASTNode* blockAstPtr = ast->lhs;
        int bodyBlockIdPtr = bodyBlockId;
        while(blockAstPtr) {
            blockAstPtr = step(cfg, bodyBlockIdPtr, blockAstPtr);
        }
        
        if(cfg->blocks[bodyBlockIdPtr].terminator.type != CFGTerminatorType::Return) {
            if(cfg->blocks[bodyBlockIdPtr].terminator.type != CFGTerminatorType::None) {
                throw logic_error("Unexpected terminator for body of wihle");
            }
            cfg->blocks[bodyBlockIdPtr].succs.push_back(condBlockId);
            cfg->blocks[condBlockId].preds.push_back(bodyBlockIdPtr);
            cfg->blocks[bodyBlockIdPtr].terminator = CFGTerminator(CFGTerminatorType::Jump);
        }

        int joinBlockId = cfg->blocks.size();
        cfg->blocks.emplace_back(joinBlockId);
        cfg->blocks[joinBlockId].preds.push_back(condBlockId);
        cfg->blocks[condBlockId].succs.push_back(joinBlockId);

        blockId = joinBlockId;
        return ast->next;
        throw logic_error("Incomplete");
    } else if(ast->type == ASTNodeType::StmtReturn) {
        CFGOperand retVal = lowerExpr(cfg, blockId, ast->lhs);
        cfg->blocks[blockId].terminator = CFGTerminator(CFGTerminatorType::Return, retVal);
        return nullptr; // TODO - don't skip dead code after a return
    } if(ast->type == ASTNodeType::StmtInput) {
        throw invalid_argument("StmtInput must be the first statement in a program");
    } else {
        throw logic_error("Unexpected ASTNode type passed to step");
    }
}

CFGOperand lowerExpr(CFG* cfg, int& blockId, ASTNode* ast) {
    if(ast == nullptr) {
        throw logic_error("lowerExpr passed a nullptr");
    }
    if(!ast->isVal() && !ast->isOp()) {
        throw logic_error(string() + "Expected an expression at line: " + to_string(ast->line)); // Todo: better error message
    }
    if(ast->type == ASTNodeType::ValIdentifier) {
        int identifierReg = cfg->symbolTable.getReg(ast->strVal);
        if(identifierReg < 0) {
            throw invalid_argument(string("Used symbol \"") + ast->strVal + "\" before definition");
        }
        return CFGOperand(CFGOperandType::Reg, identifierReg);
    } else if(ast->type == ASTNodeType::ValInt) {
        return CFGOperand(CFGOperandType::Imm, ast->intVal);
    } else {
        CFGOperand lhs = lowerExpr(cfg, blockId, ast->lhs);
        CFGOperand rhs = lowerExpr(cfg, blockId, ast->rhs);
        int dest = cfg->symbolTable.getReg();
        CFGInstrType instrType;
        switch(ast->type) {
            case ASTNodeType::OpAdd : instrType = CFGInstrType::Add; break;
            case ASTNodeType::OpSub : instrType = CFGInstrType::Sub; break;
            case ASTNodeType::OpMul : instrType = CFGInstrType::Mul; break;
            case ASTNodeType::OpDiv : instrType = CFGInstrType::Div; break;
            case ASTNodeType::OpMod : instrType = CFGInstrType::Mod; break;
            case ASTNodeType::OpLT : instrType = CFGInstrType::LT; break;
            case ASTNodeType::OpGT : instrType = CFGInstrType::GT; break;
            case ASTNodeType::OpLTEQ : instrType = CFGInstrType::LTEQ; break;
            case ASTNodeType::OpGTEQ : instrType = CFGInstrType::GTEQ; break;
            case ASTNodeType::OpEQ : instrType = CFGInstrType::EQ; break;
            default: throw logic_error("Unexpected type of ast node - expected op");
        }
        cfg->blocks[blockId].addInstr(CFGInstr(instrType, dest, lhs, rhs, ast->line));
        return CFGOperand(CFGOperandType::Reg, dest);
    }
}