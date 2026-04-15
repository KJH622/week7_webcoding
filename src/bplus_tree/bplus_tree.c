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

/* 분할 결과를 담는 구조체 */
typedef struct {
    int     key;    /* 부모로 승격될 키 */
    BPNode *right;  /* 분할로 생긴 오른쪽 BPNode */
} BPSplit;

/*
 * 꽉 찬 리프(BPLeaf)를 둘로 분할 (copy-up 방식).
 * - 왼쪽 리프(left_node->leaf): keys[0 .. mid-1]  (mid개)
 * - 오른쪽 리프(새로 생성):      keys[mid .. BP_ORDER-1]  (BP_ORDER-mid개)
 * - 승격 키 = 오른쪽 리프의 첫 번째 키 (오른쪽에도 그대로 남아 있음)
 */
static BPSplit split_leaf(BPNode *left_node) {
    BPLeaf *left  = left_node->leaf;
    BPNode *right_node = new_node(1);
    BPLeaf *right      = right_node->leaf;

    int mid         = BP_ORDER / 2;   /* 16 */
    int right_count = left->num_keys - mid;

    /* 오른쪽 리프에 mid 이후 키/포인터 복사 */
    for (int i = 0; i < right_count; i++) {
        right->keys[i] = left->keys[mid + i];
        right->ptrs[i] = left->ptrs[mid + i];
    }
    right->num_keys = right_count;
    left->num_keys  = mid;

    /* 리프 연결 리스트 연결 */
    right->next = left->next;
    left->next  = right;

    /* 승격 키 = 오른쪽 리프의 첫 번째 키 */
    BPSplit s;
    s.key   = right->keys[0];
    s.right = right_node;
    return s;
}

/*
 * 꽉 찬 내부 노드를 둘로 분할 (move-up 방식).
 * - 왼쪽(node): keys[0 .. mid-1]  (mid개)
 * - 승격 키:    keys[mid]  (부모로 올라가고 양쪽에서 제거됨)
 * - 오른쪽:     keys[mid+1 .. BP_ORDER-1]  (BP_ORDER-mid-1개)
 */
static BPSplit split_internal(BPNode *node) {
    int mid         = BP_ORDER / 2;   /* 16 */
    int right_count = node->num_keys - mid - 1;

    BPNode *right = new_node(0);

    /* 오른쪽 노드에 키/자식 복사 */
    for (int i = 0; i < right_count; i++) {
        right->keys[i]     = node->keys[mid + 1 + i];
        right->children[i] = node->children[mid + 1 + i];
    }
    right->children[right_count] = node->children[node->num_keys];
    right->num_keys = right_count;

    /* 승격 키 설정 */
    BPSplit s;
    s.key   = node->keys[mid];
    s.right = right;

    /* 왼쪽 노드는 mid개만 남김 */
    node->num_keys = mid;
    return s;
}

/*
 * 재귀 삽입.
 * 반환값: 분할이 발생했으면 1, 아니면 0.
 * 분할 결과는 split에 저장.
 */
