#pragma once

#include <string>
#include <vector>
#include <print>
#include <map>

#include "cfg.h"

using namespace std;

class RegSet {
    size_t numRegs;
    vector<uint64_t> bitmap;

public:
    RegSet(size_t size) {
        numRegs = size;
        bitmap = vector<uint64_t>(size != 0 ? (size - 1) / 64 + 1 : 0);
    }

    size_t size() const {
        return numRegs;
    }

    bool contains(size_t i) const {
        if(i >= numRegs) {
            throw logic_error("Attempt to access RegSet out of bounds");
        }
        size_t vectorIdx = i / 64;
        size_t bitmapIndex = i % 64;
        return (bitmap[vectorIdx] & (uint64_t{1} << bitmapIndex)) != 0;
    }

    bool add(size_t i) {
        if(i >= numRegs) {
            throw logic_error("Attempt to access RegSet out of bounds");
        }
        size_t vectorIdx = i / 64;
        size_t bitmapIndex = i % 64;
        uint64_t mask = uint64_t{1} << bitmapIndex;
        bool changed = (bitmap[vectorIdx] & mask) == 0;
        bitmap[vectorIdx] |= mask;
        return changed;
    }
    
    bool add(const RegSet& regSet) {
        if(regSet.numRegs != numRegs) {
            throw logic_error("Attempt to perform union of RegSet with different sizes");
        }
        bool changed = false;
        for(size_t i = 0; i < bitmap.size(); i++) {
            uint64_t before = bitmap[i];
            bitmap[i] |= regSet.bitmap[i];
            changed |= before != bitmap[i];
        }
        return changed;
    }

    bool remove(size_t i) {
        if(i >= numRegs) {
            throw logic_error("Attempt to access RegSet out of bounds");
        }
        size_t vectorIdx = i / 64;
        size_t bitmapIndex = i % 64;
        uint64_t mask = uint64_t{1} << bitmapIndex;
        bool changed = (bitmap[vectorIdx] & mask) != 0;
        bitmap[vectorIdx] &= ~(mask);
        return changed;
    }

    bool remove(const RegSet& regSet) {
        if(regSet.numRegs != numRegs) {
            throw logic_error("Attempt to perform difference of RegSet with different sizes");
        }
        bool changed = false;
        for(size_t i = 0; i < bitmap.size(); i++) {
            uint64_t before = bitmap[i];
            bitmap[i] &= ~(regSet.bitmap[i]);
            changed |= before != bitmap[i];
        }
        return changed;
    }

    bool operator==(const RegSet&) const = default;

    string toString() {
        string res = "{";
        for(size_t i = 0; i < numRegs; i++) {
            if(contains(i)) {
                res += string() + (res.size() != 1 ? ", " : "") + "r" + to_string(i);
            }
        }
        res += "}";
        return res;
    }
};

struct Liveness {
    size_t numBlocks;
    size_t numRegs;
    vector<RegSet> use;
    vector<RegSet> def;
    vector<RegSet> liveIn;
    vector<RegSet> liveOut;

    Liveness() {}
    Liveness(size_t numBlocks, size_t numRegs) : numBlocks(numBlocks), numRegs(numRegs) {
        use = vector<RegSet>(numBlocks, RegSet(numRegs));
        def = vector<RegSet>(numBlocks, RegSet(numRegs));
        liveIn = vector<RegSet>(numBlocks, RegSet(numRegs));
        liveOut = vector<RegSet>(numBlocks, RegSet(numRegs));
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