#include <vector>
#include <print>

#include "dominance.h"
#include "cfg.h"
#include "bitset.h"

Dominance computeDominance(CFG* cfg) {
    size_t numBlocks = cfg->blocks.size();
    if(numBlocks == 0) {
        throw invalid_argument("cfg->blocks cannot have size 0");
    }

    // build list of non-orphan blocks
    BitSet liveBlocks(numBlocks);
    vector<int> blocksToIter = {0};
    int currentBlock;
    while(blocksToIter.size() != 0) {
        currentBlock = blocksToIter.back();
        blocksToIter.pop_back();
        if(!liveBlocks.contains(currentBlock)) {
            liveBlocks.add(currentBlock);
            for(size_t s = 0; s < cfg->blocks[currentBlock].succs.size(); s++) {
                int succ = cfg->blocks[currentBlock].succs[s];
                blocksToIter.push_back(succ);
            }
        }
    }

    // build lists of preds which are not orphans
    vector<vector<int>> livePreds(numBlocks);
    for(size_t b = 0; b < numBlocks; b++) {
        for(int p : cfg->blocks[b].preds) {
            if(liveBlocks.contains(p)) {
                livePreds[b].push_back(p);
            }
        }
    }

    // init dominance obj and dom (dominance sets)
    Dominance dominance(numBlocks);
    dominance.dom[0].add(0);
    for(size_t b = 1; b < numBlocks; b++) {
        dominance.dom[b].setAll();
    }

    // compute dominance sets
    bool change = 1;
    while(change) {
        change = 0;
        for(size_t b = 1; b < numBlocks; b++) {
            BitSet dom = (!livePreds[b].empty()) ? dominance.dom[livePreds[b][0]] : BitSet(numBlocks);
            for(size_t p = 1; p < livePreds[b].size(); p++) {
                int pred = livePreds[b][p];
                dom.intersect(dominance.dom[pred]);
            }
            dom.add(b);
            if(dominance.dom[b] != dom) {
                change = 1;
            }
            dominance.dom[b] = dom;
        }
    }

    // compute immediate dominator
    for(size_t b = 0; b < numBlocks; b++) {
        BitSet strictDom = dominance.dom[b];
        strictDom.remove(b);
        for (size_t d = 0; d < numBlocks; d++)
        {
            if(strictDom.contains(d)) {
                // d dominates b
                if(dominance.dom[d] == strictDom) {
                    dominance.iDom[b] = d;
                    break;
                }
            }
        }
    }

    // compute dominance frontiers
    for(size_t b = 0; b < numBlocks; b++) {
        for(size_t n = 0; n < numBlocks; n++) {
            // n is in the dominance frontier of b if
            //     b does not strictly dominate n
            //     b dominates at least 1 pred of n
            if(!dominance.dom[n].contains(b) || n == b) {
                bool dominatesPred = false;
                for(size_t p = 0; p < livePreds[n].size(); p++) {
                    int pred = livePreds[n][p];
                    if(dominance.dom[pred].contains(b)) {
                        dominatesPred = true;
                        break;
                    }
                }
                if(dominatesPred) {
                    dominance.domF[b].add(n);
                }
            }
        }
    }

    return dominance;
}