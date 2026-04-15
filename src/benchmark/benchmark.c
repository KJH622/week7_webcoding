#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "../linear/linear_search.h"
#include "../btree/btree.h"
#include "../bplus_tree/bplus_tree.h"

/* 마이크로초 단위 현재 시각 */
static long long now_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000000LL + tv.tv_usec;
}

static void print_sep(void) {
    printf("--------------------------------------------------\n");
}

int main(int argc, char *argv[]) {
    int n = 1000000;
    if (argc >= 2) n = atoi(argv[1]);

    /* 데이터 생성 */
    printf("레코드 %d개 준비 중...\n", n);
    Player *table = malloc(sizeof(Player) * n);
    if (!table) { fprintf(stderr, "메모리 부족\n"); return 1; }

    srand(42);
    for (int i = 0; i < n; i++) {
        table[i].id    = i + 1;
        snprintf(table[i].name, 32, "Player_%d", i);
        table[i].score = rand() % 10000;
    }

    /* B 트리 / B+ 트리 삽입 */
    BTree  *bt  = btree_create();
    BPTree *bpt = bptree_create();
    for (int i = 0; i < n; i++) {
        btree_insert(bt,  table[i].id, &table[i]);
        bptree_insert(bpt, table[i].id, &table[i]);
    }

    int target   = n / 2;         /* 단일 탐색 대상 */
    int range_lo = n / 4;
    int range_hi = n / 4 + 1000;  /* 범위 탐색 대상 */

    print_sep();
    printf("벤치마크 시작 (n=%d)\n", n);
    print_sep();

    /* ── 단일 탐색 ── */
    printf("[단일 탐색] target id = %d\n\n", target);

    long long t0, t1;

    t0 = now_us();
    Player *res = linear_search(table, n, target);
    t1 = now_us();
    printf("  선형 탐색 : %8lld μs  결과=%s\n", t1-t0, res ? res->name : "없음");

    t0 = now_us();
    void *bres = btree_search(bt, target);
    t1 = now_us();
    printf("  B  트리   : %8lld μs  결과=%s\n", t1-t0, bres ? ((Player*)bres)->name : "없음");

    t0 = now_us();
    void *bpres = bptree_search(bpt, target);
    t1 = now_us();
    printf("  B+ 트리   : %8lld μs  결과=%s\n", t1-t0, bpres ? ((Player*)bpres)->name : "없음");

    print_sep();

    /* ── 범위 탐색 ── */
    printf("[범위 탐색] %d ~ %d\n\n", range_lo, range_hi);

    t0 = now_us();
    int lc = linear_range(table, n, range_lo, range_hi);
    t1 = now_us();
    printf("  선형 탐색 : %8lld μs  건수=%d\n", t1-t0, lc);

    t0 = now_us();
    int bc = btree_range(bt, range_lo, range_hi);
    t1 = now_us();
    printf("  B  트리   : %8lld μs  건수=%d\n", t1-t0, bc);

    t0 = now_us();
    int bpc = bptree_range(bpt, range_lo, range_hi);
    t1 = now_us();
    printf("  B+ 트리   : %8lld μs  건수=%d\n", t1-t0, bpc);

    print_sep();

    btree_free(bt);
    bptree_free(bpt);
    free(table);
    return 0;
}
