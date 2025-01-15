#include <ncurses/ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MARGIN 1
#define BAR_WIDTH 3
#define TRAY_WIDTH 4
#define HEIGHT 13
#define WIDTH (BAR_WIDTH + 36)
#define MENUX (MARGIN + WIDTH + TRAY_WIDTH + 7)
#define FIELDS 24
#define PAWNS 15
#define DICE 6

#define ZERO_GAME 3
#define ROLL_GAME 5
#define SAVE_GAME 6
#define QUIT_GAME 7
#define TOURINFO 9
#define INFO_GAME 10

#define ZERO_MENU 3
#define PLAY_MENU 5
#define LOAD_MENU 6
#define WATCH_MENU 7
#define QUIT_MENU 8
#define INFO_MENU 11

#define ZERO_WATCH 3
#define START_WATCH 5
#define PREV_WATCH 6
#define NEXT_WATCH 7
#define END_WATCH 8
#define QUIT_WATCH 9
#define INFO_WATCH 12

#define MAXINFO 7
#define MAXPAWNS 5 
#define MAXNAME 32

enum player_t {
    NONE,
    PLAYER_A, 
    PLAYER_B
};

enum menu_t { 
    MENU,
    GAME,
    WIN,
    WATCH
};

enum bar_t {
    BAR_A,
    BAR_B
};

enum color_t {
    NOCOLOR,
    CYAN,
    WHITE,
    RED
};

enum newRoundMode_t { 
    NORMAL,
    NOCHANGE
};

enum checkMove_t { 
    ERROR,
    MOVE,
    BEAT,
    BEAROFF
};

enum checkBarMove_t {
    BAR_ERR,
    BAR_MOVE,
    BAR_NULL
};

enum trayCheck_t {
    TRAY_ERR,
    TRAY_MOVE
};

enum dice_t {
    DICEA,
    DICEB
};

typedef struct {
    int field_number;
    int pawn_number;
    int player;
    int x;
    int y;
} field_t;

typedef struct {
    int who; 
    int mvnumber; 
} round_t;

typedef struct { 
    round_t actual_round;
    field_t fields[FIELDS+1];
    field_t bar[2];
    field_t tray[2];
    int points[2];
    int round;
    int starting_player;
    char playerA[MAXNAME];
    char playerB[MAXNAME];
} game_t;

void sign() { 
    mvprintw(MARGIN, MENUX, "Ostap Lozovyy 197747 - Nardy");
}

void delline(int ycoord) { 
    int i, xmax = getmaxx(stdscr);
    for (i = MENUX; i<=xmax; i++) {
        mvaddch(ycoord, i, ' ');
    }
}

void delinfo() {
    int i;
    for (i=0; i<=MAXINFO; i++){
        delline(MARGIN+INFO_GAME+i);
    }
}

void clearBoard() {
    int i, j;
    for(j = 0; j <= MARGIN+HEIGHT+2; j++) {
        for(i = 0; i < MENUX; i++) {
            mvaddch(j, i, ' ');
        }
    }
}

void runSelectedOption(int selected, int menu_type, game_t *game);
void newRound(game_t *game, field_t *fields, int mode);
void selectWhereMove(game_t *game, int d, int diceno);
int checkMove(game_t *game, int d, int p);
void saveRounds(game_t *game);
void saveGame(game_t *game);

void startPosition(int i, field_t *fields) { 
    switch(i){
        case 1:
            fields[i].player = PLAYER_A;
            fields[i].pawn_number = 2;
            break;
        case 6: case 13:
            fields[i].player = PLAYER_B;
            fields[i].pawn_number = 5;
            break;
        case 8:
            fields[i].player = PLAYER_B;
            fields[i].pawn_number = 3;
            break;
        case 12: case 19:
            fields[i].player = PLAYER_A;
            fields[i].pawn_number = 5;
            break;
        case 17:
            fields[i].player = PLAYER_A;
            fields[i].pawn_number = 3;
            break;
        case 24:
            fields[i].player = PLAYER_B;
            fields[i].pawn_number = 2;
            break;
        default:
            fields[i].player = NONE;
            fields[i].pawn_number = 0;
    }
}

void barInit(game_t *game) { 
    int i;
    for(i=BAR_A; i<=BAR_B; i++){
        game->bar[i].field_number = (1-i)*(FIELDS+1), game->tray[i].field_number = (1-i)*(FIELDS+1);;
        game->bar[i].pawn_number = 0, game->tray[i].pawn_number = 0;
        game->bar[i].player = i, game->tray[i].player = i;
        game->bar[i].x = MARGIN + ceil(WIDTH/2.0); 
        game->tray[i].x = MARGIN + WIDTH + TRAY_WIDTH;
        game->bar[i].y = MARGIN + HEIGHT - (1-i)*(HEIGHT-1);
        game->tray[i].y = MARGIN + HEIGHT - (1-i)*(HEIGHT-1);
    }
}

void extraFieldsInit(field_t *fields) {
    fields[0].x = 0;
    fields[0].y = 0;
    fields[0].pawn_number = 0;
    fields[0].field_number = 0;
    fields[0].player = 0;
}

void fieldsInit(field_t *fields) {
    int i, k=0;
    for(i=1; i<=FIELDS; i++) {
        fields[i].field_number = i;
        if(i>FIELDS/2)
            fields[i].y = MARGIN + 1;
        else 
            fields[i].y = MARGIN + HEIGHT;

        startPosition(i, fields);
    }
    for(i=MARGIN+2; i<MARGIN+WIDTH; i+=BAR_WIDTH) {
        fields[FIELDS/2 - k].x = i;
        fields[FIELDS/2 + k + 1].x = i;
        k++;
        if (k==FIELDS/4)
            i+=BAR_WIDTH; 
    }
    extraFieldsInit(fields);
}

