#pragma once

#include <string>
#include <vector>
#include <print>
#include <map>

#include "cfg.h"

using namespace std;

class BitSet {
    size_t numEntries;
    vector<uint64_t> bitmap;

public:
    BitSet() : numEntries(-1) {}
    BitSet(size_t size) {
        numEntries = size;
        bitmap = vector<uint64_t>(size != 0 ? (size - 1) / 64 + 1 : 0);
    }

    size_t size() const {
        return numEntries;
    }

    bool contains(size_t i) const {
        if(i >= numEntries) {
            throw logic_error("Attempt to access BitSet out of bounds");
        }
        size_t vectorIdx = i / 64;
        size_t bitmapIndex = i % 64;
        return (bitmap[vectorIdx] & (uint64_t{1} << bitmapIndex)) != 0;
    }

    bool add(size_t i) {
        if(i >= numEntries) {
            throw logic_error("Attempt to access BitSet out of bounds");
        }
        size_t vectorIdx = i / 64;
        size_t bitmapIndex = i % 64;
        uint64_t mask = uint64_t{1} << bitmapIndex;
        bool changed = (bitmap[vectorIdx] & mask) == 0;
        bitmap[vectorIdx] |= mask;
        return changed;
    }
    
    bool add(const BitSet& bitSet) {
        if(bitSet.numEntries != numEntries) {
            throw logic_error("Attempt to perform union of BitSet with different sizes");
        }
        bool changed = false;
        for(size_t i = 0; i < bitmap.size(); i++) {
            uint64_t before = bitmap[i];
            bitmap[i] |= bitSet.bitmap[i];
            changed |= before != bitmap[i];
        }
        return changed;
    }

    bool remove(size_t i) {
        if(i >= numEntries) {
            throw logic_error("Attempt to access BitSet out of bounds");
        }
        size_t vectorIdx = i / 64;
        size_t bitmapIndex = i % 64;
        uint64_t mask = uint64_t{1} << bitmapIndex;
        bool changed = (bitmap[vectorIdx] & mask) != 0;
        bitmap[vectorIdx] &= ~(mask);
        return changed;
    }

    bool remove(const BitSet& bitSet) {
        if(bitSet.numEntries != numEntries) {
            throw logic_error("Attempt to perform difference of BitSet with different sizes");
        }
        bool changed = false;
        for(size_t i = 0; i < bitmap.size(); i++) {
            uint64_t before = bitmap[i];
            bitmap[i] &= ~(bitSet.bitmap[i]);
            changed |= before != bitmap[i];
        }
        return changed;
    }
    
    bool intersect(const BitSet& other) {
        if(other.numEntries != numEntries) {
            throw logic_error("Attempt to perform intersection of BitSet with different sizes");
        }
        bool changed = false;
        for(size_t i = 0; i < bitmap.size(); i++) {
            uint64_t before = bitmap[i];
            bitmap[i] &= other.bitmap[i];
            changed |= before != bitmap[i];
        }
        return changed;
    }

    void setAll() {
        for(size_t i = 0; i < bitmap.size(); i++) {
            bitmap[i] = ~uint64_t{0};
        }
        size_t tail = numEntries % 64;
        if(tail != 0) {
            bitmap.back() &= (uint64_t{1} << tail) - 1;
        }
    }

    bool operator==(const BitSet&) const = default;

    vector<int> toVector() const {
        vector<int> res;
        for(size_t w = 0; w < bitmap.size(); w++) {
            uint64_t word = bitmap[w];
            while(word != 0) {
                int i = countr_zero(word);
                res.push_back(w*64+i);
                word &= ~(uint64_t{1} << i);
            }
        }
        return res;
    }

    string toString(string prefix="") const {
        string res = "{";
        for(size_t i = 0; i < numEntries; i++) {
            if(contains(i)) {
                res += string() + (res.size() != 1 ? ", " : "") + prefix + to_string(i);
            }
        }
        res += "}";
        return res;
    }
};
