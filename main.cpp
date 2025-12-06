#include <iostream>
#include <cstdio>
#include <string>
#include <conio.h>
#include <windows.h>
#include <vector>
#include <algorithm>
#include <chrono>
using namespace std;

// STRUCTS

typedef struct {
    string levelName;
    int difficultyMult;
    int timeLimit;
    int borderHeight;
    int borderWidth;
    int difficultyIncreaseCount;
    int starQuota;
    int starSpawnChance;
    int hunterMax;
    vector<pair<int,int>> hunterSizes;
    vector<int> hunterSizesChance;
    int hunterSpawnChance;
    int hunterDamage;
    int hunterMaxBounces;
} CONFIG;

typedef struct {
	int x, y;
	int speed;
    char direction;
	char symbol;
	string color;
    int HP;
    int score;
} SWALLOW;

typedef struct {
	int x, y;
	int sizeX, sizeY;
	int directionX, directionY;
    int bounces, size;
    int pauseTimer, dashTimer;
    bool paused, dashing, active;
} HUNTER;

typedef struct {
    int x, y;
    int speed, tick;
    bool active;
} STAR;

// GLOBAL VARIABLES

static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
const int maxStringLength = 64;
const int maxStarSpeed = 3;
const int minSwallowSpeed = 1;
const int maxSwallowSpeed = 5;
CONFIG config;

#define BORDERCOLOR "\033[31m"

// CONFIG LOADING

CONFIG setConfigDefaults(CONFIG c) {
    c.levelName = "0";
    c.difficultyMult = 1;
    c.timeLimit = 90;
    c.borderHeight = 20;
    c.borderWidth = 80;
    c.difficultyIncreaseCount = 10;
    c.starQuota = 10;
    c.starSpawnChance = 50;
    c.hunterMax = 2;
    c.hunterSizes = {{1,2}, {2,1}, {1,3}, {3,1}, {2,2}};
    c.hunterSizesChance = {25,25,20,20,10};
    c.hunterSpawnChance = 3;
    c.hunterDamage = 10;
    c.hunterMaxBounces = 2;
    return c;
}

CONFIG readConfig(const char* filename) {
    CONFIG c;
    setConfigDefaults(c);

    FILE* file = fopen(filename, "r");
    if (!file) {
        cout << "Couldn't find file, using defaults...";
        Sleep(1000);
    }
    else{
        char arg[maxStringLength], value[maxStringLength];
        while (fscanf(file, "%s %s", arg, value) == 2) {
            if (strcmp(arg, "levelName") == 0) c.levelName = value;
            else if (strcmp(arg, "difficultyMult") == 0) c.difficultyMult = atoi(value);
            else if (strcmp(arg, "timeLimit") == 0) c.timeLimit = atoi(value);
            else if (strcmp(arg, "borderHeight") == 0) c.borderHeight = atoi(value);
            else if (strcmp(arg, "borderWidth") == 0) c.borderWidth = atoi(value);
            else if (strcmp(arg, "difficultyIncreaseCount") == 0) c.difficultyIncreaseCount = atoi(value);
            else if (strcmp(arg, "starQuota") == 0) c.starQuota = atoi(value);
            else if (strcmp(arg, "starSpawnChance") == 0) c.starSpawnChance = atoi(value);
            else if (strcmp(arg, "hunterMax") == 0) c.hunterMax = atoi(value);
            else if (strcmp(arg, "hunterSizes") == 0) {
                char* sizeX = strtok(value, ",");
                char* sizeY = strtok(NULL, ",");
                while (sizeX != NULL) {
                    c.hunterSizes.push_back(make_pair(atoi(sizeX),atoi(sizeY)));
                    sizeX = strtok(NULL, ",");
                    sizeY = strtok(NULL, ",");
                }
            }
            else if (strcmp(arg, "hunterSizesChance") == 0) {
                char* element = strtok(value, ",");
                while (element != NULL) {
                    c.hunterSizesChance.push_back(atoi(element));
                    element = strtok(NULL, ",");
                }
            }
            else if (strcmp(arg, "hunterSpawnChance") == 0) c.hunterSpawnChance = atoi(value);
            else if (strcmp(arg, "hunterDamage") == 0) c.hunterDamage = atoi(value);
            else if (strcmp(arg, "hunterMaxBounces") == 0) c.hunterMaxBounces = atoi(value);
            else cout << "Couldn't find the variable " << arg << ", using defaults...";
        }
    }
    fclose(file);
    return c;
}

