#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int tokens[4][4];
int turn = 0;
int dice = 0;
int six_count = 0;
int winner = -1;

const int OFFSETS[4] = {0, 13, 26, 39};
const int SAFE_TILES[8] = {0, 8, 13, 21, 26, 34, 39, 47};

int get_global_pos(int player, int rel_pos) {
    if (rel_pos < 0 || rel_pos > 50) return -1;
    return (rel_pos + OFFSETS[player]) % 52;
}

int is_safe(int global_pos) {
    for (int i = 0; i < 8; i++) {
        if (SAFE_TILES[i] == global_pos) return 1;
    }
    return 0;
}

void save_state(const char *msg) {
    FILE *f = fopen("state.txt", "w");
    if (!f) return;
    fprintf(f, "%d %d %d %d\n", turn, dice, six_count, winner);
    for (int p = 0; p < 4; p++) {
        fprintf(f, "%d %d %d %d\n", tokens[p][0], tokens[p][1], tokens[p][2], tokens[p][3]);
    }
    fprintf(f, "%s\n", msg);
    fclose(f);
}

void load_state() {
    FILE *f = fopen("state.txt", "r");
    if (!f) return;
    fscanf(f, "%d %d %d %d", &turn, &dice, &six_count, &winner);
    for (int p = 0; p < 4; p++) {
        fscanf(f, "%d %d %d %d", &tokens[p][0], &tokens[p][1], &tokens[p][2], &tokens[p][3]);
    }
    fclose(f);
}

void reset_game() {
    for (int p = 0; p < 4; p++) {
        for (int t = 0; t < 4; t++) {
            tokens[p][t] = -1;
        }
    }
    turn = 0;
    dice = 0;
    six_count = 0;
    winner = -1;
    save_state("New Game Started! Red's turn to roll.");
}

int can_token_move(int p, int t, int d) {
    int pos = tokens[p][t];
    if (pos == -1) return (d == 6);
    if (pos + d <= 56) return 1;
    return 0;
}

void next_turn(const char *prefix_msg) {
    char *names[] = {"Red", "Green", "Yellow", "Blue"};
    char full_msg[150];
    turn = (turn + 1) % 4;
    dice = 0;
    six_count = 0;
    sprintf(full_msg, "%s %s's turn to roll.", prefix_msg, names[turn]);
    save_state(full_msg);
}

void roll_dice() {
    load_state();
    if (winner != -1 || dice != 0) return;

    char *names[] = {"Red", "Green", "Yellow", "Blue"};
    char msg[150];

    dice = (rand() % 6) + 1;

    if (dice == 6) {
        six_count++;
        if (six_count == 3) {
            sprintf(msg, "%s rolled three 6s in a row! Turn forfeited.", names[turn]);
            next_turn(msg);
            return;
        }
    } else {
        six_count = 0;
    }

    int valid_moves = 0;
    for (int t = 0; t < 4; t++) {
        if (can_token_move(turn, t, dice)) valid_moves++;
    }

    if (valid_moves == 0) {
        sprintf(msg, "%s rolled %d (No valid moves).", names[turn], dice);
        next_turn(msg);
    } else {
        sprintf(msg, "%s rolled %d! Click a glowing token to move.", names[turn], dice);
        save_state(msg);
    }
}

void move_token(int t) {
    load_state();
    if (winner != -1 || dice == 0 || t < 0 || t > 3) return;
    if (!can_token_move(turn, t, dice)) return;

    char *names[] = {"Red", "Green", "Yellow", "Blue"};
    char msg[150];
    int captured = 0;

    if (tokens[turn][t] == -1 && dice == 6) {
        tokens[turn][t] = 0;
    } else {
        tokens[turn][t] += dice;
    }

    int new_pos = tokens[turn][t];
    int global_pos = get_global_pos(turn, new_pos);
    
    if (global_pos != -1 && !is_safe(global_pos)) {
        for (int opp = 0; opp < 4; opp++) {
            if (opp == turn) continue;
            for (int ot = 0; ot < 4; ot++) {
                int opp_global = get_global_pos(opp, tokens[opp][ot]);
                if (opp_global == global_pos) {
                    tokens[opp][ot] = -1;
                    captured = 1;
                }
            }
        }
    }

    int finished = 0;
    for (int i = 0; i < 4; i++) {
        if (tokens[turn][i] == 56) finished++;
    }
    if (finished == 4) {
        winner = turn;
        dice = 0;
        sprintf(msg, "🏆 GAME OVER! %s Wins the Match! 🏆", names[turn]);
        save_state(msg);
        return;
    }

    int rolled_six = (dice == 6);
    dice = 0;

    if (captured) {
        sprintf(msg, "⚔️ %s captured an opponent! Bonus roll!", names[turn]);
        save_state(msg);
    } else if (new_pos == 56) {
        sprintf(msg, "🎯 %s got a token Home! Bonus roll!", names[turn]);
        save_state(msg);
    } else if (rolled_six) {
        sprintf(msg, "🎲 %s rolled a 6! Bonus roll!", names[turn]);
        save_state(msg);
    } else {
        next_turn("");
    }
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    if (argc < 2) return 0;
    if (argv[1][0] == 'r') reset_game();
    else if (argv[1][0] == 'd') roll_dice();
    else if (argv[1][0] == 'm' && argc >= 3) move_token(atoi(argv[2]));
    return 0;
}
