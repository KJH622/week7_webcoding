#include "linear_search.h"
#include <stddef.h>

Player *linear_search(Player *table, int size, int target_id) {
    for (int i = 0; i < size; i++) {
        if (table[i].id == target_id)
            return &table[i];
    }
    return NULL;
}

int linear_range(Player *table, int size, int lo, int hi) {
    int count = 0;
    for (int i = 0; i < size; i++) {
        if (table[i].id >= lo && table[i].id <= hi)
            count++;
    }
    return count;
}