void drawFieldNumbers(field_t *fields) {
    int i;
    for (i=1; i<=FIELDS; i++){
        if(fields[i].field_number <= FIELDS/2){ 
            mvprintw(fields[i].y + 2 , fields[i].x, "%02d", fields[i].field_number);
        }
        else {
            mvprintw(fields[i].y - 2 , fields[i].x, "%02d", fields[i].field_number);
        }
    }
}

void drawPawns(int i, field_t *fields) {
    int j, k, m;
    for(j=1; j<=fields[i].pawn_number; j++){
        if (fields[i].field_number <= FIELDS/2) { 
            k = j*(-1)+1;
            m = -MAXPAWNS;
        }
        else {
            k = j-1;
            m = MAXPAWNS;
        }

        if (j <= MAXPAWNS)
            mvaddch(fields[i].y + k, fields[i].x, 'O');
        else if (j >= 2*MAXPAWNS){
            mvprintw(fields[i].y + m, fields[i].x - 1, " %d", j);
        }
        else 
            mvprintw(fields[i].y + m, fields[i].x - 1,"(%d)", j);
    }
}

void drawTrayPawns(int i, game_t *game) {
    int pn = game->tray[i].pawn_number;
    if(pn <= MAXPAWNS) {
        drawPawns(i, game->tray);
    }
    else {
        game->tray[i].pawn_number = MAXPAWNS;
        drawPawns(i, game->tray);
        if (pn <= 2*MAXPAWNS) {
            game->tray[i].pawn_number = pn - MAXPAWNS;
            game->tray[i].x--;
            drawPawns(i, game->tray);
            game->tray[i].x++;
        }
        else {
            game->tray[i].pawn_number = pn - 2*MAXPAWNS;
            game->tray[i].x -= 2;
            drawPawns(i, game->tray);
            game->tray[i].x++;
            game->tray[i].pawn_number = MAXPAWNS;
            drawPawns(i, game->tray);
            game->tray[i].x++;
        }
        game->tray[i].pawn_number = pn;
    }
}

void drawFields(field_t *fields, game_t *game) {
    int i;
    drawFieldNumbers(fields);
    for(i = 1; i <= FIELDS; i++){
        if(fields[i].player == PLAYER_A){
            attron(COLOR_PAIR(WHITE));
            drawPawns(i, fields);
            attroff(COLOR_PAIR(WHITE));
        }
        if(fields[i].player == PLAYER_B){
            attron(COLOR_PAIR(RED));
            drawPawns(i, fields);
            attroff(COLOR_PAIR(RED));
        }
    }
    for(i = BAR_A; i<=BAR_B; i++){
        attron(COLOR_PAIR(i+2));
        drawPawns(i, game->bar);
        drawTrayPawns(i, game);
        attroff(COLOR_PAIR(i+2));
    }
    attron(COLOR_PAIR(CYAN));
}

void drawVerticalBorder(char ch1, char ch2) {
    addch(ch1);
    int i;
    for(i=0; i<WIDTH+TRAY_WIDTH; i++) {
        addch('=');
    }
    addch(ch2);
}

void drawHorizontalBorder(int x) {
    int i;
    for(i=1; i<=HEIGHT; i++) {
        move(MARGIN + i, x);
        addch('|');
    }
}

void drawBoard(field_t *fields, game_t *game) {
    move(MARGIN, MARGIN);
    drawVerticalBorder('/', '\\');
    move(MARGIN+HEIGHT+1, MARGIN);
    drawVerticalBorder('\\', '/');

    drawHorizontalBorder(MARGIN);
    drawHorizontalBorder(MARGIN+WIDTH+1);
    int barX = MARGIN + (WIDTH-BAR_WIDTH)/2 + 1;
    drawHorizontalBorder(barX);
    drawHorizontalBorder(barX + BAR_WIDTH - 1);
    int barCenterY = MARGIN + ceil(HEIGHT/2.0);
    mvprintw(barCenterY, barX, "BAR");
    drawHorizontalBorder(MARGIN+WIDTH+TRAY_WIDTH+1);
    mvprintw(barCenterY, MARGIN+WIDTH+2, "DWOR");

    drawFields(fields, game);
}

void selectMenu(int lowest, int highest, int menu_type, game_t *game) { 
    int selected = lowest, i;
    mvaddch(selected+MARGIN, MENUX, '>');
    for(i=MARGIN+lowest+1; i<=highest; i++)
        mvaddch(i, MENUX, ' ');
    char key;
    do {
        key = getch();
        switch(key){
        case 'W': case 'w':
            if(selected > lowest) {
                mvaddch(selected+MARGIN, MENUX, ' ');
                mvaddch(--selected+MARGIN, MENUX, '>');
            }
            break;

        case 'S': case 's':
            if(selected < highest) {
                mvaddch(selected+MARGIN, MENUX, ' ');
                mvaddch(++selected+MARGIN, MENUX, '>');
            }
            break;

        case ' ': 
            runSelectedOption(selected, menu_type, game);
            break;
        }
    }
    while(1);
}

