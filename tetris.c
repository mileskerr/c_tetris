#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>

const int HEIGHT = 20;

void enable_raw();
void disable_raw();
void vfreq(float freq);

struct mino {
    uint32_t tiles;
    uint8_t x;
    uint8_t y;
};

void drw_screen(uint16_t board[HEIGHT], struct mino t1);
int ok(uint16_t board[HEIGHT], struct mino t1);
int ipcmp(char a[3], const char b[3]);
void solidify(uint16_t * board, struct mino t1);
struct mino next_piece();

int main() {
    //init
    struct mino t1;
    t1 = next_piece();

    uint16_t board[HEIGHT];
    for (int i = 0; i < HEIGHT; i++) {
        board[i] = 0;
    }

    enable_raw();
    vfreq(30);

    char c;
    char buffer[3];
    const char RIGHT[3] = { 67,91,27 };
    const char LEFT[3] = { 68,91,27 };

    double delta_time = 0.3;

    clock_t drop_time = clock();
    while (1) {
        char c = '\0';
        read(STDIN_FILENO, &c, 1);
        struct mino t2 = t1;
        int change = 0;
        if (c == 'q') {
            break;
        } else if ( c != '\0' ) {
            for (int i = 2; i > 0; i--) {
                buffer[i] = buffer[i-1];
            }
            buffer[0] = c;
            /*for (int i = 0; i < 3; i++) {
                printf("%d\n\r",buffer[i]);
            }*/
            if (ipcmp(buffer,RIGHT)) {
                t2.x ++;
                change = 1;
            }
            if (ipcmp(buffer,LEFT)) {
                t2.x --;
                change = 1;
            }
        }
        double elapsed = ((double)clock() - drop_time)/CLOCKS_PER_SEC;
        if (elapsed > delta_time) {
            drop_time = clock();
            t2.y ++;
            change = 1;
        }
        if (change) {
            if (ok(board,t2)) {
                t2 = t1;
                t2.y ++;
                if (ok(board,t2)) {
                    solidify(board,t2);
                    t1 = next_piece();
                }
            } else {
                t1 = t2;
                drw_screen(board,t1);
                printf("\n\r");
            }
        }
    }
    disable_raw();

    return 0;
}

struct mino next_piece() {
    struct mino t1;
    t1.tiles =
        (0b0110 << 12) |
        (0b0010 << 8) |
        (0b0011 << 4) |
        (0b0000);
    t1.x = 1;
    t1.y = 3;
    return t1;
}


int ipcmp(char a[3], const char b[3]) {
    for (int i = 0; i < 3; i++) {
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}

void solidify(uint16_t * board, struct mino t1) {
    for (int y = 0; y < 4; y++) {
        int shift = (3-y) * 4;
        board[y + t1.y - 1] |= (((t1.tiles & (0b1111 << shift)) >> shift) << (t1.x));
    }
}

int ok(uint16_t board[HEIGHT], struct mino t1) {
    if (t1.x > 16) { return 1; };
    for (int y = 0; y < HEIGHT+4; y++) {
        if (y >= t1.y && y < t1.y + 4) {
            int shift = (3-(y-t1.y)) * 4;
            if ((0b111111 << 10) & (((t1.tiles & (0b1111 << shift)) >> shift) << (t1.x))) {
                return 1;
            } else if ((y >= HEIGHT) && (t1.tiles & (0b1111 << shift))) {
                return 1;
            } else if (y < HEIGHT) {
                if (board[y] & ((t1.tiles & (0b1111 << shift)) >> shift) << (t1.x)) {
                    return 1;
                }
            }
        }
    }
    return 0;
}
void drw_screen(uint16_t board[HEIGHT], struct mino t1) {
    for (int y = 0; y < HEIGHT; y++) {
        uint16_t dyn = 0;
        if (y >= t1.y && y < t1.y + 4) {
            int shift = (3-(y-t1.y)) * 4;
            dyn = ((t1.tiles & (0b1111 << shift)) >> shift) << (t1.x);
        }
        for (int x = 0; x < 10; x++) {
            printf("%d", ((board[y] | dyn) & 1 << x) >> x);
        }
        printf("\n\r");
    }
}

struct termios def_term;

void enable_raw() {
    tcgetattr(STDIN_FILENO, &def_term);
    atexit(disable_raw);

    struct termios raw = def_term;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN);
    raw.c_cc[VMIN] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
void vfreq(float freq) {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_cc[VTIME] = (1/freq) * 10;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
}
void disable_raw() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &def_term);
}
