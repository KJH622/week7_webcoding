#include "btree.h"
#include <stdlib.h>

static BTNode *new_node(int is_leaf) {
    BTNode *n = calloc(1, sizeof(BTNode));
    n->is_leaf = is_leaf;
    return n;
}

BTree *btree_create(void) {
    BTree *t = calloc(1, sizeof(BTree));
    t->root   = new_node(1);
    return t;
}

/* TODO: 삽입 구현 */
void btree_insert(BTree *tree, int key, void *record_ptr) {
    (void)tree; (void)key; (void)record_ptr;
}

/* TODO: 탐색 구현 — 내부/리프 노드 모두 키 비교 */
void *btree_search(BTree *tree, int key) {
    (void)tree; (void)key;
    return NULL;
}

/* TODO: 범위 탐색 — 트리 전체 순회 필요 */
int btree_range(BTree *tree, int lo, int hi) {
    (void)tree; (void)lo; (void)hi;
    return 0;
}

void btree_free(BTree *tree) {
    free(tree);
}
