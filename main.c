#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <ncurses.h>


#define BLANK 0

#define FRAME_DELAY 55

#define COLOR_PAIR_DEFAULT 10

int board[4][4];

int score = 0;

bool won = false;

enum Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

enum MoveType {
    NOMOVE,
    MOVE,
    MERGE,
    WIN
};

void delay(int ms) {
    /* delay the program by a given number of ms*/
    clock_t start_time = clock();
 
    while (clock() < start_time + (ms * 1000))
        ;
}

void draw_grid() {
    /* draw the gridlines of the board */

    attron(COLOR_PAIR(COLOR_PAIR_DEFAULT));
    move(0, 0);
    printw("                                 ");
    move(0, 0);
    printw("Score: %d", score);
    // horizontal gridlines
    for (int a = 0; a < 5; a++) {
        move(a * 4 + 1, 0);
        int left_piece = ACS_LTEE;
        int middle_piece = ACS_PLUS;
        int right_piece = ACS_RTEE;
        if (a == 0) { // top row
            left_piece = ACS_ULCORNER;
            middle_piece = ACS_TTEE;
            right_piece = ACS_URCORNER;
        } else if (a == 4) { // bottom row
            left_piece = ACS_LLCORNER;
            middle_piece = ACS_BTEE;
            right_piece = ACS_LRCORNER;
        }
        addch(left_piece);
        for (int n = 0; n < 3; n++) {
            for (int m = 0; m < 7; m++) addch(ACS_HLINE); addch(middle_piece);
        }
        for (int n = 0; n < 7; n++) addch(ACS_HLINE); addch(right_piece);

        // vertical gridlines
        if (a != 4) {
            for (int n = 1; n < 4; n++) {
                for (int m = 0; m < 5; m++) {
                    move(a * 4 + n + 1, m * 8);
                    addch(ACS_VLINE);
                }
            }
        }
    }
}


void _addstr_centered(int tile_value) {
    /* draw the tile number so that it's centered */
    int length = 1;
    if (tile_value != 0) {
        // calculate length of integer
        length = log10(tile_value) + 1;
    }
    switch (length) {
        case 1:
            printw("   %d   ", tile_value);
            break;
        case 2:
            printw("  %d   ", tile_value);
            break;
        case 3:
            printw("  %d  ", tile_value);
            break;
        case 4:
            printw(" %d  ", tile_value);
            break;
    }
}

void display_board() {
    /* display the board */
    clear();
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int tile_value = board[i][j];
            int color_pair = COLOR_PAIR_DEFAULT;
            if (tile_value != 0) {
                color_pair = (int)log2(tile_value);
            }
            attron(COLOR_PAIR(color_pair));
            move(i * 4 + 2, j * 8 + 1);
            addstr("       ");
            move(i * 4 + 3, j * 8 + 1);
            _addstr_centered(tile_value);
            move(i * 4 + 4, j * 8 + 1);
            addstr("       ");
        }
    }
    draw_grid();
    refresh();
}

void place_random() {
    /* place a "2" (90% chance) or a "4" (10% chance) somewhere random on the board */
    int possible_positions[16][2];
    int n = 0;
    // collect all free squares
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (board[i][j] == BLANK) {
                possible_positions[n][0] = i;
                possible_positions[n][1] = j;
                n++;
            }
        }
    }
    // choose one of the free squares at random
    int rindex = rand() % n;
    // choose a 2 or a 4
    int rnum = rand() % 10;
    int num;
    if (rnum == 0) {
        num = 4;
    } else {
        num = 2;
    }

    board[possible_positions[rindex][0]][possible_positions[rindex][1]] = num;
}

void reset_board() {
    /* reset everything */
    score = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            board[i][j] = BLANK;
        }
    }
    place_random();
    place_random();
}

bool game_lost() {
    /* check if the game is lost */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int tile_value = board[i][j];
            if ((tile_value == BLANK) || 
                (i != 3 && board[i + 1][j] == tile_value) ||
                (j != 3 && board[i][j+1] == tile_value)) {
                return false;
            }
        }
    }
    return true;
}

enum MoveType move_tile(int row, int column, enum Direction direction, bool dont_merge) {
    /* attempt to move a tile in a direction, and return how it moved */
    int tile_value = board[row][column];
    if (tile_value == BLANK) {
        return NOMOVE;
    }
    int next_row = row;
    int next_column = column;
    switch (direction) {
        case RIGHT:
            next_column = column + 1;
            break;
        case LEFT:
            next_column = column - 1;
            break;
        case UP:
            next_row = row - 1;
            break;
        case DOWN:
            next_row = row + 1;
            break;
    }
    int next_value = board[next_row][next_column];
    if (next_value == BLANK) {
        board[row][column] = BLANK;
        board[next_row][next_column] = tile_value;
        return MOVE;
    }
    else if (next_value == tile_value && !dont_merge) {
        board[row][column] = BLANK;
        int new_value = tile_value << 1; // lshift just for fun 
        if (new_value == 2048) {
            won = true;
            return WIN;
        }
        board[next_row][next_column] = new_value;
        score += new_value;
        return MERGE;
    } else {
        return NOMOVE;
    }
}

