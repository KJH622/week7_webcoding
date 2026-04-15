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

int main(int argc, char *argv[]) {
    int n = 1000000;
    if (argc >= 2) n = atoi(argv[1]);

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

    printf("생성 완료. 샘플 출력 (처음 5개):\n");
    printf("%-10s %-24s %-8s %s\n", "ID", "Name", "Score", "Tier");
    for (int i = 0; i < 5; i++) {
        printf("%-10d %-24s %-8d %s\n",
               table[i].id, table[i].name, table[i].score, table[i].tier);
    }

    free(table);
    return 0;
}
