#include "cache_timing.h"

#include <stdio.h>
#include <stdlib.h>

#include "util.h"

const size_t REPEATS = 100000;

int main() {
    uint64_t sum_miss = 0;
    uint64_t sum_hit = 0;

    for (size_t i = 0; i < REPEATS; i++) {
        page_t *page = malloc(sizeof(page_t));
        flush_cache_line(page);
        uint64_t time_one = time_read(page); // miss time
        uint64_t time_two = time_read(page); // hit time
        if (time_two <= time_one) {
            sum_miss += time_one;
            sum_hit += time_two;
        }
    }

    printf("average miss = %" PRIu64 "\n", sum_miss / REPEATS);
    printf("average hit  = %" PRIu64 "\n", sum_hit / REPEATS);
}
