#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "../linear/linear_search.h"
#include "../btree/btree.h"
#include "../bplus_tree/bplus_tree.h"

/* ── 전역 연산 횟수 카운터 ─────────────────────────
 * linear_search.c / btree.c / bplus_tree.c 에서
 * extern long long g_op_count 로 참조하여 비교 횟수를 기록함 */
long long g_op_count = 0;

/* ── 상수 ─────────────────────────────────────────── */

#define SIZE_COUNT 4   /* 측정 데이터셋 종류 수 */
#define ITERS      5   /* 반복 횟수 (평균 산출용) */

static const int SIZES[SIZE_COUNT] = {100000, 500000, 1000000, 5000000};

/* ── 헬퍼 ─────────────────────────────────────────── */

static long long now_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000000LL + tv.tv_usec;
}

static void print_sep(void) {
    printf("--------------------------------------------------\n");
}

/* ── JSON 출력 ─────────────────────────────────────── */

static void write_arr(FILE *fp, const char *key, long long *arr, int last) {
    fprintf(fp, "    \"%s\": [", key);
    for (int i = 0; i < SIZE_COUNT; i++)
        fprintf(fp, "%lld%s", arr[i], i < SIZE_COUNT - 1 ? ", " : "");
    fprintf(fp, last ? "]\n" : "],\n");
}

/* results.json 스키마에 맞게 시간(μs)과 비교 횟수를 파일로 저장 */
static void write_json(
    const char *path,
    long long sl[SIZE_COUNT],      long long sl_ops[SIZE_COUNT],
    long long sb[SIZE_COUNT],      long long sb_ops[SIZE_COUNT],
    long long sbp[SIZE_COUNT],     long long sbp_ops[SIZE_COUNT],
    long long rl[SIZE_COUNT],      long long rl_ops[SIZE_COUNT],
    long long rb[SIZE_COUNT],      long long rb_ops[SIZE_COUNT],
    long long rbp[SIZE_COUNT],     long long rbp_ops[SIZE_COUNT]
) {
    FILE *fp = fopen(path, "w");
    if (!fp) { fprintf(stderr, "JSON 출력 실패: %s\n", path); return; }

    /* sizes */
    fprintf(fp, "{\n  \"sizes\": [");
    for (int i = 0; i < SIZE_COUNT; i++)
        fprintf(fp, "%d%s", SIZES[i], i < SIZE_COUNT - 1 ? ", " : "");
    fprintf(fp, "],\n");

    /* single */
    fprintf(fp, "  \"single\": {\n");
    write_arr(fp, "linear",     sl,  0);
    write_arr(fp, "linear_ops", sl_ops, 0);
    write_arr(fp, "btree",      sb,  0);
    write_arr(fp, "btree_ops",  sb_ops, 0);
    write_arr(fp, "bptree",     sbp, 0);
    write_arr(fp, "bptree_ops", sbp_ops, 1);
    fprintf(fp, "  },\n");

    /* range */
    fprintf(fp, "  \"range\": {\n");
    write_arr(fp, "linear",     rl,  0);
    write_arr(fp, "linear_ops", rl_ops, 0);
    write_arr(fp, "btree",      rb,  0);
    write_arr(fp, "btree_ops",  rb_ops, 0);
    write_arr(fp, "bptree",     rbp, 0);
    write_arr(fp, "bptree_ops", rbp_ops, 1);
    fprintf(fp, "  }\n}\n");

    fclose(fp);
    printf("JSON 저장 완료: %s\n", path);
}

/* ── main ──────────────────────────────────────────── */

