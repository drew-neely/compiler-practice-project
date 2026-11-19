#pragma once

#include <string>
#include <vector>
#include <print>

#include "cfg.h"
#include "bitset.h"

using namespace std;

struct Dominance {
    size_t numBlocks = 0;
    vector<BitSet> dom; // set of blocks dominating blocks[i]
    vector<int>    iDom; // the immediate dominator of blocks[i]
    vector<BitSet> domF; // the dominance frontier of blocks[i]

    Dominance() {}
    Dominance(size_t numBlocks) : numBlocks(numBlocks) {
        dom = vector<BitSet>(numBlocks, BitSet(numBlocks));
        iDom = vector<int>(numBlocks, -1);
        domF = vector<BitSet>(numBlocks, BitSet(numBlocks));
    }

    string toString() {
        string res = "";
        for(size_t b = 0; b < numBlocks; b++) {
            res += string("B") + to_string(b) + ":\n";
            res += string("\t") + "dominators    : " + dom[b].toString("B") + "\n";
            res += string("\t") + "immediate dom : " + to_string(iDom[b]) + "\n";
            res += string("\t") + "dom frontier  : " + domF[b].toString("B") + "\n";
        }
        return res;
    }
};

Dominance computeDominance(CFG* cfg);