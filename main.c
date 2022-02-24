#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "heap.h"

#define MAP_X_LENGTH 80
#define MAP_Y_LENGTH 21
#define POKE_CENTER 0
#define POKE_MART 1
#define CLEARING 2
#define TALL_GRASS 3
#define BOULDER 4
#define PATH 5
#define PLAYER_SYMBOL 6
#define HIKER 0
#define RIVAL 1
#define PLAYER 2

char SYMBOLS[] = {'C', 'M', '.', ';', '%', '#', '@'};
//COST ORDER = HIKER->RIVAL->PC->OTHERS
int32_t COST_PATH_OR_CLEARING[] = {10, 10, 10, 10};
int32_t COST_BUILDING[] = {INT32_MAX, INT32_MAX, 10, INT32_MAX};
int32_t COST_TALL_GRASS[] = {15, 20, 20, 20};

typedef struct mapGrid{
    char map[MAP_X_LENGTH][MAP_Y_LENGTH];
    int northOpening, eastOpening, southOpening, westOpening;
    uint32_t hikerMap[MAP_X_LENGTH][MAP_Y_LENGTH];
    uint32_t rivalMap[MAP_X_LENGTH][MAP_Y_LENGTH];
} mapGrid;
mapGrid *currentMap;
mapGrid *worldMap[399][399];

typedef struct Point{
    uint8_t xPos, yPos;
} point_t;

typedef struct playerCharacter{
    int worldMapX, worldMapY, mapX, mapY;
} playerCharacter;
playerCharacter *pc;

typedef struct path {
    heap_node_t *hn;
    uint8_t pos[2];
    uint8_t from[2];
    int32_t cost;
} path_t;

int CheckBoundsValid(int x, int xMax){
    if(x > xMax || x < 0){
        return 0;
    }else{
        return 1;
    }
}

