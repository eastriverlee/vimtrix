#include "vimtrix.h"

const int padding = 10;
const int width = 800;
const int height = 800;

SDL_Event event;
SDL_Window *window;
SDL_Surface *screen;
SDL_Renderer *renderer;

TTF_Font *font;
SDL_Color white = {255,255,255};
SDL_Color body = {200,220,255};
SDL_Color red = {255,0,80};
SDL_Color green = {100,255,140};
SDL_Color blue = {0,220,255};
SDL_Color black = {30,30,40};
#define PLAIN 0
#define BODY 1
#define RED 2
#define GREEN 3
#define BLUE 4
SDL_Surface *glyph[5][128];
SDL_Surface *inverted[128];

char cell[256][256];
position cursor = {LEFT, TOP};
position end;

void fill()
{
	const int x_lim = width - padding;
	const int y_lim = height - padding;
	const int x_unit = glyph[BODY][' ']->w;
	const int y_unit = glyph[BODY][' ']->h;
	SDL_Rect origin = {padding, padding};
	short x, y, line;
	char index;
	bool selected;

	for (y = 0; origin.y < y_lim; origin.y += y_unit, y++)
	{
		line = y + 1;
		for (x = 0, origin.x = padding; origin.x < x_lim; origin.x += x_unit, x++)
		{
			switch (x)
			{
				case 0:
					cell[y][x] = line < 10 ? ' ' : line/10 + '0'; break;
				case 1:
					cell[y][x] = line%10 + '0'; break;
				case 2:
					cell[y][x] = ' '; break;
				default:
					cell[y][x] = max(32, rand() % 127);
			}
		}
		cell[y][x] = 0;
	}
	end.x = --x;
	end.y = --y;
}

void render()
{
	const int x_unit = glyph[BODY][' ']->w;
	const int y_unit = glyph[BODY][' ']->h;
	SDL_Rect origin = {padding, padding};
	short x, y;
	char index;
	bool selected;

	for (y = 0; y <= end.y; origin.y += y_unit, y++)
		for (x = 0, origin.x = padding; x <= end.x; origin.x += x_unit, x++)
		{
			selected = cursor.x == x && cursor.y == y;
			index = cell[y][x];
			if (x < 3)
				SDL_BlitSurface(selected ? inverted[index] : glyph[PLAIN][index], NULL, screen, &origin);
			else
				SDL_BlitSurface(selected ? inverted[index] : glyph[GREEN][index], NULL, screen, &origin);
		}
	SDL_UpdateWindowSurface(window);
}

#define NONE -1
#define DONE previous_key = NONE; break
#define DIRECTION(X) (number ? X*number : X)
void handle_key_input(SDL_Event *event)
{
	static int previous_key = NONE;
	const int key = *event->text.text;
	static short number = 0;

	if (isdigit(key) && (number || key != '0') && !strchr("fF", previous_key))
	{
		number *= 10;
		number += key - '0';
		number = min(number, end.x);
		return;
	}
	if (previous_key != NONE) switch (previous_key)
	{
		case 'g':
			if (key == 'g')
				move(START, LEFT, ABSOLUTE); DONE;
		case 'f':
			find(key, AFTER); DONE;
		case 'F':
			find(key, BEFORE); DONE;
		default:
			DONE;
	}
	switch (key)
	{
		case 'h': 
			move(DIRECTION(-1), 0, RELATIVE); DONE;
		case 'j': 
			move(0, DIRECTION(1), RELATIVE); DONE;
		case 'k': 
			move(0, DIRECTION(-1), RELATIVE); DONE;
		case 'l': 
			move(DIRECTION(1), 0, RELATIVE); DONE;
		case 'w':
			word(number); DONE;
		case 'W':
			WORD(number); DONE;
		case 'b':
			back(number); DONE;
		case 'B':
			BACK(number); DONE;
		case '|':
			move(number, STAY, ABSOLUTE); DONE;
		case '^': 
			first_non_space(); DONE;
		case '0': 
			move(LEFT, STAY, ABSOLUTE); DONE;
		case '$': 
			move(end.x, STAY, ABSOLUTE); DONE;
		case '%': 
			match_pair(); DONE;
		case 'H': 
			move(START, TOP, ABSOLUTE); DONE;
		case 'M': 
			move(START, end.y/2, ABSOLUTE); DONE;
		case 'G': 
			if (number)
			{
				cursor.y = --number;
				first_non_space();
				DONE;
			}
		case 'L':
			move(START, end.y, ABSOLUTE); DONE;
		default: 
			previous_key = key;
			if (isprint(key)) printf("%c\n", key);
	}
	number = 0;
}


void setup()
{
	char *filename[SOUND_COUNT] = {
		"sound/block.wav",
		"sound/delete.wav",
		"sound/explode.wav",
		"sound/hurt.wav",
		"sound/jump.wav",
		"sound/move.wav",
		"sound/pickup.wav"
	};
	char string[2] = {0, 0};

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	TTF_Init();
	window = SDL_CreateWindow("vimtrix", CENTER, CENTER, width, height, SDL_WINDOW_METAL);
	screen = SDL_GetWindowSurface(window);
	SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, black.r, black.g, black.b));
	font = TTF_OpenFont("JetBrainsMono-Regular.ttf", 16);
	for (int c = ' '; c <= '~'; c++)
	{
		*string = c;
		glyph[PLAIN][c] = TTF_RenderText_Shaded(font, string, white, black);
		glyph[BODY][c] = TTF_RenderText_Shaded(font, string, body, black);
		glyph[RED][c] = TTF_RenderText_Shaded(font, string, red, black);
		glyph[GREEN][c] = TTF_RenderText_Shaded(font, string, green, black);
		glyph[BLUE][c] = TTF_RenderText_Shaded(font, string, blue, black);
		inverted[c] = SDL_CreateRGBSurface(0, glyph[0][c]->w, glyph[0][c]->h, 32, 255, 255, 255, 255);
		inverted[c] = TTF_RenderText_Shaded(font, string, black, white);
	}
	for (int i = 0; i < SOUND_COUNT; i++)
	{
		SDL_LoadWAV(filename[i], &audio[i], &buffer[i], &length[i]);
		device[i] = SDL_OpenAudioDevice(NULL, 0, &audio[i], NULL, 0);
	}
}

int main()
{
	bool is_running = true;

	setup();
	fill();
	render();
	while (is_running) 
	{
		if (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
				is_running = false;
			if (event.type == SDL_TEXTINPUT)
				handle_key_input(&event);
		}
		usleep(1000);
	}
	SDL_DestroyWindow(window);
	SDL_Quit();
}
