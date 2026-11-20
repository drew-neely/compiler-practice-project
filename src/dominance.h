#pragma once

#include <string>
#include <vector>
#include <print>

#include "cfg.h"
#include "bitset.h"

using namespace std;

struct Dominance;

struct DominatorTree {
    vector<vector<int>> children;
    DominatorTree(const Dominance& dominance);

    string toString() {
        string res = "";
        for(size_t b = 0; b < children.size(); b++) {
            if(b != 0) {
                res += "\n";
            }
            res += "B" + to_string(b) + ": {" ;
            for(size_t c = 0; c < children[b].size(); c++) {
                int child = children[b][c];
                if(c != 0) {
                    res += ", ";
                }
                res += "B" + to_string(child);
            }
            res += "}";
        }
        return res;
    }
};

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

    DominatorTree toTree() const {
        return DominatorTree(*this);
    }

    string toString() const {
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