int playerDiceRoll(char *gracz, int ycoord) { 
    char r = 0;
    int result;
    mvprintw(ycoord, MENUX, "%s rzuca kostka (R): ", gracz);
    while (r != 'r' && r != 'R'){
        r = getch();
    }
    srand(time(NULL));
    result = rand() % 6 + 1;
    printw("%d", result);
    return result;
}

int firstDiceRollCheck(game_t *game, int resultA, int resultB) { 
    if(resultA > resultB) {
        mvprintw(MARGIN + INFO_GAME + 2, MENUX, "%s zaczyna gre!", game->playerA);
        return PLAYER_A;
    } 
    else if(resultB > resultA) {
        mvprintw(MARGIN + INFO_GAME + 2, MENUX, "%s zaczyna gre!", game->playerB);
        return PLAYER_B;
    }
    else {
        mvprintw(MARGIN + INFO_GAME + 2, MENUX, "Rzuc jeszcze raz");
        return NONE;
    }
}

void firstDiceRoll(game_t *game) {
    int resultA, resultB;
    do {
        delinfo();
        resultA = playerDiceRoll(game->playerA, MARGIN+INFO_GAME);
        resultB = playerDiceRoll(game->playerB, MARGIN+INFO_GAME+1);

        game->starting_player = firstDiceRollCheck(game, resultA, resultB);

        mvprintw(MARGIN + INFO_GAME + 3, MENUX, "Spacja aby kontynuowac ...");
        char space = 0;
        while (space != ' '){
            space = getch();
        }

    } while (resultA == resultB);
}

void newGameInit(game_t *game) { 
    echo();
    curs_set(1);
    sign();
    mvprintw(MARGIN + ZERO_GAME, MENUX, "Podaj nazwe Gracza A: ");
    scanw("%s", game->playerA);
    mvprintw(MARGIN + ZERO_GAME + 1, MENUX, "Podaj nazwe Gracza B: ");
    scanw("%s", game->playerB);

    if(!(game->playerA[0]))
        (*game).playerA[0] = 'A';
    if(!(game->playerB[0]))
        (*game).playerB[0] = 'B';
    
    noecho();
    firstDiceRoll(game);
    curs_set(0);

    game->actual_round.who = game->starting_player;
    game->round = 1;
}

void writeVersus(game_t *game, char *sp){
    move(MARGIN + ZERO_GAME - 1, MENUX);
    attron(COLOR_PAIR(WHITE));
    printw(game->playerA);
    attron(COLOR_PAIR(CYAN));
    printw(" vs ");
    attron(COLOR_PAIR(RED));
    printw(game->playerB);
    attron(COLOR_PAIR(CYAN));
    printw(" (zaczyna %s)", sp);
}

void addWinPoints(game_t *game) { 
    int player = game->actual_round.who, second;
    if(player == PLAYER_A) 
        second = BAR_B;
    else 
        second = BAR_A;
    if(game->bar[second].pawn_number > 0) { 
        game->points[player - 1]+=3;
    }
    else{
        game->points[player - 1]++;
        if(game->tray[second].pawn_number == 0)
            game->points[player - 1]++; 
    }
}

void writeGameWinMenu(game_t *game) { 
    char startingplayer[MAXNAME];
    if (game->starting_player == PLAYER_A)
        strcpy(startingplayer, game->playerA);
    else 
        strcpy(startingplayer, game->playerB);
    sign();
    writeVersus(game, startingplayer);
    mvprintw(MARGIN + ZERO_GAME, MENUX, "Runda %d (%d : %d)", game->round, game->points[BAR_A], game->points[BAR_B]);
    mvprintw(MARGIN + SAVE_GAME, MENUX, "  Zapisz (Auto)");
    mvprintw(MARGIN + QUIT_GAME, MENUX, "  Powrot do menu");
}

void writeGameMenu(game_t *game) {
    writeGameWinMenu(game);
    mvprintw(MARGIN + ROLL_GAME, MENUX, "> Rzuc kostkami");

    selectMenu(ROLL_GAME, QUIT_GAME, GAME, game);
}

void writeWinMenu(game_t *game, char *player) {
    drawBoard(game->fields, game);

    writeGameWinMenu(game);
    mvprintw(MARGIN + ROLL_GAME, MENUX, "> Nowa gra");

    attron(COLOR_PAIR(game->actual_round.who + 1));
    mvprintw(MARGIN + TOURINFO, MENUX, "WYGRYWA: %s", player);
    attron(COLOR_PAIR(CYAN));

    selectMenu(ROLL_GAME, QUIT_GAME, WIN, game);
}

void pawnMove(game_t *game, int d, int p, int type) { 
    int result;
    if (game->actual_round.who == PLAYER_A)
        result = p + d;
    else 
        result = p - d;
    if(!(--game->fields[p].pawn_number)) 
        game->fields[p].player = NONE;
    game->fields[result].player = game->actual_round.who;

    if(type == 1) {
        game->fields[result].pawn_number++;
    }
    if(type == 2) {
        if(game->actual_round.who == PLAYER_A)
            game->bar[BAR_B].pawn_number++;
        else   
            game->bar[BAR_A].pawn_number++;
    }
}

int trayMoveCheck(game_t *game, int d, int p, int diceno) { 
    int i, checker=0;
    if(game->actual_round.who == PLAYER_A){
        for(i=0; i<=FIELDS-DICE; i++) {
            if(game->fields[i].player == PLAYER_A)
                checker++;
        }
    }
    else {
        for(i=FIELDS; i>DICE; i--) {
            if(game->fields[i].player == PLAYER_B)
                checker++;
        }
    }
    return checker;
}

