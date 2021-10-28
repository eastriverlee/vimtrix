#ifndef VIMTRIX_H
# define VIMTRIX_H

#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_ttf.h>

#define CENTER SDL_WINDOWPOS_CENTERED
#define RELATIVE true
#define ABSOLUTE false
#define AFTER true
#define BEFORE false
#define STAY -1
#define START -2
#define LEFT 3
#define TOP 0

#define SOUND_COUNT 7
#define BLOCK 0 
#define DELETE 1 
#define EXPLODE 2 
#define HURT 3
#define JUMP 4
#define MOVE 5
#define PICKUP 6

#define MOVED (original.x != cursor.x || original.y != cursor.y)
#define JUMPED (abs(original.x - cursor.x) + abs(original.y - cursor.y) > 1)

typedef struct position {
	short x;
	short y;
} position;

SDL_AudioSpec audio[SOUND_COUNT];
SDL_AudioDeviceID device[SOUND_COUNT];
unsigned int length[SOUND_COUNT];
unsigned char *buffer[SOUND_COUNT];

extern const int padding;
extern const int width;
extern const int height;

extern SDL_Event event;
extern SDL_Window *window;
extern SDL_Surface *screen;
extern SDL_Renderer *renderer;

extern TTF_Font *font;
extern SDL_Color white;
extern SDL_Color blue;
extern SDL_Color black;
extern SDL_Surface *letter[128];
extern SDL_Surface *letter_blue[128];
extern SDL_Surface *inverted[128];

extern char cell[256][256];
extern position cursor;
extern position end;

int min(int a, int b);
int max(int a, int b);
int clamp(int minimum, int n, int maximum);
void play(char sound);
void word(short count);
void WORD(short count);
void back(short count);
void BACK(short count);
void move(short x, short y, bool relative);
void find(int letter, bool after);
void render();
void first_non_space();
void match_pair();

#endif
