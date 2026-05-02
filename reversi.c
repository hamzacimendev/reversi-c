//230101031 Hamza Çimen
//230101012 Ömer Abay

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#define BOARD_SIZE    8
#define ACCOUNTS_FILE "accounts.dat"
#define SAVES_FILE    "saved_games.dat"
#define MAX_USERNAME  50
#define MAX_ATTEMPTS  3

#define EMPTY  0
#define BLACK  1
#define WHITE  2


typedef struct {
    char username[MAX_USERNAME];
    unsigned long pin_hash;
    int games_won;
    int games_lost;
} UserRecord;

typedef struct {
    int game_id;
    char username[MAX_USERNAME];
    int board[BOARD_SIZE][BOARD_SIZE];
    int current_turn;
    unsigned long checksum;
} GameState;


UserRecord g_current_user;


static void read_line(char *buf, int size) {
    if (fgets(buf, size, stdin)) {
        buf[strcspn(buf, "\n")] = '\0';
    } else {
        buf[0] = '\0';
    }
}


static int read_int(void) {
    char buf[32];
    read_line(buf, sizeof(buf));
    if (buf[0] == '\0') return -1;
    char *end;
    long val = strtol(buf, &end, 10);
    /* Butun karakterler sayi olmali */
    if (end == buf || *end != '\0') return -1;
    return (int)val;
}

//djb2 Hash Fonksiyonu
unsigned long djb2(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char)*str++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}


unsigned long compute_checksum(const GameState *gs) {
    char buf[4096];
    char tmp[64];
    buf[0] = '\0';

    sprintf(tmp, "%d", gs->game_id);   strcat(buf, tmp);
    strcat(buf, gs->username);

    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++) {
            sprintf(tmp, "%d", gs->board[i][j]);
            strcat(buf, tmp);
        }

    sprintf(tmp, "%d", gs->current_turn); strcat(buf, tmp);
    return djb2(buf);
}


void init_board(int board[BOARD_SIZE][BOARD_SIZE]) {
    memset(board, 0, sizeof(int) * BOARD_SIZE * BOARD_SIZE);
    board[3][3] = WHITE;
    board[3][4] = BLACK;
    board[4][3] = BLACK;
    board[4][4] = WHITE;
}

void print_board(int board[BOARD_SIZE][BOARD_SIZE]) {
    printf("\n   ");
    for (int j = 0; j < BOARD_SIZE; j++) printf(" %d", j + 1);
    printf("\n");
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf(" %c ", 'A' + i);
        for (int j = 0; j < BOARD_SIZE; j++) {
            char c = '.';
            if      (board[i][j] == BLACK) c = 'B';
            else if (board[i][j] == WHITE) c = 'W';
            printf(" %c", c);
        }
        printf("\n");
    }
    printf("\n");
}

void count_pieces(int board[BOARD_SIZE][BOARD_SIZE], int *black, int *white) {
    *black = 0; *white = 0;
    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++) {
            if      (board[i][j] == BLACK) (*black)++;
            else if (board[i][j] == WHITE) (*white)++;
        }
}


int dr[] = {-1,-1,-1, 0, 0, 1, 1, 1};
int dc[] = {-1, 0, 1,-1, 1,-1, 0, 1};

int check_direction(int board[BOARD_SIZE][BOARD_SIZE], int row, int col,
                    int d, int player, int flip) {
    int opponent = (player == BLACK) ? WHITE : BLACK;
    int r = row + dr[d], c = col + dc[d];
    int count = 0;

    if (r < 0 || r >= BOARD_SIZE || c < 0 || c >= BOARD_SIZE) return 0;
    if (board[r][c] != opponent) return 0;

    while (r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE
           && board[r][c] == opponent) {
        count++;
        r += dr[d]; c += dc[d];
    }

    if (r < 0 || r >= BOARD_SIZE || c < 0 || c >= BOARD_SIZE) return 0;
    if (board[r][c] != player) return 0;

    if (flip) {
        int fr = row + dr[d], fc = col + dc[d];
        for (int k = 0; k < count; k++) {
            board[fr][fc] = player;
            fr += dr[d]; fc += dc[d];
        }
    }
    return count;
}

