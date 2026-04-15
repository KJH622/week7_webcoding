#include "dataset_io.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *tiers[] = {
    "Bronze",
    "Silver",
    "Gold",
    "Platinum",
    "Diamond"
};

void populate_players(Player *players, int count) {
    int i;

    for (i = 0; i < count; ++i) {
        players[i].id = i + 1;
        snprintf(players[i].name, sizeof(players[i].name), "StarlightX_%d", i % 10000);
        players[i].score = 900 + (i * 37) % 4200;
        snprintf(players[i].tier, sizeof(players[i].tier), "%s", tiers[i % 5]);
    }
}

int write_players_csv(const char *path, int count) {
    FILE *fp;
    Player *players;
    int i;

    if (!path || count <= 0) {
        return 0;
    }

    fp = fopen(path, "w");
    if (!fp) {
        return 0;
    }

    players = (Player *)malloc(sizeof(Player) * count);
    if (!players) {
        fclose(fp);
        return 0;
    }

    populate_players(players, count);
    fprintf(fp, "id,name,score,tier\n");
    for (i = 0; i < count; ++i) {
        fprintf(fp, "%d,%s,%d,%s\n", players[i].id, players[i].name, players[i].score, players[i].tier);
    }

    free(players);
    fclose(fp);
    return 1;
}

Player *load_players_csv(const char *path, int *out_count) {
    FILE *fp;
    Player *players = NULL;
    int capacity = 0;
    int count = 0;
    char line[256];

    if (out_count) {
        *out_count = 0;
    }

    if (!path) {
        return NULL;
    }

    fp = fopen(path, "r");
    if (!fp) {
        return NULL;
    }

    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        return NULL;
    }

    while (fgets(line, sizeof(line), fp)) {
        Player player;
        int parsed;

        parsed = sscanf(line, "%d,%31[^,],%d,%15s", &player.id, player.name, &player.score, player.tier);
        if (parsed != 4) {
            continue;
        }

        if (count == capacity) {
            int next_capacity = capacity == 0 ? 1024 : capacity * 2;
            Player *next = (Player *)realloc(players, sizeof(Player) * next_capacity);
            if (!next) {
                free(players);
                fclose(fp);
                return NULL;
            }
            players = next;
            capacity = next_capacity;
        }

        players[count++] = player;
    }

    fclose(fp);

    if (out_count) {
        *out_count = count;
    }
    return players;
}
