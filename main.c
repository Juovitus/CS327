#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "heap.h"
#include<ncurses.h>

#define MAP_X_LENGTH 80
#define MAP_Y_LENGTH 21
#define SYMBOL_POKE_CENTER 0
#define SYMBOL_POKE_MART 1
#define SYMBOL_CLEARING 2
#define SYMBOL_TALL_GRASS 3
#define SYMBOL_BOULDER 4
#define SYMBOL_PATH 5
#define SYMBOL_PLAYER 6
#define SYMBOL_HIKER 7
#define SYMBOL_RIVAL 8
#define SYMBOL_PACER 9
#define SYMBOL_WANDERER 10
#define SYMBOL_STATIONARY 11
#define SYMBOL_RANDOM_WALKER 12
#define HIKER_COST 0
#define RIVAL_COST 1
#define PLAYER_COST 2
#define OTHER_COST 3
#define ESCAPE_KEY 27

char SYMBOLS[] = {'C', 'M', '.', ';', '%', '#', '@', 'h', 'r', 'p', 'w', 's', 'n'};
//COST ORDER = HIKER_COST->RIVAL_COST->PC->OTHERS
int32_t COST_PATH_OR_CLEARING[] = {10, 10, 10, 10};
int32_t COST_BUILDING[] = {INT32_MAX, INT32_MAX, 10, INT32_MAX};
int32_t COST_TALL_GRASS[] = {15, 20, 20, 20};
//Default number of trainers if not passed in a number
int numTrainers = 10;
int quit_game = 0;

typedef struct nonPlayerCharacter{
    int npcType, mapX, mapY, isAlive, nextMoveTime, direction;
    char spawnTileType;
} nonPlayerCharacter;

typedef struct mapGrid{
    char map[MAP_X_LENGTH][MAP_Y_LENGTH];
    int northOpening, eastOpening, southOpening, westOpening;
    uint32_t hikerMap[MAP_X_LENGTH][MAP_Y_LENGTH];
    uint32_t rivalMap[MAP_X_LENGTH][MAP_Y_LENGTH];
    nonPlayerCharacter npc[MAP_X_LENGTH * MAP_Y_LENGTH];
    heap_t npcHeap;
    int currentTime;
} mapGrid;
mapGrid *currentMap;
mapGrid *worldMap[399][399];

typedef struct Point{
    uint8_t xPos, yPos;
} point_t;

typedef struct playerCharacter{
    int mapX, mapY, isValidMovement;
} playerCharacter;
playerCharacter *pc;

typedef struct path {
    heap_node_t *hn;
    uint8_t pos[2];
    uint8_t from[2];
    int32_t cost;
} path_t;