int trayForcedCheckA(game_t *game, int d, int p, int i, int farest, int j) {
    for(i=FIELDS; i>FIELDS-DICE; i--) {
        if(game->fields[i].player == PLAYER_A)
            farest = i;
    }
    if(FIELDS-d+1 >= farest){
        for(j=FIELDS-d+1; j<p; j++){
            if(game->fields[j].player == PLAYER_A)
                return TRAY_ERR;
        }
        if(p>=FIELDS-d+1)
            return TRAY_MOVE;
        else 
            return TRAY_ERR;
    }
    else {
        if(p==farest) 
            return TRAY_MOVE;
        else 
            return TRAY_ERR;
    }
}

int trayForcedCheckB(game_t *game, int d, int p, int i, int farest, int j) {
    for(i=1; i<=DICE; i++) {
        if(game->fields[i].player == PLAYER_B)
            farest = i;
    }
    if(d <= farest){
        for(j=d; j>p; j--){
            if(game->fields[j].player == PLAYER_B)
                return TRAY_ERR;
        }
        if(p<=d)
            return TRAY_MOVE;
        else 
            return TRAY_ERR;
    }
    else {
        if(p==farest) 
            return TRAY_MOVE;
        else 
            return TRAY_ERR;
    }
}

int trayForcedCheck(game_t *game, int d, int p) { 
    int i, j, farest = 0;
    if(game->actual_round.who == PLAYER_B){
        trayForcedCheckB(game, d, p, i, farest, j); 
    }
    else {
        trayForcedCheckA(game, d, p, i, farest, j); 
    }
}

void trayMove(game_t *game, int d, int p, int diceno) {
    if(trayForcedCheck(game, d, p) == TRAY_MOVE){ 
        if(--game->fields[p].pawn_number == 0){
            game->fields[p].player = NONE;
        }
        game->tray[game->actual_round.who - 1].pawn_number++;
    }
    else {
        mvprintw(MARGIN+INFO_GAME+3, MENUX, "Musisz wprowadzic na dwor wlasciwy pionek");
        do {
            mvprintw(MARGIN+INFO_GAME+4, MENUX, "Spacja aby ponowic probe...");
        } while(getch() != ' ');
        delline(MARGIN+INFO_GAME+3);
        delline(MARGIN+INFO_GAME+4);
        selectWhereMove(game, d, diceno);
    }
}

void barMove(game_t *game, int d) { 
    int player = game->actual_round.who, second;
    if(player == PLAYER_A)
        second = BAR_B;
    else 
        second = BAR_A;
    game->bar[player-1].pawn_number--;
    if(game->fields[d].player == player || game->fields[d].player == NONE) {
        game->fields[d].pawn_number++;
    }
    else {
        game->bar[second].pawn_number++;
    }
    game->fields[d].player = player;
}

int ifTrayCheck(game_t *game, int d, int p, int diceno) { 
    int i, checker=0;
    if(trayMoveCheck(game, d, p, diceno) == 0) {
        if(checkMove(game, d, p) != BEAROFF) {
            if(checkMove(game, d, p) != ERROR){
                for(i=1; i<=FIELDS; i++){
                    if(game->fields[i].player == game->actual_round.who){
                        if(checkMove(game, d, i) == BEAROFF)
                            checker++;
                    }
                }
                if(checker != 0)
                    return 1;
                else return 0;
            }
            else return 0;
        }
        else return 1;
    } 
    else return 0;
}

void makeMove(game_t *game, int d, int p, int type, int diceno) {
    if(type == ERROR) {
        mvprintw(MARGIN+INFO_GAME+3, MENUX, "Niedozwolony ruch");
        do {
            mvprintw(MARGIN+INFO_GAME+4, MENUX, "Spacja aby ponowic probe...");
        } while(getch() != ' ');
        delline(MARGIN+INFO_GAME+3);
        delline(MARGIN+INFO_GAME+4);
        selectWhereMove(game, d, diceno);
    }
    else { 
        if(ifTrayCheck(game, d, p, diceno) == 1)
            trayMove(game, d, p, diceno);
        else if (checkMove(game, d, p) != BEAROFF)
            pawnMove(game, d, p, type);
        else
            makeMove(game, d, p, ERROR, diceno);
    }
}

int checkBarMove(game_t *game, int d) {
    int player = game->actual_round.who;
    if(game->bar[player-1].pawn_number > 0) {
        if(player == PLAYER_A){
            if(game->fields[d].player != PLAYER_B || game->fields[d].pawn_number <=1)
                return BAR_MOVE;
            else return BAR_ERR;
        }
        else {
            if(game->fields[FIELDS-d+1].player != PLAYER_A || game->fields[FIELDS-d+1].pawn_number <=1)
                return BAR_MOVE;
            else return BAR_ERR;
        }
    }
    else return BAR_NULL;
}

int checkMove(game_t *game, int d, int p) {
    int player = game->actual_round.who, second, result;
    if(player == PLAYER_A) {
        second = PLAYER_B;
        result = p + d;
    }
    else {
        second = PLAYER_A;
        result = p - d;
    }
    if(game->fields[p].player == player){
        if (result <= FIELDS && result >0){
            if(game->fields[result].player == player || game->fields[result].player == NONE) {
                return MOVE;
            }
            else {
                if(game->fields[result].player == second && game->fields[result].pawn_number == 1)
                    return BEAT;
                else    
                    return ERROR;
            }
        }
        else {
            return BEAROFF; 
        }
    }
    else return ERROR;
}

