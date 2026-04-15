#ifndef BTREE_H
#define BTREE_H

#define BT_ORDER 32  /* 노드당 최대 키 수. 100만 건 기준 트리 높이 ~4 레벨 */

typedef struct BTNode {
    int            keys[BT_ORDER];
    void          *ptrs[BT_ORDER];          /* 키에 대응하는 레코드 포인터 (리프처럼 사용) */
    struct BTNode *children[BT_ORDER + 1];
    int            num_keys;
    int            is_leaf;
} BTNode;

typedef struct {
    BTNode *root;
} BTree;

BTree *btree_create(void);
void   btree_insert(BTree *tree, int key, void *record_ptr);
void  *btree_search(BTree *tree, int key);
int    btree_range(BTree *tree, int lo, int hi);
void   btree_free(BTree *tree);

#endif
