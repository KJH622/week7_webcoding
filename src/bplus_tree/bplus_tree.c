#include "bplus_tree.h"
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------- 내부 헬퍼 */

static BPLeaf *new_leaf(void) {
    BPLeaf *l = calloc(1, sizeof(BPLeaf));
    return l;
}

static BPNode *new_node(int is_leaf) {
    BPNode *n = calloc(1, sizeof(BPNode));
    n->is_leaf = is_leaf;
    if (is_leaf) n->leaf = new_leaf();
    return n;
}

/* -------------------------------------------------- 공개 API */

BPTree *bptree_create(void) {
    BPTree *t = calloc(1, sizeof(BPTree));
    t->root   = new_node(1);   /* 처음엔 루트 = 리프 */
    return t;
}

/*
 * TODO (구현 조):
 *   1. 리프 노드 탐색 → 키 삽입
 *   2. 오버플로우 시 노드 분할(split) 및 부모로 키 올리기
 *   3. 루트 분할 처리
 */
void bptree_insert(BPTree *tree, int key, void *record_ptr) {
    /* 구현 예정 */
    (void)tree; (void)key; (void)record_ptr;
}

/*
 * TODO (구현 조):
 *   1. 루트부터 리프까지 키 비교로 내려가기
 *   2. 리프에서 key 탐색 후 포인터 반환
 */
void *bptree_search(BPTree *tree, int key) {
    /* 구현 예정 */
    (void)tree; (void)key;
    return NULL;
}

/*
 * TODO (구현 조):
 *   1. lo에 해당하는 리프 노드 탐색
 *   2. 리프 연결 리스트를 따라가며 hi 이하 키 카운트
 */
int bptree_range(BPTree *tree, int lo, int hi) {
    /* 구현 예정 */
    (void)tree; (void)lo; (void)hi;
    return 0;
}

void bptree_free(BPTree *tree) {
    /* TODO: 전체 노드 순회 후 free */
    free(tree);
}