void forcedBeatWriteResult(game_t *game, int d, int p, int checker, int diceno) {
    if (checker){
        mvprintw(MARGIN+INFO_GAME+3, MENUX, "Musisz zbic wlasciwy pionek przeciwnika");
        do {
            mvprintw(MARGIN+INFO_GAME+4, MENUX, "Spacja aby ponowic probe...");
        } while(getch() != ' ');
        delline(MARGIN+INFO_GAME+3);
        delline(MARGIN+INFO_GAME+4);
        selectWhereMove(game, d, diceno);
    }
    else 
        makeMove(game, d, p, checkMove(game, d, p), diceno);
}

void forcedBeatCheck(game_t *game, int d, int p, int diceno) { 
    int checker=0, i, player = game->actual_round.who;
    if(checkMove(game, d, p) == BEAT){ 
        if(player==PLAYER_A){
            for(i=1; i<p && checker==0; i++){
                if (checkMove(game, d, i) == BEAT && (p+d)!=(i+d))
                    checker++;
            }
        }
        else{
            for(i=FIELDS; i>p && checker==0; i--){
                if (checkMove(game, d, i) == BEAT && (p-d)!=(i-d))
                    checker++;
            }
        }
    }
    else { 
        for(i=1; i<=FIELDS; i++){
            if (checkMove(game, d, i) == BEAT)
                checker++;
        }
    }
    forcedBeatWriteResult(game, d, p, checker, diceno);
}

void selectBarMove(game_t *game, int d, int diceno) {
    int p;
    int startd = d;
    if(game->actual_round.who == PLAYER_B)
        d = FIELDS - d + 1;
    mvprintw(MARGIN+INFO_GAME+2, MENUX, "Wpisz 0 aby zejsc z bandy na pole %d: ", d);
    echo();
    curs_set(1);
    scanw("%d", &p);
    curs_set(0);
    noecho();

    if(p!=0) {
        mvprintw(MARGIN+INFO_GAME+3, MENUX, "Musisz wpisac wlasciwa liczbe");
        do {
            mvprintw(MARGIN+INFO_GAME+4, MENUX, "Spacja aby ponowic probe...");
        } while(getch() != ' ');
        delline(MARGIN+INFO_GAME+3);
        delline(MARGIN+INFO_GAME+4);
        selectWhereMove(game, startd, diceno);
    }
    else {
        barMove(game, d);
    }
}

void selectWhereMoveWrite(game_t *game, int d, int p, int checker, int diceno) {
    if(!checker) {
        printw("Brak mozliwych ruchow");
        do {
            mvprintw(MARGIN+INFO_GAME+3, MENUX, "Nastepna tura... (SPACJA)");
        } while(getch() != ' ');
        if(diceno == DICEB){
            newRound(game, game->fields, NORMAL);
        }
        else delline(MARGIN+INFO_GAME+3);
    }
    else {
        printw("Wybierz skad ruszyc o %d: ", d);
        echo();
        curs_set(1);
        scanw("%d", &p);
        curs_set(0);
        noecho();
        int traychecker = trayMoveCheck(game, d, p, diceno);
        if(ifTrayCheck(game, d, p, diceno) == 1)
            makeMove(game, d, p, !traychecker, diceno);
        else
            forcedBeatCheck(game, d, p, diceno);
    }
}

void selectWhereMove(game_t *game, int d, int diceno) { 
    delline(MARGIN + INFO_GAME + 2);
    int p, i, checker=0;

    if(game->bar[game->actual_round.who - 1].pawn_number == 0){
        for(i=1; i<=FIELDS; i++) {
            if(checkMove(game, d, i)!= BEAROFF || ifTrayCheck(game, d, i, diceno) == 1) {
                checker+=checkMove(game, d, i);
            }
        }
        move(MARGIN+INFO_GAME+2,MENUX);
        selectWhereMoveWrite(game, d, p, checker, diceno);
    }
    else 
        selectBarMove(game, d, diceno); 

    clearBoard();
    drawBoard(game->fields, game);
}

void chooseDiceDraw(game_t *game, int d1, int d2, int choosed) { 
    if(choosed == DICEA && d1 != d2){
        attron(COLOR_PAIR(1 + game->actual_round.who));
        printw("[%d]", d1);
        attroff(COLOR_PAIR(1 + game->actual_round.who));
        attron(COLOR_PAIR(CYAN));
        printw("[%d]", d2);
    }
    else if (d1 != d2) {
        printw("[%d]", d1);
        attron(COLOR_PAIR(1 + game->actual_round.who));
        printw("[%d]", d2);
        attroff(COLOR_PAIR(1 + game->actual_round.who));
        attron(COLOR_PAIR(CYAN));
    }
    else {
        printw("[%d][%d]", d1, d2);
    }
}

void whereBarMove(game_t *game, int d1, int d2) {
    if(checkBarMove(game, d1) == BAR_MOVE){
        if(game->bar[game->actual_round.who - 1].pawn_number == 1){
            selectWhereMove(game, d1, DICEA);
            selectWhereMove(game, d2, DICEB);
        }
        else
            selectWhereMove(game, d1, DICEB);
    }
    else {
        if(game->bar[game->actual_round.who - 1].pawn_number == 1){
            selectWhereMove(game, d2, DICEA);
            selectWhereMove(game, d1, DICEB);
        }
        else
            selectWhereMove(game, d2, DICEB);
    }
}

