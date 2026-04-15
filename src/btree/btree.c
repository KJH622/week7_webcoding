#include "btree.h"
#include <stdlib.h>
#include <string.h>

/* benchmark.c에서 정의된 전역 연산 횟수 카운터 */
extern long long g_op_count;

/* -------------------------------------------------- 내부 헬퍼 */

static BTNode *new_node(int is_leaf) {
    BTNode *n = calloc(1, sizeof(BTNode));
    n->is_leaf = is_leaf;
    return n;
}

/* node 내에서 key가 삽입될 위치(첫 번째 keys[i] >= key인 i) 반환 */
static int find_pos(BTNode *node, int key) {
    int i = 0;
    while (i < node->num_keys && node->keys[i] < key)
        i++;
    return i;
}

/* 분할 결과를 담는 구조체 */
typedef struct {
    int     key;    /* 부모로 올라갈 키 */
    void   *ptr;    /* 해당 키의 레코드 포인터 */
    BTNode *right;  /* 새로 생긴 오른쪽 노드 */
} Split;

/*
 * 꽉 찬 노드(num_keys == BT_ORDER)를 둘로 분할.
 * - 왼쪽(node): keys[0 .. mid-1]  (mid개)
 * - 승격 키:    keys[mid]
 * - 오른쪽:     keys[mid+1 .. BT_ORDER-1]  (BT_ORDER-mid-1개)
 */
static void split_node(BTNode *node, Split *out) {
    int mid         = BT_ORDER / 2;   /* 16 */
    int right_count = node->num_keys - mid - 1;

    BTNode *right = new_node(node->is_leaf);

    /* 오른쪽 노드에 키/포인터 복사 */
    for (int i = 0; i < right_count; i++) {
        right->keys[i] = node->keys[mid + 1 + i];
        right->ptrs[i] = node->ptrs[mid + 1 + i];
    }
    /* 자식 포인터 복사 (내부 노드일 때) */
    if (!node->is_leaf) {
        for (int i = 0; i <= right_count; i++)
            right->children[i] = node->children[mid + 1 + i];
    }
    right->num_keys = right_count;

    /* 승격 키 설정 */
    out->key   = node->keys[mid];
    out->ptr   = node->ptrs[mid];
    out->right = right;

    /* 왼쪽 노드는 mid개만 남김 */
    node->num_keys = mid;
}

/*
 * 재귀 삽입.
 * 반환값: 분할이 발생했으면 1, 아니면 0.
 * 분할 결과는 s에 저장.
 */
static int insert_rec(BTNode *node, int key, void *record_ptr, Split *s) {
    int pos = find_pos(node, key);

    /* 키 중복: 포인터만 업데이트 */
    if (pos < node->num_keys && node->keys[pos] == key) {
        node->ptrs[pos] = record_ptr;
        return 0;
    }

    if (node->is_leaf) {
        /* 리프 노드: 해당 위치에 키/포인터 삽입 */
        memmove(&node->keys[pos + 1], &node->keys[pos],
                sizeof(int) * (node->num_keys - pos));
        memmove(&node->ptrs[pos + 1], &node->ptrs[pos],
                sizeof(void *) * (node->num_keys - pos));
        node->keys[pos] = key;
        node->ptrs[pos] = record_ptr;
        node->num_keys++;
    } else {
        /* 내부 노드: 적절한 자식으로 재귀 삽입 */
        Split child_split;
        int promoted = insert_rec(node->children[pos], key, record_ptr, &child_split);

        if (promoted) {
            /* 자식에서 분할 발생 → 승격된 키를 현재 노드에 삽입 */
            memmove(&node->keys[pos + 1], &node->keys[pos],
                    sizeof(int) * (node->num_keys - pos));
            memmove(&node->ptrs[pos + 1], &node->ptrs[pos],
                    sizeof(void *) * (node->num_keys - pos));
            memmove(&node->children[pos + 2], &node->children[pos + 1],
                    sizeof(BTNode *) * (node->num_keys - pos));
            node->keys[pos]         = child_split.key;
            node->ptrs[pos]         = child_split.ptr;
            node->children[pos + 1] = child_split.right;
            node->num_keys++;
        }
    }

    /* 오버플로우 확인 */
    if (node->num_keys == BT_ORDER) {
        split_node(node, s);
        return 1;
    }
    return 0;
}

/* -------------------------------------------------- 공개 API */

BTree *btree_create(void) {
    BTree *t = calloc(1, sizeof(BTree));
    t->root  = new_node(1);
    return t;
}

void btree_insert(BTree *tree, int key, void *record_ptr) {
    Split s;
    int promoted = insert_rec(tree->root, key, record_ptr, &s);
    if (promoted) {
        /* 루트가 분할됨 → 새 루트 생성 */
        BTNode *new_root      = new_node(0);
        new_root->keys[0]     = s.key;
        new_root->ptrs[0]     = s.ptr;
        new_root->children[0] = tree->root;
        new_root->children[1] = s.right;
        new_root->num_keys    = 1;
        tree->root = new_root;
    }
}

/*
 * 단일 탐색: 루트부터 내려가며 모든 노드에서 키 비교.
 * (B 트리는 리프뿐 아니라 내부 노드에도 레코드가 있음)
 */
void *btree_search(BTree *tree, int key) {
    BTNode *node = tree->root;
    while (node != NULL) {
        int i = 0;
        while (i < node->num_keys && node->keys[i] < key) {
            g_op_count++;          /* 키 비교 1회 */
            i++;
        }
        g_op_count++;              /* 최종 비교 (== 또는 범위 초과) */
        if (i < node->num_keys && node->keys[i] == key)
            return node->ptrs[i];
        if (node->is_leaf)
            return NULL;
        node = node->children[i];
    }
    return NULL;
}

/*
 * 범위 탐색 재귀 헬퍼.
 * 모든 노드(내부 + 리프)를 순회하며 lo ~ hi 범위의 키 개수를 센다.
 *
 * children[i] 에는 keys[i] 보다 작은 키만 있으므로:
 *   - lo < keys[i] 일 때만 children[i] 방문
 * keys[i] > hi 이면 이후 키/자식은 방문 불필요 → 조기 종료
 */
static int range_rec(BTNode *node, int lo, int hi) {
    if (node == NULL) return 0;
    int count = 0;

    for (int i = 0; i < node->num_keys; i++) {
        if (!node->is_leaf && lo < node->keys[i])
            count += range_rec(node->children[i], lo, hi);

        g_op_count++;              /* 키 비교 1회 */
        if (node->keys[i] > hi)
            return count;

        if (node->keys[i] >= lo)
            count++;
    }

    /* 가장 오른쪽 자식 방문 (마지막 키보다 큰 키들) */
    if (!node->is_leaf)
        count += range_rec(node->children[node->num_keys], lo, hi);

    return count;
}

int btree_range(BTree *tree, int lo, int hi) {
    return range_rec(tree->root, lo, hi);
}

/* -------------------------------------------------- 메모리 해제 */

static void free_node(BTNode *node) {
    if (node == NULL) return;
    if (!node->is_leaf) {
        for (int i = 0; i <= node->num_keys; i++)
            free_node(node->children[i]);
    }
    free(node);
}

void btree_free(BTree *tree) {
    free_node(tree->root);
    free(tree);
}
