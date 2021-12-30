#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <Windows.h>

enum KEY{ONE = '1', TWO, THREE, FOUR, FIVE, ESC = 27, YES = 'y', NO = 'n'};

typedef struct _gameState {
	int score;
	unsigned int planePos;
	unsigned int bombX;
	unsigned int bombY;
	unsigned char letterCount;
	unsigned char wordGuessed;
	unsigned char misTyped, corTyped;
	char currentWord[10];
} GameState;

struct screen {
	int width;
	int height;
} Screen;


void nextWord(GameState* gs);
int startNewGame(GameState* gs);
int setWindowProperties(HANDLE* output, CONSOLE_SCREEN_BUFFER_INFO* csbi, CONSOLE_CURSOR_INFO* ci);
int readGraphicsFromFiles();
int saveGame(GameState* gs, const char* const fileName);
int loadGame(GameState* gs, const char* const fileName);
int clocksPassed(clock_t start, clock_t clocks);
void gameOver();
void printRepeatedChar(char ch, int repetition);
void getFileName();
void drawScene(GameState* gs);
void drawMenu();

char gameRunning = 0, gamePaused = 0;
const char city[378], plane[30], bomb[10], words[313];
char* fileName[255];
unsigned int width = 0;

int main(void) {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	CONSOLE_CURSOR_INFO ci;
	HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);

	GetConsoleScreenBufferInfo(output, &csbi);
	if (!setWindowProperties(&output, &csbi, &ci)) {
		return 1;
	}
	Screen.width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	Screen.height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

	if (!readGraphicsFromFiles()) {
		return 1;
	}

	for (; city[width] != '\n'; width++);

	drawMenu(); // Initial start

	GameState gs;

	clock_t bombTimer = clock(), menuTimer = clock(), planeTimer = clock();
	while (1) { // Main game loop

		// if a key is pressed
		if (_kbhit()) {
			const enum KEY key = _getch();

			if (gameRunning && !gamePaused && key != ESC) {
				if (strlen(gs.currentWord) - 1) {
					if (gs.currentWord[0] == key) {
						gs.corTyped++;
						strcpy(gs.currentWord, gs.currentWord + 1);
						drawScene(&gs);
					}
					else {
						gs.misTyped++;
					}
				}
				else {
					gs.score += gs.corTyped - gs.misTyped + 1;
					gs.corTyped = gs.misTyped = gs.bombY = 0;
					gs.bombX = gs.planePos;
					if (gs.letterCount != 10 && gs.wordGuessed++ == 4) {
						gs.letterCount++;
						gs.wordGuessed = 0;
					}
					nextWord(&gs);
					drawScene(&gs);
				}
			}
			else {
				// Conditional for the pressed key
				switch (key)
				{
				case ESC:
					gamePaused = 1;
					drawMenu(&gs);
					// Update Screen
					break;

				case ONE:
					startNewGame(&gs);
					break;

				case TWO:
					system("cls");
					getFileName();
					if (!loadGame(&gs, fileName)) {
						printRepeatedChar('\t', (Screen.width) / 20);
						printf("No Such File as %s", fileName);
						gameRunning = 0;
						Sleep(1500);
						system("cls");
						drawMenu();
					}
					else {
						gameRunning = 1;
						gamePaused = 0;
					}
					break;

				case THREE:
					system("cls");
					getFileName();
					saveGame(&gs, fileName);
					gameRunning = 0;
					system("cls");
					drawMenu();
					break;

				case FOUR:
					if (!gameRunning) {
						return 0;
					}
					gamePaused = 0;
					break;

				case FIVE:
					if (gameRunning) {
						return 0;
					}
					break;

				default:
					break;
				}
			}
		}
		if (gameRunning && !gamePaused) {
			if (clocksPassed(planeTimer, 500)) {
				planeTimer = clock();
				gs.planePos = (gs.planePos + 1) % (width - 10);
				drawScene(&gs);
			}

			if (clocksPassed(bombTimer, (11 - gs.letterCount) * 100)) {
				bombTimer = clock();
				drawScene(&gs);
				if (gs.bombY++ > 11) {
					gameOver(&gs);
				}
			}
		}
		
		GetConsoleScreenBufferInfo(output, &csbi);
		if (Screen.width != csbi.srWindow.Right - csbi.srWindow.Left + 1 || Screen.height != csbi.srWindow.Bottom - csbi.srWindow.Top + 1) {
			Screen.width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
			Screen.height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
			setWindowProperties(&output, &csbi, &ci);
			if (gamePaused || !gameRunning) {
				drawMenu(&gs);
			}
		}
		//fflush(stdout);

		Sleep(5); // Saves CPU Time
	}

	return 0;
}

int setWindowProperties(HANDLE* output, CONSOLE_SCREEN_BUFFER_INFO* csbi, CONSOLE_CURSOR_INFO* ci) {
	ci->dwSize = 100;
	ci->bVisible = false;
	if (!SetConsoleCursorInfo(*output, ci)) {
		printf("Couldn't uptade console cursor info");
		return 0;
	}
	COORD newSize = { csbi->dwSize.X, csbi->srWindow.Bottom - csbi->srWindow.Top + 1};
	if (!SetConsoleScreenBufferSize(*output, newSize)) {
		printf("Couldn't uptade console buffer size");
		return 0;
	}

	return 1;
}

void nextWord(GameState* gs) {
	int passedWordCount = (gs->letterCount - 3) * 5 + rand() % 5;;
	char* cptr = words;
	for (; passedWordCount > 0; cptr++) {
		if (*cptr == '\0') {
			passedWordCount--;
		}
	}
	strcpy(gs->currentWord, cptr);
}