// GAME HANDLING

void loadLevel(){
    string arg;
    cout << "Choose the level you'd like to play: " << endl << "1 - EASY | 2 - MEDIUM | 3 - HARD" << endl << "...or load your own level by typing LOAD [levelname.txt]" << endl;
    cin >> arg;

    if (arg == "1") config = readConfig("1.txt");
    else if (arg == "2") config = readConfig("2.txt");
    else if (arg == "3") config = readConfig("3.txt");
    else if (arg == "LOAD") {
        char name[maxStringLength];
        cin >> name;
        config = readConfig(name);
    }
}

void hideCursor(){
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.dwSize = 1;
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hOut, &cursorInfo);
}

void moveCursor(int x, int y) {
    COORD coords = {x, y};
    SetConsoleCursorPosition(hOut, coords);
}

void printScreen() {
    cout << BORDERCOLOR;
    for (int y = 0; y < config.borderHeight; y++) {
        for (int x = 0; x < config.borderWidth; x++) {
            if ((x == 0 && y == 0) || ((x == config.borderWidth-1) && (y == config.borderHeight-1)) || ((x == config.borderWidth-1) && y == 0) || (x == 0 && (y == config.borderHeight-1))){
                cout << "+";
            }
            else if (x == 0 || (x == config.borderWidth-1)){
                cout << "|";
            }
            else if (y == 0 || (y == config.borderHeight-1)){
                cout << "-";
            }
            else {
                cout << " ";
            }
        }
        cout << endl;
    }
}

void printStatus(int timeLeft, SWALLOW *swallow) {
    moveCursor(0, config.borderHeight);
    cout << swallow->color;
    cout << "{ LEVEL " << config.levelName << " }     HP: " << swallow->HP << "   SCORE: "  << swallow->score << "/" << config.starQuota << "   TIME: " << timeLeft << "   SPEED: "  << swallow->speed << "   POSITION: (" << swallow->x << "," << swallow->y << ")";
}

void printGameOver(){
    cout << "   _____                         ____                 "<< endl 
        << "  / ____|                       / __ \                " << endl
        << " | |  __  __ _ _ __ ___   ___  | |  | |_   _____ _ __ " << endl
        << " | | |_ |/ _` | '_ ` _ \ / _ \ | |  | \ \ / / _ \ '__|" << endl
        << " | |__| | (_| | | | | | |  __/ | |__| |\ V /  __/ |   " << endl
        << "  \_____|\__,_|_| |_| |_|\___|  \____/  \_/ \___|_|   " << endl;
}

bool checkWinLoseConditions(SWALLOW* swallow , int timeLeft) {
    if (timeLeft <= 0) {
        system("CLS");
        printGameOver();
        cout << "You ran out of time!" << endl;
        return 1;
    }
    if (swallow->HP == 0) {
        system("CLS");
        printGameOver();
        cout << "YOU RAN OUT OF HP. GAME OVER!" << endl;
        return 1;
    }
    if (swallow->score == config.starQuota){
        system("CLS");
        cout << " __     __          __          ___       _ " << endl
            << " \ \   / /          \ \        / (_)     | |" << endl
            << "  \ \_/ /__  _   _   \ \  /\  / / _ _ __ | |" << endl
            << "   \   / _ \| | | |   \ \/  \/ / | | '_ \| |" << endl
            << "    | | (_) | |_| |    \  /\  /  | | | | |_|" << endl
            << "    |_|\___/ \__,_|     \/  \/   |_|_| |_(_)" << endl;
        return 1;
    }
    else return 0;
}

