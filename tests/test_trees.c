#include <stdio.h>
#include <assert.h>
#include "../src/common/player.h"
#include "../src/btree/btree.h"
#include "../src/bplus_tree/bplus_tree.h"

static int pass = 0, fail = 0;

#define CHECK(cond, msg) \
    do { \
        if (cond) { printf("  [PASS] %s\n", msg); pass++; } \
        else       { printf("  [FAIL] %s\n", msg); fail++; } \
    } while(0)

/* ─── B 트리 단위 테스트 ─────────────────────────── */

static void test_btree_single(void) {
    printf("\n[B 트리 - 단일 탐색]\n");
    BTree *bt = btree_create();

    Player p1 = {1, "Alpha", 9000, "Challenger"};
    Player p2 = {2, "Beta",  7500, "Platinum"};
    Player p3 = {3, "Gamma", 5000, "Gold"};

    btree_insert(bt, p1.id, &p1);
    btree_insert(bt, p2.id, &p2);
    btree_insert(bt, p3.id, &p3);

    CHECK(btree_search(bt, 1) == &p1, "id=1 탐색 성공");
    CHECK(btree_search(bt, 2) == &p2, "id=2 탐색 성공");
    CHECK(btree_search(bt, 3) == &p3, "id=3 탐색 성공");
    CHECK(btree_search(bt, 99) == NULL, "없는 id 탐색 시 NULL 반환");

    btree_free(bt);
}

static void test_btree_range(void) {
    printf("\n[B 트리 - 범위 탐색]\n");
    BTree *bt = btree_create();

    Player players[10];
    for (int i = 0; i < 10; i++) {
        players[i].id = i + 1;
        btree_insert(bt, players[i].id, &players[i]);
    }

    CHECK(btree_range(bt, 3, 7) == 5, "id 3~7 범위: 5건");
    CHECK(btree_range(bt, 1, 10) == 10, "id 1~10 범위: 10건");
    CHECK(btree_range(bt, 11, 20) == 0, "범위 밖: 0건");

    btree_free(bt);
}

static void test_btree_split(void) {
    printf("\n[B 트리 - 노드 분할]\n");
    BTree *bt = btree_create();

    /* ORDER=4 이므로 5개 이상 삽입 시 split 발생 */
    Player players[20];
    for (int i = 0; i < 20; i++) {
        players[i].id = i + 1;
        btree_insert(bt, players[i].id, &players[i]);
    }

    int ok = 1;
    for (int i = 0; i < 20; i++) {
        if (btree_search(bt, i + 1) != &players[i]) { ok = 0; break; }
    }
    CHECK(ok, "split 후 20개 전체 탐색 정상");

    btree_free(bt);
}

/* ─── B+ 트리 단위 테스트 ────────────────────────── */

static void test_bptree_single(void) {
    printf("\n[B+ 트리 - 단일 탐색]\n");
    BPTree *bpt = bptree_create();

    Player p1 = {10, "ShadowBlade", 9980, "Challenger"};
    Player p2 = {20, "NightFury",   9870, "Challenger"};
    Player p3 = {30, "IronWolf",    5400, "Gold"};

    bptree_insert(bpt, p1.id, &p1);
    bptree_insert(bpt, p2.id, &p2);
    bptree_insert(bpt, p3.id, &p3);

    CHECK(bptree_search(bpt, 10) == &p1, "id=10 탐색 성공");
    CHECK(bptree_search(bpt, 20) == &p2, "id=20 탐색 성공");
    CHECK(bptree_search(bpt, 30) == &p3, "id=30 탐색 성공");
    CHECK(bptree_search(bpt, 99) == NULL, "없는 id 탐색 시 NULL 반환");

    bptree_free(bpt);
}

static void test_bptree_range(void) {
    printf("\n[B+ 트리 - 범위 탐색 (리프 체인)]\n");
    BPTree *bpt = bptree_create();

    Player players[10];
    for (int i = 0; i < 10; i++) {
        players[i].id = i + 1;
        bptree_insert(bpt, players[i].id, &players[i]);
    }

    CHECK(bptree_range(bpt, 3, 7) == 5, "id 3~7 범위: 5건");
    CHECK(bptree_range(bpt, 1, 10) == 10, "id 1~10 범위: 10건");
    CHECK(bptree_range(bpt, 11, 20) == 0, "범위 밖: 0건");

    bptree_free(bpt);
}

static void test_bptree_split(void) {
    printf("\n[B+ 트리 - 노드 분할]\n");
    BPTree *bpt = bptree_create();

    Player players[20];
    for (int i = 0; i < 20; i++) {
        players[i].id = i + 1;
        bptree_insert(bpt, players[i].id, &players[i]);
    }

    int ok = 1;
    for (int i = 0; i < 20; i++) {
        if (bptree_search(bpt, i + 1) != &players[i]) { ok = 0; break; }
    }
    CHECK(ok, "split 후 20개 전체 탐색 정상");

    bptree_free(bpt);
}

/* ─── main ───────────────────────────────────────── */

int main(void) {
    printf("==============================\n");
    printf("  Tree Unit Tests\n");
    printf("==============================\n");

    test_btree_single();
    test_btree_range();
    test_btree_split();

    test_bptree_single();
    test_bptree_range();
    test_bptree_split();

    printf("\n==============================\n");
    printf("  결과: %d 통과 / %d 실패\n", pass, fail);
    printf("==============================\n");

    return fail > 0 ? 1 : 0;
}
