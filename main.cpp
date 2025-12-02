#include <iostream>
#include <conio.h>
#include <string>
#include <fstream>
#include <windows.h>
#include <vector>
using namespace std;

static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
const int maxStarSpeed = 3;

#define HEIGHT 20
#define WIDTH 80
#define BORDERCOLOR "\033[31m"

struct CONFIG {
    string levelName;
    int borderHeight;
    int borderWidth;
    int borderColor;
    int starSpawnChance;
};

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
    int x, y, speed, tick;
    bool active;
} STAR;

void hideCursor(){
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.dwSize = 1;
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hOut, &cursorInfo);
}

SWALLOW initSwallow(){
    SWALLOW s;
    s.x = WIDTH/2;
    s.y = HEIGHT/2;
    s.symbol = 'V';
    s.color = "\033[36m";
    s.HP = 100;
    s.score = 0;

    return s;
}

void moveCursor(int x, int y) {
    COORD coords = {x, y};
    SetConsoleCursorPosition(hOut, coords);
}

void spawnStar(vector<STAR>& stars) {
    STAR s;
    s.x = rand() % (WIDTH - 2) + 1;
    s.y = 1;
    s.speed = (rand() % maxStarSpeed);
    s.tick = 0;
    s.active = true;

    stars.push_back(s);

    moveCursor(s.x, s.y);
    cout << "\033[33m*";
}

void updateStars(vector<STAR>& stars, SWALLOW* swallow) {
    for (auto& star : stars) {
        if (star.x == swallow->x && star.y == swallow->y && star.active == true) {
            swallow->score += 1;
            star.active = false;
        }

        if (!star.active) continue;
        star.tick++;

        if (star.tick < (maxStarSpeed - star.speed)) { 
            continue;
        }

        if (star.y % 2 == 0) cout << "\033[33m";
        else cout << "\033[29m";
        star.tick = 0;

        moveCursor(star.x, star.y);
        cout << " ";
        star.y++;

        if (star.y >= HEIGHT - 1) {
            star.active = false;
        }
        else {
            moveCursor(star.x, star.y);
            cout << "*";
        }
    }
}

void printScreen() {
    cout << BORDERCOLOR;
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if ((x == 0 && y == 0) || ((x == WIDTH-1) && (y == HEIGHT-1)) || ((x == WIDTH-1) && y == 0) || (x == 0 && (y == HEIGHT-1))){
                cout << "+";
            }
            else if (x == 0 || (x == WIDTH-1)){
                cout << "|";
            }
            else if (y == 0 || (y == HEIGHT-1)){
                cout << "-";
            }
            else {
                cout << " ";
            }
        }
        cout << endl;
    }
}

void printStatus(SWALLOW *swallow) {
    moveCursor(0, HEIGHT);
    cout << BORDERCOLOR;
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if ((x == 0 && y == 0) || ((x == WIDTH-1) && (y == HEIGHT-1)) || ((x == WIDTH-1) && y == 0) || (x == 0 && (y == HEIGHT-1))){
                cout << "+";
            }
            else if (x == 0){
                cout << "|";
            }
            else if (y == 0 || (y == HEIGHT-1)){
                cout << "-";
            }
            else {
                cout << " ";
            }
        }
        cout << endl;
    }
    cout << "HP: " << swallow->HP << "   SCORE: " << swallow->score << "   POSITION: [" << swallow->x << "," << swallow->y << "] |";
}

void moveSwallow(SWALLOW *swallow){
    moveCursor(swallow->x, swallow->y);
    cout << " ";

    if (_kbhit()) {
        char key = _getch();
        if (key == 'w' || key == 'a' || key == 's' || key == 'd')
            swallow->direction = key;
    }

    switch (swallow->direction) {
    case 'w':
        if (swallow->y > 1) swallow->y -= 1;
        break;
    case 's':
        if (swallow->y < HEIGHT - 2) swallow->y += 1;
        break;
    case 'a':
        if (swallow->x > 1) swallow->x -= 1;
        break;
    case 'd':
        if (swallow->x < WIDTH - 2) swallow->x += 1;
        break;
    }
    
    moveCursor(swallow->x, swallow->y);
    cout << "\033[36mV";
}

int main(){
    hideCursor();
    SWALLOW swallow = initSwallow();
    printScreen();
    vector<STAR> stars;
    int spawnTimer = 0;
    while(true){
        moveSwallow(&swallow);

        if (spawnTimer >= 7) {
            spawnStar(stars);
            spawnTimer = 0;
        }
        printStatus(&swallow);

        updateStars(stars, &swallow);
        Sleep(50);
        spawnTimer++;
    }
}