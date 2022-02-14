#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define MAP_X_WIDTH 80
#define MAP_Y_WIDTH 21

char symbols[] = {'C', 'M', '.', ';', '%', '#', '@'};
// 'C' = PokeCenter, 'M' = PokeMart, '.' = Clearing, ';' = TallGrass, '%' = Boulder, '#' = FootPath, '@' = Player


typedef struct mapGrid{
    char map[MAP_X_WIDTH][MAP_Y_WIDTH];
    int northOpening, eastOpening, southOpening, westOpening;
} mapGrid;

mapGrid *currentMap;
mapGrid *worldMap[399][399];

typedef struct playerCharacter{
    int worldMapX, worldMapY, mapX, mapY;
} playerCharacter;

playerCharacter *pc;

void DisplayMap(mapGrid *map){
    for(int y = 0; y < MAP_Y_WIDTH; y++){
        for(int x = 0; x < MAP_X_WIDTH; x++){
            //Lets color code the output so it looks better :)
            char c = map->map[x][y];
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
    if(newMapX > 398 || newMapY > 398 || newMapX < 0 || newMapY < 0){
        DisplayMap(currentMap);
        printf("\nGenerating map in invalid spot.\n");
        return;
    }
    currentMap = malloc(sizeof(mapGrid));

    //Generate random openings for map on N/E/S/W sides
    int randomNorthOpening = (rand() % ((MAP_X_WIDTH - 2) - 2)) + 2;
    int randomEastOpening = (rand() % ((MAP_Y_WIDTH - 2) - 2)) + 2;
    int randomSouthOpening = (rand() % ((MAP_X_WIDTH - 2) - 2)) + 2;
    int randomWestOpening = (rand() % ((MAP_Y_WIDTH - 2) - 2)) + 2;

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
    for(int x = 0; x < MAP_X_WIDTH; x++){
        for(int y = 0; y < MAP_Y_WIDTH; y++){
            //Fill entire array with Clearing
            currentMap->map[x][y] = '.';
            if(y == 0 || y == MAP_Y_WIDTH - 1 || x == 0 || x == MAP_X_WIDTH - 1){
                currentMap->map[x][y] = symbols[4];
                //Fill map North opening
                if(x == randomNorthOpening && y == 0){
                    currentMap->map[x][y] = symbols[5];
                }
                //Fill map East opening
                if(y == randomEastOpening && x == MAP_X_WIDTH - 1){
                    currentMap->map[x][y] = symbols[5];
                }
                //Fill map South opening
                if(x == randomSouthOpening && y == MAP_Y_WIDTH - 1){
                    currentMap->map[x][y] = symbols[5];
                }
                //Fill map West opening
                if(y == randomWestOpening && x == 0){
                    currentMap->map[x][y] = symbols[5];
                }
            }
        }
    }


    //Pathfind East
    for(int x = 1; x < MAP_X_WIDTH / 2; x++){
        currentMap->map[x][randomWestOpening] = symbols[5];
    }
    //Pathfind West
    for(int x = MAP_X_WIDTH - 2; x > (MAP_X_WIDTH / 2) - 1; x--){
        currentMap->map[x][randomEastOpening] = symbols[5];
    }
    //Pathfind between E/W path
    if(randomEastOpening > randomWestOpening){
        int f = randomEastOpening;
        while(currentMap->map[(MAP_X_WIDTH / 2) - 1][f] != symbols[5]){
            currentMap->map[(MAP_X_WIDTH/2)-1][f] = symbols[5];
            f--;
        }
    }else if(randomWestOpening > randomEastOpening){
        int f = randomEastOpening;
        while(currentMap->map[(MAP_X_WIDTH / 2) - 1][f] != symbols[5]){
            currentMap->map[(MAP_X_WIDTH/2)-1][f] = symbols[5];
            f++;
        }
    }

    //Pathfind between N/S path to E/W path(We can improve this later to make it less UGLY)
    for(int y = 1; y < MAP_Y_WIDTH; y++){
        if(currentMap->map[randomNorthOpening][y] == symbols[2]){
            currentMap->map[randomNorthOpening][y] = symbols[5];
        }else{
            break;
        }
    }
    for(int y = MAP_Y_WIDTH - 2; y > 0; y--){
        if(currentMap->map[randomSouthOpening][y] == symbols[2]){
            currentMap->map[randomSouthOpening][y] = symbols[5];
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
            randomPokeLocationX[spotFound] = (rand() % ((MAP_X_WIDTH - 3) - 3)) + 3;
            for(foundPokeLocationY[spotFound] = 1; foundPokeLocationY[spotFound] < MAP_Y_WIDTH - 1; foundPokeLocationY[spotFound]++){
                //Search for path on Y plane
                if(currentMap->map[randomPokeLocationX[spotFound]][foundPokeLocationY[spotFound] + 1] == symbols[5]){
                    //When path is found, check if we can place it.
                    if(currentMap->map[randomPokeLocationX[spotFound]][foundPokeLocationY[spotFound]] == symbols[2] && currentMap->map[randomPokeLocationX[spotFound] + 1][foundPokeLocationY[spotFound]] == symbols[2]
                       && currentMap->map[randomPokeLocationX[spotFound]][foundPokeLocationY[spotFound] - 1] == symbols[2] && currentMap->map[randomPokeLocationX[spotFound] + 1][foundPokeLocationY[spotFound] - 1] == symbols[2]){
                        //Place pokecenter/mart at found spot
                        currentMap->map[randomPokeLocationX[spotFound]][foundPokeLocationY[spotFound]] = symbols[spotFound];
                        currentMap->map[randomPokeLocationX[spotFound] + 1][foundPokeLocationY[spotFound]] = symbols[spotFound];
                        currentMap->map[randomPokeLocationX[spotFound]][foundPokeLocationY[spotFound] - 1] = symbols[spotFound];
                        currentMap->map[randomPokeLocationX[spotFound] + 1][foundPokeLocationY[spotFound] - 1] = symbols[spotFound];
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
        randomPokeLocationX[spotFound] = (rand() % ((MAP_X_WIDTH - 1) - 1)) + 1;
        foundPokeLocationY[spotFound] = (rand() % ((MAP_Y_WIDTH - 1) - 1)) + 1;
        //Place correct number of X grasses
        for (int x = 0; x < randomTallGrassX[spotFound - 2]; x++) {
            //Place correct number of Y grasses
            for (int y = 0; y < randomTallGrassY[spotFound - 2]; y++) {
                //Place all available X/Y coordinate grass
                if ((randomPokeLocationX[spotFound] + x) < MAP_X_WIDTH - 2) {
                    if (currentMap->map[randomPokeLocationX[spotFound] + x][foundPokeLocationY[spotFound] + y] == '.') {
                        currentMap->map[randomPokeLocationX[spotFound] + x][foundPokeLocationY[spotFound] +
                                                                            y] = symbols[3];
                    }
                }
            }
        }
        spotFound++;
    }
    //Check for edge of map N/S/E/W and if found replacing edge with barrier
    if(newMapY == 0){
        //If on north edge of world map, we shouldn't have an opening so replace it
        currentMap->map[randomNorthOpening][0] = symbols[4];
    }
    if(newMapY == 398) {
        //If on South edge of world map, we shouldn't have an opening so replace it
        currentMap->map[randomSouthOpening][MAP_Y_WIDTH - 1] = symbols[4];
    }
    if(newMapX == 398){
        //If on East edge of world map, we shouldn't have an opening so replace it
        currentMap->map[MAP_X_WIDTH - 1][randomEastOpening] = symbols[4];
    }
    if(newMapX == 0){
        //If on West edge of world map, we shouldn't have an opening so replace it
        currentMap->map[0][randomWestOpening] = symbols[4];
    }

    currentMap->northOpening = randomNorthOpening;
    currentMap->eastOpening = randomEastOpening;
    currentMap->southOpening = randomSouthOpening;
    currentMap->westOpening = randomWestOpening;
    worldMap[newMapX][newMapY] = currentMap;
}

int CheckBoundsValid(int x, int xMax){
    if(x > xMax || x < 0){
        return 0;
    }else{
        return 1;
    }
}

void PlacePlayerCharacter(int currX, int currY, playerCharacter *pc){
    pc = malloc(sizeof(playerCharacter));
    int randomPlayerLocation = (rand() % ((MAP_X_WIDTH - 2) - 2)) + 2;
    //Let's look for a spot we can place a player, and if it's found then place em idfk
    for(int y = 1; y < MAP_Y_WIDTH - 1; y++){
        if(worldMap[currX][currY]->map[randomPlayerLocation][y] == symbols[5]){
            worldMap[currX][currY]->map[randomPlayerLocation][y] = symbols[6];
            //We now want to also update player info
            pc->worldMapX = currX;
            pc->worldMapY = currY;
            pc->mapX = randomPlayerLocation;
            pc->mapY = y;
        }
    }
}

void GetUserInput() {
    //Start in center
    int currX = 199;
    int currY = 199;
    int isValidMovement = ' ';
    GenerateMap(currX, currY);
    PlacePlayerCharacter(currX, currY, pc);
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
        scanf("%c", &userInputCharacter);
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
    //Start with generating random
    srand(time(NULL));
    //Get user input
    GetUserInput();
}




