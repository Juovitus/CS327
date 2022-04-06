#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "heap.h"
#include<ncurses.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <regex>
#include <random>

using namespace std;

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
#define BATTLE_CHANCE 1 //Battle chance is 1/BATTLE_CHANCE when walking over grass
#define SHINY_RATE 3   //Shiny rate is 1/SHINY RATE when encountering a pokemon
#define MESSAGE_RED 99

char SYMBOLS[] = {'C', 'M', '.', ';', '%', '#', '@', 'h', 'r', 'p', 'w', 's', 'n'};
//COST ORDER = HIKER_COST->RIVAL_COST->PC->OTHERS
int COST_PATH_OR_CLEARING[] = {10, 10, 10, 10};
int COST_BUILDING[] = {INT32_MAX, INT32_MAX, 10, INT32_MAX};
int COST_TALL_GRASS[] = {15, 20, 20, 20};
//Default number of trainers if not passed in a number
int numTrainers = 10;
string inputFiles[] = {"experience", "moves", "pokemon", "pokemon_moves", "pokemon_species", "type_names", "pokemon_stats"};
int quit_game = 0;
char debugMessage[] = {'D', 'E', 'B', 'U', 'G', ' ', 'L', 'I', 'N', 'E'};
ifstream currFile;
double manhattanDistance = 999.99;
//string pokemonStats[] = {"", "hp", "attack", "defense", "special-attack", "special-defense", "speed", "accuracy", "evasion"};

class nonPlayerCharacter{
public:
    int npcType, mapX, mapY, isAlive, nextMoveTime, direction;
    char spawnTileType;
};

class mapGrid{
public:
    char map[MAP_X_LENGTH][MAP_Y_LENGTH];
    int northOpening, eastOpening, southOpening, westOpening;
    int worldLocationX, worldLocationY;
    int hikerMap[MAP_X_LENGTH][MAP_Y_LENGTH];
    int rivalMap[MAP_X_LENGTH][MAP_Y_LENGTH];
    class nonPlayerCharacter npc[MAP_X_LENGTH * MAP_Y_LENGTH];
    heap_t npcHeap;
    int currentTime;
};
class mapGrid *currentMap;
class mapGrid *worldMap[399][399];

class Move{
public:
    string name;
    int ID, generation, type, power, pp, accuracy, priority, target,
    damage_class, effect, effectChance, contestType, contestEffect, superContestEffect;
};

class PokemonMove{
public:
    int pokemonID, version, moveID, moveMethod, level, order;
};

class PokemonSpecies{
public:
    string name;
    int ID, generation, evolvesFromID, evolutionID, color, shape,
    habitat, gender, captureRate, baseHappiness, isBaby, hatchCounter, genderDifferences,
    growthRate, differentForms, isLegendary, isMythical, order, conquestOrder;
};

class Experience{
public:
    int growthRate, level, experience;
};

class TypeNames{
public:
    string type;
    int ID, localLanguage;
};

class PokemonStats{
public:
    int ID, statID, base_Stat, effort;
};

class Pokemon{
public:
    string name;
    //1 is male 2 is female
    int ID, speciesID, height, weight, baseEXP, order, is_default, level, gender,
    hp, attack, defense, special_attack, special_defense, speed;
    //A pokemon can only have a max number of four moves.
    vector <PokemonMove> moves;
    vector <PokemonMove> availableMoves;
    bool isShiny = false;
};

vector<Pokemon> pokemon;
vector<Move> moves;
vector<PokemonMove> pokemonMoves;
vector<PokemonSpecies> pokemonSpecies;
vector<Experience> experience;
vector<TypeNames> typeNames;
vector<PokemonStats> pokemonStats;

typedef struct Point{
    int xPos, yPos;
} point_t;

typedef struct playerCharacter{
    int mapX, mapY, isValidMovement;
    Pokemon team[6];
} playerCharacter;
playerCharacter *pc;

typedef struct path {
    heap_node_t *hn;
    int pos[2];
    int from[2];
    int cost;
} path_t;