int computeScore(SWALLOW* swallow, int timeLeft) {
    int timeScore = timeLeft*100;
    int starScore = (config.starQuota - swallow->score)*100;
    int HPScore = swallow->HP*10;
    return (timeScore - starScore + HPScore)*config.difficultyMult;
}

void saveData(SWALLOW* swallow, int timeLeft) {
    int score = computeScore(swallow, timeLeft);
    cout << "Would you like to save your score of " << score << " to the leaderboard? [Y/N]" << endl;
    char yesno;
    cin >> yesno;
    if (yesno == 'Y' || yesno == 'y') {
        cout << "Please input your name: ";
        char name[maxStringLength];
        cin >> name;
        FILE* file = fopen("leaderboard.txt", "a");
        fprintf(file, "%s %d\n", name, score);
        fclose(file);
    }
}

void gameLoop() {
    loadLevel();
    system("CLS");
    SWALLOW swallow = initSwallow();
    vector<STAR> stars;
    vector<HUNTER> hunters;
    srand (time(NULL));
    int timeInterval = config.timeLimit / (config.difficultyIncreaseCount + 1);
    printScreen();
    auto startTime = chrono::steady_clock::now();
    while(true){
        auto now = chrono::steady_clock::now();
        int elapsed = chrono::duration_cast<chrono::seconds>(now - startTime).count();
        int timeLeft = config.timeLimit - elapsed;
        if (config.difficultyIncreaseCount != 0) (timeLeft, &timeInterval);
        printStatus(timeLeft, &swallow);
        moveSwallow(&swallow);
        spawnHunters(hunters, &swallow);
        spawnStar(stars);
        updateHunters(hunters, &swallow);
        updateStars(stars, &swallow);
        eraseInactiveHunters(hunters);
        if (checkWinLoseConditions(&swallow, timeLeft)) {          
            saveData(&swallow, timeLeft);
            break;
        }
        Sleep(50);
    }
}

void loadLeaderboard() {
    system("CLS");
    cout << "Displaying top 5 scores..." << endl;
    FILE* file = fopen("leaderboard.txt", "r");
    if (!file) cout << "No scores found.";
    else {
        char name[maxStringLength], score[maxStringLength];
        vector <pair<string, int>> scoreSet;
        while (fscanf(file, "%s %s", name, score) == 2) {
            scoreSet.push_back(make_pair(string(name), atoi(score)));
        }
        sort(scoreSet.begin(), scoreSet.end(), [](auto &left, auto &right) {
            return left.second > right.second;
        });
        fclose(file);
        for (int i = 0; i < min(5, scoreSet.size()); i++) {
            cout << i+1 << " " << scoreSet[i].first << " " << scoreSet[i].second << endl;
        }
    }
    cout << endl << "Press any key to leave...";
    getch();
}

// SWALLOW

SWALLOW initSwallow(){
    SWALLOW s;
    s.x = config.borderWidth/2;
    s.y = config.borderHeight/2;
    s.speed = 1;
    s.symbol = 'V';
    s.color = "\033[36m";
    s.HP = 100;
    s.score = 0;
    return s;
}

void changeSwallowSpeed(SWALLOW *swallow, char key) {
}

void moveSwallow(SWALLOW *swallow){
    moveCursor(swallow->x, swallow->y);
    cout << " ";

    if (_kbhit()) {
        char key = _getch();
        if (key == 'w' || key == 'a' || key == 's' || key == 'd') swallow->direction = key;
        else if (key == 'o' && swallow->speed > minSwallowSpeed) {
            swallow->speed--;
        }
        else if (key == 'p' && swallow->speed < maxSwallowSpeed) {
            swallow->speed++;
        }
    }

    switch (swallow->direction) {
        case 'w':
            swallow->y -= swallow->speed;
            break;

        case 's':
            swallow->y += swallow->speed;
            break;

        case 'a':
            swallow->x -= swallow->speed;
            break;

        case 'd':
            swallow->x += swallow->speed;
            break;
    }

    if (swallow->x < 1) swallow->x = 1;
    else if (swallow->x > config.borderWidth - 2)
    swallow->x = config.borderWidth - 2;
    if (swallow->y < 1) swallow->y = 1;
    else if (swallow->y > config.borderHeight - 2)
    swallow->y = config.borderHeight - 2;
    
    moveCursor(swallow->x, swallow->y);
    cout << "\033[36mV";
}

