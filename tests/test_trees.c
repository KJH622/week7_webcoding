#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/common/player.h"
#include "../src/btree/btree.h"
#include "../src/bplus_tree/bplus_tree.h"

static int tests_passed = 0;
static int tests_failed = 0;

#define CHECK(condition, label)                                                     \
    do {                                                                            \
        if (condition) {                                                            \
            printf("  [PASS] %s\n", label);                                         \
            tests_passed += 1;                                                      \
        } else {                                                                    \
            printf("  [FAIL] %s\n", label);                                         \
            tests_failed += 1;                                                      \
        }                                                                           \
    } while (0)

static void fill_players(Player *players, int count) {
    int i;

    for (i = 0; i < count; ++i) {
        players[i].id = i + 1;
        snprintf(players[i].name, sizeof(players[i].name), "Player_%02d", i + 1);
        players[i].score = 1000 + (i * 17) % 5000;
        snprintf(players[i].tier, sizeof(players[i].tier), "Tier_%d", (i % 5) + 1);
    }
}

static void test_btree_single_search(void) {
    Player players[3];
    BTree *tree = btree_create();

    printf("\n[B-Tree single search]\n");
    fill_players(players, 3);

    btree_insert(tree, players[0].id, &players[0]);
    btree_insert(tree, players[1].id, &players[1]);
    btree_insert(tree, players[2].id, &players[2]);

    CHECK(((Player *)btree_search(tree, 1))->id == 1, "search finds id=1");
    CHECK(((Player *)btree_search(tree, 2))->id == 2, "search finds id=2");
    CHECK(((Player *)btree_search(tree, 3))->id == 3, "search finds id=3");
    CHECK(btree_search(tree, 99) == NULL, "missing id returns NULL");

    btree_free(tree);
}

static void test_btree_range_search(void) {
    Player players[10];
    BTree *tree = btree_create();
    int i;

    printf("\n[B-Tree range search]\n");
    fill_players(players, 10);

    for (i = 0; i < 10; ++i) {
        btree_insert(tree, players[i].id, &players[i]);
    }

    CHECK(btree_range(tree, 3, 7) == 5, "range 3..7 returns 5 rows");
    CHECK(btree_range(tree, 1, 10) == 10, "range 1..10 returns 10 rows");
    CHECK(btree_range(tree, 50, 60) == 0, "range outside keys returns 0");

    btree_free(tree);
}

static void test_btree_split_handling(void) {
    Player *players = (Player *)calloc(BT_ORDER * 3, sizeof(Player));
    BTree *tree = btree_create();
    int i;
    int ok = 1;

    printf("\n[B-Tree split handling]\n");
    fill_players(players, BT_ORDER * 3);

    for (i = 0; i < BT_ORDER * 3; ++i) {
        btree_insert(tree, players[i].id, &players[i]);
    }

    for (i = 0; i < BT_ORDER * 3; ++i) {
        Player *found = (Player *)btree_search(tree, players[i].id);
        if (!found || found->id != players[i].id) {
            ok = 0;
            break;
        }
    }

    CHECK(ok, "split keeps all keys searchable");

    btree_free(tree);
    free(players);
}

static void test_bptree_single_search(void) {
    Player players[3];
    BPTree *tree = bptree_create();

    printf("\n[B+ Tree single search]\n");
    fill_players(players, 3);
    players[0].id = 10;
    players[1].id = 20;
    players[2].id = 30;

    bptree_insert(tree, players[0].id, &players[0]);
    bptree_insert(tree, players[1].id, &players[1]);
    bptree_insert(tree, players[2].id, &players[2]);

    CHECK(((Player *)bptree_search(tree, 10))->id == 10, "search finds id=10");
    CHECK(((Player *)bptree_search(tree, 20))->id == 20, "search finds id=20");
    CHECK(((Player *)bptree_search(tree, 30))->id == 30, "search finds id=30");
    CHECK(bptree_search(tree, 99) == NULL, "missing id returns NULL");

    bptree_free(tree);
}

static void test_bptree_range_search(void) {
    Player players[10];
    BPTree *tree = bptree_create();
    int i;

    printf("\n[B+ Tree range search]\n");
    fill_players(players, 10);

    for (i = 0; i < 10; ++i) {
        bptree_insert(tree, players[i].id, &players[i]);
    }

    CHECK(bptree_range(tree, 3, 7) == 5, "range 3..7 returns 5 rows");
    CHECK(bptree_range(tree, 1, 10) == 10, "range 1..10 returns 10 rows");
    CHECK(bptree_range(tree, 50, 60) == 0, "range outside keys returns 0");

    bptree_free(tree);
}

static void test_bptree_split_handling(void) {
    Player *players = (Player *)calloc(BP_ORDER * 3, sizeof(Player));
    BPTree *tree = bptree_create();
    int i;
    int ok = 1;

    printf("\n[B+ Tree split handling]\n");
    fill_players(players, BP_ORDER * 3);

    for (i = 0; i < BP_ORDER * 3; ++i) {
        bptree_insert(tree, players[i].id, &players[i]);
    }

    for (i = 0; i < BP_ORDER * 3; ++i) {
        Player *found = (Player *)bptree_search(tree, players[i].id);
        if (!found || found->id != players[i].id) {
            printf("    mismatch at id=%d\n", players[i].id);
            ok = 0;
            break;
        }
    }

    CHECK(ok, "split keeps all keys searchable");

    bptree_free(tree);
    free(players);
}

int main(void) {
    printf("==============================\n");
    printf("  Tree Unit Tests\n");
    printf("==============================\n");

    test_btree_single_search();
    test_btree_range_search();
    test_btree_split_handling();
    test_bptree_single_search();
    test_bptree_range_search();
    test_bptree_split_handling();

    printf("\n==============================\n");
    printf("  Result: %d passed / %d failed\n", tests_passed, tests_failed);
    printf("==============================\n");

    return tests_failed == 0 ? 0 : 1;
}
