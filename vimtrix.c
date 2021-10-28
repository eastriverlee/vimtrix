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
SDL_Color blue = {200,220,255};
SDL_Color black = {0,0,0};
SDL_Surface *letter[128];
SDL_Surface *letter_blue[128];
SDL_Surface *inverted[128];

char cell[256][256];
position cursor;
position end;

void fill()
{
	const int x_lim = width - padding;
	const int y_lim = height - padding;
	const int x_unit = letter[' ']->w;
	const int y_unit = letter[' ']->h;
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
	const int x_unit = letter[' ']->w;
	const int y_unit = letter[' ']->h;
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
				SDL_BlitSurface(selected ? inverted[index] : letter[index], NULL, screen, &origin);
			else
				SDL_BlitSurface(selected ? inverted[index] : letter_blue[index], NULL, screen, &origin);
		}
	SDL_UpdateWindowSurface(window);
}

#define DONE previous_key = 0; break
void handle_key_input(SDL_Event *event)
{
	static int previous_key = 0;
	const int key = *event->text.text;
	static short number = 0;

	if (isdigit(key) && (number || key != '0'))
	{
		number *= 10;
		number += key - '0';
		number = min(number, end.x);
		return;
	}
	if (previous_key) switch (previous_key)
	{
		case 'g':
			if (key == 'g')
				move(START, 0, ABSOLUTE); DONE;
		case 'f':
			find(key, AFTER); DONE;
		case 'F':
			find(key, BEFORE); DONE;
		default:
			DONE;
	}
	switch (key)
	{
		case 'w':
			word(); DONE;
		case 'W':
			WORD(); DONE;
		case 'b':
			back(); DONE;
		case 'B':
			BACK(); DONE;
		case '|':
			move(number, STAY, ABSOLUTE); DONE;
		case '^': 
			first_non_space(); DONE;
		case '0': 
			move(0, STAY, ABSOLUTE); DONE;
		case '$': 
			move(end.x, STAY, ABSOLUTE); DONE;
		case 'H': 
			move(START, 0, ABSOLUTE); DONE;
		case 'M': 
			move(START, end.y/2, ABSOLUTE); DONE;
		case 'L': case 'G':
			move(START, end.y, ABSOLUTE); DONE;
		case 'h': 
			move(-1, 0, RELATIVE); DONE;
		case 'j': 
			move(0, 1, RELATIVE); DONE;
		case 'k': 
			move(0, -1, RELATIVE); DONE;
		case 'l': 
			move(1, 0, RELATIVE); DONE;
		case '%': 
			match_pair(); DONE;
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
	font = TTF_OpenFont("JetBrainsMono-Regular.ttf", 16);
	for (int c = 32; c < 127; c++)
	{
		*string = c;
		letter[c] = TTF_RenderText_Shaded(font, string, white, black);
		letter_blue[c] = TTF_RenderText_Shaded(font, string, blue, black);
		inverted[c] = SDL_CreateRGBSurface(0, letter[c]->w, letter[c]->h, 32, 255, 255, 255, 255);
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
