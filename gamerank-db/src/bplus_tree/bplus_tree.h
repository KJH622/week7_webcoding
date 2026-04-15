#ifndef BPLUS_TREE_H
#define BPLUS_TREE_H

#define BP_ORDER 4   /* 노드당 최대 키 수. 필요에 따라 조정 */

typedef struct BPLeaf {
    int            keys[BP_ORDER];
    void          *ptrs[BP_ORDER];   /* 레코드 포인터 */
    int            num_keys;
    struct BPLeaf *next;             /* 리프 연결 리스트 */
} BPLeaf;

typedef struct BPNode {
    int            keys[BP_ORDER];
    struct BPNode *children[BP_ORDER + 1];
    int            num_keys;
    int            is_leaf;
    BPLeaf        *leaf;             /* is_leaf == 1 일 때 사용 */
} BPNode;

typedef struct {
    BPNode *root;
} BPTree;

/* 트리 초기화 */
BPTree *bptree_create(void);

/* 삽입: (key, record_ptr) */
void    bptree_insert(BPTree *tree, int key, void *record_ptr);

/* 단일 탐색: key에 해당하는 레코드 포인터 반환. 없으면 NULL */
void   *bptree_search(BPTree *tree, int key);

/* 범위 탐색: lo ~ hi 범위의 레코드 개수 반환 */
int     bptree_range(BPTree *tree, int lo, int hi);

/* 메모리 해제 */
void    bptree_free(BPTree *tree);

#endif
