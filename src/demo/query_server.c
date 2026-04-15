#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../common/dataset_io.h"
#include "../common/player.h"
#include "../linear/linear_search.h"
#include "../btree/btree.h"
#include "../bplus_tree/bplus_tree.h"

#define SEARCH_ITERS 5

long long g_op_count = 0;

static long long now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

static void print_search_result(Player *players, int count, BTree *btree, BPTree *bptree, int target_id) {
    Player *linear_found;
    Player *btree_found;
    Player *bptree_found;
    long long start;
    double linear_us;
    double btree_us;
    double bptree_us;

    start = now_ns();
    linear_found = linear_search(players, count, target_id);
    linear_us = (double)(now_ns() - start) / 1000.0;

    start = now_ns();
    btree_found = (Player *)btree_search(btree, target_id);
    btree_us = (double)(now_ns() - start) / 1000.0;

    start = now_ns();
    bptree_found = (Player *)bptree_search(bptree, target_id);
    bptree_us = (double)(now_ns() - start) / 1000.0;

    printf("{");
    printf("\"ok\":true,");
    printf("\"dataset_size\":%d,", count);
    printf("\"target_id\":%d,", target_id);
    printf("\"found\":%s,", linear_found ? "true" : "false");
    printf("\"timings\":{");
    printf("\"linear_us\":%.3f,", linear_us);
    printf("\"btree_us\":%.3f,", btree_us);
    printf("\"bptree_us\":%.3f", bptree_us);
    printf("},");

    if (linear_found && btree_found && bptree_found) {
        printf("\"player\":{");
        printf("\"id\":%d,", linear_found->id);
        printf("\"name\":\"%s\",", linear_found->name);
        printf("\"score\":%d,", linear_found->score);
        printf("\"tier\":\"%s\"", linear_found->tier);
        printf("}");
    } else {
        printf("\"player\":null");
    }

    printf("}\n");
    fflush(stdout);
}

static void print_range_result(Player *players, int count, BTree *btree, BPTree *bptree, int lo, int hi) {
    int linear_count = 0;
    int btree_count = 0;
    int bptree_count = 0;
    long long linear_total = 0;
    long long btree_total = 0;
    long long bptree_total = 0;
    long long linear_ops_total = 0;
    long long btree_ops_total = 0;
    long long bptree_ops_total = 0;
    int i;

    for (i = 0; i < SEARCH_ITERS; ++i) {
        long long start;

        g_op_count = 0;
        start = now_ns();
        linear_count = linear_range(players, count, lo, hi);
        linear_total += now_ns() - start;
        linear_ops_total += g_op_count;

        g_op_count = 0;
        start = now_ns();
        btree_count = btree_range(btree, lo, hi);
        btree_total += now_ns() - start;
        btree_ops_total += g_op_count;

        g_op_count = 0;
        start = now_ns();
        bptree_count = bptree_range(bptree, lo, hi);
        bptree_total += now_ns() - start;
        bptree_ops_total += g_op_count;
    }

    printf("{");
    printf("\"ok\":true,");
    printf("\"size\":%d,", count);
    printf("\"lo\":%d,", lo);
    printf("\"hi\":%d,", hi);
    printf("\"range_count\":%d,", linear_count);
    printf("\"linear_time\":%.3f,", (double)linear_total / SEARCH_ITERS / 1000.0);
    printf("\"linear_ops\":%lld,", linear_ops_total / SEARCH_ITERS);
    printf("\"btree_time\":%.3f,", (double)btree_total / SEARCH_ITERS / 1000.0);
    printf("\"btree_ops\":%lld,", btree_ops_total / SEARCH_ITERS);
    printf("\"bptree_time\":%.3f,", (double)bptree_total / SEARCH_ITERS / 1000.0);
    printf("\"bptree_ops\":%lld,", bptree_ops_total / SEARCH_ITERS);
    printf("\"counts_match\":%s", (linear_count == btree_count && linear_count == bptree_count) ? "true" : "false");
    printf("}\n");
    fflush(stdout);
}

int main(int argc, char **argv) {
    const char *csv_path;
    Player *players;
    BTree *btree;
    BPTree *bptree;
    int count = 0;
    int i;
    char line[128];

    if (argc < 2) {
        fprintf(stderr, "usage: %s <csv_path>\n", argv[0]);
        return 1;
    }

    csv_path = argv[1];
    players = load_players_csv(csv_path, &count);
    btree = btree_create();
    bptree = bptree_create();
    if (!players || count <= 0 || !btree || !bptree) {
        fprintf(stderr, "failed to load csv or allocate memory\n");
        free(players);
        btree_free(btree);
        bptree_free(bptree);
        return 1;
    }

    for (i = 0; i < count; ++i) {
        btree_insert(btree, players[i].id, &players[i]);
        bptree_insert(bptree, players[i].id, &players[i]);
    }

    printf("{\"ok\":true,\"ready\":true,\"dataset_size\":%d}\n", count);
    fflush(stdout);

    while (fgets(line, sizeof(line), stdin)) {
        int target_id = 0;
        int lo = 0;
        int hi = 0;

        if (strncmp(line, "search ", 7) == 0) {
            target_id = atoi(line + 7);
            if (target_id <= 0) {
                printf("{\"ok\":false,\"message\":\"id must be positive\"}\n");
                fflush(stdout);
                continue;
            }

            print_search_result(players, count, btree, bptree, target_id);
            continue;
        }

        if (sscanf(line, "range %d %d", &lo, &hi) == 2) {
            if (lo <= 0 || hi <= 0 || hi < lo) {
                printf("{\"ok\":false,\"message\":\"lo/hi must be positive and hi >= lo\"}\n");
                fflush(stdout);
                continue;
            }

            print_range_result(players, count, btree, bptree, lo, hi);
            continue;
        }

        if (strncmp(line, "quit", 4) == 0) {
            break;
        }

        printf("{\"ok\":false,\"message\":\"unknown command\"}\n");
        fflush(stdout);
    }

    free(players);
    btree_free(btree);
    bptree_free(bptree);
    return 0;
}
