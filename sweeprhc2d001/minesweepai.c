#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

#define FORESIGHT 6
#define DEFN 64
#define W DEFN
#define H DEFN
#define NG DEFN
#define RLEN W + 1
#define BSIZE (W * H)
#define BUFF (H * RLEN)

#define NORTH 0
#define SOUTH H - 1
#define EAST W - 1
#define WEST 0

#define AIDAT_DIR "C:\\aidat\\"
#define ENVIRONMENT_FN "environment"
#define PERCEPT_FN "percept"
#define SCORE_FN "score"
#define FN_INC "%03d"
#define FN_EXT ".dat"

#define OPEN 1000
#define MINE 2000
#define QERY 3000
#define FLAG 4000
#define WRNG 5000
#define EXPL 6000

#define OSYM '-'
#define MSYM '@'
#define QSYM '+'
#define FSYM 'F'
#define WSYM 'X'
#define XSYM '#'
#define UNKN '?'

void save(char* fn, int g[][W], int inc);
void savescore(void);
int survey(Coord);

typedef struct { int r, c; } Coord;
int rr() { return rand() % H; }
int rc() { return rand() % W; }

Coord rl() { 
    Coord rloc;
    rloc.r = rr();
    rloc.c = rc();
    return rloc;
}

Coord climbhill(Coord loc);
typedef struct { int n, s, w, e; } Bounds;

Bounds findBounds(Coord loc) {
    Bounds b;
    int checkn = loc.r - 1;
    int checks = loc.r + 1;
    int checkw = loc.c - 1;
    int checke = loc.c + 1;
    b.n = (checkn >= NORTH) ? checkn : NORTH;
    b.s = (checks <= SOUTH) ? checks : SOUTH;
    b.w = (checkw >= WEST) ? checkw : WEST;
    b.e = (checke <= EAST) ? checke : EAST;
    return b;
}

int environment[H][W] = { {0} };
int percept[H][W] = { {0} };
int difficulty = DEFN;
int gamecount = 0;
int score = 0;
char view[H][W + 1];

int rint(int min, int max) { return rand() % (max - min + 1) + min; }

char trans(int l) {
    if (l >= 0 && l <= 9) return '0' + l;
    switch (l) {
    case OPEN: return OSYM;
    case MINE: return MSYM;
    case QERY: return QSYM;
    case FLAG: return FSYM;
    case WRNG: return WSYM;
    case EXPL: return XSYM;
    default: return UNKN;
    }
}

void genboard() { for (int i = 0; i < difficulty; i++) { environment[rint(NORTH, SOUTH)][rint(WEST, EAST)] = MINE; } }

int count(int target) {
    int res = 0;
    for (int r = 0; r < H; r++) {
        for (int c = 0; c < W; c++) {
            if (environment[r][c] == target) res++;
        }
    }
    return res;
}

Coord* find(int target) {
    int n = count(target), i = 0;
    Coord* locs = malloc(n * sizeof(Coord));
    if (!locs) { perror("find fail"); exit(1); }
    for (int r = 0; r < H; r++)
        for (int c = 0; c < W; c++)
            if (percept[r][c] == target) {
                Coord l = { r, c };
                locs[i] = l;
                i++;
            }
    return locs;
}

bool isfirstmove() { return (score == 0) ? true : false; }

Coord choose() { return climbhill(rl()); }

Coord climbhill(Coord loc) {
    Coord nextstep = loc;
    Bounds searcharea = findBounds(loc);
    for (int i = 0; i < FORESIGHT; i++) {
        int laty = rint(searcharea.n, searcharea.s);
        int longx = rint(searcharea.w, searcharea.e);
        Coord searchloc = { laty, longx };
        if (survey(searchloc) < survey(nextstep)) nextstep = searchloc;
    }
    return nextstep;
}

int survey(Coord loc) {
    int minecount = 0;
    Bounds surveyarea = findBounds(loc);
    for (int laty = surveyarea.n; laty <= surveyarea.s; laty++)
        for (int longx = surveyarea.w; longx <= surveyarea.e; longx++)
            if (environment[laty][longx] == MINE) minecount++;
    return minecount;
}

int reveal(Coord loc) {
    if (environment[loc.r][loc.c] == MINE) { percept[loc.r][loc.c] = EXPL; }
    else {
        Bounds surveyarea = findBounds(loc);
        for (int laty = surveyarea.n; laty <= surveyarea.s; laty++) 
            for (int longx = surveyarea.w; longx <= surveyarea.e; longx++) {
                Coord coord = { laty, longx };
                percept[laty][longx] = survey(coord);
            }
    }
    return environment[loc.r][loc.c];
}

void newgame() {
    for (int r = 0; r < H; r++)
        for (int c = 0; c < W; c++)
            environment[r][c] = 0;
    for (int r = 0; r < H; r++)
        for (int c = 0; c < W; c++)
            percept[r][c] = 0;
    score = 0;
    difficulty++;
    genboard();
}

void play() {
    for (gamecount = 0; gamecount < NG; gamecount++) {
        newgame();
        bool safe = true;
        while (safe) {
            Coord q = choose();
            if (reveal(q) == MINE) safe = false;
            score++;
        }
        save(ENVIRONMENT_FN, environment, gamecount);
        save(PERCEPT_FN, percept, gamecount);
        savescore();
    }
}

void save(char* fn, int g[][W], int inc) {
    char path[BUFF];
    sprintf(path, AIDAT_DIR "%s" FN_INC FN_EXT, fn, inc);
    FILE* f = fopen(path, "w");
    if (f == NULL) { perror("save fail"); exit(1); }
    for (int r = 0; r < H; r++) {
        for (int c = 0; c < W; c++) {
            char ch = trans(g[r][c]);
            fputc(ch, f);
        }
        fputc('\n', f);
    }
    fclose(f);
}

void savescore() {
    char fname[BUFF];
    sprintf(fname, AIDAT_DIR "%s%s", SCORE_FN, FN_EXT);
    FILE* f = fopen(fname, "a");
    if (f == NULL) { perror("savescore fail"); exit(1); }
    fprintf(f, "Game %d: %d\n", gamecount, score);
    fclose(f);
}

int main(int argc, char* argv[]) {
    srand((unsigned int)time(NULL));
    play();
    return 0;
}


// TODO: Reduce time complexity with hash tables