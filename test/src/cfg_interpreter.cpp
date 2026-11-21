#include <string>
#include <vector>
#include <print>

#include "cfg_interpreter.h"
#include "cfg.h"
#include "liveness.h"

void scrambleDeadRegs(vector<int>& regs, BitSet& liveIn, BitSet& skipRegs) {
    for(size_t i = 0; i < regs.size(); i++) {
        if(!liveIn.contains(i) && !skipRegs.contains(i)) {
            regs[i] = rand();
        }
    }
}

int interpretCFG(CFG* cfg, vector<int>& parameters, bool testLiveness) {

    // compute liveness
    Liveness liveness;
    if(testLiveness) {
        liveness = computeLiveness(cfg);
    }

    // Init registers
    if(cfg->symbolTable.parameters.size() != parameters.size()) {
        throw invalid_argument("Unexpected number of parameters passed");
    } else if(cfg->symbolTable.parameters.size() > cfg->symbolTable.regCount) {
        throw invalid_argument("CFG has more parameters than symbols allocated");
    }
    vector<int> regs(cfg->symbolTable.regCount);
    for(size_t i = 0; i < parameters.size(); i++) {
        regs[i] = parameters[i];
    }

    // pointer to current instr
    int prevBlockPtr = -1;
    int blockPtr = 0;
    int instrPtr = 0;
    bool returnReached = false;
    int returnValue;
    
    // iterate cfg
    while(!returnReached) {
        if(instrPtr == 0) {
            for(size_t p = 0; p < cfg->blocks[blockPtr].phis.size(); p++) {
                CFGPhi& phi = cfg->blocks[blockPtr].phis[p];
                if(prevBlockPtr == -1 || !phi.incoming.contains(prevBlockPtr)) {
                    throw invalid_argument("phi does not contain entry for previous block");
                }
                CFGOperand res = phi.incoming[prevBlockPtr];
                switch(res.type) {
                    case CFGOperandType::Imm  : regs[phi.dest] = res.value; break;
                    case CFGOperandType::Reg  : regs[phi.dest] = regs[res.value]; break;
                    case CFGOperandType::None : throw "Unexpected operand in phi assignment";
                }
            }
        }
        if(testLiveness && instrPtr == 0) {
            BitSet skipRegs(cfg->symbolTable.regCount);
            for(size_t p = 0; p < cfg->blocks[blockPtr].phis.size(); p++) {
                skipRegs.add(cfg->blocks[blockPtr].phis[p].dest);
            }
            scrambleDeadRegs(regs, liveness.liveIn[blockPtr], skipRegs);
        }
        if(instrPtr >= (int)cfg->blocks[blockPtr].instrs.size()) {
            // execute a terminal
            CFGBlock& block = cfg->blocks[blockPtr];
            CFGTerminator& term = block.terminator;
            switch(term.type) {
                case CFGTerminatorType::Jump : 
                    if(block.succs.size() == 1) {
                        prevBlockPtr = blockPtr;
                        blockPtr = block.succs[0];
                        instrPtr = 0;
                    } else {
                        throw logic_error("Unexpected number of succesors for jump");
                    }
                    break;
                case CFGTerminatorType::Branch : 
                    if(block.succs.size() == 2) {
                        int cond;
                        if(term.value.type == CFGOperandType::Reg) {
                            cond = regs[term.value.value];
                        } else if(term.value.type == CFGOperandType::Imm) {
                            cond = term.value.value;
                        } else {
                            throw logic_error("Unexpected terminator value type for branch");
                        }
                        prevBlockPtr = blockPtr;
                        blockPtr = cond ? block.succs[0] : block.succs[1];
                        instrPtr = 0;
                    } else {
                        throw logic_error("Unexpected number of succesors for branch");
                    }
                    break;
                case CFGTerminatorType::Return : 
                    if(block.succs.size() == 0) {
                        if(term.value.type == CFGOperandType::Reg) {
                            returnValue = regs[term.value.value];
                        } else if(term.value.type == CFGOperandType::Imm) {
                            returnValue = term.value.value;
                        } else {
                            throw logic_error("Unexpected terminator value type for branch");
                        }
                        returnReached = true;
                    }
                    else {
                        throw logic_error("Unexpected number of succesors for return");
                    }
                    break;
                default: 
                    throw logic_error("Unexpected terminator type");
            }
        } else { 
            // execute an instr
            CFGInstr instr = cfg->blocks[blockPtr].instrs[instrPtr];
            bool hasLhs;
            int lhs;
            bool hasRhs;
            int rhs;
            if(instr.lhs.type == CFGOperandType::Reg) {
                lhs = regs[instr.lhs.value];
                hasLhs = true;
            } else if(instr.lhs.type == CFGOperandType::Imm) {
                lhs = instr.lhs.value;
                hasLhs = true;
            } else {
                hasLhs = false;
            }
            if(instr.rhs.type == CFGOperandType::Reg) {
                rhs = regs[instr.rhs.value];
                hasRhs = true;
            } else if(instr.rhs.type == CFGOperandType::Imm) {
                rhs = instr.rhs.value;
                hasRhs = true;
            } else {
                hasRhs = false;
            }
            if(instr.type == CFGInstrType::None) {
                throw logic_error("Found instruction with type NONE");
            }

            int expectedNumOperands;
            switch(instr.type) {
                case CFGInstrType::Copy : expectedNumOperands = 1; break;
                case CFGInstrType::Add  : 
                case CFGInstrType::Sub  : 
                case CFGInstrType::Mul  : 
                case CFGInstrType::Div  : 
                case CFGInstrType::Mod  : 
                case CFGInstrType::LT   : 
                case CFGInstrType::GT   : 
                case CFGInstrType::LTEQ : 
                case CFGInstrType::GTEQ : 
                case CFGInstrType::EQ   : expectedNumOperands = 2; break;
                default                 : expectedNumOperands = 0; break;
            }
            if(expectedNumOperands != 1 && expectedNumOperands != 2) {
                throw logic_error("No instruction matches expected number of operands");
            } else if(expectedNumOperands == 1 && (!hasLhs || hasRhs)) {
                throw logic_error("Number of operands unexpected for instruction");
            } else if(expectedNumOperands == 2 && (!hasLhs || !hasRhs)) {
                throw logic_error("Number of operands unexpected for instruction");
            }

            int res;
            switch(instr.type) {
                case CFGInstrType::Copy : res = lhs       ; break;
                case CFGInstrType::Add  : res = lhs + rhs ; break;
                case CFGInstrType::Sub  : res = lhs - rhs ; break;
                case CFGInstrType::Mul  : res = lhs * rhs ; break;
                case CFGInstrType::Div  : res = lhs / rhs ; break;
                case CFGInstrType::Mod  : res = lhs % rhs ; break;
                case CFGInstrType::LT   : res = lhs < rhs ; break;
                case CFGInstrType::GT   : res = lhs > rhs ; break;
                case CFGInstrType::LTEQ : res = lhs <= rhs; break;
                case CFGInstrType::GTEQ : res = lhs >= rhs; break;
                case CFGInstrType::EQ   : res = lhs == rhs; break;
                default: throw logic_error("Unexpected instr type");
            }

            regs[instr.dest] = res;
            instrPtr++;
        }
    }

    return returnValue;
}