void DisplayMap(mapGrid *map){
    //Display dijkstra's stuff
    for(int i = 0; i < 2; i++){
        //GenerateCostMap(i);//THIS GENERATES COST MAP FOR RIVAL AND HIKER---
        //PrintCostMap(i);   //---THIS WILL PRINT THE COST MAP OF THE CURRENT MAP FOR HIKERS AND RIVALS---
    }
    initscr();
    init_pair(SYMBOL_POKE_CENTER, COLOR_WHITE, COLOR_BLACK);
    init_pair(SYMBOL_POKE_MART, COLOR_WHITE, COLOR_BLACK);
    init_pair(SYMBOL_CLEARING, COLOR_YELLOW, COLOR_BLACK);
    init_pair(SYMBOL_TALL_GRASS, COLOR_GREEN, COLOR_BLACK);
    init_pair(SYMBOL_BOULDER, COLOR_RED, COLOR_BLACK);
    init_pair(SYMBOL_RIVAL, COLOR_BLUE, COLOR_BLACK);
    init_pair(SYMBOL_PATH, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(SYMBOL_PLAYER, COLOR_WHITE, COLOR_BLACK);
    for(int y = 0; y < MAP_Y_LENGTH; y++){
        for(int x = 0; x < MAP_X_LENGTH; x++){
            //Lets color code the output so it looks better :)
            char c;

            if(pc != NULL && x == pc->mapX && y == pc->mapY){
                c = SYMBOLS[SYMBOL_PLAYER];
            }else{
                c = map->map[x][y];
            }

            for(int n = 0; n < numTrainers; n++){
                if(currentMap->npc[n].mapX == x && currentMap->npc[n].mapY == y){
                    c = SYMBOLS[currentMap->npc[n].npcType];
                }
            }

            switch(c) {
                case 'C' :
                    attron(COLOR_PAIR(SYMBOL_POKE_CENTER));
                    mvaddch(y,x,c);
                    attroff(COLOR_PAIR(SYMBOL_POKE_CENTER));
                    break;
                case 'M' :
                    attron(COLOR_PAIR(SYMBOL_POKE_MART));
                    mvaddch(y,x,c);
                    attroff(COLOR_PAIR(SYMBOL_POKE_MART));
                    break;
                case '.' :
                    attron(COLOR_PAIR(SYMBOL_CLEARING));
                    mvaddch(y,x,c);
                    attroff(COLOR_PAIR(SYMBOL_CLEARING));
                    break;
                case ';' :
                    attron(COLOR_PAIR(SYMBOL_TALL_GRASS));
                    mvaddch(y,x,c);
                    attroff(COLOR_PAIR(SYMBOL_TALL_GRASS));
                    break;
                case '%' :
                    attron(COLOR_PAIR(SYMBOL_BOULDER));
                    mvaddch(y,x,c);
                    attroff(COLOR_PAIR(SYMBOL_BOULDER));
                    break;
                case '#' :
                    attron(COLOR_PAIR(SYMBOL_PATH));
                    mvaddch(y,x,c);
                    attroff(COLOR_PAIR(SYMBOL_PATH));
                    break;
                case '@' :
                    attron(COLOR_PAIR(SYMBOL_PLAYER));
                    mvaddch(y,x,c);
                    attroff(COLOR_PAIR(SYMBOL_PLAYER));
                    break;
                case 'h' :
                    attron(COLOR_PAIR(SYMBOL_RIVAL));
                    mvaddch(y,x,c);
                    attroff(COLOR_PAIR(SYMBOL_RIVAL));
                    break;
                case 'r' :
                    attron(COLOR_PAIR(SYMBOL_RIVAL));
                    mvaddch(y,x,c);
                    attroff(COLOR_PAIR(SYMBOL_RIVAL));
                    break;
                case 'p' :
                    attron(COLOR_PAIR(SYMBOL_RIVAL));
                    mvaddch(y,x,c);
                    attroff(COLOR_PAIR(SYMBOL_RIVAL));
                    break;
                case 'w' :
                    attron(COLOR_PAIR(SYMBOL_RIVAL));
                    mvaddch(y,x,c);
                    attroff(COLOR_PAIR(SYMBOL_RIVAL));
                    break;
                case 's' :
                    attron(COLOR_PAIR(SYMBOL_RIVAL));
                    mvaddch(y,x,c);
                    attroff(COLOR_PAIR(SYMBOL_RIVAL));
                    break;
                case 'n' :
                    attron(COLOR_PAIR(SYMBOL_RIVAL));
                    mvaddch(y,x,c);
                    attroff(COLOR_PAIR(SYMBOL_RIVAL));
                    break;
                default:
                    attron(COLOR_PAIR(SYMBOL_TALL_GRASS));
                    mvaddch(y,x,c);
                    attroff(COLOR_PAIR(SYMBOL_TALL_GRASS));
            }
        }
        if(y!=MAP_Y_LENGTH-1){
            //Break to a new line
            printw("\n");
        }


    }
    refresh();
}

int IsBoundsValid(int x, int xMax){
    if(x > xMax || x < 0){
        return 0;
    }else{
        return 1;
    }
}

static int32_t Path_Compare(const void *key, const void *with) {
    return ((path_t *) key)->cost - ((path_t *) with)->cost;
}

static int NPC_COMPARE(const void *key, const void *with){
    return ((nonPlayerCharacter *) key)->nextMoveTime - ((nonPlayerCharacter *) with)->nextMoveTime;
}

int IsNpcAtXY(int x, int y){
    for(int i = 0; i < numTrainers; i++){
        if(currentMap->npc[i].mapX == x && currentMap->npc[i].mapY == y){
            return 1;
        }
    }
    return 0;
}

void StartBattle(){
    int leaveBattle = 0;
    while(!leaveBattle){
        //CLEAR SCREEN BEFORE BATTLE
        clear();
        printw("Temporary battle page\n");
        printw("A wild Persian Appears!\n");
        printw("   |\\---/|\n");
        printw("   | ,_, |\n");
        printw("    \\_`_/-..----.\n");
        printw(" ___/ `   ' ,\"\"+ \\ \n");
        printw("(__...'   __\\    |`.___.';\n");
        printw("  (_,...'(_,.`__)/'.....+\n");
        printw("You can use your escape key to exit,\notherwise you're stuck here cuddling with this cat.\nI know it sucks.");
        refresh();
        char userInput = getch();
        if(userInput == ESCAPE_KEY){
            leaveBattle = 1;
        }
    }
}

void MoveHiker(nonPlayerCharacter* npc){
    int minCost = INT32_MAX;
    int xMin = 0, yMin = 0;
    for(int x = -1; x < 2; x++){
        for(int y = -1; y < 2; y++){
            if(x == 0 && y == 0){
                continue;
            }else{
                //Start battle if moving onto player
                if(npc->mapX + x == pc->mapX && npc->mapY + y == pc->mapY){
                    StartBattle();
                } else if(currentMap->hikerMap[npc->mapX + x][npc->mapY + y] == INT32_MAX || IsNpcAtXY(npc->mapX + x, npc->mapY + y)){
                    continue;
                }else if(currentMap->hikerMap[npc->mapX + x][npc->mapY + y] <= minCost){
                    if(rand() < RAND_MAX / 2 && currentMap->hikerMap[npc->mapX + x][npc->mapY + y] == minCost){
                        continue;
                    }
                    minCost = currentMap->hikerMap[npc->mapX + x][npc->mapY + y];
                    xMin = x;
                    yMin = y;
                }
            }
        }
    }

    int cost;

    if(currentMap->map[npc->mapX + xMin][npc->mapY + yMin] == SYMBOLS[SYMBOL_POKE_CENTER]){
        cost = COST_BUILDING[HIKER_COST];
    } else if(currentMap->map[npc->mapX + xMin][npc->mapY + yMin] == SYMBOLS[SYMBOL_POKE_MART]){
        cost = COST_BUILDING[HIKER_COST];
    } else if(currentMap->map[npc->mapX + xMin][npc->mapY + yMin] == SYMBOLS[SYMBOL_PATH]){
        cost = COST_PATH_OR_CLEARING[HIKER_COST];
    } else if(currentMap->map[npc->mapX + xMin][npc->mapY + yMin] == SYMBOLS[SYMBOL_TALL_GRASS]){
        cost = COST_TALL_GRASS[HIKER_COST];
    } else if(currentMap->map[npc->mapX + xMin][npc->mapY + yMin] == SYMBOLS[SYMBOL_CLEARING]){
        cost = COST_PATH_OR_CLEARING[HIKER_COST];
    }else{
        cost = INT32_MAX;
    }
    npc->nextMoveTime += cost;
    npc->mapX += xMin;
    npc->mapY += yMin;
}

void MoveRival (nonPlayerCharacter* npc){
    int minCost = INT32_MAX;
    int xMin = 0, yMin = 0;
    for(int x = -1; x < 2; x++){
        for(int y = -1; y < 2; y++){
            if(x == 0 && y == 0){
                continue;
            }else{
                //Start battle if moving onto player
                if(npc->mapX + x == pc->mapX && npc->mapY + y == pc->mapY){
                    StartBattle();
                } else if(currentMap->rivalMap[npc->mapX + x][npc->mapY + y] == INT32_MAX || IsNpcAtXY(npc->mapX + x, npc->mapY + y)){
                    continue;
                }else if(currentMap->rivalMap[npc->mapX + x][npc->mapY + y] <= minCost){
                    if(rand() < RAND_MAX / 2 && currentMap->rivalMap[npc->mapX + x][npc->mapY + y] == minCost){
                        continue;
                    }
                    minCost = currentMap->rivalMap[npc->mapX + x][npc->mapY + y];
                    xMin = x;
                    yMin = y;
                }
            }
        }
    }

    int cost;

    if(currentMap->map[npc->mapX + xMin][npc->mapY + yMin] == SYMBOLS[SYMBOL_POKE_CENTER]){
        cost = COST_BUILDING[OTHER_COST];
    } else if(currentMap->map[npc->mapX + xMin][npc->mapY + yMin] == SYMBOLS[SYMBOL_POKE_MART]){
        cost = COST_BUILDING[RIVAL_COST];
    } else if(currentMap->map[npc->mapX + xMin][npc->mapY + yMin] == SYMBOLS[SYMBOL_PATH]){
        cost = COST_PATH_OR_CLEARING[RIVAL_COST];
    } else if(currentMap->map[npc->mapX + xMin][npc->mapY + yMin] == SYMBOLS[SYMBOL_TALL_GRASS]){
        cost = COST_TALL_GRASS[RIVAL_COST];
    } else if(currentMap->map[npc->mapX + xMin][npc->mapY + yMin] == SYMBOLS[SYMBOL_CLEARING]){
        cost = COST_PATH_OR_CLEARING[RIVAL_COST];
    }else{
        cost = INT32_MAX;
    }
    npc->nextMoveTime += cost;
    npc->mapX += xMin;
    npc->mapY += yMin;
}

void MovePacer (nonPlayerCharacter* npc){
    int xDiff = 0, yDiff = 0;
    switch(npc->direction){
        case 0:
            yDiff--;
            break;
        case 1:
            xDiff++;
            break;
        case 2:
            yDiff++;
            break;
        case 3:
            xDiff--;
            break;
        default:
            printf("\nShouldn't be here?\n");
            break;
    }

    int cost;

    if(currentMap->map[npc->mapX + xDiff][npc->mapY + yDiff] == SYMBOLS[SYMBOL_POKE_CENTER]){
        cost = COST_BUILDING[OTHER_COST];
    } else if(currentMap->map[npc->mapX + xDiff][npc->mapY + yDiff] == SYMBOLS[SYMBOL_POKE_MART]){
        cost = COST_BUILDING[OTHER_COST];
    } else if(currentMap->map[npc->mapX + xDiff][npc->mapY + yDiff] == SYMBOLS[SYMBOL_PATH]){
        cost = COST_PATH_OR_CLEARING[OTHER_COST];
    } else if(currentMap->map[npc->mapX + xDiff][npc->mapY + yDiff] == SYMBOLS[SYMBOL_TALL_GRASS]){
        cost = COST_TALL_GRASS[OTHER_COST];
    } else if(currentMap->map[npc->mapX + xDiff][npc->mapY + yDiff] == SYMBOLS[SYMBOL_CLEARING]){
        cost = COST_PATH_OR_CLEARING[OTHER_COST];
    }else{
        cost = INT32_MAX;
    }

    //Start battle if moving onto player
    if(npc->mapX + xDiff == pc->mapX && npc->mapY + yDiff == pc->mapY) {
        StartBattle(npc->npcType);
    } else if(cost == INT32_MAX || IsNpcAtXY(npc->mapX + xDiff, npc->mapY + yDiff)){
        npc->direction = (npc->direction + 2) % 4;
    } else{
        npc->mapX += xDiff;
        npc->mapY += yDiff;
    }
    if(cost == INT32_MAX){
        cost = 10;
    }
    npc->nextMoveTime += cost;
}

void MoveWanderer (nonPlayerCharacter* npc){
    int xDiff = 0, yDiff = 0;
    switch(npc->direction){
        case 0:
            yDiff--;
            break;
        case 1:
            xDiff++;
            break;
        case 2:
            yDiff++;
            break;
        case 3:
            xDiff--;
            break;
    }

    int cost;

    if(currentMap->map[npc->mapX + xDiff][npc->mapY + yDiff] == SYMBOLS[SYMBOL_POKE_CENTER] && currentMap->map[npc->mapX + xDiff][npc->mapY + yDiff] == npc->spawnTileType){
        cost = COST_BUILDING[OTHER_COST];
    } else if(currentMap->map[npc->mapX + xDiff][npc->mapY + yDiff] == SYMBOLS[SYMBOL_POKE_MART] && currentMap->map[npc->mapX + xDiff][npc->mapY + yDiff] == npc->spawnTileType){
        cost = COST_BUILDING[OTHER_COST];
    } else if(currentMap->map[npc->mapX + xDiff][npc->mapY + yDiff] == SYMBOLS[SYMBOL_PATH] && currentMap->map[npc->mapX + xDiff][npc->mapY + yDiff] == npc->spawnTileType){
        cost = COST_PATH_OR_CLEARING[OTHER_COST];
    } else if(currentMap->map[npc->mapX + xDiff][npc->mapY + yDiff] == SYMBOLS[SYMBOL_TALL_GRASS] && currentMap->map[npc->mapX + xDiff][npc->mapY + yDiff] == npc->spawnTileType){
        cost = COST_TALL_GRASS[OTHER_COST];
    } else if(currentMap->map[npc->mapX + xDiff][npc->mapY + yDiff] == SYMBOLS[SYMBOL_CLEARING] && currentMap->map[npc->mapX + xDiff][npc->mapY + yDiff] == npc->spawnTileType){
        cost = COST_PATH_OR_CLEARING[OTHER_COST];
    }else{
        cost = INT32_MAX;
    }

    //Start battle if moving onto player
    if(npc->mapX + xDiff == pc->mapX && npc->mapY + yDiff == pc->mapY){
        StartBattle();
    }else if(cost == INT32_MAX || IsNpcAtXY(npc->mapX + xDiff, npc->mapY + yDiff)){
        npc->direction = rand() % 4; //Set random direction 1-4
    } else{
        npc->mapX += xDiff;
        npc->mapY += yDiff;
    }
    if(cost == INT32_MAX){
        cost = 10;
    }
    npc->nextMoveTime += cost;
}

void MoveRandomWalker(nonPlayerCharacter* npc){
    int xDiff = 0, yDiff = 0;
    switch(npc->direction){
        case 0:
            yDiff--;
            break;
        case 1:
            xDiff++;
            break;
        case 2:
            yDiff++;
            break;
        case 3:
            xDiff--;
            break;
    }

    int cost;

    if(currentMap->map[npc->mapX + xDiff][npc->mapY + yDiff] == SYMBOLS[SYMBOL_POKE_CENTER]){
        cost = COST_BUILDING[OTHER_COST];
    } else if(currentMap->map[npc->mapX + xDiff][npc->mapY + yDiff] == SYMBOLS[SYMBOL_POKE_MART]){
        cost = COST_BUILDING[OTHER_COST];
    } else if(currentMap->map[npc->mapX + xDiff][npc->mapY + yDiff] == SYMBOLS[SYMBOL_PATH]){
        cost = COST_PATH_OR_CLEARING[OTHER_COST];
    } else if(currentMap->map[npc->mapX + xDiff][npc->mapY + yDiff] == SYMBOLS[SYMBOL_TALL_GRASS]){
        cost = COST_TALL_GRASS[OTHER_COST];
    } else if(currentMap->map[npc->mapX + xDiff][npc->mapY + yDiff] == SYMBOLS[SYMBOL_CLEARING]){
        cost = COST_PATH_OR_CLEARING[OTHER_COST];
    }else{
        cost = INT32_MAX;
    }

    //Start battle if moving onto player
    if(npc->mapX + xDiff == pc->mapX && npc->mapY + yDiff == pc->mapY){
        StartBattle();
    } else if(cost == INT32_MAX || IsNpcAtXY(npc->mapX + xDiff, npc->mapY + yDiff)){
        npc->direction = rand() % 4; //Set random direction 1-4
    } else{
        npc->mapX += xDiff;
        npc->mapY += yDiff;
    }
    if(cost == INT32_MAX){
        cost = 10;
    }
    npc->nextMoveTime += cost;
}

void MoveNPCS(){
    //Find lowest cost NPC(S)
    while(currentMap->npcHeap.size > 0 && ((nonPlayerCharacter*) heap_peek_min(&currentMap->npcHeap))->nextMoveTime <= currentMap->currentTime){
        nonPlayerCharacter* npc = (nonPlayerCharacter*) heap_remove_min(&currentMap->npcHeap);
        if(npc->npcType == SYMBOL_HIKER){
            MoveHiker(npc);
        }else if(npc->npcType == SYMBOL_RIVAL){
            MoveRival(npc);
        }else if(npc->npcType == SYMBOL_PACER){
            MovePacer(npc);
        }else if(npc->npcType == SYMBOL_WANDERER){
            MoveWanderer(npc);
        }else if(npc->npcType == SYMBOL_RANDOM_WALKER){
           MoveRandomWalker(npc);
        }else{
            continue;
        }
        //Add to heap
        heap_insert(&currentMap->npcHeap, npc);
    }
}

int32_t Dijkstra_Path(mapGrid *map, struct Point from, struct Point to, uint32_t characterType){
    static path_t path[MAP_Y_LENGTH][MAP_X_LENGTH], *p;

    heap_t h;
    uint32_t x, y;
    uint8_t dim_x = 1, dim_y = 0;
    //Set costs for current character type
    int costCenter = COST_BUILDING[characterType];
    int costMart = COST_BUILDING[characterType];
    int costPath = COST_PATH_OR_CLEARING[characterType];
    int costTallGrass = COST_TALL_GRASS[characterType];
    int costClearing = COST_PATH_OR_CLEARING[characterType];
    uint32_t costMap[MAP_X_LENGTH][MAP_Y_LENGTH];

    //Set positions and cost
    for(y = 0; y < MAP_Y_LENGTH; y++){
        for(x = 0; x < MAP_X_LENGTH; x++){
            path[y][x].pos[dim_x] = x;
            path[y][x].pos[dim_y] = y;
            path[y][x].cost = INT32_MAX;
            //Set costs for hiker and rival in map
            if(map->map[x][y] == SYMBOLS[SYMBOL_POKE_CENTER]){
                costMap[x][y] = costCenter;
            } else if(map->map[x][y] == SYMBOLS[SYMBOL_POKE_MART]){
                costMap[x][y] = costMart;
            } else if(map->map[x][y] == SYMBOLS[SYMBOL_PATH]){
                costMap[x][y] = costPath;
            } else if(map->map[x][y] == SYMBOLS[SYMBOL_TALL_GRASS]){
                costMap[x][y] = costTallGrass;
            } else if(map->map[x][y] == SYMBOLS[SYMBOL_CLEARING]){
                costMap[x][y] = costClearing;
            } else if(x == pc->mapX && y == pc->mapY){
                costMap[x][y] = 0;
            }else{
                costMap[x][y] = INT32_MAX;
            }
        }
    }
    //Set character position
    path[from.yPos][from.xPos].cost = 0;
    //initialize heap
    heap_init(&h, Path_Compare, NULL);
    for(y = 1; y < MAP_Y_LENGTH-1; y++){
        for(x = 1; x < MAP_X_LENGTH-1; x++){
            if(map->map[x][y] != SYMBOLS[SYMBOL_BOULDER] && !IsNpcAtXY(x, y)){
                if(characterType == PLAYER_COST){
                    path[y][x].hn = heap_insert(&h, &path[y][x]);
                }else if(map->map[x][y] != SYMBOLS[SYMBOL_POKE_CENTER] && map->map[x][y] != SYMBOLS[SYMBOL_POKE_MART]){
                    path[y][x].hn = heap_insert(&h, &path[y][x]);
                }else{
                    path[y][x].hn = NULL;
                }
            }else{
                path[y][x].hn = NULL;
            }
        }
    }

    //While there is something in the heap
    while((p = (path_t*) heap_remove_min(&h))){
        if(from.xPos > MAP_X_LENGTH - 1 || from.xPos < 1 || from.yPos > MAP_Y_LENGTH - 1 || from.yPos < 1){
            break;
        }
        //Set p's HeapNode to null
        p->hn = NULL;

        //Check if destination is same as position
        if((p->pos[dim_x] == to.xPos) && (p->pos[dim_y] == to.yPos)){
            if(p->cost < 0){
                return INT32_MAX;
            }else{
                return p->cost;
            }
        }
        //Let's see if I can write this if-statement correctly
        //Left middle
        if ((path[p->pos[dim_y]][p->pos[dim_x] - 1].hn) && (path[p->pos[dim_y]][p->pos[dim_x] - 1].cost > p->cost + (costMap[p->pos[dim_x]][p->pos[dim_y]]))) {
            path[p->pos[dim_y]][p->pos[dim_x] - 1].cost = p->cost + (costMap[p->pos[dim_x]][p->pos[dim_y]]);
            path[p->pos[dim_y]][p->pos[dim_x] - 1].from[dim_y] = p->pos[dim_y];
            path[p->pos[dim_y]][p->pos[dim_x] - 1].from[dim_x] = p->pos[dim_x];
            heap_decrease_key_no_replace(&h, path[p->pos[dim_y]][p->pos[dim_x] - 1].hn);
        }
        //Top left
        if ((path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].hn) && (path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].cost > p->cost + (costMap[p->pos[dim_x]][p->pos[dim_y]]))) {
            path[p->pos[dim_y] - 1][p->pos[dim_x]  - 1].cost = p->cost + (costMap[p->pos[dim_x]][p->pos[dim_y]]);
            path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].from[dim_y] = p->pos[dim_y];
            path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].from[dim_x] = p->pos[dim_x];
            heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].hn);
        }
        //Top middle
        if ((path[p->pos[dim_y] - 1][p->pos[dim_x]].hn) && (path[p->pos[dim_y] - 1][p->pos[dim_x]].cost > p->cost + (costMap[p->pos[dim_x]][p->pos[dim_y]]))) {
            path[p->pos[dim_y] - 1][p->pos[dim_x]].cost = p->cost + (costMap[p->pos[dim_x]][p->pos[dim_y]]);
            path[p->pos[dim_y] - 1][p->pos[dim_x]].from[dim_y] = p->pos[dim_y];
            path[p->pos[dim_y] - 1][p->pos[dim_x]].from[dim_x] = p->pos[dim_x];
            heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1][p->pos[dim_x]].hn);
        }
        //Top right
        if ((path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].hn) && (path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].cost > p->cost + (costMap[p->pos[dim_x]][p->pos[dim_y]]))) {
            path[p->pos[dim_y] - 1][p->pos[dim_x]  + 1].cost = p->cost + (costMap[p->pos[dim_x]][p->pos[dim_y]]);
            path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].from[dim_y] = p->pos[dim_y];
            path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].from[dim_x] = p->pos[dim_x];
            heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].hn);
        }
        //Right middle
        if ((path[p->pos[dim_y]][p->pos[dim_x] + 1].hn) && (path[p->pos[dim_y]    ][p->pos[dim_x] + 1].cost > p->cost + (costMap[p->pos[dim_x]][p->pos[dim_y]]))) {
            path[p->pos[dim_y]][p->pos[dim_x] + 1].cost = p->cost + (costMap[p->pos[dim_x]][p->pos[dim_y]]);
            path[p->pos[dim_y]][p->pos[dim_x] + 1].from[dim_y] = p->pos[dim_y];
            path[p->pos[dim_y]][p->pos[dim_x] + 1].from[dim_x] = p->pos[dim_x];
            heap_decrease_key_no_replace(&h, path[p->pos[dim_y]][p->pos[dim_x] + 1].hn);
        }
        //Bottom Right
        if ((path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].hn) && (path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].cost > p->cost + (costMap[p->pos[dim_x]][p->pos[dim_y]]))) {
            path[p->pos[dim_y] + 1][p->pos[dim_x]  + 1].cost = p->cost + (costMap[p->pos[dim_x]][p->pos[dim_y]]);
            path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].from[dim_y] = p->pos[dim_y];
            path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].from[dim_x] = p->pos[dim_x];
            heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].hn);
        }
        //Bottom middle
        if ((path[p->pos[dim_y] + 1][p->pos[dim_x]].hn) &&(path[p->pos[dim_y] + 1][p->pos[dim_x]].cost > p->cost + (costMap[p->pos[dim_x]][p->pos[dim_y]]))) {
            path[p->pos[dim_y] + 1][p->pos[dim_x]].cost = p->cost +(costMap[p->pos[dim_x]][p->pos[dim_y]]);
            path[p->pos[dim_y] + 1][p->pos[dim_x]].from[dim_y] = p->pos[dim_y];
            path[p->pos[dim_y] + 1][p->pos[dim_x]].from[dim_x] = p->pos[dim_x];
            heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1][p->pos[dim_x]].hn);
        }
        //Bottom left
        if ((path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].hn) && (path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].cost > p->cost + (costMap[p->pos[dim_x]][p->pos[dim_y]]))) {
            path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].cost = p->cost + (costMap[p->pos[dim_x]][p->pos[dim_y]]);
            path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].from[dim_y] = p->pos[dim_y];
            path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].from[dim_x] = p->pos[dim_x];
            heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].hn);
        }
    }
    return INT32_MAX;
}

