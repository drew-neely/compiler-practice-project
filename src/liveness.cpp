#include <string>
#include <fstream>
#include <vector>
#include <print>
#include <map>

#include "liveness.h"
#include "cfg.h"

Liveness computeLiveness(CFG* cfg) {
    Liveness liveness(cfg->blocks.size(), cfg->symbolTable.regCount);
    
    // Compute def and use
    for(size_t b = 0; b < cfg->blocks.size(); b++) {
        vector<CFGInstr>& instrs = cfg->blocks[b].instrs;
        for(size_t i = 0; i < instrs.size(); i++) {
            vector<int> usedRegs = instrs[i].getUsedRegs();
            for(size_t regIdx = 0; regIdx < usedRegs.size(); regIdx++) {
                if(!liveness.def[b].contains(usedRegs[regIdx])) {
                    liveness.use[b].add(usedRegs[regIdx]);
                }
            }
            liveness.def[b].add(instrs[i].dest);
        }
        CFGTerminator& term = cfg->blocks[b].terminator;
        vector<int> usedRegs = term.getUsedRegs();
        for(size_t regIdx = 0; regIdx < usedRegs.size(); regIdx++) {
            if(!liveness.def[b].contains(usedRegs[regIdx])) {
                liveness.use[b].add(usedRegs[regIdx]);
            }
        }
    }

    // compute liveIn and liveOut
    for(size_t b = 0; b < cfg->blocks.size(); b++) {
        liveness.liveIn[b] = liveness.use[b];
    }
    bool changed = true;
    while(changed) {
        changed = false;
        for(size_t b = cfg->blocks.size(); b-- > 0;) {
            bool blockChanged = false;
            for(size_t i = 0; i < cfg->blocks[b].succs.size(); i++) {
                int succ = cfg->blocks[b].succs[i];
                bool c = liveness.liveOut[b].add(liveness.liveIn[succ]);
                changed |= c;
                blockChanged |= c;
            }
            if(blockChanged) {
                // liveIn = use + (liveOut - def)
                RegSet tmp1 = liveness.use[b]; // copy
                RegSet tmp2 = liveness.liveOut[b]; // copy
                tmp2.remove(liveness.def[b]);
                tmp1.add(tmp2);
                liveness.liveIn[b] = tmp1;
            }
        }
    }

    return liveness;
}