int _tile_index(int row, int column) {
    /* map a tile's coords to a single index number */
    return (row << 2) | column;
}
bool move_board(enum Direction direction) {
    /* move every tile on the board in a direction */
    int row_start = 0;
    int row_increment = 1;
    int row_end = 4;
    int column_start = 0;
    int column_increment = 1;
    int column_end = 4;
    switch (direction) {
        case LEFT: 
            column_start = 1;
            break;
        case RIGHT: 
            column_start = 2;
            column_increment = -1;
            column_end = -1;
            break;
        case UP: 
            row_start = 1;
            break;
        case DOWN: 
            row_start = 2;
            row_increment = -1;
            row_end = -1;
            break;
    }

    bool merged[16] = {false}; // keeps track of which tiles have been merged (via index)
    int tile_index;
    bool moved = false;
    enum MoveType move_type;
    for (int s = 0; s < 3; s++) { // tiles can move at max 3 spaces in one move
        for (int i = row_start; i != row_end; i += row_increment) {
            for (int j = column_start; j != column_end; j += column_increment) {
                tile_index = _tile_index(i, j);
                move_type = move_tile(i, j, direction, merged[tile_index]);
                if (move_type == WIN) {
                    return true;
                }
                else if (move_type == MERGE) {
                    // blocks the tile from being merged a second time 
                    merged[tile_index] = true;
                    // also blocks off the next tile
                    int next_row = i + ((direction == UP || direction == DOWN) ? row_increment * -1 : 0);
                    int next_column = j + ((direction == RIGHT || direction == LEFT) ? column_increment * -1 : 0);
                    int next_tile_index = _tile_index(next_row, next_column);
                    merged[next_tile_index] = true;
                }
                else if (move_type != NOMOVE) {
                    moved = true;
                }
            }
        }
        delay(FRAME_DELAY);
        display_board();
    }
    return moved;
}

void init_color_255 (int colornum, int r, int g, int b) {
    /* same thing as init_color but using 0-255 instead of 0-1000 */
    init_color(colornum, (r/255.0)*1000, (g/255.0)*1000, (b/255.0)*1000);
}

int main() {
    // initialize ncurses
    initscr();
    keypad(stdscr, TRUE);
    start_color();

    // set color values 
    // start at 8 as not to interfere with any of the default color codes
    init_color_255(8, 238, 227, 218);  // 2
    init_color_255(9, 238, 224, 201);  // 4
    init_color_255(10, 243, 178, 121); // 8
    init_color_255(11, 246, 149, 99);  // 16
    init_color_255(12, 247, 124, 95);  // 32
    init_color_255(13, 246, 94, 58);   // 64
    init_color_255(14, 237, 208, 115); // 128
    init_color_255(15, 237, 204, 97);  // 256
    init_color_255(16, 237, 199, 80);  // 512
    init_color_255(17, 237, 197, 62);  // 1024
    init_color_255(18, 190, 175, 157); // background

    // set color pairs
    for (int i = 1; i < 10; i++) {
        init_pair(i, COLOR_BLACK, i+7);
    }
    init_pair(COLOR_PAIR_DEFAULT, COLOR_BLACK, 18);
    attron(A_BOLD);
    attron(COLOR_PAIR(COLOR_PAIR_DEFAULT));

    // initialize the rand() function
    time_t t;
    srand((unsigned) time(&t));

    // set up the board 
    reset_board();
    while (true) {
        display_board();
        int moved = false;

        int ch = getch();
        switch (ch) {
            case KEY_UP:
                moved = move_board(UP);
                break;
            case KEY_DOWN:
                moved = move_board(DOWN);
                break;
            case KEY_LEFT:
                moved = move_board(LEFT);
                break;
            case KEY_RIGHT:
                moved = move_board(RIGHT);
                break;
            case 'r':
                reset_board();
            default:
                continue;
        }
        if (moved) {
            place_random();
        }
        if (game_lost() || won) {
            break;
        }
    }
    endwin();
    if (won) {
        printf("Congratulations nerd, you won! Final Score: %d\n", score);
    } else {
        printf("You Lost! Final Score: %d\n", score);
    }
    return 1;
}