static int insert_rec(BPNode *node, int key, void *record_ptr, BPSplit *split) {
    if (node->is_leaf) {
        BPLeaf *leaf = node->leaf;

        /* 삽입 위치 탐색 */
        int i = 0;
        while (i < leaf->num_keys && leaf->keys[i] < key) i++;

        /* 키 중복: 포인터만 업데이트 */
        if (i < leaf->num_keys && leaf->keys[i] == key) {
            leaf->ptrs[i] = record_ptr;
            return 0;
        }

        /* 삽입 */
        memmove(&leaf->keys[i + 1], &leaf->keys[i],
                sizeof(int) * (leaf->num_keys - i));
        memmove(&leaf->ptrs[i + 1], &leaf->ptrs[i],
                sizeof(void *) * (leaf->num_keys - i));
        leaf->keys[i] = key;
        leaf->ptrs[i] = record_ptr;
        leaf->num_keys++;

        /* 오버플로우 확인 */
        if (leaf->num_keys == BP_ORDER) {
            *split = split_leaf(node);
            return 1;
        }
        return 0;
    }

    /* 내부 노드: 적절한 자식으로 재귀.
     * keys[i]가 자식[i+1]의 최솟값이므로, key >= keys[i]이면 오른쪽으로 이동 */
    int i = 0;
    while (i < node->num_keys && key >= node->keys[i]) i++;

    BPSplit child_split;
    int promoted = insert_rec(node->children[i], key, record_ptr, &child_split);

    if (promoted) {
        /* 자식 분할 → 승격 키를 현재 노드에 삽입 */
        memmove(&node->keys[i + 1], &node->keys[i],
                sizeof(int) * (node->num_keys - i));
        memmove(&node->children[i + 2], &node->children[i + 1],
                sizeof(BPNode *) * (node->num_keys - i));
        node->keys[i]         = child_split.key;
        node->children[i + 1] = child_split.right;
        node->num_keys++;

        /* 오버플로우 확인 */
        if (node->num_keys == BP_ORDER) {
            *split = split_internal(node);
            return 1;
        }
    }
    return 0;
}

/* -------------------------------------------------- 공개 API */

BPTree *bptree_create(void) {
    BPTree *t = calloc(1, sizeof(BPTree));
    t->root   = new_node(1);   /* 처음엔 루트 = 리프 */
    return t;
}

void bptree_insert(BPTree *tree, int key, void *record_ptr) {
    BPSplit s;
    int promoted = insert_rec(tree->root, key, record_ptr, &s);
    if (promoted) {
        /* 루트 분할 → 새 루트 생성 */
        BPNode *new_root      = new_node(0);
        new_root->keys[0]     = s.key;
        new_root->children[0] = tree->root;
        new_root->children[1] = s.right;
        new_root->num_keys    = 1;
        tree->root = new_root;
    }
}

/*
 * 단일 탐색: 루트부터 리프까지 내려간 뒤 BPLeaf에서 키 검색.
 */
void *bptree_search(BPTree *tree, int key) {
    BPNode *node = tree->root;

    /* 루트부터 리프까지 내려가기 */
    while (!node->is_leaf) {
        int i = 0;
        while (i < node->num_keys && key >= node->keys[i]) i++;
        node = node->children[i];
    }

    /* 리프에서 키 탐색 */
    BPLeaf *leaf = node->leaf;
    for (int i = 0; i < leaf->num_keys; i++) {
        if (leaf->keys[i] == key) return leaf->ptrs[i];
        if (leaf->keys[i] > key)  break;   /* 정렬되어 있으므로 조기 종료 */
    }
    return NULL;
}

/*
 * 범위 탐색: lo에 해당하는 리프까지 내려간 뒤
 * 리프 연결 리스트(next)를 따라가며 hi 이하 키 카운트.
 * B+ 트리의 범위 탐색 핵심 장점.
 */
int bptree_range(BPTree *tree, int lo, int hi) {
    BPNode *node = tree->root;

    /* lo에 해당하는 리프 탐색 */
    while (!node->is_leaf) {
        int i = 0;
        while (i < node->num_keys && lo >= node->keys[i]) i++;
        node = node->children[i];
    }

    /* 리프 연결 리스트를 따라가며 카운트 */
    int count    = 0;
    BPLeaf *leaf = node->leaf;
    while (leaf != NULL) {
        for (int i = 0; i < leaf->num_keys; i++) {
            if (leaf->keys[i] > hi)  return count;  /* 범위 초과 → 종료 */
            if (leaf->keys[i] >= lo) count++;
        }
        leaf = leaf->next;
    }
    return count;
}

/* -------------------------------------------------- 메모리 해제 */

static void free_bp_node(BPNode *node) {
    if (node == NULL) return;
    if (node->is_leaf) {
        free(node->leaf);
    } else {
        for (int i = 0; i <= node->num_keys; i++)
            free_bp_node(node->children[i]);
    }
    free(node);
}

void bptree_free(BPTree *tree) {
    free_bp_node(tree->root);
    free(tree);
}
