#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    int  id;
    char name[32];
    int  score;
    char tier[16];
} Player;

static const char *NAMES[] = {
    "ShadowBlade","NightFury","CrimsonAce","StarlightX","IronWolf",
    "PixelKing","VoidHunter","FrostByte","ThunderX","NeonRift",
    "DarkMatter","CyberGhost","RedStorm","BlueFang","GoldRush",
    "SilverEdge","PurpleRain","OmegaZero","AlphaStrike","BetaForce"
};
#define NAME_COUNT 20

static const char *get_tier(int score) {
    if (score >= 9500) return "Challenger";
    if (score >= 8500) return "Diamond";
    if (score >= 7000) return "Platinum";
    if (score >= 5000) return "Gold";
    return "Silver";
}

/* 사용법: ./bin/datagen [레코드수] [출력경로]
 * 기본값: 레코드수=1000000, 출력경로=data/players_{n}.csv */
int main(int argc, char *argv[]) {
    int n = 1000000;
    if (argc >= 2) n = atoi(argv[1]);

    /* 출력 경로: 인수로 지정하거나 data/players_{n}.csv 자동 생성 */
    char default_path[64];
    snprintf(default_path, sizeof(default_path), "data/players_%d.csv", n);
    const char *out_path = (argc >= 3) ? argv[2] : default_path;

    srand((unsigned)time(NULL));

    Player *table = malloc(sizeof(Player) * n);
    if (!table) { fprintf(stderr, "메모리 부족\n"); return 1; }

    printf("레코드 %d개 생성 중...\n", n);
    for (int i = 0; i < n; i++) {
        table[i].id    = i + 1;
        int ni = (i + rand() % 5) % NAME_COUNT;
        snprintf(table[i].name, 32, "%s_%d", NAMES[ni], i % 9999);
        table[i].score = rand() % 10000;
        strncpy(table[i].tier, get_tier(table[i].score), 16);
    }

    /* CSV 파일 저장 */
    FILE *fp = fopen(out_path, "w");
    if (!fp) {
        fprintf(stderr, "파일 열기 실패: %s\n", out_path);
        free(table);
        return 1;
    }

    /* 헤더 */
    fprintf(fp, "id,name,score,tier\n");

    /* 레코드 출력 */
    for (int i = 0; i < n; i++) {
        fprintf(fp, "%d,%s,%d,%s\n",
                table[i].id, table[i].name, table[i].score, table[i].tier);
    }
    fclose(fp);

    printf("저장 완료: %s (%d건)\n", out_path, n);

    /* 샘플 5개 콘솔 출력 */
    printf("\n샘플 (처음 5개):\n");
    printf("%-10s %-24s %-8s %s\n", "ID", "Name", "Score", "Tier");
    for (int i = 0; i < 5 && i < n; i++) {
        printf("%-10d %-24s %-8d %s\n",
               table[i].id, table[i].name, table[i].score, table[i].tier);
    }

    free(table);
    return 0;
}
