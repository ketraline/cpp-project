#include <iostream>
#include <conio.h>
#include <windows.h>
using namespace std;

void resetCursor() {
    COORD topLeft = {0, 0};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), topLeft);
}

void printScreen(int swallowPos[2], int playAreaX, int playAreaY) {
    for (int y = 0; y < playAreaY; y++) {
        for (int x = 0; x < playAreaX; x++) {
            if (x == 0 && y == 0){
                cout << "\033[31m+";
            }
            else if ((x == playAreaX-1) && (y == playAreaY-1)){
                cout << "\033[31m+";
            }
            else if ((x == playAreaX-1) && y == 0){
                cout << "\033[31m+";
            }
            else if (x == 0 && (y == playAreaY-1)){
                cout << "\033[31m+";
            }
            else if (x == 0 || (x == playAreaX-1)){
                cout << "\033[31m|";
            }
            else if (y == 0 || (y == playAreaY-1)){
                cout << "\033[31m-";
            }
            else if (x == swallowPos[0] && y == swallowPos[1]){
                cout << "\033[34mV";
            }
            else {
                cout << "\033[37m.";
            }
        }
        cout << endl;
    }
}

void calculateStartPos(int swallowPos[2], int playAreaX, int playAreaY){
    swallowPos[0] = playAreaX/2;
    swallowPos[1] = playAreaY/2;
}

void moveSwallow(int swallowPos[2], int playAreaX, int playAreaY, char &direction){
    if (_kbhit()) {
        char key = _getch();
        if (key == 'w' || key == 'a' || key == 's' || key == 'd')
            direction = key;
    }

    switch (direction) {
    case 'w':
        if (swallowPos[1] > 1) swallowPos[1]--;
        break;
    case 's':
        if (swallowPos[1] < playAreaY - 2) swallowPos[1]++;
        break;
    case 'a':
        if (swallowPos[0] > 1) swallowPos[0]--;
        break;
    case 'd':
        if (swallowPos[0] < playAreaX - 2) swallowPos[0]++;
        break;
    }
}

int main(){
    CONSOLE_CURSOR_INFO cursorInfo;
    cursorInfo.dwSize = 1;
    cursorInfo.bVisible = FALSE;
    int playerHP = 100;
    int playAreaX = 90;
    int playAreaY = 20;
    int swallowPos[2];
    char direction;
    calculateStartPos(swallowPos, playAreaX, playAreaY);
    while(true){
        resetCursor();
        printScreen(swallowPos, playAreaX, playAreaY);
        moveSwallow(swallowPos, playAreaX, playAreaY, direction);
        Sleep(50);
    }
}