void DisplayMap(class mapGrid map){
    for(int i = 0; i < (signed)sizeof(debugMessage); i++){
        mvaddch(0,i,debugMessage[i]);
    }
    printw("\n");//add +1 to y so it doesnt overwrite debug
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

            if(pc != nullptr && x == pc->mapX && y == pc->mapY){
                c = SYMBOLS[SYMBOL_PLAYER];
            }else{
                c = map.map[x][y];
            }

            for(int n = 0; n < numTrainers; n++){
                if(currentMap->npc[n].mapX == x && currentMap->npc[n].mapY == y){
                    c = SYMBOLS[currentMap->npc[n].npcType];
                }
            }

            switch(c) {
                case 'C' :
                    attron(COLOR_PAIR(SYMBOL_POKE_CENTER));
                    mvaddch(y+1,x,c);
                    attroff(COLOR_PAIR(SYMBOL_POKE_CENTER));
                    break;
                case 'M' :
                    attron(COLOR_PAIR(SYMBOL_POKE_MART));
                    mvaddch(y+1,x,c);
                    attroff(COLOR_PAIR(SYMBOL_POKE_MART));
                    break;
                case '.' :
                    attron(COLOR_PAIR(SYMBOL_CLEARING));
                    mvaddch(y+1,x,c);
                    attroff(COLOR_PAIR(SYMBOL_CLEARING));
                    break;
                case ';' :
                    attron(COLOR_PAIR(SYMBOL_TALL_GRASS));
                    mvaddch(y+1,x,c);
                    attroff(COLOR_PAIR(SYMBOL_TALL_GRASS));
                    break;
                case '%' :
                    attron(COLOR_PAIR(SYMBOL_BOULDER));
                    mvaddch(y+1,x,c);
                    attroff(COLOR_PAIR(SYMBOL_BOULDER));
                    break;
                case '#' :
                    attron(COLOR_PAIR(SYMBOL_PATH));
                    mvaddch(y+1,x,c);
                    attroff(COLOR_PAIR(SYMBOL_PATH));
                    break;
                case '@' :
                    attron(COLOR_PAIR(SYMBOL_PLAYER));
                    mvaddch(y+1,x,c);
                    attroff(COLOR_PAIR(SYMBOL_PLAYER));
                    break;
                case 'h' :
                    attron(COLOR_PAIR(SYMBOL_RIVAL));
                    mvaddch(y+1,x,c);
                    attroff(COLOR_PAIR(SYMBOL_RIVAL));
                    break;
                case 'r' :
                    attron(COLOR_PAIR(SYMBOL_RIVAL));
                    mvaddch(y+1,x,c);
                    attroff(COLOR_PAIR(SYMBOL_RIVAL));
                    break;
                case 'p' :
                    attron(COLOR_PAIR(SYMBOL_RIVAL));
                    mvaddch(y+1,x,c);
                    attroff(COLOR_PAIR(SYMBOL_RIVAL));
                    break;
                case 'w' :
                    attron(COLOR_PAIR(SYMBOL_RIVAL));
                    mvaddch(y+1,x,c);
                    attroff(COLOR_PAIR(SYMBOL_RIVAL));
                    break;
                case 's' :
                    attron(COLOR_PAIR(SYMBOL_RIVAL));
                    mvaddch(y+1,x,c);
                    attroff(COLOR_PAIR(SYMBOL_RIVAL));
                    break;
                case 'n' :
                    attron(COLOR_PAIR(SYMBOL_RIVAL));
                    mvaddch(y+1,x,c);
                    attroff(COLOR_PAIR(SYMBOL_RIVAL));
                    break;
                default:
                    attron(COLOR_PAIR(SYMBOL_TALL_GRASS));
                    mvaddch(y+1,x,c);
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

void StartBattle(nonPlayerCharacter* npc){
    int leaveBattle = 0;
    if(npc->isAlive){
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
            printw("You can use your escape key to exit,\n"
                   "otherwise you're stuck here cuddling with this cat.\n"
                   "I know it sucks.");
            refresh();
            char userInput = getch();
            if(userInput == ESCAPE_KEY){
                leaveBattle = 1;
                npc->isAlive = 0;
            }
        }
    }else{
        pc->isValidMovement = 0;
    }
    //Clear screen before going back
    clear();
}

void EnterPokeCenter(){
    int leaveBuilding = 0;
    while(!leaveBuilding){
        //CLEAR SCREEN BEFORE STUFF
        clear();
        printw("Temporary PokeCenter page\n"
               "Something appears!\n");
        printw("                        ____\n"
               "                   .---'-    \\\n"
               "      .-----------/           \\\n"
               "     /           (         ^  |   __\n"
               "&   (             \\        O  /  / .'\n"
               "'._/(              '-'  (.   (_.' /\n"
               "     \\                    \\     ./\n"
               "      |    |       |    |/ '._.'\n"
               "       )   @).____\\|  @ |\n"
               "   .  /    /       (    | \n"
               "  \\|, '_:::\\  . ..  '_:::\\ ..\\).");
        printw("\nYou can use your < key to exit,\n"
               "otherwise you're stuck here.\n"
               "I know it sucks.");
        refresh();
        char userInput = getch();
        if(userInput == '<'){
            leaveBuilding = 1;
        }
    }
    //Clear screen before going back
    clear();
}

void EnterPokeMart(){
    int leaveBuilding = 0;
    while(!leaveBuilding){
        //CLEAR SCREEN BEFORE STUFF
        clear();
        printw("Temporary PokeMart page\n");
        printw("   |\\---/|\n");
        printw("   | ,_, |\n");
        printw("    \\_`_/-..----.\n");
        printw(" ___/ `   ' ,\"\"+ \\ \n");
        printw("(__...'   __\\    |`.___.';\n");
        printw("  (_,...'(_,.`__)/'.....+\n");
        printw("You can use your < key to exit,\n"
               "otherwise you're stuck here.\n"
               "I know it sucks.");
        refresh();
        char userInput = getch();
        if(userInput == '<'){
            leaveBuilding = 1;
        }
    }
    //Clear screen before going back
    clear();
}

void EnterPokemonBattle(){
    int maxLevel, minLevel;
    if(manhattanDistance > 200){
        minLevel = (int)((manhattanDistance - 200) / 2);
        maxLevel = 100;
        if(minLevel < 1){
            minLevel = 1;
        }
    }else{
        minLevel = 1;
        maxLevel = (int)(manhattanDistance / 2);
        if(maxLevel < 1){
            maxLevel = 1;
        }
    }
    //Set level
    Pokemon encounterPokemon = pokemon.at((rand()%((pokemon.size() - 1))));
    encounterPokemon.level = (rand()%(maxLevel-minLevel + 1) + minLevel);
    //Set all available pokemon moves
    for(PokemonMove pm: pokemonMoves){
        if(pm.version == 19 && pm.pokemonID == encounterPokemon.speciesID && pm.moveMethod == 1 && pm.level <= encounterPokemon.level){
            encounterPokemon.availableMoves.push_back(pm);
        }
    }
    bool noMoves = false;
    //IDK why moves are empty sometimes but if they are add tackle IG?
    if(encounterPokemon.availableMoves.empty()){
        encounterPokemon.availableMoves.push_back(pokemonMoves.at(3));
        noMoves = true;
    }
    //Set pokemon name, include if its shiny.
    string printPokemonName;
    if((rand()%(SHINY_RATE-1 + 1) + 1) == 1){
        encounterPokemon.isShiny = true;
        printPokemonName += "Shiny ";
    }
    //Pokemon gender?
    encounterPokemon.gender = (rand()%(2-1 + 1) + 1);
    //Set pokemon moves?
    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(0,(int)encounterPokemon.availableMoves.size() - 1);
    for(int i = 0; i < 200; i++){
        PokemonMove uniformMove = encounterPokemon.availableMoves.at(distribution(generator));
        if(encounterPokemon.moves.size() < 2){
            if(encounterPokemon.moves.size() == 0){
                encounterPokemon.moves.push_back(uniformMove);
            }else{
                //Check if unique move, if it is then add it
                bool unique = true;
                for(PokemonMove pm: encounterPokemon.moves){
                    if(pm.moveID == uniformMove.moveID){
                        unique = false;
                        break;
                    }
                }
                if(unique){
                    encounterPokemon.moves.push_back(uniformMove);
                }
            }
        }else{
            break;
        }
    }
    //Add in pokemon IV'S?
    for(PokemonStats pmstat: pokemonStats){
        if(pmstat.ID == encounterPokemon.speciesID){
            switch(pmstat.statID){
                case 1:
                    encounterPokemon.hp = pmstat.base_Stat;
                    break;
                case 2:
                    encounterPokemon.attack = pmstat.base_Stat;
                    break;
                case 3:
                    encounterPokemon.defense = pmstat.base_Stat;
                    break;
                case 4:
                    encounterPokemon.special_attack = pmstat.base_Stat;
                    break;
                case 5:
                    encounterPokemon.special_defense = pmstat.base_Stat;
                    break;
                case 6:
                    encounterPokemon.speed = pmstat.base_Stat;
                    break;
            }
        }
    }

    init_pair(SHINY_RATE, COLOR_YELLOW, COLOR_BLACK);
    init_pair(MESSAGE_RED, COLOR_RED, COLOR_BLACK);
    init_pair(1, COLOR_MAGENTA, COLOR_BLACK);
    printPokemonName += encounterPokemon.name;
    int leaveBattle = 0;
    while(!leaveBattle){
        //CLEAR SCREEN BEFORE STUFF
        clear();
        printw("A wild pokemon has appeared! ~Battle Music~\n");
        attron(COLOR_PAIR(1));
        printw("Shiny chance is currently 1/%d, before you pog-off Zach ;)\n", SHINY_RATE);
        attroff(COLOR_PAIR(1));
        printw("Level %d ", encounterPokemon.level);
        if(encounterPokemon.isShiny){
            attron(COLOR_PAIR(SHINY_RATE));
            printw("%s\n", printPokemonName.c_str());
            attroff(COLOR_PAIR(SHINY_RATE));
        }else{
            printw("%s\n", printPokemonName.c_str());
        }
        printw("Pokemon Moveset:\n");
        for(PokemonMove pm: encounterPokemon.moves){
            for(Move m: moves){
                if(m.ID == pm.moveID){
                    printw("                %s\n", m.name.c_str());
                }
            }
        }
        printw("----------------------------------------------------------------------------\n");
        printw("Pokemon IVs:\n");
        printw("    HP: %d  |  Attack: %d  |  Defense: %d  |  Special-Attack: %d\n", encounterPokemon.hp, encounterPokemon.attack, encounterPokemon.defense, encounterPokemon.special_attack);
        printw("    Special-Defense: %d  |  Speed: %d\n", encounterPokemon.special_defense, encounterPokemon.speed);
        printw("----------------------------------------------------------------------------\n");
        if(noMoves){
            attron(COLOR_PAIR(MESSAGE_RED));
            printw("Pokemon had no available moves so tackle was added.\n");
            attroff(COLOR_PAIR(MESSAGE_RED));
        }
        printw("Use < to exit battle.\n");
        refresh();
        char userInput = getch();
        if(userInput == '<'){
            leaveBattle = 1;
        }
    }
    //Clear screen before going back
    clear();
}

void PrintTrainers(int screenNum, int totalScreens){
    printw("------------------TRAINERS PAGE %d/%d------------------\n", screenNum, totalScreens);
    printw("Hiker = %d, Rival = %d, Pacer = %d, Wanderer = %d,\n"
           "Stationary = %d, Random Walker = %d", SYMBOL_HIKER, SYMBOL_RIVAL, SYMBOL_PACER, SYMBOL_WANDERER, SYMBOL_STATIONARY, SYMBOL_RANDOM_WALKER);
    printw("\n-----------------------------------------------------\n");
    int startPos = (screenNum * 19) - 19;
    for(int i = startPos; i < startPos + 19; i++) {
        if(i > numTrainers - 1){
            break;
        }
        printw("NpcNUM: %d, Type: %d, Steps North: %d, Steps East: %d\n", i, currentMap->npc[i].npcType, pc->mapY - currentMap->npc[i].mapY, currentMap->npc[i].mapX - pc->mapX);
    }
    printw("Up, Down and ESC to navigate.");
    refresh();
}

void DisplayTrainers(){
    int leaveTrainerScreen = 0;
    int screenNum = 1;
    int totalScreens = (numTrainers / 20) + 1; //Min one screen
    //char location[] = "Trainer is";
    while(!leaveTrainerScreen){
        //CLEAR SCREEN BEFORE STUFF
        clear();
        PrintTrainers(screenNum, totalScreens);
        refresh();
        int userInput = getch();
        if(userInput == ESCAPE_KEY) {
            leaveTrainerScreen = 1;
        }else if(userInput == KEY_DOWN){
            if(screenNum >= totalScreens){
                PrintTrainers(screenNum, totalScreens);
            }else{
                screenNum++;
                PrintTrainers(screenNum, totalScreens);
            }
        }else if(userInput == KEY_UP){
            if(screenNum <= 1){
                PrintTrainers(screenNum, totalScreens);
            }else{
                screenNum--;
                PrintTrainers(screenNum, totalScreens);
            }
        }
    }
    //Clear screen before going back
    clear();
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
                if(npc->mapX + x == pc->mapX && npc->mapY + y == pc->mapY &&
                   currentMap->map[pc->mapX][pc->mapY] != SYMBOLS[SYMBOL_POKE_CENTER] && currentMap->map[pc->mapX][pc->mapY] != SYMBOLS[SYMBOL_POKE_MART]){
                    StartBattle(npc);
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
                if(npc->mapX + x == pc->mapX && npc->mapY + y == pc->mapY  &&
                   currentMap->map[pc->mapX][pc->mapY] != SYMBOLS[SYMBOL_POKE_CENTER] && currentMap->map[pc->mapX][pc->mapY] != SYMBOLS[SYMBOL_POKE_MART]){
                    StartBattle(npc);
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
    if(npc->mapX + xDiff == pc->mapX && npc->mapY + yDiff == pc->mapY &&
       currentMap->map[pc->mapX][pc->mapY] != SYMBOLS[SYMBOL_POKE_CENTER] && currentMap->map[pc->mapX][pc->mapY] != SYMBOLS[SYMBOL_POKE_MART]) {
        StartBattle(npc);
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
    if(npc->mapX + xDiff == pc->mapX && npc->mapY + yDiff == pc->mapY &&
       currentMap->map[pc->mapX][pc->mapY] != SYMBOLS[SYMBOL_POKE_CENTER] && currentMap->map[pc->mapX][pc->mapY] != SYMBOLS[SYMBOL_POKE_MART]){
        StartBattle(npc);
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
    if(npc->mapX + xDiff == pc->mapX && npc->mapY + yDiff == pc->mapY &&
       currentMap->map[pc->mapX][pc->mapY] != SYMBOLS[SYMBOL_POKE_CENTER] && currentMap->map[pc->mapX][pc->mapY] != SYMBOLS[SYMBOL_POKE_MART]){
        StartBattle(npc);
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
        if(npc->npcType == SYMBOL_HIKER && npc->isAlive){
            MoveHiker(npc);
        }else if(npc->npcType == SYMBOL_RIVAL && npc->isAlive){
            MoveRival(npc);
        }else if(npc->npcType == SYMBOL_PACER && npc->isAlive){
            MovePacer(npc);
        }else if(npc->npcType == SYMBOL_WANDERER && npc->isAlive){
            MoveWanderer(npc);
        }else if(npc->npcType == SYMBOL_RANDOM_WALKER && npc->isAlive){
           MoveRandomWalker(npc);
        }else{
            continue;
        }
        //Add to heap
        heap_insert(&currentMap->npcHeap, npc);
    }
}

int32_t Dijkstra_Path(class mapGrid map, struct Point from, struct Point to, int characterType){
    static path_t path[MAP_Y_LENGTH][MAP_X_LENGTH], *p;

    heap_t h;
    int x, y;
    int dim_x = 1, dim_y = 0;
    //Set costs for current character type
    int costCenter = COST_BUILDING[characterType];
    int costMart = COST_BUILDING[characterType];
    int costPath = COST_PATH_OR_CLEARING[characterType];
    int costTallGrass = COST_TALL_GRASS[characterType];
    int costClearing = COST_PATH_OR_CLEARING[characterType];
    int costMap[MAP_X_LENGTH][MAP_Y_LENGTH];

    //Set positions and cost
    for(y = 0; y < MAP_Y_LENGTH; y++){
        for(x = 0; x < MAP_X_LENGTH; x++){
            path[y][x].pos[dim_x] = x;
            path[y][x].pos[dim_y] = y;
            path[y][x].cost = INT32_MAX;
            //Set costs for hiker and rival in map
            if(map.map[x][y] == SYMBOLS[SYMBOL_POKE_CENTER]){
                costMap[x][y] = costCenter;
            } else if(map.map[x][y] == SYMBOLS[SYMBOL_POKE_MART]){
                costMap[x][y] = costMart;
            } else if(map.map[x][y] == SYMBOLS[SYMBOL_PATH]){
                costMap[x][y] = costPath;
            } else if(map.map[x][y] == SYMBOLS[SYMBOL_TALL_GRASS]){
                costMap[x][y] = costTallGrass;
            } else if(map.map[x][y] == SYMBOLS[SYMBOL_CLEARING]){
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
            if(map.map[x][y] != SYMBOLS[SYMBOL_BOULDER] && !IsNpcAtXY(x, y)){
                if(characterType == PLAYER_COST){
                    path[y][x].hn = heap_insert(&h, &path[y][x]);
                }else if(map.map[x][y] != SYMBOLS[SYMBOL_POKE_CENTER] && map.map[x][y] != SYMBOLS[SYMBOL_POKE_MART]){
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

void GenerateCostMap(int characterType){
    for(int y = 0; y < MAP_Y_LENGTH; y++){
        for(int x = 0; x < MAP_X_LENGTH; x++){
            point_t to;
            to.xPos = x;
            to.yPos = y;
            point_t from;
            from.xPos = pc->mapX;
            from.yPos = pc->mapY;
            if(characterType == HIKER_COST){
                currentMap->hikerMap[x][y] = Dijkstra_Path(*currentMap, from, to, HIKER_COST);
            }else if (characterType == RIVAL_COST){
                currentMap->rivalMap[x][y] = Dijkstra_Path(*currentMap, from, to, RIVAL_COST);
            }else{
                printf("\nWAT?\n");
            }
        }
    }
}

void PrintCostMap(int characterType){
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

void PlacePlayerCharacter() {
    int randomPlayerLocationX = (rand() % ((MAP_X_LENGTH - 2) - 2)) + 2;
    //Let's look for a spot we can place a player, and if it's found then update where we want to place the player for starts
    for (int y = 1; y < MAP_Y_LENGTH - 1; y++) {
        if (currentMap->map[randomPlayerLocationX][y] == SYMBOLS[SYMBOL_PATH]) {
            //We now want to also update player info
            pc->mapX = randomPlayerLocationX;
            pc->mapY = y;
            break;
        }
    }
    GenerateCostMap(0);
    GenerateCostMap(1);
}

void GenerateMap(int newMapX, int newMapY) {
    //If out of bounds say error
    if(newMapX > 398 || newMapY > 398 || newMapX < 0 || newMapY < 0){
        DisplayMap(*currentMap);
        strcpy(debugMessage, "InvMapLoc");
        return;
    }

    currentMap = static_cast<mapGrid *>(malloc(sizeof(mapGrid)));
    //Generate random openings for map on N/E/S/W sides
    int randomNorthOpening = GenerateRandomX(2);
    int randomEastOpening = GenerateRandomY(2);
    int randomSouthOpening = GenerateRandomX(2);
    int randomWestOpening = GenerateRandomY(2);

    //Align North opening with applicable maps South opening
    if(worldMap[newMapX][newMapY - 1] != NULL){
        randomNorthOpening = worldMap[newMapX][newMapY - 1]->southOpening;
    }
    //Align South opening with applicable maps North opening
    if(worldMap[newMapX][newMapY + 1] != NULL){
        randomSouthOpening = worldMap[newMapX][newMapY + 1]->northOpening;
    }
    //Align East opening with applicable maps West opening
    if(worldMap[newMapX + 1][newMapY] != NULL){
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
    manhattanDistance = xDifference + yDifference;
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
    currentMap->worldLocationX = newMapX;
    currentMap->worldLocationY = newMapY;
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
                StartBattle(&currentMap->npc[i]);
                return 0;
            }
        }
        pc->isValidMovement = 1;
        return 1;
    }
}

void MoveMap(int x, int y, char c){
    if(worldMap[x][y] == NULL){
        GenerateMap(x, y);
        switch(c){
            case 'n':
                pc->mapY = MAP_Y_LENGTH - 2;
                break;
            case 'e':
                pc->mapX = 1;
                break;
            case 's':
                pc->mapY = 1;
                break;
            case 'w':
                pc->mapX = MAP_X_LENGTH - 2;
                break;
        }
    }else{
        currentMap = worldMap[x][y];
        switch(c){
            case 'n':
                pc->mapY = MAP_Y_LENGTH - 2;
                break;
            case 'e':
                pc->mapX = 1;
                break;
            case 's':
                pc->mapY = 1;
                break;
            case 'w':
                pc->mapX = MAP_X_LENGTH - 2;
                break;
        }
    }
    pc->isValidMovement = 1; //Jank
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
        }else if(currentMap->map[pc->mapX][pc->mapY - 1] == SYMBOLS[SYMBOL_PATH] && pc->mapY - 1 == 0){
            //If we want to move to the map to the north then do so?
            MoveMap(currentMap->worldLocationX, currentMap->worldLocationY - 1, 'n');
            isValidMovement = 1;
        }
    } else if (userInputCharacter == '6' || userInputCharacter == 'l') {
        //Check if the incremented movement is valid(Which we're going up so its x+1)
        if (IsValidPlayerMovement(pc->mapX + 1, pc->mapY)) {
            //Set updated position of player
            pc->mapX = pc->mapX + 1;
            isValidMovement = 1;
        }else if(currentMap->map[pc->mapX + 1][pc->mapY] == SYMBOLS[SYMBOL_PATH] && pc->mapX + 1 == MAP_X_LENGTH - 1){
            //If we want to move to the map to the east then do so?
            MoveMap(currentMap->worldLocationX + 1, currentMap->worldLocationY, 'e');
            isValidMovement = 1;
        }
    } else if (userInputCharacter == '2' || userInputCharacter == 'j') {
        //Check if the incremented movement is valid(Which we're going down so its y+1)
        if (IsValidPlayerMovement(pc->mapX, pc->mapY + 1)) {
            //Set updated position of player
            pc->mapY = pc->mapY + 1;
            isValidMovement = 1;
        }else if(currentMap->map[pc->mapX][pc->mapY + 1] == SYMBOLS[SYMBOL_PATH] && pc->mapY + 2 == MAP_Y_LENGTH){
            //If we want to move to the map to the south then do so?
            MoveMap(currentMap->worldLocationX, currentMap->worldLocationY + 1, 's');
            isValidMovement = 1;
        }
    } else if (userInputCharacter == '4' || userInputCharacter == 'h') {
        //Check if the incremented movement is valid(Which we're going up so its x-1)
        if (IsValidPlayerMovement(pc->mapX - 1, pc->mapY)) {
            //Set updated position of player
            pc->mapX = pc->mapX - 1;
            isValidMovement = 1;
        }else if(currentMap->map[pc->mapX - 1][pc->mapY] == SYMBOLS[SYMBOL_PATH] && pc->mapX - 1 == 0){
            //If we want to move to the map to the west then do so?
            MoveMap(currentMap->worldLocationX - 1, currentMap->worldLocationY, 'w');
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
        }else if(currentMap->map[pc->mapX - 1][pc->mapY - 1] == SYMBOLS[SYMBOL_PATH] && (pc->mapY - 1 == 0 || pc->mapX - 1 == 0)){
            //If we want to move to the map to the north then do so?
            if(pc->mapY - 1 == 0){
                MoveMap(currentMap->worldLocationX, currentMap->worldLocationY - 1, 'n');
                pc->mapX--;
            }else if(pc->mapX - 1 == 0){
                MoveMap(currentMap->worldLocationX - 1, currentMap->worldLocationY, 'w');
                pc->mapY--;
            }
            isValidMovement = 1;
        }
    } else if(userInputCharacter == '9' || userInputCharacter == 'u'){
        //Upper right
        if (IsValidPlayerMovement(pc->mapX + 1, pc->mapY - 1)) {
            //Set updated position of player
            pc->mapX = pc->mapX + 1;
            pc->mapY = pc->mapY - 1;
            isValidMovement = 1;
        }else if(currentMap->map[pc->mapX + 1][pc->mapY - 1] == SYMBOLS[SYMBOL_PATH] && (pc->mapY - 1 == 0 || pc->mapX + 2 == MAP_X_LENGTH)){
            //If we want to move to the map to the north then do so?
            if(pc->mapY - 1 == 0){
                MoveMap(currentMap->worldLocationX, currentMap->worldLocationY - 1, 'n');
                pc->mapX++;
            }else if(pc->mapX + 1 == MAP_X_LENGTH - 1){
                MoveMap(currentMap->worldLocationX + 1, currentMap->worldLocationY, 'e');
                pc->mapY--;
            }
            isValidMovement = 1;
        }
    } else if(userInputCharacter == '1' || userInputCharacter == 'b'){
        //Bottom left
        if (IsValidPlayerMovement(pc->mapX - 1, pc->mapY + 1)) {
            //Set updated position of player
            pc->mapX = pc->mapX - 1;
            pc->mapY = pc->mapY + 1;
            isValidMovement = 1;
        }else if(currentMap->map[pc->mapX - 1][pc->mapY + 1] == SYMBOLS[SYMBOL_PATH] && (pc->mapY + 2 == MAP_Y_LENGTH || pc->mapX - 1 == 0)){
            //If we want to move to the map to the south then do so?
            if(pc->mapY + 2 == MAP_Y_LENGTH){
                MoveMap(currentMap->worldLocationX, currentMap->worldLocationY + 1, 's');
                pc->mapX--;
            }else if(pc->mapX - 1 == 0){
                MoveMap(currentMap->worldLocationX - 1, currentMap->worldLocationY, 'w');
                pc->mapY++;
            }
            isValidMovement = 1;
        }
    } else if(userInputCharacter == '3' || userInputCharacter == 'n'){
        //Bottom right
        if (IsValidPlayerMovement(pc->mapX + 1, pc->mapY + 1)) {
            //Set updated position of player
            pc->mapX = pc->mapX + 1;
            pc->mapY = pc->mapY + 1;
            isValidMovement = 1;
        }else if(currentMap->map[pc->mapX + 1][pc->mapY + 1] == SYMBOLS[SYMBOL_PATH] && (pc->mapY + 2 == MAP_Y_LENGTH || pc->mapX + 2 == MAP_X_LENGTH)){
            //If we want to move to the map to the south then do so?
            if(pc->mapY + 2 == MAP_Y_LENGTH){
                MoveMap(currentMap->worldLocationX, currentMap->worldLocationY + 1, 's');
                pc->mapX++;
            }else if(pc->mapX + 2 == MAP_X_LENGTH){
                MoveMap(currentMap->worldLocationX + 1, currentMap->worldLocationY, 'e');
                pc->mapY++;
            }
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
        if(numTrainers > 0){
            GenerateCostMap(0);
            GenerateCostMap(1);
        }
    }
    //After the player moves we move NPC'S
    MoveNPCS();
    //After NPC's move lets check for a pokemon battle? Only applicable to grass tiles
    if(currentMap->map[pc->mapX][pc->mapY] == SYMBOLS[SYMBOL_TALL_GRASS]){
        int pokemonBattle = ((rand() % BATTLE_CHANCE) + 1);
        if(pokemonBattle == 1){
            EnterPokemonBattle();
        }
    }
}

void GetUserInput() {
    //Start in center
    int currX = 199;
    int currY = 199;

    GenerateMap(currX, currY);
    PlacePlayerCharacter();

    while (!quit_game) {
        DisplayMap(*currentMap);
        //Printout current location
        if(pc->isValidMovement){
            printw("Current Location in world: (%d, %d)", currentMap->worldLocationX - 199, currentMap->worldLocationY - 199);
            printw("\nEnter New Command: ");
        }else {
            //If previous command was invalid, show it and the command IG
            printw("Current Location: (%d, %d) --- Invalid movement command.", currentMap->worldLocationX - 199, currentMap->worldLocationY - 199);
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
            if(currentMap->map[pc->mapX][pc->mapY] == 'C'){
                EnterPokeCenter();
            }else if(currentMap->map[pc->mapX][pc->mapY] == 'M'){
                EnterPokeMart();
            }
        }else if(userInputCharacter == 'q'){
            quit_game = 1;
        }else if(userInputCharacter == 't'){
            DisplayTrainers();
        }
    }
    endwin();
}

void PrintParsedData(string inputFile){
    if(inputFile == "pokemon"){
        for(Pokemon p: pokemon){
            string outputString = "";
            outputString += to_string(p.ID) + "," + p.name + "," + to_string(p.speciesID) + "," + to_string(p.height) +
                            "," + to_string(p.weight) + "," + to_string(p.baseEXP) + "," + to_string(p.order) +
                            "," + to_string(p.is_default);
            outputString = regex_replace(outputString, regex("-1"), " ");
            cout << outputString << endl;
        }
    }else if(inputFile == "moves"){
        for(Move m: moves){
            string outputString = "";
            outputString += to_string(m.ID) + "," + m.name + "," + to_string(m.generation) + "," + to_string(m.type) +
                            "," + to_string(m.power) + "," + to_string(m.pp) + "," + to_string(m.accuracy) +
                            "," + to_string(m.priority) + "," + to_string(m.target) + "," + to_string(m.damage_class) +
                            "," + to_string(m.effect) + "," + to_string(m.effectChance) + "," + to_string(m.contestType) +
                            "," + to_string(m.contestEffect) + "," + to_string(m.superContestEffect);
            outputString = regex_replace(outputString, regex("-1"), " ");
            cout << outputString << endl;
        }
    }else if(inputFile == "pokemon_moves"){
        for(PokemonMove pm: pokemonMoves){
            string outputString = "";
            outputString += to_string(pm.pokemonID) + "," + to_string(pm.version) + "," + to_string(pm.moveID) +
                            "," + to_string(pm.moveMethod) + "," + to_string(pm.level) + "," + to_string(pm.order);
            outputString = regex_replace(outputString, regex("-1"), " ");
            cout << outputString << endl;
        }
    }else if(inputFile == "pokemon_species"){
        for(PokemonSpecies ps: pokemonSpecies){
            string outputString = "";
            outputString += to_string(ps.ID) + "," + ps.name + "," + to_string(ps.generation) + "," + to_string(ps.evolvesFromID) +
                            "," + to_string(ps.evolutionID) + "," + to_string(ps.color) + "," + to_string(ps.shape) +
                            "," + to_string(ps.habitat) + "," + to_string(ps.gender) + "," + to_string(ps.captureRate) +
                            "," + to_string(ps.baseHappiness) + "," + to_string(ps.isBaby) + "," + to_string(ps.hatchCounter) +
                            "," + to_string(ps.genderDifferences) + "," + to_string(ps.growthRate) + "," + to_string(ps.differentForms) +
                            "," + to_string(ps.isLegendary) + "," + to_string(ps.isMythical) + "," + to_string(ps.order) + "," + to_string(ps.conquestOrder);
            outputString = regex_replace(outputString, regex("-1"), " ");
            cout << outputString << endl;
        }
    }else if(inputFile == "experience"){
        for(Experience e: experience){
            string outputString = "";
            outputString += to_string(e.growthRate) + "," + to_string(e.level) + "," + to_string(e.experience);
            outputString = regex_replace(outputString, regex("-1"), " ");
            cout << outputString << endl;
        }
    }else if(inputFile == "type_names"){
        for(TypeNames tn: typeNames){
            string outputString = "";
            outputString += to_string(tn.ID) + "," + to_string(tn.localLanguage) + "," + tn.type;
            outputString = regex_replace(outputString, regex("-1"), " ");
            cout << outputString << endl;
        }
    }else{
        cout << "IDK how you got here honestly." << endl;
    }
}

void ParseFile(string inputFile) {

    string line = "";
    //Get rid of first line
    getline(currFile, line);
    while(getline(currFile, line)){
        string temp = "";
        stringstream currString(line);
        if(inputFile == "pokemon"){
            Pokemon currPoke = *new Pokemon;
            //Set Pokémon ID
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currPoke.ID = atoi(temp.c_str());
            //Set Pokémon name
            getline(currString, currPoke.name, ',');
            //Set Pokémon speciesID
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currPoke.speciesID = atoi(temp.c_str());
            //Set Pokémon height
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currPoke.height = atoi(temp.c_str());
            //Set Pokémon weight
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currPoke.weight = atoi(temp.c_str());
            //set Pokémon base experience
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currPoke.baseEXP = atoi(temp.c_str());
            //Set Pokémon order
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currPoke.order = atoi(temp.c_str());
            //Set Pokémon is_default
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currPoke.is_default = atoi(temp.c_str());
            //Add current pokemon to pokemon vector and print out the current pokemon
            pokemon.push_back(currPoke);
        }else if(inputFile == "moves"){
            Move currMove = *new Move;
            //Set Move ID
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currMove.ID = atoi(temp.c_str());
            //Set Move name
            getline(currString, currMove.name, ',');
            //Set Move generation
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currMove.generation = atoi(temp.c_str());
            //Set Move type
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currMove.type = atoi(temp.c_str());
            //Set Move power
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currMove.power = atoi(temp.c_str());
            //Set Move pp
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currMove.pp = atoi(temp.c_str());
            //Set Move accuracy
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currMove.accuracy = atoi(temp.c_str());
            //Set Move priority
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currMove.priority = atoi(temp.c_str());
            //Set Move target
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currMove.target = atoi(temp.c_str());
            //Set Move damage class
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currMove.damage_class = atoi(temp.c_str());
            //Set Move effect
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currMove.effect = atoi(temp.c_str());
            //Set Move effect chance
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currMove.effectChance = atoi(temp.c_str());
            //Set Move contest type
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currMove.contestType = atoi(temp.c_str());
            //Set Move contest effect
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currMove.contestEffect = atoi(temp.c_str());
            //Set Move super contest effect
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currMove.superContestEffect = atoi(temp.c_str());
            //Add current move to list of moves
            moves.push_back(currMove);
        }else if(inputFile == "pokemon_moves"){
            PokemonMove currPokemonMove = *new PokemonMove;
            //Set Pokemon ID
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currPokemonMove.pokemonID = atoi(temp.c_str());
            //Set Version
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currPokemonMove.version = atoi(temp.c_str());
            //Set Move ID
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currPokemonMove.moveID = atoi(temp.c_str());
            //Set Move Method
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currPokemonMove.moveMethod = atoi(temp.c_str());
            //Set Level
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currPokemonMove.level = atoi(temp.c_str());
            //Set Order
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currPokemonMove.order = atoi(temp.c_str());
            //Add current move to list of moves
            pokemonMoves.push_back(currPokemonMove);
        }else if(inputFile == "pokemon_species"){
            PokemonSpecies currPokemonSpecies = *new PokemonSpecies;
            //Set Pokemon Species ID
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currPokemonSpecies.ID = atoi(temp.c_str());
            //Set Pokemon Species name
            getline(currString, currPokemonSpecies.name, ',');
            //Set Pokemon Species generation
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currPokemonSpecies.generation = atoi(temp.c_str());
            //Set Pokemon Species evolves from speciesID id
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currPokemonSpecies.evolutionID = atoi(temp.c_str());
            //Set Pokemon Species evolution chain id
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currPokemonSpecies.evolvesFromID = atoi(temp.c_str());
            //Set Pokemon Species color
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currPokemonSpecies.color = atoi(temp.c_str());
            //Set Pokemon Species shape
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currPokemonSpecies.shape = atoi(temp.c_str());
            //Set Pokemon Species habitat
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currPokemonSpecies.habitat = atoi(temp.c_str());
            //Set Pokemon Species gender
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currPokemonSpecies.gender = atoi(temp.c_str());
            //Set Pokemon Species captureRate rate
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currPokemonSpecies.captureRate = atoi(temp.c_str());
            //Set Pokemon Species base baseHappiness
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currPokemonSpecies.baseHappiness = atoi(temp.c_str());
            //Set Pokemon Species is baby
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currPokemonSpecies.isBaby = atoi(temp.c_str());
            //Set Pokemon Species hatch counter
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currPokemonSpecies.hatchCounter = atoi(temp.c_str());
            //Set Pokemon Species gender differences
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currPokemonSpecies.genderDifferences = atoi(temp.c_str());
            //Set Pokemon Species growth rate
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currPokemonSpecies.growthRate = atoi(temp.c_str());
            //Set Pokemon Species form switchable
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currPokemonSpecies.differentForms = atoi(temp.c_str());
            //Set Pokemon Species is legendary
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currPokemonSpecies.isLegendary = atoi(temp.c_str());
            //Set Pokemon Species is mythic
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currPokemonSpecies.isMythical = atoi(temp.c_str());
            //Set Pokemon Species order
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currPokemonSpecies.order = atoi(temp.c_str());
            //Set Pokemon Species conquestOrder order
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currPokemonSpecies.conquestOrder = atoi(temp.c_str());
            //Add current speciesID to list of speciesID
            pokemonSpecies.push_back(currPokemonSpecies);
        }else if(inputFile == "experience"){
            Experience currExperience = *new Experience;
            //Set Growth rate id
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currExperience.growthRate = atoi(temp.c_str());
            //Set level
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currExperience.level = atoi(temp.c_str());
            //Set experience
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currExperience.experience = atoi(temp.c_str());
            //Add current experience to list of experiences
            experience.push_back(currExperience);
        }else if(inputFile == "type_names"){
            TypeNames currTypeName = *new TypeNames;
            //Set Type
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currTypeName.ID = atoi(temp.c_str());
            //Set Local language
            getline(currString, temp, ',');
            if(temp != "9") continue; //if language is not english we don't want it.
            if(temp == "") temp = "-1";
            currTypeName.localLanguage = atoi(temp.c_str());
            //Set move name
            getline(currString, currTypeName.type, ',');
            if(currTypeName.type == "") currTypeName.type = "-1";
            //Add current type to list of types
            typeNames.push_back(currTypeName);
        }else if(inputFile == "pokemon_stats"){
            PokemonStats currStat = *new PokemonStats;
            //Set ID
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currStat.ID = atoi(temp.c_str());
            //Set Stat ID
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currStat.statID = atoi(temp.c_str());
            //Set Base Stat
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currStat.base_Stat = atoi(temp.c_str());
            //Set Effort
            getline(currString, temp, ',');
            if(temp == "") temp = "-1";
            currStat.effort = atoi(temp.c_str());
            pokemonStats.push_back(currStat);
        }
    }
    //PrintParsedData(inputFile);
}

void OpenFile() {
    const char* env = getenv("HOME");
    if(env == nullptr){
        cout << "Home dir not found?" << endl;
        return;
    }
    string homeEnv(env);
    for(const string& inputFile: inputFiles){
        //Default location
        //cout << "Your chosen input file: " << inputFile << endl;
        currFile.open("/share/cs327/" + inputFile + ".csv");
        if(currFile.fail()){
            currFile.open(homeEnv + "/.poke327/" += inputFile + ".csv");
            if(currFile.fail()){
                //I want to look in project directory into a folder named Pokedex
                currFile.open("Pokedex/" + inputFile + ".csv");
                if(currFile.fail()){
                    cout << "\nBro where the files at?\n---No file named '" + inputFile + ".csv' found in any of the three directories---\n";
                }
            }
        }
        ParseFile(inputFile);
        currFile.close();
    }
    cout << "End of opening all files" << endl;
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
        }else{
            //inputFiles = argv[i];
        }
    }
    if(numTrainers > 1200){
        printf("You've entered too many trainers, please keep it below 1200.\n");
        printf("Can't overfill the map!:)\n");
        return 0;
    }
    OpenFile();
    initscr();
    keypad(stdscr, TRUE);
    start_color();
    pc = static_cast<playerCharacter *>(malloc(sizeof(playerCharacter)));
    //Start with generating random
    srand(time(nullptr));
    pc->isValidMovement = 1;



    //Get user input
    GetUserInput();
}