int startNewGame(GameState* gs) {
	srand(time(NULL) + clock());
	memset(gs, 0, sizeof(GameState));
	gs->letterCount = 3;
	nextWord(gs);
	gameRunning = 1;
	gamePaused = 0;
}

void gameOver(GameState* gs) {
	system("cls");
	printRepeatedChar('\n', Screen.height / 2);
	printRepeatedChar(' ', (Screen.width - 60) / 2);
	printf("%s", "GAME OVER\n");
	printRepeatedChar(' ', (Screen.width - 60) / 2);
	printf("%s", "Press 'y' to play again, 'n' to return menu");
	while (1) {
		if (_kbhit()) {
			const enum KEY key = _getch();
			switch (key) {
				case YES:
					startNewGame(gs);
					return;
				case NO:
					gameRunning = 0;
					drawMenu(gs);
					return;
			}
		}
		Sleep(20);
	}
}

void drawScene(GameState* gs) {
	char copyPlane[30], copyBomb[10];
	strcpy(copyPlane, plane);
	strcpy(copyBomb, bomb);

	system("cls");
	printRepeatedChar('\n', (Screen.height - 25) / 2);
	printRepeatedChar(' ', gs->planePos);
	printf("%s", strtok(copyPlane, "\n"));
	printRepeatedChar(' ', width - 10 - gs->planePos);
	printf("Score: %d\n", gs->score);
	printRepeatedChar(' ', gs->planePos);
	printf("%s\n", strtok(NULL, "\n"));
	printRepeatedChar(' ', gs->planePos);
	printf("%s\n", strtok(NULL, "\n"));

	printRepeatedChar('\n', gs->bombY);
	printRepeatedChar(' ', gs->bombX + 1);
	printf("%s\n", strtok(copyBomb, "\n"));
	printRepeatedChar(' ', gs->bombX + 1);
	printf("%s %s\n", strtok(NULL, "\n"), gs->currentWord);
	printRepeatedChar(' ', gs->bombX + 1);
	printf("%s\n", strtok(NULL, "\n"));

	printRepeatedChar('\n', 12 - gs->bombY);
	printf("%s", city);
}

void getFileName() {
	printRepeatedChar('\n', (Screen.height - 5) / 2);
	printRepeatedChar(' ', (Screen.width - 30) / 2);
	printf("Please enter fileName.data fileName:\n");
	printRepeatedChar(' ', (Screen.width - 30) / 2);
	scanf("%s", fileName);
	strcat(fileName, ".data");
}

int readGraphicsFromFiles() {
	int end_of_file;
	FILE* fptr;

	// Read CITY graphic
	if ((fptr = fopen("city.txt", "r")) == NULL) {
		printf("City file not found");
		return 0;
	}
	fseek(fptr, 0, SEEK_END);
	end_of_file = ftell(fptr);
	rewind(fptr);
	fread(city, end_of_file, 1, fptr);
	fclose(fptr);

	// Read BOMB graphic
	if ((fptr = fopen("bomb.txt", "r")) == NULL) {
		printf("Bomb file not found");
		return 0;
	}
	fseek(fptr, 0, SEEK_END);
	end_of_file = ftell(fptr);
	rewind(fptr);
	fread(bomb, end_of_file, 1, fptr);
	fclose(fptr);

	// Read PLANE graphic
	if ((fptr = fopen("plane.txt", "r")) == NULL) {
		printf("Plane file not found");
		return 0;
	}
	fseek(fptr, 0, SEEK_END);
	end_of_file = ftell(fptr);
	rewind(fptr);
	fread(plane, end_of_file, 1, fptr);
	fclose(fptr);

	// Read CODEWORDS 
	if ((fptr = fopen("codewords.txt", "r")) == NULL) {
		printf("Codewords file not found");
		return 0;
	}
	fseek(fptr, 0, SEEK_END);
	end_of_file = ftell(fptr);
	rewind(fptr);
	fread(words, end_of_file, 1, fptr);
	fclose(fptr);

	// Replace \n with \0 in code words
	for (char* cptr = words; cptr < words + end_of_file; cptr++) {
		if (*cptr == '\n')
			*cptr++ = '\0';
	}

	return 1;
}

int saveGame(GameState* gs, const char* const fileName) {
	FILE* fptr;
	if ((fptr = fopen(fileName, "wb")) == NULL) {
		return 0;
	}
	fwrite(gs, sizeof(GameState), 1, fptr);
	fclose(fptr);
	return 1;
}

int loadGame(GameState* gs, const char* const fileName) {
	FILE* fptr;
	if ((fptr = fopen(fileName, "rb")) == NULL) {
		return 0;
	}
	fread(gs, sizeof(GameState), 1, fptr);
	fclose(fptr);
	return 1;
}

void printRepeatedChar(char ch, int repetition) {
	for (unsigned char i = 0; i < repetition; i++)
		putchar(ch);
}

int clocksPassed(clock_t start, clock_t clocks) {
	if (clock() - start > clocks)
		return 1;
	return 0;
}

void drawMenu() {
	system("cls");
	printRepeatedChar('\n', (Screen.height - 5) / 2);

	printRepeatedChar('\t', (Screen.width) / 20);
	printf("1. New Game\n");
	printRepeatedChar('\t', (Screen.width) / 20);
	printf("2. Load a Saved Game\n");
	printRepeatedChar('\t', (Screen.width) / 20);
	printf("3. Save Current Game\n");
	if (gameRunning) {
		printRepeatedChar('\t', (Screen.width) / 20);
		printf("4. Return to Game\n");
	}
	printRepeatedChar('\t', (Screen.width) / 20);
	printf("%d. Exit\n", gameRunning ? 5 : 4);
}