static int32_t Path_Compare(const void *key, const void *with) {
    return ((path_t *) key)->cost - ((path_t *) with)->cost;
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
            if(map->map[x][y] == SYMBOLS[POKE_CENTER]){
                costMap[x][y] = costCenter;
            } else if(map->map[x][y] == SYMBOLS[POKE_MART]){
                costMap[x][y] = costMart;
            } else if(map->map[x][y] == SYMBOLS[PATH]){
                costMap[x][y] = costPath;
            } else if(map->map[x][y] == SYMBOLS[TALL_GRASS]){
                costMap[x][y] = costTallGrass;
            } else if(map->map[x][y] == SYMBOLS[CLEARING]){
                costMap[x][y] = costClearing;
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
            if(map->map[x][y] != SYMBOLS[BOULDER]){
                if(characterType == PLAYER){
                    path[y][x].hn = heap_insert(&h, &path[y][x]);
                }else if(map->map[x][y] != SYMBOLS[POKE_CENTER] && map->map[x][y] != SYMBOLS[POKE_MART]){
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
            if(characterType == HIKER){
                currentMap->hikerMap[x][y] = Dijkstra_Path(currentMap, from, to, HIKER);
            }else if (characterType == RIVAL){
                currentMap->rivalMap[x][y] = Dijkstra_Path(currentMap, from, to, RIVAL);
            }else{
                printf("\nWAT?\n");
            }
        }
    }
}

void PrintCostMap(uint32_t characterType){
    for(int y = 0; y < MAP_Y_LENGTH; y++){
        for(int x = 0; x < MAP_X_LENGTH; x++){
            if(characterType == HIKER){
                if(currentMap->hikerMap[x][y] % 100 == 47){
                    printf("   ");
                }else if(currentMap->hikerMap[x][y] % 100 < 10){
                    printf(" 0%d", currentMap->hikerMap[x][y] % 100);
                }else{
                    printf(" %2d", currentMap->hikerMap[x][y] % 100);
                }
            }else if (characterType == RIVAL) {
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

void PlacePlayerCharacter(int currX, int currY){
    int randomPlayerLocationX = (rand() % ((MAP_X_LENGTH - 2) - 2)) + 2;
    //Let's look for a spot we can place a player, and if it's found then update where we want to place the player for starts
    for(int y = 1; y < MAP_Y_LENGTH - 1; y++){
        if(worldMap[currX][currY]->map[randomPlayerLocationX][y] == SYMBOLS[PATH]){
            //worldMap[currX][currY]->map[randomPlayerLocationX][y] = SYMBOLS[PLAYER_SYMBOL];
            //We now want to also update player info
            pc->worldMapX = currX;
            pc->worldMapY = currY;
            pc->mapX = randomPlayerLocationX;
            pc->mapY = y;
            break;
        }
    }
}

void DisplayMap(mapGrid *map){
    //Display dijkstra's stuff
    for(int i = 0; i < 2; i++){
        GenerateCostMap(i);
        PrintCostMap(i);
    }
    for(int y = 0; y < MAP_Y_LENGTH; y++){
        for(int x = 0; x < MAP_X_LENGTH; x++){
            //Lets color code the output so it looks better :)
            char c;

            if(pc != NULL && x == pc->mapX && y == pc->mapY){
                c = SYMBOLS[PLAYER_SYMBOL];
            }else{
                c = map->map[x][y];
            }

            switch(c) {
                case 'C' :
                    printf("\033[0;35m%c", c);
                    break;
                case 'M' :
                    printf("\033[0;34m%c", c);
                    break;
                case '.' :
                    printf("\033[0;30m%c", c);
                    break;
                case ';' :
                    printf("\033[0;32m%c", c);
                    break;
                case '%' :
                    printf("\033[0;31m%c", c);
                    break;
                case '#' :
                    printf("\033[0;36m%c", c);
                    break;
                case '@' :
                    printf("\033[0;35m%c", c);
                    break;
                default:
                    printf("\033[0;30m%c", map->map[x][y]);
            }
        }
        //Break to a new line
        printf("\n");
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
    int randomNorthOpening = (rand() % ((MAP_X_LENGTH - 2) - 2)) + 2;
    int randomEastOpening = (rand() % ((MAP_Y_LENGTH - 2) - 2)) + 2;
    int randomSouthOpening = (rand() % ((MAP_X_LENGTH - 2) - 2)) + 2;
    int randomWestOpening = (rand() % ((MAP_Y_LENGTH - 2) - 2)) + 2;

    //Align North opening with applicable maps South opening
    if(worldMap[newMapX][newMapY - 1] != NULL){
        randomNorthOpening = worldMap[newMapX][newMapY - 1]->southOpening;
        //printf("\nNorth opening: %d\n", randomNorthOpening);
    }
    //Align South opening with applicable maps North opening
    if(worldMap[newMapX][newMapY + 1] != NULL){
        randomSouthOpening = worldMap[newMapX][newMapY + 1]->northOpening;
    }
    //Align East opening with applicable maps West opening
    if(newMapX > 398 && worldMap[newMapX + 1][newMapY] != NULL){
        randomEastOpening = worldMap[newMapX + 1][newMapY]->westOpening;
    }
    //Align west opening with applicable maps east opening
    if(worldMap[newMapX - 1][newMapY] != NULL){
        randomWestOpening = worldMap[newMapX - 1][newMapY]->eastOpening;
    }

    //Fill map Array boundaries
    for(int x = 0; x < MAP_X_LENGTH; x++){
        for(int y = 0; y < MAP_Y_LENGTH; y++){
            //Fill entire array with Clearing
            currentMap->map[x][y] = SYMBOLS[CLEARING];
            if(y == 0 || y == MAP_Y_LENGTH - 1 || x == 0 || x == MAP_X_LENGTH - 1){
                currentMap->map[x][y] = SYMBOLS[BOULDER];
                //Fill map North opening
                if(x == randomNorthOpening && y == 0){
                    currentMap->map[x][y] = SYMBOLS[PATH];
                }
                //Fill map East opening
                if(y == randomEastOpening && x == MAP_X_LENGTH - 1){
                    currentMap->map[x][y] = SYMBOLS[PATH];
                }
                //Fill map South opening
                if(x == randomSouthOpening && y == MAP_Y_LENGTH - 1){
                    currentMap->map[x][y] = SYMBOLS[PATH];
                }
                //Fill map West opening
                if(y == randomWestOpening && x == 0){
                    currentMap->map[x][y] = SYMBOLS[PATH];
                }
            }
        }
    }


    //Pathfind East
    for(int x = 1; x < MAP_X_LENGTH / 2; x++){
        currentMap->map[x][randomWestOpening] = SYMBOLS[PATH];
    }
    //Pathfind West
    for(int x = MAP_X_LENGTH - 2; x > (MAP_X_LENGTH / 2) - 1; x--){
        currentMap->map[x][randomEastOpening] = SYMBOLS[PATH];
    }
    //Pathfind between E/W path
    if(randomEastOpening > randomWestOpening){
        int f = randomEastOpening;
        while(currentMap->map[(MAP_X_LENGTH / 2) - 1][f] != SYMBOLS[PATH]){
            currentMap->map[(MAP_X_LENGTH / 2) - 1][f] = SYMBOLS[PATH];
            f--;
        }
    }else if(randomWestOpening > randomEastOpening){
        int f = randomEastOpening;
        while(currentMap->map[(MAP_X_LENGTH / 2) - 1][f] != SYMBOLS[PATH]){
            currentMap->map[(MAP_X_LENGTH / 2) - 1][f] = SYMBOLS[PATH];
            f++;
        }
    }

    //Pathfind between N/S path to E/W path(We can improve this later to make it less UGLY)
    for(int y = 1; y < MAP_Y_LENGTH; y++){
        if(currentMap->map[randomNorthOpening][y] == SYMBOLS[CLEARING]){
            currentMap->map[randomNorthOpening][y] = SYMBOLS[PATH];
        }else{
            break;
        }
    }
    for(int y = MAP_Y_LENGTH - 2; y > 0; y--){
        if(currentMap->map[randomSouthOpening][y] == SYMBOLS[CLEARING]){
            currentMap->map[randomSouthOpening][y] = SYMBOLS[PATH];
        }else{
            break;
        }
    }

    //Calculate Manhattan distance for dumb shit
    int xDifference = abs(newMapX - 199);
    int yDifference = abs(newMapY - 199);
    double manhattanDistance = xDifference + yDifference;
    double chanceForBuildingsIDFK;
    if(manhattanDistance > 200){
        chanceForBuildingsIDFK = 0.05;
    }else{
        chanceForBuildingsIDFK = (((-45 * (manhattanDistance) )/ 200) + 50) / 100;
    }
    double ranNum = ((double) rand() / (RAND_MAX));
    int spotFound = 0;
    //Time to place Poke Center + Mart
    //Create two arrays to hold locations for Poke Center and mart(I could do this with just one IG)
    int randomPokeLocationX[4];
    int foundPokeLocationY[4];
    if(ranNum < chanceForBuildingsIDFK){
        //Boolean for checking if a location is found
        while(spotFound != 2){
            //Randomly generate location for Poke Center/Poke Mart
            randomPokeLocationX[spotFound] = (rand() % ((MAP_X_LENGTH - 3) - 3)) + 3;
            for(foundPokeLocationY[spotFound] = 1; foundPokeLocationY[spotFound] < MAP_Y_LENGTH - 1; foundPokeLocationY[spotFound]++){
                //Search for path on Y plane
                if(currentMap->map[randomPokeLocationX[spotFound]][foundPokeLocationY[spotFound] + 1] == SYMBOLS[PATH]){
                    //When path is found, check if we can place it.
                    if(currentMap->map[randomPokeLocationX[spotFound]][foundPokeLocationY[spotFound]] == SYMBOLS[CLEARING] && currentMap->map[randomPokeLocationX[spotFound] + 1][foundPokeLocationY[spotFound]] == SYMBOLS[CLEARING]
                       && currentMap->map[randomPokeLocationX[spotFound]][foundPokeLocationY[spotFound] - 1] == SYMBOLS[CLEARING] && currentMap->map[randomPokeLocationX[spotFound] + 1][foundPokeLocationY[spotFound] - 1] == SYMBOLS[CLEARING]){
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
                        currentMap->map[randomPokeLocationX[spotFound] + x][foundPokeLocationY[spotFound] + y] = SYMBOLS[TALL_GRASS];
                    }
                }
            }
        }
        spotFound++;
    }
    //Check for edge of map N/S/E/W and if found replacing edge with barrier
    if(newMapY == 0){
        //If on north edge of world map, we shouldn't have an opening so replace it
        currentMap->map[randomNorthOpening][0] = SYMBOLS[BOULDER];
    }
    if(newMapY == 398) {
        //If on South edge of world map, we shouldn't have an opening so replace it
        currentMap->map[randomSouthOpening][MAP_Y_LENGTH - 1] = SYMBOLS[BOULDER];
    }
    if(newMapX == 398){
        //If on East edge of world map, we shouldn't have an opening so replace it
        currentMap->map[MAP_X_LENGTH - 1][randomEastOpening] = SYMBOLS[BOULDER];
    }
    if(newMapX == 0){
        //If on West edge of world map, we shouldn't have an opening so replace it
        currentMap->map[0][randomWestOpening] = SYMBOLS[BOULDER];
    }

    currentMap->northOpening = randomNorthOpening;
    currentMap->eastOpening = randomEastOpening;
    currentMap->southOpening = randomSouthOpening;
    currentMap->westOpening = randomWestOpening;
    worldMap[newMapX][newMapY] = currentMap;
}

void GetUserInput() {
    //Start in center
    int currX = 199;
    int currY = 199;
    int isValidMovement = ' ';
    GenerateMap(currX, currY);
    PlacePlayerCharacter(currX, currY);
    while (1) {
        system("clear");
        DisplayMap(worldMap[currX][currY]);
        char userInputCharacter = ' ';
        int userInputX = -1, userInputY = -1;
        //This cleanses color output
        printf("\033[0m");
        //Printout current location
        if (isValidMovement != ' ') {
            printf("Invalid movement command(%c), ", isValidMovement);
            isValidMovement = ' ';
        }
        printf("Current Location: (%d, %d)", currX - 199, currY - 199);
        //If previous command was invalid, show it and the command IG
        printf("\nEnter New Command: ");
        //Get Input
        scanf(" %c", &userInputCharacter);
        //printf("\n");
        if (userInputCharacter == 'f' || userInputCharacter == 'F') {
            scanf(" %d %d", &userInputX, &userInputY);
            //format for like stupid user input shit
            userInputX = userInputX + 199;
            userInputY = userInputY + 199;
            if (CheckBoundsValid(userInputX, 398) == 1 && CheckBoundsValid(userInputY, 398) == 1) {
                if (worldMap[userInputX][userInputY] == NULL) {
                    GenerateMap(userInputX, userInputY);
                    currX = userInputX;
                    currY = userInputY;
                    continue;
                } else {
                    currentMap = worldMap[userInputX][userInputY];
                    currX = userInputX;
                    currY = userInputY;
                    continue;
                }
            }
        } else if (userInputCharacter == 'q' || userInputCharacter == 'Q') {
            printf("You don't want to play this poorly made game anymore?='(\n");
            break;
        } else if (userInputCharacter == 'n' || userInputCharacter == 'N') {
            //Generate map to the north if applicable
            if (currY <= 0) {
                isValidMovement = userInputCharacter;
                continue;
            } else if (worldMap[currX][currY - 1] == NULL) {
                currY--;
                GenerateMap(currX, currY);
            } else {
                currY--;
                currentMap = worldMap[currX][currY];
            }
        } else if (userInputCharacter == 's' || userInputCharacter == 'S') {
            //Generate map to the south if applicable
            if (currY >= 398) {
                isValidMovement = userInputCharacter;
                continue;
            } else if (worldMap[currX][currY + 1] == NULL) {
                currY++;
                GenerateMap(currX, currY);
            } else {
                currY++;
                currentMap = worldMap[currX][currY];
            }
        } else if (userInputCharacter == 'e' || userInputCharacter == 'E') {
            //Generate map the east if applicable
            if (currX >= 398) {
                isValidMovement = userInputCharacter;
                continue;
            } else if (worldMap[currX + 1][currY] == NULL) {
                currX++;
                GenerateMap(currX, currY);
            } else {
                currX++;
                currentMap = worldMap[currX][currY];
            }
        } else if (userInputCharacter == 'w' || userInputCharacter == 'W') {
            //Generate map to the west if applicable
            if (currX <= 0) {
                isValidMovement = userInputCharacter;
                continue;
            } else if (worldMap[currX - 1][currY] == NULL) {
                currX--;
                GenerateMap(currX, currY);
            } else {
                currX--;
                currentMap = worldMap[currX][currY];
            }
        }
        char cleanse = '?';
        while (cleanse != '\n') {
            scanf("%c", &cleanse);
        }
    }
}

int main() {
    pc = malloc(sizeof(playerCharacter));
    //Start with generating random
    srand(time(NULL));
    //Get user input
    GetUserInput();
}