// HUNTERS

void chooseHunterSize(HUNTER* h) {
    int sizeRand = rand() % 101;
    int percentage = 100;
    for (int i = (config.hunterSizesChance.size()-1); i >= 0; i--){
        if ((percentage - config.hunterSizesChance[i]) < sizeRand) {
            auto size = config.hunterSizes[i];
            h->sizeX = size.first;
            h->sizeY = size.second;
            break;
        }
        else percentage -= config.hunterSizesChance[i];
    }
}

void spawnHunters(vector<HUNTER>& hunters, const SWALLOW* swallow) {
    int randNumber = rand() % 101;
    if (randNumber < config.hunterSpawnChance && hunters.size() < config.hunterMax){
        HUNTER hunter;    
        hunter.active = false;

        chooseHunterSize(&hunter);

        int borderSide = rand() % 4;
        switch (borderSide) {
            case 0:
                hunter.x = rand() % (config.borderWidth-hunter.sizeX-2);
                hunter.y = 1;
                break;
            case 1:
                hunter.x = rand() % (config.borderWidth-hunter.sizeX-2);
                hunter.y = config.borderHeight - hunter.sizeY - 1;
                break;
            case 2:
                hunter.x = 1;
                hunter.y = rand() % (config.borderHeight-hunter.sizeY-2);
            break;
            case 3:
                hunter.x = config.borderWidth - hunter.sizeX - 2;
                hunter.y = rand() % (config.borderHeight-hunter.sizeY-2);
                break;
        }

        float vectorX = swallow->x - hunter.x;
        float vectorY = swallow->y - hunter.y;
        float length = sqrt(vectorX*vectorX + vectorY*vectorY);
        vectorX /= length;
        vectorY /= length;

        if (vectorX>0) hunter.directionX = 1;
        else hunter.directionX = -1;
        if (vectorY>0) hunter.directionY = 1;
        else hunter.directionY = -1;

        hunter.paused = false;
        hunter.pauseTimer = 0;
        hunter.dashing = false;
        hunter.dashTimer = 0;
        hunter.bounces = config.hunterMaxBounces;

        hunter.active = true;
        hunters.push_back(h);
    }
}

bool willMissSwallow(const HUNTER& hunter, const SWALLOW* swallow) {
    int nextX = hunter.x;
    int nextY = hunter.y;
    int distance = sqrt(nextX-swallow->x)*(nextX-swallow->x) + (nextY-swallow->y)+(nextY-swallow->y);

    for (int i = 0; i < distance; i++) {
        nextX += hunter.directionX;
        nextY += hunter.directionY;
        if (nextX >= swallow->x && nextX < swallow->x + 1 && nextY >= swallow->y && nextY < swallow->y + 1)
            return false;
    }
    return true; 
}