void barDiceCheck(game_t *game, int d1, int d2, int choosed) {
    if(checkBarMove(game, d1) + checkBarMove(game,d2) == BAR_ERR) {
        mvprintw(MARGIN + INFO_GAME, MENUX, "Wylosowano: [%d][%d]", d1, d2);
        mvprintw(MARGIN + INFO_GAME + 1, MENUX, "Nie mozna zdjac pionka z bandy");
        do {
            mvprintw(MARGIN+INFO_GAME+2, MENUX, "Nastepna tura... (SPACJA)");
        } while(getch() != ' ');
        newRound(game, game->fields, NORMAL);
    }
    else if(checkBarMove(game, d1) + checkBarMove(game, d2) == BAR_MOVE) {
        whereBarMove(game, d1, d2);
    }
}

void moveSelecting(game_t *game, int d1, int d2, int choosed) {
    if(checkBarMove(game, d1) + checkBarMove(game, d2) > BAR_MOVE){
        if(d1 == d2){
            printw(" DUBLET!");
            selectWhereMove(game, d1, DICEB);
            selectWhereMove(game, d1, DICEB);
        }
        if (choosed == DICEA) {
            selectWhereMove(game, d1, DICEA);
            selectWhereMove(game, d2, DICEB);
        } 
        else {
            selectWhereMove(game, d2, DICEA);
            selectWhereMove(game, d1, DICEB);
        }
    }
}

void chooseDice(game_t *game, int d1, int d2) {
    int choosed = DICEA, bearoff = 0;
    char c = 0;
    while (c!=' '){
        delline(MARGIN + INFO_GAME + 1);
        mvprintw(MARGIN + INFO_GAME + 1, MENUX, "Wylosowano: ");
        chooseDiceDraw(game, d1, d2, choosed);
        if (checkBarMove(game, d1) + checkBarMove(game, d2) <= BAR_MOVE) {
            barDiceCheck(game, d1, d2, choosed);
            bearoff++;
            break;
        }
        if (d1 != d2){
            c = getch();
            switch(c){
            case 'a': case 'A':
                if(choosed!=DICEA) 
                    choosed=DICEA;
                break;
            case 'd': case 'D':
                if(choosed!=DICEB)
                    choosed=DICEB;
                break;
            default: break;
        }
        }
        else c = ' ';
    }
    if(!bearoff)
        moveSelecting(game, d1, d2, choosed);
}

void rollDice(game_t *game, field_t *fields) { 
    srand(time(NULL));
    int d1 = rand() % DICE + 1;
    int d2 = rand() % DICE + 1;
    
    chooseDice(game, d1, d2);
    newRound(game, fields, NORMAL);
}

void roundPlayerChange(game_t *game, field_t *fields){ 
    game->actual_round.mvnumber++;
    saveRounds(game);

    if(game->actual_round.who != game->starting_player){
        game->actual_round.who = game->starting_player;
        game->round++;
    }
    else {
        if(game->starting_player == PLAYER_A)
            game->actual_round.who = PLAYER_B;
        else    
            game->actual_round.who = PLAYER_A;
    }
    saveGame(game);
}

void winNewRound(game_t *game, char *player, int mode) {
    if(mode != NOCHANGE) {
        game->actual_round.mvnumber++;
        saveRounds(game);
        saveGame(game);
    }
    if(game->actual_round.who == PLAYER_A)
        strcpy(player, game->playerA);
    else 
        strcpy(player, game->playerB);

    writeWinMenu(game, player);
}

void newRound(game_t *game, field_t *fields, int mode) {
    clear();
    char player[MAXNAME];

    if(game->tray[game->actual_round.who - 1].pawn_number == PAWNS) {
        winNewRound(game, player, mode);
    }
    else {
        if(mode != NOCHANGE) {
            roundPlayerChange(game, fields);
        }  
        if(game->actual_round.who == PLAYER_A)
            strcpy(player, game->playerA);
        else 
            strcpy(player, game->playerB);
        drawBoard(fields, game);
        delline(MARGIN + TOURINFO);
        mvprintw(MARGIN + TOURINFO, MENUX, "Tura gracza "); 
        attron(COLOR_PAIR(game->actual_round.who + 1));
        printw("%s: ", player);
        attron(COLOR_PAIR(CYAN));

        writeGameMenu(game);
    }
}

void continueGameBar(game_t *loadGame, game_t *game) {
    int i;
    for(i=BAR_A; i<=BAR_B; i++){
        game->bar[i] = loadGame->bar[i];
        game->tray[i] = loadGame->tray[i];
        game->points[i] = loadGame->points[i];
    }
}

void continueGame(game_t *loadGame) { 
    clear();
    game_t game;
    game.starting_player = loadGame->starting_player;
    game.round = loadGame->round;
    game.actual_round.who = loadGame->actual_round.who;
    game.actual_round.mvnumber = loadGame->actual_round.mvnumber;
    strcpy(game.playerA, loadGame->playerA);
    strcpy(game.playerB, loadGame->playerB);
    field_t fields[FIELDS + 1];
    int i;
    for (i=0; i<=FIELDS; i++){
        game.fields[i] = loadGame->fields[i];
        fields[i] = game.fields[i];
    }
    continueGameBar(loadGame, &game);
    newRound(&game, fields, NOCHANGE);
}