void GenerateCostMap(uint32_t characterType){
    for(int y = 0; y < MAP_Y_LENGTH; y++){
        for(int x = 0; x < MAP_X_LENGTH; x++){
            point_t to;
            to.xPos = x;
            to.yPos = y;
            point_t from;
            from.xPos = pc->mapX;
            from.yPos = pc->mapY;
            if(characterType == HIKER_COST){
                currentMap->hikerMap[x][y] = Dijkstra_Path(currentMap, from, to, HIKER_COST);
            }else if (characterType == RIVAL_COST){
                currentMap->rivalMap[x][y] = Dijkstra_Path(currentMap, from, to, RIVAL_COST);
            }else{
                printf("\nWAT?\n");
            }
        }
    }
}

void PrintCostMap(uint32_t characterType){
    for(int y = 0; y < MAP_Y_LENGTH; y++){
        for(int x = 0; x < MAP_X_LENGTH; x++){
            if(characterType == HIKER_COST){
                if(currentMap->hikerMap[x][y] % 100 == 47){
                    printf("   ");
                }else if(currentMap->hikerMap[x][y] % 100 < 10){
                    printf(" 0%d", currentMap->hikerMap[x][y] % 100);
                }else{
                    printf(" %2d", currentMap->hikerMap[x][y] % 100);
                }
            }else if (characterType == RIVAL_COST) {
                if (currentMap->rivalMap[x][y] % 100 == 47) {
                    printf("   ");
                } else if (currentMap->rivalMap[x][y] % 100 < 10) {
                    printf(" 0%d", currentMap->rivalMap[x][y] % 100);
                } else {
                    printf(" %2d", currentMap->rivalMap[x][y] % 100);
                }
            }
        }
        printf("\n");
    }
}