void updateHunters(vector<HUNTER>& hunters, SWALLOW* swallow) {
    for (auto& h : hunters) {
        if (!h.active) continue;

        for (int cursorY = 0; cursorY < h.sizeY; cursorY++) {
            for (int cursorX = 0; cursorX < h.sizeX; cursorX++) {
                moveCursor(h.x + cursorX, h.y + cursorY);
                cout << " ";
            }
        }
        if (swallow->x >= h.x && swallow->x <  h.x + h.sizeX && swallow->y >= h.y && swallow->y <  h.y + h.sizeY && h.active == true) {
            swallow->HP -= config.hunterDamage;
            h.active = false;
            continue;
        }

        bool hitX = false, hitY = false;

        int nextX = h.x + h.directionX;
        int nextY = h.y + h.directionY;

        if (nextX < 1 || nextX + h.sizeX >= config.borderWidth - 1) hitX = true;

        if (nextY < 1 || nextY + h.sizeY >= config.borderHeight - 1) hitY = true;

        if (hitX || hitY) {
            h.bounces--;
            if (h.bounces <= 0) {
                h.active = false;
                continue;
            }
        }

        if (hitX) h.directionX = -h.directionX;
        if (hitY) h.directionY = -h.directionY;

        h.x += h.directionX;
        h.y += h.directionY;
        
        for (int yy = 0; yy < h.sizeY; yy++) {
            for (int xx = 0; xx < h.sizeX; xx++) {
                moveCursor(h.x + xx, h.y + yy);
                cout << "\033[31m#";
            }
        }
    }
}

void eraseInactiveHunters(vector<HUNTER>& hunters){
    hunters.erase(
        remove_if(hunters.begin(), hunters.end(), [](const HUNTER& h) {return !h.active;}),
        hunters.end()
    );
}

void checkDiffIncrease(int timeLeft, int* timeInterval) {
    if (timeLeft == config.timeLimit-*timeInterval) {
        config.hunterMax++;
        config.hunterMaxBounces++;
        timeInterval += config.timeLimit / (config.difficultyIncreaseCount + 1); 
    }
}

// STARS

void spawnStar(vector<STAR>& stars) {
    int randNumber = rand() % 101;
    if (randNumber < config.starSpawnChance){
        STAR s;
        s.x = rand() % (config.borderWidth - 2) + 1;
        s.y = 1;
        s.speed = (rand() % maxStarSpeed);
        s.tick = 0;
        s.active = true;

        stars.push_back(s);

        moveCursor(s.x, s.y);
        cout << "\033[33m*";
    }
}

void updateStars(vector<STAR>& stars, SWALLOW* swallow) {
    for (auto& s : stars) {
        if (s.x == swallow->x && s.y == swallow->y && s.active == true) {
            swallow->score += 1;
            s.active = false;
        }

        if (!s.active) continue;
        s.tick++;

        if (s.tick < (maxStarSpeed - s.speed)) { 
            continue;
        }

        if (s.y % 2 == 0) cout << "\033[33m";
        else cout << "\033[29m";
        s.tick = 0;

        moveCursor(s.x, s.y);
        cout << " ";
        s.y++;

        if (s.y >= config.borderHeight - 1) {
            s.active = false;
        }
        else {
            moveCursor(s.x, s.y);
            cout << "*";
        }
    }
}

int main(){
    hideCursor();
    while (true) {
        system("CLS");
        cout << "  _________               .__  .__                    _________ __                       " << endl
        << " /   _____/_  _  _______  |  | |  |   ______  _  __  /   _____//  |______ _______  ______" << endl
        << " \\_____  \\\\ \\/ \\/ /\\__  \\ |  | |  |  /  _ \\ \\/ \\/ /  \\_____  \\   __\\__  \\_  __ \\/  ___/" << endl
        << " /        \\\\     /  / __ \\|  |_|  |_(  <_> )     /   /        \\|  |  / __ \\|  | \\/\\___ \\ " << endl
        << "/_______  / \\/\\_/  (____  /____/____/\\____/ \\/\\_/   /_______  /|__| (____  /__|  /____  >" << endl
        << "        \\/              \\/                                  \\/           \\/           \\/ " << endl << endl
        << "[P] Play   [L] Leaderboard   [E] Exit" << endl;
        char selectOption;
        cin >> selectOption;
        switch (selectOption) {
            case 'P': 
                gameLoop();
                continue;
            case 'L': 
                loadLeaderboard();
                continue;
            case 'E': 
                break;
        }
        break;
    }
}