void writeWatchMenu(game_t *game, char *sp) { 
    sign();
    writeVersus(game, sp);
    mvprintw(MARGIN + ZERO_WATCH, MENUX, "Runda %d (%d : %d)", game->round, game->points[BAR_A], game->points[BAR_B]);
    mvprintw(MARGIN + START_WATCH, MENUX, "> Przewin na poczatek");
    mvprintw(MARGIN + PREV_WATCH, MENUX, "  Poprzedni ruch");
    mvprintw(MARGIN + NEXT_WATCH, MENUX, "  Nastepny ruch");
    mvprintw(MARGIN + END_WATCH, MENUX, "  Przewin na koniec");
    mvprintw(MARGIN + QUIT_WATCH, MENUX, "  Wyjdz do menu");
}

void watchMenu(game_t *game) {
    char startingplayer[MAXNAME];
    if (game->starting_player == PLAYER_A)
        strcpy(startingplayer, game->playerA);
    else 
        strcpy(startingplayer, game->playerB);
    drawBoard(game->fields, game);

    writeWatchMenu(game, startingplayer);
    selectMenu(START_WATCH, QUIT_WATCH, WATCH, game);
}

void watchGame(game_t *loadGame) { 
    clear();
    game_t game;
    game.starting_player = loadGame->starting_player;
    game.round = loadGame->round;
    game.actual_round.who = loadGame->actual_round.who;
    game.actual_round.mvnumber = loadGame->actual_round.who;
    strcpy(game.playerA, loadGame->playerA);
    strcpy(game.playerB, loadGame->playerB);
    field_t fields[FIELDS + 1];
    int i;
    for (i=0; i<=FIELDS; i++){
        game.fields[i] = loadGame->fields[i];
        fields[i] = game.fields[i];
    }
    for(i=BAR_A; i<=BAR_B; i++){
        game.bar[i] = loadGame->bar[i];
        game.tray[i] = loadGame->tray[i];
        game.points[i] = loadGame->points[i];
    }
    watchMenu(&game);
}

void saveFilePrint(FILE *save, game_t *game){
    int i;
    for(i = 0; i <= FIELDS + 1; i++) {
        fprintf(save, "%d %02d %d %d %d ", game->fields[i].field_number, game->fields[i].pawn_number, game->fields[i].player, game->fields[i].x, game->fields[i].y );
    }
    for(i = BAR_A; i<=BAR_B; i++) {
        fprintf(save, "%d %02d %d %d %d ", game->bar[i].field_number, game->bar[i].pawn_number, game->bar[i].player, game->bar[i].x, game->bar[i].y);
        fprintf(save, "%d %02d %d %d %d ", game->tray[i].field_number, game->tray[i].pawn_number, game->tray[i].player, game->tray[i].x, game->tray[i].y);
    }
    fprintf(save, "\n");
}

void saveRounds(game_t *game) {
    FILE *save;
    save = fopen("rounds.txt", "a+");
    if(save != NULL) {
        fprintf(save, "%03d %d %02d %02d %03d %d %s %s ", game->actual_round.mvnumber, game->actual_round.who, game->points[0], game->points[1], game->round, game->starting_player, game->playerA, game->playerB);
        saveFilePrint(save, game);
        fclose(save);
    }
}

void saveGame(game_t *game) { 
    FILE * save;
    save = fopen("save.txt", "w");
    if (save != NULL) {
        fprintf(save, "%03d %d %02d %02d %03d %d %s %s ", game->actual_round.mvnumber, game->actual_round.who, game->points[0], game->points[1], game->round, game->starting_player, game->playerA, game->playerB);
        saveFilePrint(save, game);
        mvprintw(MARGIN + INFO_GAME, MENUX, "Pomyslnie zapisano");
        fclose(save);
    }
    else
        mvprintw(MARGIN + INFO_GAME, MENUX, "Blad zapisu");

    FILE *num = fopen("num.txt", "w");
    if(num != NULL) {
        fprintf(num, "%d", 1);
        fclose(num);
    }
}

void loadFilePrintInt(FILE *load, game_t *game) {
    fscanf(load, "%03d %d %02d %02d %03d %d %s %s ", &game->actual_round.mvnumber, &game->actual_round.who, &game->points[0], &game->points[1], &game->round, &game->starting_player, &game->playerA, &game->playerB);
}

void loadFilePrint(FILE *load, game_t *game) {
    int i;
    for(i = 0; i <= FIELDS + 1; i++) {
        fscanf(load, "%d %02d %d %d %d ", &game->fields[i].field_number, &game->fields[i].pawn_number, &game->fields[i].player, &game->fields[i].x, &game->fields[i].y );
    }
    for(i = BAR_A; i<=BAR_B; i++) {
        fscanf(load, "%d %02d %d %d %d ", &game->bar[i].field_number, &game->bar[i].pawn_number, &game->bar[i].player, &game->bar[i].x, &game->bar[i].y);
        fscanf(load, "%d %02d %d %d %d ", &game->tray[i].field_number, &game->tray[i].pawn_number, &game->tray[i].player, &game->tray[i].x, &game->tray[i].y);
    }
}

void loadRound(game_t *game, int n) {
    game_t loadGame;
    FILE *load;
    load = fopen("save.txt", "r");
    if(load != NULL) {
        fclose(load);
        load = fopen("save.txt", "a+");
        fseek(load, 0, SEEK_END);
        long size = ftell(load);
        fclose(load);

        load = fopen("rounds.txt", "r");
        if(load != NULL) {
            fclose(load);
            load = fopen("rounds.txt", "a+");
            fseek(load, (n-1)*size, SEEK_SET);
            loadFilePrintInt(load, &loadGame);
            loadFilePrint(load, &loadGame);
            fclose(load);
            watchGame(&loadGame);
        }
    }
    else {
        mvprintw(MARGIN + INFO_MENU, MENUX, "Blad odczytu");
        selectMenu(PLAY_MENU, QUIT_MENU, MENU, game);
    }
}