int GenerateRandomX(int num){
    return (rand() % ((MAP_X_LENGTH - num) - num)) + num;
}

int GenerateRandomY(int num){
    return (rand() % ((MAP_Y_LENGTH - num) - num)) + num;
}

int isValidNPCPlacement(int x, int y){
    if(currentMap->map[x][y] == SYMBOLS[SYMBOL_POKE_CENTER] || currentMap->map[x][y] == SYMBOLS[SYMBOL_POKE_MART] ||
       currentMap->map[x][y] == SYMBOLS[SYMBOL_BOULDER] || currentMap->map[x][y] == SYMBOLS[SYMBOL_PATH]){
        return 0;
    }else{
        for(int i = 0; i < (MAP_Y_LENGTH * MAP_X_LENGTH) - 1; i++){
            if(currentMap->npc[i].mapX == x && currentMap->npc[i].mapY == y){
                return 0;
            }
        }
    }
    return 1;
}

void PlaceNPCs(){
    //Initialize NPC heap
    heap_init(&currentMap->npcHeap, NPC_COMPARE, NULL);
    int x = GenerateRandomX(1);
    int y = GenerateRandomY(1);
    for(int i = 0; i < numTrainers; i++){
        while(!isValidNPCPlacement(x, y)){
            x = GenerateRandomX(1);
            y = GenerateRandomY(1);
        }
            //Place type of npc
            currentMap->npc[i].npcType = 7 + (i % 6);  // <----Generate all types of NPCS

            currentMap->npc[i].mapX = x;
            currentMap->npc[i].mapY = y;
            currentMap->npc[i].isAlive = 1;
            currentMap->npc[i].direction = rand() % 4;
            currentMap->npc[i].spawnTileType = currentMap->map[x][y];
            //Set time of NPC and add to heap
            currentMap->npc[i].nextMoveTime = currentMap->currentTime;
            heap_insert(&currentMap->npcHeap, &currentMap->npc[i]);
    }
}

