#include "btree.h"

#include <stdlib.h>
#include <string.h>

/* benchmark.c에서 정의된 전역 연산 횟수 카운터 */
extern long long g_op_count;

/* -------------------------------------------------- 내부 헬퍼 */

typedef struct {
    int did_split;
    int promoted_key;
    void *promoted_ptr;
    BTNode *right;
} BTInsertResult;

static BTNode *new_node(int is_leaf) {
    BTNode *node = (BTNode *)calloc(1, sizeof(BTNode));
    if (!node) {
        return NULL;
    }

    node->is_leaf = is_leaf;
    return node;
}

static int lower_bound_keys(const int *keys, int count, int key) {
    int lo = 0;
    int hi = count;

    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (keys[mid] < key) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }

    return lo;
}

static BTInsertResult insert_recursive(BTNode *node, int key, void *record_ptr) {
    BTInsertResult result = {0, 0, NULL, NULL};
    int idx = lower_bound_keys(node->keys, node->num_keys, key);

    if (node->is_leaf) {
        int i;

        if (idx < node->num_keys && node->keys[idx] == key) {
            node->ptrs[idx] = record_ptr;
            return result;
        }

        for (i = node->num_keys; i > idx; --i) {
            node->keys[i] = node->keys[i - 1];
            node->ptrs[i] = node->ptrs[i - 1];
        }

        node->keys[idx] = key;
        node->ptrs[idx] = record_ptr;
        node->num_keys += 1;

        if (node->num_keys <= BT_ORDER) {
            return result;
        }
    } else {
        BTInsertResult child_result = insert_recursive(node->children[idx], key, record_ptr);
        int i;

        if (!child_result.did_split) {
            return result;
        }

        for (i = node->num_keys; i > idx; --i) {
            node->keys[i] = node->keys[i - 1];
            node->ptrs[i] = node->ptrs[i - 1];
        }
        for (i = node->num_keys + 1; i > idx + 1; --i) {
            node->children[i] = node->children[i - 1];
        }

        node->keys[idx] = child_result.promoted_key;
        node->ptrs[idx] = child_result.promoted_ptr;
        node->children[idx + 1] = child_result.right;
        node->num_keys += 1;

        if (node->num_keys <= BT_ORDER) {
            return result;
        }
    }

    {
        BTNode *right = new_node(node->is_leaf);
        int total = node->num_keys;
        int mid = total / 2;
        int right_keys;
        int i;

        result.did_split = 1;
        result.right = right;

        if (node->is_leaf) {
            right_keys = total - mid;

            for (i = 0; i < right_keys; ++i) {
                right->keys[i] = node->keys[mid + i];
                right->ptrs[i] = node->ptrs[mid + i];
            }

            right->num_keys = right_keys;
            node->num_keys = mid;
            result.promoted_key = right->keys[0];
            result.promoted_ptr = right->ptrs[0];
        } else {
            int promoted_index = mid;
            int right_children = total - promoted_index - 1;

            result.promoted_key = node->keys[promoted_index];
            result.promoted_ptr = node->ptrs[promoted_index];

            for (i = 0; i < right_children; ++i) {
                right->keys[i] = node->keys[promoted_index + 1 + i];
                right->ptrs[i] = node->ptrs[promoted_index + 1 + i];
            }
            for (i = 0; i < right_children + 1; ++i) {
                right->children[i] = node->children[promoted_index + 1 + i];
            }

            right->num_keys = right_children;
            node->num_keys = promoted_index;
        }
    }

    return result;
}

static void *search_recursive(BTNode *node, int key) {
    int idx;

    if (!node) {
        return NULL;
    }

    idx = lower_bound_keys(node->keys, node->num_keys, key);

    if (node->is_leaf) {
        if (idx < node->num_keys && node->keys[idx] == key) {
            return node->ptrs[idx];
        }
        return NULL;
    }

    if (idx < node->num_keys && node->keys[idx] == key) {
        return node->ptrs[idx];
    }

    return search_recursive(node->children[idx], key);
}

static int range_recursive(BTNode *node, int lo, int hi) {
    int count = 0;
    int i;

    if (!node) {
        return 0;
    }

    if (node->is_leaf) {
        for (i = 0; i < node->num_keys; ++i) {
            if (node->keys[i] >= lo && node->keys[i] <= hi) {
                count += 1;
            }
        }
        return count;
    }

    for (i = 0; i < node->num_keys; ++i) {
        if (lo < node->keys[i]) {
            count += range_recursive(node->children[i], lo, hi);
        }
        if (node->keys[i] >= lo && node->keys[i] <= hi) {
            count += 1;
        }
        if (node->keys[i] > hi) {
            return count;
        }
    }

    count += range_recursive(node->children[node->num_keys], lo, hi);
    return count;
}

static void free_recursive(BTNode *node) {
    int i;

    if (!node) {
        return;
    }

    if (!node->is_leaf) {
        for (i = 0; i <= node->num_keys; ++i) {
            free_recursive(node->children[i]);
        }
    }

    free(node);
}

BTree *btree_create(void) {
    BTree *tree = (BTree *)calloc(1, sizeof(BTree));
    if (!tree) {
        return NULL;
    }

    tree->root = new_node(1);
    if (!tree->root) {
        free(tree);
        return NULL;
    }

    return tree;
}

void btree_insert(BTree *tree, int key, void *record_ptr) {
    BTInsertResult result;
    BTNode *new_root;

    if (!tree || !tree->root) {
        return;
    }

    result = insert_recursive(tree->root, key, record_ptr);
    if (!result.did_split) {
        return;
    }

    new_root = new_node(0);
    if (!new_root) {
        return;
    }

    new_root->keys[0] = result.promoted_key;
    new_root->ptrs[0] = result.promoted_ptr;
    new_root->children[0] = tree->root;
    new_root->children[1] = result.right;
    new_root->num_keys = 1;
    tree->root = new_root;
}

void *btree_search(BTree *tree, int key) {
    if (!tree) {
        return NULL;
    }

    return search_recursive(tree->root, key);
}

int btree_range(BTree *tree, int lo, int hi) {
    if (!tree || lo > hi) {
        return 0;
    }

    return range_recursive(tree->root, lo, hi);
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
    if (!tree) {
        return;
    }

    free_recursive(tree->root);
    free(tree);
}