void loadGame(game_t *game) { 
    game_t loadGame;
    FILE * load;
    load = fopen("save.txt", "r");
    if (load != NULL) {
        fclose(load);
        load = fopen("save.txt", "a+");
        loadFilePrintInt(load, &loadGame);
        loadFilePrint(load, &loadGame);
        fclose(load);
        continueGame(&loadGame);
    }
    else {
        mvprintw(MARGIN + INFO_MENU, MENUX, "Blad odczytu");
        selectMenu(PLAY_MENU, QUIT_MENU, MENU, game);
    }
    
}

void startNewGame(game_t *game) { 
    clear();
    game->starting_player = NONE;
    game->actual_round.mvnumber = 1;
    field_t fields[FIELDS+1];
    fieldsInit(fields);

    int i;
    for (i=0; i<=FIELDS; i++){
        game->fields[i] = fields[i];
    }

    if(game->round){ 
        curs_set(1);
        sign();
        firstDiceRoll(game);
        game->actual_round.who = game->starting_player;
        game->round = 1;
        curs_set(0);
    }
    else {
        game->round = 0;
        game->playerA[0] = 'A', game->playerB[0] = 'B';
        game->points[0] = 0, game->points[1] = 0;
        newGameInit(game);
    }

    barInit(game); 
    remove("rounds.txt");
    saveRounds(game);
    saveGame(game);
    newRound(game, fields, NOCHANGE);
}

void writeMenu() { 
    clear();
    game_t game;

    FILE *num = fopen("num.txt", "w");
    if(num != NULL) {
        fprintf(num, "%d", 1);
        fclose(num);
    }

    sign();
    mvprintw(MARGIN + ZERO_MENU, MENUX, "MENU: ");
    mvprintw(MARGIN + PLAY_MENU, MENUX, "> Zagraj");
    mvprintw(MARGIN + LOAD_MENU, MENUX, "  Wczytaj");
    mvprintw(MARGIN + WATCH_MENU, MENUX, "  Wizualizacja rozgrywki");
    mvprintw(MARGIN + QUIT_MENU, MENUX, "  Wyjdz");

    selectMenu(PLAY_MENU, QUIT_MENU, MENU, &game);
}

int endnoDeterminer(){
    FILE *load = fopen("rounds.txt", "a+");
    fseek(load, 0, SEEK_END);
    int endno = ftell(load);
    fclose(load);
    load = fopen("save.txt", "a+");
    fseek(load, 0, SEEK_END);
    endno /= ftell(load); 
    fclose(load);

    return endno;
}

void runWatchOption(game_t *game, int selected) {
    int n, endno = endnoDeterminer();
    FILE *num = fopen("num.txt", "r");
    if(num != NULL){
        fscanf(num, "%d", &n);
    }
    else 
        n = 1;
    fclose(num);

    delline(MARGIN+INFO_MENU);
    switch(selected){
        case START_WATCH:
            n = 1;
            break;
        case PREV_WATCH:
            if(n != 1) 
                n--;
            break;
        case NEXT_WATCH:
            if(n != endno) 
                n++;
            break;
        case END_WATCH:
            n = endno;
            break;
        case QUIT_WATCH:
            writeMenu();
            break;
    }

    num = fopen("num.txt", "w");
    if(num != NULL) {
        fprintf(num, "%d", n);
        fclose(num);
    }
    loadRound(game, n);
}

void runMenuOption(game_t *game, int selected) {
    delline(MARGIN+INFO_MENU);
    switch(selected){
        case PLAY_MENU:
            game->round = 0;
            startNewGame(game);
            break;
        case LOAD_MENU:
            loadGame(game);
            break;
        case WATCH_MENU:
            loadRound(game, 1);
            break;
        case QUIT_MENU:
            exit(0);
            break;
    }
}

void runWinOption(game_t *game, int selected) {
    delline(MARGIN+INFO_GAME);
    switch(selected){
        case ROLL_GAME:
            addWinPoints(game);
            startNewGame(game);
            break;
        case SAVE_GAME:
            saveGame(game);
            break;
        case QUIT_GAME:
            writeMenu();
            break;
    }
}

void runGameOption(game_t *game, int selected) {
    delline(MARGIN+INFO_GAME);
    switch(selected){
        case ROLL_GAME:
            rollDice(game, game->fields);
            break;
        case SAVE_GAME:
            saveGame(game);
            selectMenu(ROLL_GAME, QUIT_GAME, GAME, game);
            break;
        case QUIT_GAME:
            writeMenu();
            break;
    }
}

void runSelectedOption(int selected, int menu_type, game_t *game) { 
    switch(menu_type){
        case MENU:
        runMenuOption(game, selected);
        break;

        case WIN:
        runWinOption(game, selected);
        break;

        case GAME:
        runGameOption(game, selected);
        break;

        case WATCH:
        runWatchOption(game, selected);
        break;
    }
}

int main() { 
    initscr();
    noecho();
    curs_set(0);
    start_color();
    init_pair(CYAN, COLOR_CYAN, COLOR_BLACK);
    init_pair(WHITE, COLOR_WHITE, COLOR_BLACK);
    init_pair(RED, COLOR_RED, COLOR_BLACK);
    attron(COLOR_PAIR(CYAN));

    writeMenu(); 

    attroff(COLOR_PAIR(CYAN));
    endwin();
    return 0;
}