void PlacePlayerCharacter(){
    int randomPlayerLocationX = (rand() % ((MAP_X_LENGTH - 2) - 2)) + 2;
    //Let's look for a spot we can place a player, and if it's found then update where we want to place the player for starts
    for(int y = 1; y < MAP_Y_LENGTH - 1; y++){
        if(currentMap->map[randomPlayerLocationX][y] == SYMBOLS[SYMBOL_PATH]){
            //We now want to also update player info
            pc->mapX = randomPlayerLocationX;
            pc->mapY = y;
            break;
        }
    }
    GenerateCostMap(0);
    GenerateCostMap(1);
}

int IsPositionInMap(int x, int y){
    if(x > MAP_X_LENGTH - 1 || x < 0 || y > MAP_Y_LENGTH - 1 || y < 0){
        return 0;
    }else{
        return 1;
    }
}

void GenerateMap(int newMapX, int newMapY) {
    //If out of bounds say error
    if(newMapX > 398 || newMapY > 398 || newMapX < 0 || newMapY < 0){
        DisplayMap(currentMap);
        printf("\nGenerating map in invalid spot.\n");
        return;
    }
    currentMap = malloc(sizeof(mapGrid));

    //Generate random openings for map on N/E/S/W sides
    int randomNorthOpening = GenerateRandomX(2);
    int randomEastOpening = GenerateRandomY(2);
    int randomSouthOpening = GenerateRandomX(2);
    int randomWestOpening = GenerateRandomY(2);

    //Align North opening with applicable maps South opening
    if(IsPositionInMap(newMapX, newMapY - 1) && worldMap[newMapX][newMapY - 1] != NULL){
        randomNorthOpening = worldMap[newMapX][newMapY - 1]->southOpening;
    }
    //Align South opening with applicable maps North opening
    if(IsPositionInMap(newMapX, newMapY + 1) && worldMap[newMapX][newMapY + 1] != NULL){
        randomSouthOpening = worldMap[newMapX][newMapY + 1]->northOpening;
    }
    //Align East opening with applicable maps West opening
    if(IsPositionInMap(newMapX + 1, newMapY) && worldMap[newMapX + 1][newMapY] != NULL){
        randomEastOpening = worldMap[newMapX + 1][newMapY]->westOpening;
    }
    //Align west opening with applicable maps east opening
    if(IsPositionInMap(newMapX - 1, newMapY) && worldMap[newMapX - 1][newMapY] != NULL){
        randomWestOpening = worldMap[newMapX - 1][newMapY]->eastOpening;
    }

    //Fill map Array boundaries
    for(int x = 0; x < MAP_X_LENGTH; x++){
        for(int y = 0; y < MAP_Y_LENGTH; y++){
            //Fill entire array with Clearing
            currentMap->map[x][y] = SYMBOLS[SYMBOL_CLEARING];
            if(y == 0 || y == MAP_Y_LENGTH - 1 || x == 0 || x == MAP_X_LENGTH - 1){
                currentMap->map[x][y] = SYMBOLS[SYMBOL_BOULDER];
                //Fill map North opening
                if(x == randomNorthOpening && y == 0){
                    currentMap->map[x][y] = SYMBOLS[SYMBOL_PATH];
                }
                //Fill map East opening
                if(y == randomEastOpening && x == MAP_X_LENGTH - 1){
                    currentMap->map[x][y] = SYMBOLS[SYMBOL_PATH];
                }
                //Fill map South opening
                if(x == randomSouthOpening && y == MAP_Y_LENGTH - 1){
                    currentMap->map[x][y] = SYMBOLS[SYMBOL_PATH];
                }
                //Fill map West opening
                if(y == randomWestOpening && x == 0){
                    currentMap->map[x][y] = SYMBOLS[SYMBOL_PATH];
                }
            }
        }
    }


    //Pathfind East
    for(int x = 1; x < MAP_X_LENGTH / 2; x++){
        currentMap->map[x][randomWestOpening] = SYMBOLS[SYMBOL_PATH];
    }
    //Pathfind West
    for(int x = MAP_X_LENGTH - 2; x > (MAP_X_LENGTH / 2) - 1; x--){
        currentMap->map[x][randomEastOpening] = SYMBOLS[SYMBOL_PATH];
    }
    //Pathfind between E/W path
    if(randomEastOpening > randomWestOpening){
        int f = randomEastOpening;
        while(currentMap->map[(MAP_X_LENGTH / 2) - 1][f] != SYMBOLS[SYMBOL_PATH]){
            currentMap->map[(MAP_X_LENGTH / 2) - 1][f] = SYMBOLS[SYMBOL_PATH];
            f--;
        }
    }else if(randomWestOpening > randomEastOpening){
        int f = randomEastOpening;
        while(currentMap->map[(MAP_X_LENGTH / 2) - 1][f] != SYMBOLS[SYMBOL_PATH]){
            currentMap->map[(MAP_X_LENGTH / 2) - 1][f] = SYMBOLS[SYMBOL_PATH];
            f++;
        }
    }

    //Pathfind between N/S path to E/W path(We can improve this later to make it less UGLY)
    for(int y = 1; y < MAP_Y_LENGTH; y++){
        if(currentMap->map[randomNorthOpening][y] == SYMBOLS[SYMBOL_CLEARING]){
            currentMap->map[randomNorthOpening][y] = SYMBOLS[SYMBOL_PATH];
        }else{
            break;
        }
    }
    for(int y = MAP_Y_LENGTH - 2; y > 0; y--){
        if(currentMap->map[randomSouthOpening][y] == SYMBOLS[SYMBOL_CLEARING]){
            currentMap->map[randomSouthOpening][y] = SYMBOLS[SYMBOL_PATH];
        }else{
            break;
        }
    }

    //Calculate Manhattan distance for chance at placing pokemarts/pokecenters
    int xDifference = abs(newMapX - 199);
    int yDifference = abs(newMapY - 199);
    double manhattanDistance = xDifference + yDifference;
    double buildingChance;
    if(manhattanDistance > 200){
        buildingChance = 0.05;
    }else{
        buildingChance = (((-45 * (manhattanDistance) ) / 200) + 50) / 100;
    }
    double ranNum = ((double) rand() / (RAND_MAX));
    int spotFound = 0;
    //Time to place Poke Center + Mart
    //Create two arrays to hold locations for Poke Center and mart(I could do this with just one IG)
    int randomPokeLocationX[4];
    int foundPokeLocationY[4];
    if(ranNum < buildingChance){
        //Boolean for checking if a location is found
        while(spotFound != 2){
            //Randomly generate location for Poke Center/Poke Mart
            randomPokeLocationX[spotFound] = (rand() % ((MAP_X_LENGTH - 3) - 3)) + 3;
            for(foundPokeLocationY[spotFound] = 1; foundPokeLocationY[spotFound] < MAP_Y_LENGTH - 1; foundPokeLocationY[spotFound]++){
                //Search for path on Y plane
                if(currentMap->map[randomPokeLocationX[spotFound]][foundPokeLocationY[spotFound] + 1] == SYMBOLS[SYMBOL_PATH]){
                    //When path is found, check if we can place it.
                    if(currentMap->map[randomPokeLocationX[spotFound]][foundPokeLocationY[spotFound]] == SYMBOLS[SYMBOL_CLEARING] && currentMap->map[randomPokeLocationX[spotFound] + 1][foundPokeLocationY[spotFound]] == SYMBOLS[SYMBOL_CLEARING]
                       && currentMap->map[randomPokeLocationX[spotFound]][foundPokeLocationY[spotFound] - 1] == SYMBOLS[SYMBOL_CLEARING] && currentMap->map[randomPokeLocationX[spotFound] + 1][foundPokeLocationY[spotFound] - 1] == SYMBOLS[SYMBOL_CLEARING]){
                        //Place pokecenter/mart at found spot
                        currentMap->map[randomPokeLocationX[spotFound]][foundPokeLocationY[spotFound]] = SYMBOLS[spotFound];
                        currentMap->map[randomPokeLocationX[spotFound] + 1][foundPokeLocationY[spotFound]] = SYMBOLS[spotFound];
                        currentMap->map[randomPokeLocationX[spotFound]][foundPokeLocationY[spotFound] - 1] = SYMBOLS[spotFound];
                        currentMap->map[randomPokeLocationX[spotFound] + 1][foundPokeLocationY[spotFound] - 1] = SYMBOLS[spotFound];
                        spotFound++;
                        break;
                    }
                }else{
                    //Haven't found location, continue;
                    continue;
                }
            }
        }
    }

    //Variables to hold grass sizes
    int randomTallGrassX[2];
    int randomTallGrassY[2];
    //Now we'll generate random grass clearing sizes
    for(int i = 0; i < 2; i++){
        randomTallGrassX[i] = (rand() % (20 - 3)) + 3;
        randomTallGrassY[i] = (rand() % (7 - 3)) + 3;
    }

    //Boolean for checking if a location is found
    spotFound = 2;
    while(spotFound != 4) {
        //Randomly generate location for Grass
        randomPokeLocationX[spotFound] = (rand() % ((MAP_X_LENGTH - 1) - 1)) + 1;
        foundPokeLocationY[spotFound] = (rand() % ((MAP_Y_LENGTH - 1) - 1)) + 1;
        //Place correct number of X grasses
        for (int x = 0; x < randomTallGrassX[spotFound - 2]; x++) {
            //Place correct number of Y grasses
            for (int y = 0; y < randomTallGrassY[spotFound - 2]; y++) {
                //Place all available X/Y coordinate grass
                if ((randomPokeLocationX[spotFound] + x) < MAP_X_LENGTH - 2) {
                    if (currentMap->map[randomPokeLocationX[spotFound] + x][foundPokeLocationY[spotFound] + y] == '.') {
                        currentMap->map[randomPokeLocationX[spotFound] + x][foundPokeLocationY[spotFound] + y] = SYMBOLS[SYMBOL_TALL_GRASS];
                    }
                }
            }
        }
        spotFound++;
    }
    //Check for edge of map N/S/E/W and if found replacing edge with barrier
    if(newMapY == 0){
        //If on north edge of world map, we shouldn't have an opening so replace it
        currentMap->map[randomNorthOpening][0] = SYMBOLS[SYMBOL_BOULDER];
    }
    if(newMapY == 398) {
        //If on South edge of world map, we shouldn't have an opening so replace it
        currentMap->map[randomSouthOpening][MAP_Y_LENGTH - 1] = SYMBOLS[SYMBOL_BOULDER];
    }
    if(newMapX == 398){
        //If on East edge of world map, we shouldn't have an opening so replace it
        currentMap->map[MAP_X_LENGTH - 1][randomEastOpening] = SYMBOLS[SYMBOL_BOULDER];
    }
    if(newMapX == 0){
        //If on West edge of world map, we shouldn't have an opening so replace it
        currentMap->map[0][randomWestOpening] = SYMBOLS[SYMBOL_BOULDER];
    }

    currentMap->currentTime = 0;
    currentMap->northOpening = randomNorthOpening;
    currentMap->eastOpening = randomEastOpening;
    currentMap->southOpening = randomSouthOpening;
    currentMap->westOpening = randomWestOpening;
    worldMap[newMapX][newMapY] = currentMap;

    //Place NPS on map
    PlaceNPCs();
}