int is_valid_move(int board[BOARD_SIZE][BOARD_SIZE], int row, int col, int player) {
    if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) return 0;
    if (board[row][col] != EMPTY) return 0;
    for (int d = 0; d < 8; d++)
        if (check_direction(board, row, col, d, player, 0) > 0) return 1;
    return 0;
}

int place_piece(int board[BOARD_SIZE][BOARD_SIZE], int row, int col, int player) {
    if (!is_valid_move(board, row, col, player)) return 0;
    board[row][col] = player;
    for (int d = 0; d < 8; d++)
        check_direction(board, row, col, d, player, 1);
    return 1;
}

int has_valid_move(int board[BOARD_SIZE][BOARD_SIZE], int player) {
    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            if (is_valid_move(board, i, j, player)) return 1;
    return 0;
}

//Greedy AI
void ai_move(int board[BOARD_SIZE][BOARD_SIZE]) {
    int best_row = -1, best_col = -1, best_flips = -1;

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (!is_valid_move(board, i, j, WHITE)) continue;
            int total = 0;
            for (int d = 0; d < 8; d++)
                total += check_direction(board, i, j, d, WHITE, 0);
            if (total > best_flips) {
                best_flips = total;
                best_row = i; best_col = j;
            }
        }
    }

    if (best_row != -1) {
        printf("  Computer played %c%d.\n", 'A' + best_row, best_col + 1);
        place_piece(board, best_row, best_col, WHITE);
    }
}


int account_exists(const char *username) {
    FILE *f = fopen(ACCOUNTS_FILE, "rb");
    if (!f) return 0;
    UserRecord ur;
    while (fread(&ur, sizeof(UserRecord), 1, f) == 1)
        if (strcmp(ur.username, username) == 0) { fclose(f); return 1; }
    fclose(f); return 0;
}

void register_user(void) {
    char username[MAX_USERNAME];
    char pin_str[16];

    printf("\n=== Create New Account ===\n");
    printf("Username: ");
    read_line(username, sizeof(username));

    if (strlen(username) == 0) { printf("Username cannot be empty!\n"); return; }

    if (account_exists(username)) {
        printf("This username is already taken!\n");
        return;
    }

    printf("4-digit PIN: ");
    read_line(pin_str, sizeof(pin_str));

    if (strlen(pin_str) != 4) {
        printf("PIN must be exactly 4 digits!\n");
        return;
    }

    UserRecord ur;
    memset(&ur, 0, sizeof(ur));
    strncpy(ur.username, username, MAX_USERNAME - 1);
    ur.pin_hash   = djb2(pin_str);
    ur.games_won  = 0;
    ur.games_lost = 0;

    FILE *f = fopen(ACCOUNTS_FILE, "ab");
    if (!f) { printf("Could not open file!\n"); return; }
    fwrite(&ur, sizeof(UserRecord), 1, f);
    fclose(f);

    printf("Account created! You can now log in.\n");
}

int login_user(void) {
    char username[MAX_USERNAME];
    char pin_str[16];

    printf("\n=== Login ===\n");
    printf("Username: ");
    read_line(username, sizeof(username));

    if (strlen(username) == 0) { printf("Username cannot be empty!\n"); return 0; }

    int attempts = 0;
    while (attempts < MAX_ATTEMPTS) {
        printf("PIN (Attempts left: %d): ", MAX_ATTEMPTS - attempts);
        read_line(pin_str, sizeof(pin_str));

        unsigned long entered_hash = djb2(pin_str);

        FILE *f = fopen(ACCOUNTS_FILE, "rb");
        if (!f) { printf("No accounts found!\n"); return 0; }

        UserRecord ur;
        int found = 0;
        while (fread(&ur, sizeof(UserRecord), 1, f) == 1) {
            if (strcmp(ur.username, username) == 0) {
                found = 1;
                if (ur.pin_hash == entered_hash) {
                    fclose(f);
                    g_current_user = ur;
                    printf("Login successful! Welcome, %s!\n", username);
                    return 1;
                }
                break;
            }
        }
        fclose(f);

        if (!found) { printf("User not found!\n"); return 0; }

        attempts++;
        if (attempts < MAX_ATTEMPTS)
            printf("Wrong PIN! Try again.\n");
    }

    printf("\nWarning: Too many failed attempts! Exiting program.\n");
    exit(1);
}