int main(int argc, char *argv[]) {
    const char *out_path = (argc >= 2) ? argv[1] : "web/assets/results.json";

    long long sl[SIZE_COUNT]      = {0};  long long sl_ops[SIZE_COUNT]  = {0};
    long long sb[SIZE_COUNT]      = {0};  long long sb_ops[SIZE_COUNT]  = {0};
    long long sbp[SIZE_COUNT]     = {0};  long long sbp_ops[SIZE_COUNT] = {0};
    long long rl[SIZE_COUNT]      = {0};  long long rl_ops[SIZE_COUNT]  = {0};
    long long rb[SIZE_COUNT]      = {0};  long long rb_ops[SIZE_COUNT]  = {0};
    long long rbp[SIZE_COUNT]     = {0};  long long rbp_ops[SIZE_COUNT] = {0};

    for (int si = 0; si < SIZE_COUNT; si++) {
        int n      = SIZES[si];
        int target = n / 2;
        int lo     = 1;
        int hi     = (int)(n * 0.01);

        print_sep();
        printf("[ %d/%d ] n=%d  single=id#%d  range=%d~%d  반복=%d회\n",
               si + 1, SIZE_COUNT, n, target, lo, hi, ITERS);
        print_sep();

        printf("  데이터 생성 중...\n");
        Player *table = malloc(sizeof(Player) * n);
        if (!table) { fprintf(stderr, "메모리 부족 (n=%d)\n", n); return 1; }

        srand(42);
        for (int i = 0; i < n; i++) {
            table[i].id    = i + 1;
            snprintf(table[i].name, 32, "Player_%d", i);
            table[i].score = rand() % 10000;
        }

        printf("  트리 삽입 중...\n");
        BTree  *bt  = btree_create();
        BPTree *bpt = bptree_create();
        for (int i = 0; i < n; i++) {
            btree_insert(bt,   table[i].id, &table[i]);
            bptree_insert(bpt, table[i].id, &table[i]);
        }

        long long t0, t1;
        long long ls_sum=0, bt_sum=0, bpt_sum=0;
        long long lr_sum=0, br_sum=0, bpr_sum=0;
        long long ls_ops_sum=0, bt_ops_sum=0, bpt_ops_sum=0;
        long long lr_ops_sum=0, br_ops_sum=0, bpr_ops_sum=0;

        printf("  측정 중...\n");
        for (int it = 0; it < ITERS; it++) {
            /* 단일 탐색 */
            g_op_count = 0; t0 = now_us(); linear_search(table, n, target);  t1 = now_us();
            ls_sum += t1-t0;  ls_ops_sum  += g_op_count;

            g_op_count = 0; t0 = now_us(); btree_search(bt, target);          t1 = now_us();
            bt_sum += t1-t0;  bt_ops_sum  += g_op_count;

            g_op_count = 0; t0 = now_us(); bptree_search(bpt, target);        t1 = now_us();
            bpt_sum += t1-t0; bpt_ops_sum += g_op_count;

            /* 범위 탐색 */
            g_op_count = 0; t0 = now_us(); linear_range(table, n, lo, hi);   t1 = now_us();
            lr_sum += t1-t0;  lr_ops_sum  += g_op_count;

            g_op_count = 0; t0 = now_us(); btree_range(bt, lo, hi);           t1 = now_us();
            br_sum += t1-t0;  br_ops_sum  += g_op_count;

            g_op_count = 0; t0 = now_us(); bptree_range(bpt, lo, hi);         t1 = now_us();
            bpr_sum += t1-t0; bpr_ops_sum += g_op_count;
        }

        sl[si]      = ls_sum  / ITERS;  sl_ops[si]  = ls_ops_sum  / ITERS;
        sb[si]      = bt_sum  / ITERS;  sb_ops[si]  = bt_ops_sum  / ITERS;
        sbp[si]     = bpt_sum / ITERS;  sbp_ops[si] = bpt_ops_sum / ITERS;
        rl[si]      = lr_sum  / ITERS;  rl_ops[si]  = lr_ops_sum  / ITERS;
        rb[si]      = br_sum  / ITERS;  rb_ops[si]  = br_ops_sum  / ITERS;
        rbp[si]     = bpr_sum / ITERS;  rbp_ops[si] = bpr_ops_sum / ITERS;

        printf("  [단일] 선형=%6lld μs (%lld회)  B트리=%6lld μs (%lld회)  B+트리=%6lld μs (%lld회)\n",
               sl[si], sl_ops[si], sb[si], sb_ops[si], sbp[si], sbp_ops[si]);
        printf("  [범위] 선형=%6lld μs (%lld회)  B트리=%6lld μs (%lld회)  B+트리=%6lld μs (%lld회)\n",
               rl[si], rl_ops[si], rb[si], rb_ops[si], rbp[si], rbp_ops[si]);

        btree_free(bt);
        bptree_free(bpt);
        free(table);
    }

    print_sep();
    write_json(out_path,
               sl, sl_ops, sb, sb_ops, sbp, sbp_ops,
               rl, rl_ops, rb, rb_ops, rbp, rbp_ops);
    return 0;
}
