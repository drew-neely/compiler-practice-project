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

void toSSA(CFG* cfg) {
    Liveness liveness = computeLiveness(cfg);
    Dominance dominance = computeDominance(cfg);

    size_t numRegs = cfg->symbolTable.regCount;
    size_t numBlocks = cfg->blocks.size();

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

    // TODO : Renaming unimplemented
}

void verifySSA(CFG* cfg) {
    // TODO : Unimplemented
}