//
// Created by Taylor Whatley on 2018-11-30.
//

#include "Ranges.h"

#include <iostream>

namespace Nem {
    int Ranges::SubRange::end() {
        return start + count - 1;
    }

    Ranges::SubRange::SubRange(int start, int count) : start(start), count(count) { }

    void Ranges::mergeRanges(int index) {
        SubRange& range = ranges[index];
        for (int a = 0; a < ranges.size(); a++) {
            if (range.start - 1 == ranges[a].end()) {
                ranges[a].count += range.count;
                ranges.erase(ranges.begin() + index);
                break;
            }
        }
        for (int a = 0; a < ranges.size(); a++) {
            if (range.end() + 1 == ranges[a].start) {
                range.count += ranges[a].count;
                ranges.erase(ranges.begin() + a);
                break;
            }
        }
    }

    void Ranges::add(int i) {
        bool foundRange = false;

        for (int a = 0; a < ranges.size(); a++) {
            if (ranges[a].start - 1 == i) {
                ranges[a].start--;
                ranges[a].count++;
                mergeRanges(a);
                foundRange = true;
                break;
            }
            if (ranges[a].end() + 1 == i) {
                ranges[a].count++;
                mergeRanges(a);
                foundRange = true;
                break;
            }
            if (i >= ranges[a].start && i <= ranges[a].end()) {
                foundRange = true;
                break;
            }
        }

        if (!foundRange) ranges.emplace_back(i, 1);
    }

    void Ranges::remove(int i) {
        for (int a = 0; a < ranges.size(); a++) {
            if (ranges[a].start == i) {
                ranges[a].count--;
                ranges[a].start++;
                if (ranges[a].count <= 0) ranges.erase(ranges.begin() + a);
                break;
            }
            if (ranges[a].end() == i) {
                ranges[a].count--;
                if (ranges[a].count <= 0) ranges.erase(ranges.begin() + a);
                break;
            }
            if (ranges[a].start < i && ranges[a].end() > i) {
                int temp = ranges[a].count;
                ranges[a].count = i - ranges[a].start;
                ranges.emplace_back(i + 1, temp - ranges[a].count + 1);
                break;
            }
        }
    }

    bool Ranges::get(int i) {
        for (SubRange &range : ranges) {
            if (range.start <= i && range.end() >= i) return true;
        }
        return false;
    }

    void Ranges::fill(int start, int count) {
        ranges.clear();
        ranges.emplace_back(start, count);
    }
}