void update_user_stats(int won) {
    FILE *f = fopen(ACCOUNTS_FILE, "r+b");
    if (!f) return;
    UserRecord ur;
    while (fread(&ur, sizeof(UserRecord), 1, f) == 1) {
        if (strcmp(ur.username, g_current_user.username) == 0) {
            if (won) ur.games_won++;
            else     ur.games_lost++;
            fseek(f, -(long)sizeof(UserRecord), SEEK_CUR);
            fwrite(&ur, sizeof(UserRecord), 1, f);
            g_current_user = ur;
            break;
        }
    }
    fclose(f);
}


void save_game(GameState *gs) {
    gs->checksum = compute_checksum(gs);

    FILE *f = fopen(SAVES_FILE, "r+b");
    if (!f) f = fopen(SAVES_FILE, "wb");
    if (!f) { printf("Could not open save file!\n"); return; }

    GameState tmp;
    int found = 0;
    while (fread(&tmp, sizeof(GameState), 1, f) == 1) {
        if (tmp.game_id == gs->game_id) {
            fseek(f, -(long)sizeof(GameState), SEEK_CUR);
            fwrite(gs, sizeof(GameState), 1, f);
            found = 1;
            break;
        }
    }
    if (!found) fwrite(gs, sizeof(GameState), 1, f);
    fclose(f);
    printf("Game saved. (ID: %d)\n", gs->game_id);
}

void save_as_game(GameState *gs) {
    gs->game_id  = (int)time(NULL) + rand() % 1000;
    gs->checksum = compute_checksum(gs);

    FILE *f = fopen(SAVES_FILE, "ab");
    if (!f) { printf("Could not open save file!\n"); return; }
    fwrite(gs, sizeof(GameState), 1, f);
    fclose(f);
    printf("Game saved with new ID. (ID: %d)\n", gs->game_id);
}

void list_saves(void) {
    FILE *f = fopen(SAVES_FILE, "rb");
    if (!f) { printf("No saved games found.\n"); return; }

    GameState gs;
    int found = 0;
    printf("\n--- Your Saved Games ---\n");
    while (fread(&gs, sizeof(GameState), 1, f) == 1) {
        if (strcmp(gs.username, g_current_user.username) == 0) {
            printf("  GameId: %d, Turn: %s\n",
                   gs.game_id,
                   gs.current_turn == BLACK ? "Black (You)" : "White (Computer)");
            found = 1;
        }
    }
    if (!found) printf("You have no saved games.\n");
    fclose(f);
}

int load_game(GameState *out_gs) {
    list_saves();

    FILE *f = fopen(SAVES_FILE, "rb");
    if (!f) return 0;

    printf("Enter the ID of the game to load (0=cancel): ");
    int target_id = read_int();
    if (target_id <= 0) { fclose(f); return 0; }

    GameState gs;
    while (fread(&gs, sizeof(GameState), 1, f) == 1) {
        if (gs.game_id == target_id &&
            strcmp(gs.username, g_current_user.username) == 0) {

            
            unsigned long saved_cs = gs.checksum;
            gs.checksum = 0;
            unsigned long calc_cs = compute_checksum(&gs);
            gs.checksum = saved_cs;

            if (saved_cs != calc_cs) {
                printf("Error: Save file corrupted or tampered with.\n");
                fclose(f);
                return 0;
            }
            *out_gs = gs;
            fclose(f);
            printf("Game loaded!\n");
            return 1;
        }
    }
    printf("Game not found or does not belong to you.\n");
    fclose(f);
    return 0;
}


int in_game_menu(GameState *gs) {
    printf("\n--- Game Menu ---\n");
    printf("  1. Save\n");
    printf("  2. Save As\n");
    printf("  3. Exit Without Saving\n");
    printf("  4. Continue\n");
    printf("Choice: ");
    int choice = read_int();
    switch (choice) {
        case 1: save_game(gs);    return 0;
        case 2: save_as_game(gs); return 0;
        case 3: return 1;
        default: return 0;
    }
}