//We want to pass in the incremented movement command to this function for validity checking.
int IsValidPlayerMovement(int currX, int currY){
    //if out of bounds it's not a valid movement
    if(currX > MAP_X_LENGTH - 2 || currX < 1 || currY > MAP_Y_LENGTH - 2 || currY < 1){
        pc->isValidMovement = 0;
        return 0;
    }
    //Check if we're trying to move into a boulder, or some NPC type as we can't do that.
    if(currentMap->map[currX][currY] == SYMBOLS[SYMBOL_BOULDER]){
        pc->isValidMovement = 0;
        return 0;
    }else{
        for(int i = 0; i < (MAP_X_LENGTH * MAP_Y_LENGTH) - 1; i++){
            if(currentMap->npc[i].mapX == currX && currentMap->npc[i].mapY == currY){
                StartBattle();
                return 0;
            }
        }
        pc->isValidMovement = 1;
        return 1;
    }
}

void MovePlayerCharacter(char userInputCharacter) {
    //This is moving the pc for now IG idc dude
    //8 is moving up so lets move the PC up
    int isValidMovement = 0;
    if (userInputCharacter == '8' || userInputCharacter == 'k') {
        //Check if the incremented movement is valid(Which we're going up so its y-1)
        if (IsValidPlayerMovement(pc->mapX, pc->mapY - 1)) {
            //Set updated position of player
            pc->mapY = pc->mapY - 1;
            isValidMovement = 1;
        }
    } else if (userInputCharacter == '6' || userInputCharacter == 'l') {
        //Check if the incremented movement is valid(Which we're going up so its x+1)
        if (IsValidPlayerMovement(pc->mapX + 1, pc->mapY)) {
            //Set updated position of player
            pc->mapX = pc->mapX + 1;
            isValidMovement = 1;
        }
    } else if (userInputCharacter == '2' || userInputCharacter == 'j') {
        //Check if the incremented movement is valid(Which we're going down so its y+1)
        if (IsValidPlayerMovement(pc->mapX, pc->mapY + 1)) {
            //Set updated position of player
            pc->mapY = pc->mapY + 1;
            isValidMovement = 1;
        }
    } else if (userInputCharacter == '4' || userInputCharacter == 'h') {
        //Check if the incremented movement is valid(Which we're going up so its x-1)
        if (IsValidPlayerMovement(pc->mapX - 1, pc->mapY)) {
            //Set updated position of player
            pc->mapX = pc->mapX - 1;
            isValidMovement = 1;
        }
    } else if(userInputCharacter == '5' || userInputCharacter == ' ' || userInputCharacter == '.'){
        //Stand still - We don't want to regen a cost map for standing still
        //Adding diagonal movement next
    } else if(userInputCharacter == '7' || userInputCharacter == 'y'){
        //Upper left
        if (IsValidPlayerMovement(pc->mapX - 1, pc->mapY - 1)) {
            //Set updated position of player
            pc->mapX = pc->mapX - 1;
            pc->mapY = pc->mapY - 1;
            isValidMovement = 1;
        }
    } else if(userInputCharacter == '9' || userInputCharacter == 'u'){
        //Upper right
        if (IsValidPlayerMovement(pc->mapX + 1, pc->mapY - 1)) {
            //Set updated position of player
            pc->mapX = pc->mapX + 1;
            pc->mapY = pc->mapY - 1;
            isValidMovement = 1;
        }
    } else if(userInputCharacter == '1' || userInputCharacter == 'b'){
        //Bottom left
        if (IsValidPlayerMovement(pc->mapX - 1, pc->mapY + 1)) {
            //Set updated position of player
            pc->mapX = pc->mapX - 1;
            pc->mapY = pc->mapY + 1;
            isValidMovement = 1;
        }
    } else if(userInputCharacter == '3' || userInputCharacter == 'n'){
        //Bottom right
        if (IsValidPlayerMovement(pc->mapX + 1, pc->mapY + 1)) {
            //Set updated position of player
            pc->mapX = pc->mapX + 1;
            pc->mapY = pc->mapY + 1;
            isValidMovement = 1;
        }
    }


    if(currentMap->map[pc->mapX][pc->mapY] == SYMBOLS[SYMBOL_POKE_CENTER]){
        currentMap->currentTime += COST_BUILDING[PLAYER_COST];
    } else if(currentMap->map[pc->mapX][pc->mapY] == SYMBOLS[SYMBOL_POKE_MART]){
        currentMap->currentTime += COST_BUILDING[PLAYER_COST];
    } else if(currentMap->map[pc->mapX][pc->mapY] == SYMBOLS[SYMBOL_PATH]){
        currentMap->currentTime += COST_PATH_OR_CLEARING[PLAYER_COST];
    } else if(currentMap->map[pc->mapX][pc->mapY] == SYMBOLS[SYMBOL_TALL_GRASS]){
        currentMap->currentTime +=COST_TALL_GRASS[PLAYER_COST];
    } else if(currentMap->map[pc->mapX][pc->mapY] == SYMBOLS[SYMBOL_CLEARING]){
        currentMap->currentTime += COST_PATH_OR_CLEARING[PLAYER_COST];
    }
    //If we're not standing still then regen cost map
    if(userInputCharacter != '5' && userInputCharacter != ' ' && userInputCharacter != '.' && isValidMovement){
        GenerateCostMap(0);
        GenerateCostMap(1);
    }
    //After the player moves we move NPC'S
    MoveNPCS();
}

