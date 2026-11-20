#include <vector>
#include <print>
#include <map>

#include "ssa.h"
#include "cfg.h"
#include "liveness.h"
#include "dominance.h"

using namespace std;

// Check all instructions in block to see if reg is written to
// ignores phis
bool containsWriteToReg(int reg, const CFGBlock& block) {
    size_t numInstr = block.instrs.size();
    for(size_t i = 0; i < numInstr; i++) {
        const CFGInstr& instr = block.instrs[i];
        if(instr.dest == reg) {
            return true;
        }
    }
    return false;
}

void placePhis(CFG* cfg, const Liveness& liveness, const Dominance& dominance) {
    size_t numBlocks = cfg->blocks.size();
    size_t numRegs = cfg->symbolTable.regCount;
    for(size_t reg = 0; reg < numRegs; reg++) {
        BitSet phiAddedToBlock(numBlocks);
        BitSet allWriters(numBlocks);
        vector<int> writers;
        // check if block 0 is a writer
        if(reg < cfg->symbolTable.parameters.size() || containsWriteToReg(reg, cfg->blocks[0])) {
            writers.push_back(0);
            allWriters.add(0);
        }
        // check for all other writers
        for(size_t b = 1; b < numBlocks; b++) {
            if(containsWriteToReg(reg, cfg->blocks[b])) {
                writers.push_back(b);
                allWriters.add(b);
            }
        }
        while (!writers.empty()) {
            int writerBlock = writers.back();
            writers.pop_back();

            vector<int> domF = dominance.domF[writerBlock].toVector();
            for(size_t i = 0; i < domF.size(); i++) {
                int frontier = domF[i];
                if(liveness.liveIn[frontier].contains(reg) && !phiAddedToBlock.contains(frontier)) {
                    CFGPhi phi(reg);
                    for(size_t p = 0; p < cfg->blocks[frontier].preds.size(); p++) {
                        int pred = cfg->blocks[frontier].preds[p];
                        phi.incoming[pred] = CFGOperand(CFGOperandType::Reg, reg);
                    }
                    cfg->blocks[frontier].phis.push_back(phi);
                    phiAddedToBlock.add(frontier);
                    if(!allWriters.contains(frontier)) {
                        writers.push_back(frontier);
                        allWriters.add(frontier);
                    }
                }
            }
        }
    }
}

// returns if the dest was renamed
bool renamePhi(CFGPhi& phi, CFGSymbolTable& symbolTable, vector<vector<int>>& regStacks) {
    int dest = phi.dest;
    if(!regStacks[dest].empty()) {
        int newDest = symbolTable.getReg();
        phi.dest = newDest;
        regStacks[dest].push_back(newDest);
        return true;
    } else {
        regStacks[dest].push_back(dest);
        return false;
    }
}

// returns if the dest was renamed
bool renameInstr(CFGInstr& instr, CFGSymbolTable& symbolTable, vector<vector<int>>& regStacks) {
    // rename lhs op
    if(instr.lhs.type == CFGOperandType::Reg && regStacks[instr.lhs.value].size() > 1) {
        instr.lhs.value = regStacks[instr.lhs.value].back();
    }
    
    // rename rhs op
    if(instr.rhs.type == CFGOperandType::Reg && regStacks[instr.rhs.value].size() > 1) {
        instr.rhs.value = regStacks[instr.rhs.value].back();
    }

    // rename dest
    if(regStacks[instr.dest].empty()) {
        regStacks[instr.dest].push_back(instr.dest);
        return false;
    } else {
        int dest = instr.dest;
        int newDest = symbolTable.getReg();
        instr.dest = newDest;
        regStacks[dest].push_back(newDest);
        return true;
    }
}

void renameTerm(CFGTerminator& term, vector<vector<int>>& regStacks) {
    switch(term.type) {
        case CFGTerminatorType::Branch :  
        case CFGTerminatorType::Return : 
            if(term.value.type == CFGOperandType::Reg && regStacks[term.value.value].size() > 1) {
                term.value.value = regStacks[term.value.value].back();
            }
            break;
        case CFGTerminatorType::Jump   : 
        case CFGTerminatorType::None   : 
            break;
    }
}

void rename(CFG* cfg, const Dominance& dominance) {
    size_t numBlocks = cfg->blocks.size();
    size_t numRegs = cfg->symbolTable.regCount;

    DominatorTree domTree = dominance.toTree();
    
    // regStack[<reg>] = stack of renamed values
    vector<vector<int>> regStacks(numRegs);
    // renamedCount[<block>][<reg>] = number of new names var recieved while renaming block
    vector<vector<int>> renamedCount(numBlocks, vector<int>(numRegs, 0));

    for(size_t p = 0; p < cfg->symbolTable.parameters.size(); p++) {
        regStacks[p].push_back(p);
    }
    
    vector<int> blocksToIter = {0};
    int currentBlock = -1;
    BitSet blocksVisited(numBlocks);
    
    while(!blocksToIter.empty()) {
        currentBlock = blocksToIter.back();
        blocksToIter.pop_back();
        if(!blocksVisited.contains(currentBlock)) {
            // visiting this block on the way down the tree - do renaming
            for(size_t p = 0; p < cfg->blocks[currentBlock].phis.size(); p++) {
                CFGPhi& phi = cfg->blocks[currentBlock].phis[p];
                int origDest = phi.dest;
                bool renamedDest = renamePhi(phi, cfg->symbolTable, regStacks);
                if(renamedDest) {
                    renamedCount[currentBlock][origDest]++;
                }
            }
            for(size_t i = 0; i < cfg->blocks[currentBlock].instrs.size(); i++) {
                CFGInstr& instr = cfg->blocks[currentBlock].instrs[i];
                int origDest = instr.dest;
                bool renamedDest = renameInstr(instr, cfg->symbolTable, regStacks);
                if(renamedDest) {
                    renamedCount[currentBlock][origDest]++;
                }
            }
            renameTerm(cfg->blocks[currentBlock].terminator, regStacks);

            // rename incoming regs for succesor blocks
            for(size_t s = 0; s < cfg->blocks[currentBlock].succs.size(); s++) {
                int succ = cfg->blocks[currentBlock].succs[s];
                for(size_t p = 0; p < cfg->blocks[succ].phis.size(); p++) {
                    CFGPhi& phi = cfg->blocks[succ].phis[p];
                    CFGOperand& op = phi.incoming[currentBlock];
                    if(op.type == CFGOperandType::Reg) {
                        if(!regStacks[op.value].empty()) {
                            op.value = regStacks[op.value].back();
                        }
                    }
                }
            }
            
            // record next blocks to visit
            blocksVisited.add(currentBlock);
            blocksToIter.push_back(currentBlock);
            for(size_t b = 0; b < domTree.children[currentBlock].size(); b++) {
                blocksToIter.push_back(domTree.children[currentBlock][b]);
            }
        } else {
            // visiting this block on the way up the tree - pop regs
            for(size_t r = 0; r < numRegs; r++) {
                if(renamedCount[currentBlock][r] > 0) {
                    regStacks[r].resize(regStacks[r].size() - renamedCount[currentBlock][r]);
                }
            }
        }
    }
}

void toSSA(CFG* cfg) {
    Liveness liveness = computeLiveness(cfg);
    Dominance dominance = computeDominance(cfg);

    placePhis(cfg, liveness, dominance);
    rename(cfg, dominance);
}

void verifySSA(CFG* cfg) {
    // TODO : Unimplemented
}