void play_game(GameState *gs) {
    int black_count, white_count;

    while (1) {
        print_board(gs->board);
        count_pieces(gs->board, &black_count, &white_count);
        printf("  Black (You): %d  |  White (Computer): %d\n",
               black_count, white_count);

        int human_can = has_valid_move(gs->board, BLACK);
        int ai_can    = has_valid_move(gs->board, WHITE);

        /* Oyun bitis kosulu */
        if (!human_can && !ai_can) {
            printf("\n=== GAME OVER ===\n");
            count_pieces(gs->board, &black_count, &white_count);
            printf("Black: %d  |  White: %d\n", black_count, white_count);
            if      (black_count > white_count) { printf("You win!\n");  update_user_stats(1); }
            else if (white_count > black_count) { printf("You lose.\n"); update_user_stats(0); }
            else                                  printf("Draw!\n");
            return;
        }

        if (gs->current_turn == BLACK) {
            if (!human_can) {
                printf("  No valid move for you. Skipping turn.\n");
                gs->current_turn = WHITE;
                continue;
            }

            printf("  Your turn! Enter move (e.g. D3) or M=Menu: ");
            char input[16];
            read_line(input, sizeof(input));

            //Bos satir: tekrar sor
            if (strlen(input) == 0) continue;

            if (input[0] == 'M' || input[0] == 'm') {
                if (in_game_menu(gs)) return;
                continue;
            }

            if (strlen(input) < 2) { printf("Invalid input! Example: D3\n"); continue; }

            
            int row = (input[0] >= 'a') ? input[0] - 'a' : input[0] - 'A';
            int col = input[1] - '1';

            if (!place_piece(gs->board, row, col, BLACK)) {
                printf("  Invalid move! Try again.\n");
                continue;
            }
            gs->current_turn = WHITE;

        } else {
            if (!ai_can) {
                printf("  Computer has no valid move. Skipping turn.\n");
                gs->current_turn = BLACK;
                continue;
            }
            ai_move(gs->board);
            gs->current_turn = BLACK;
        }
    }
}


void dashboard(void) {
    while (1) {
        printf("\n=============================\n");
        printf("  Welcome, %s!\n", g_current_user.username);
        printf("  Wins: %d  |  Losses: %d\n",
               g_current_user.games_won, g_current_user.games_lost);
        printf("=============================\n");
        printf("  1. New Game\n");
        printf("  2. Load Game\n");
        printf("  3. Logout\n");
        printf("Choice: ");

        int choice = read_int();

        if (choice == 1) {
            GameState gs;
            memset(&gs, 0, sizeof(gs));
            gs.game_id = (int)time(NULL);
            strncpy(gs.username, g_current_user.username, MAX_USERNAME - 1);
            gs.current_turn = BLACK;
            init_board(gs.board);
            printf("\nNew game started! (ID: %d)\n", gs.game_id);
            printf("You are Black (B). Computer is White (W).\n");
            printf("Move: Row letter + Column number (e.g. D3). M=Menu\n");
            play_game(&gs);

        } else if (choice == 2) {
            GameState gs;
            if (load_game(&gs)) {
                printf("Resuming game...\n");
                play_game(&gs);
            }

        } else if (choice == 3) {
            printf("Goodbye!\n");
            return;
        } else {
            printf("Invalid choice!\n");
        }
    }
}


void welcome_screen(void) {
    while (1) {
        printf("\n+============================+\n");
        printf("|   REVERSI (OTHELLO)        |\n");
        printf("+============================+\n");
        printf("  1. Login\n");
        printf("  2. Create Account\n");
        printf("  3. Exit\n");
        printf("Choice: ");

        int choice = read_int();

        if      (choice == 1) { if (login_user()) dashboard(); }
        else if (choice == 2) { register_user(); }
        else if (choice == 3) { printf("Exiting...\n"); return; }
        else                  { printf("Invalid choice!\n"); }
    }
}

int main(void) {
    srand((unsigned)time(NULL));
    welcome_screen();
    return 0;
}