void GetUserInput() {
    //Start in center
    int currX = 199;
    int currY = 199;

    GenerateMap(currX, currY);
    PlacePlayerCharacter();

    while (!quit_game) {
        DisplayMap(currentMap);
        int userInputX = -1, userInputY = -1;
        //Printout current location
        if(pc->isValidMovement){
            printw("Current Location in world: (%d, %d)", currX - 199, currY - 199);
            printw("\nEnter New Command: ");
        }else {
            //If previous command was invalid, show it and the command IG
            printw("Current Location: (%d, %d) --- Invalid movement command.", currX - 199, currY - 199);
            printw("\nEnter New Command: ");
        }
        refresh();
        //Get input char
        char userInputCharacter = getch();

        //Handle all inputs that determine player movement(including standing still)
        if(userInputCharacter == '8' || userInputCharacter == '6' || userInputCharacter == '2' || userInputCharacter == '4' || userInputCharacter == '5' ||
           userInputCharacter == '7' || userInputCharacter == '9' || userInputCharacter == '1' || userInputCharacter == '3' || userInputCharacter == 'y' ||
           userInputCharacter == 'k' || userInputCharacter == 'u' || userInputCharacter == 'l' || userInputCharacter == 'n' || userInputCharacter == 'j' ||
           userInputCharacter == 'b' || userInputCharacter == 'h' || userInputCharacter == ' ' || userInputCharacter == '.'){
            //Let's move the player
            MovePlayerCharacter(userInputCharacter);
        }else if(userInputCharacter == '>'){
            //This should attempt to enter a building(You have to be standing on it)
            if(currentMap->map[pc->mapX][pc->mapY] == 'C' || currentMap->map[pc->mapX][pc->mapY] == 'M'){
                //enterBuilding(userInputCharacter);
            }
        }
    }
}

int main(int argc, char *argv[]) {
   //Check for passed in number of trainers, if there is one then set it. (No error checking)
    int isNum = 0;
    for(int i = 0; i < argc; i++){
        if(isNum){
            numTrainers = atoi(argv[i]);
        }
        if(strstr(argv[i], "--numTrainers") || strstr(argv[i], "--numtrainers")){
            isNum = 1;
        }
    }
    if(numTrainers > 1200){
        printf("You've entered too many trainers, please keep it below 1200.\n");
        printf("Can't overfill the map!:)\n");
        return 0;
    }
    initscr();
    pc = malloc(sizeof(playerCharacter));
    //Start with generating random
    srand(time(NULL));
    pc->isValidMovement = 1;
    //Get user input
    GetUserInput();
}