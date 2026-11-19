#pragma once

#include <string>
#include <vector>
#include <print>
#include <map>

#include "cfg.h"
#include "bitset.h"

using namespace std;

struct Liveness {
    size_t numBlocks;
    size_t numRegs;
    vector<BitSet> use;
    vector<BitSet> def;
    vector<BitSet> liveIn;
    vector<BitSet> liveOut;

    Liveness() {}
    Liveness(size_t numBlocks, size_t numRegs) : numBlocks(numBlocks), numRegs(numRegs) {
        use = vector<BitSet>(numBlocks, BitSet(numRegs));
        def = vector<BitSet>(numBlocks, BitSet(numRegs));
        liveIn = vector<BitSet>(numBlocks, BitSet(numRegs));
        liveOut = vector<BitSet>(numBlocks, BitSet(numRegs));
    }

    string toString() {
        string res = "";
        for(size_t b = 0; b < numBlocks; b++) {
            res += string("B") + to_string(b) + ":\n";
            res += string("\t") + "use     : " + use[b].toString() + "\n";
            res += string("\t") + "def     : " + def[b].toString() + "\n";
            res += string("\t") + "liveIn  : " + liveIn[b].toString() + "\n";
            res += string("\t") + "liveOut : " + liveOut[b].toString() + "\n";
        }
        return res;
    }
};

Liveness computeLiveness(CFG* cfg);