//
// Created by Taylor Whatley on 2018-11-30.
//

#ifndef NEM_RANGES_H
#define NEM_RANGES_H

#include <vector>

namespace Nem {
    class Ranges {
        void mergeRanges(int index);

    public:
        class SubRange {
        public:
            int start, count;
            inline int end();

            SubRange(int start, int count);
        };

        std::vector<SubRange> ranges;

        void add(int i);
        void remove(int i);

        bool get(int i);

        void fill(int start, int count);
    };
}

#endif //NEM_RANGES_H
