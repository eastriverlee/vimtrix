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
SDL_Color red = {255,80,220};
SDL_Color green = {100,255,140};
SDL_Color blue = {0,220,255};
SDL_Color black = {10,20,40};
#define BODY 0
#define RED 1
#define GREEN 2
#define BLUE 3
#define PLAIN 4
SDL_Surface *glyph[5][128];
SDL_Surface *inverted[128];
pthread_t thread;
pthread_mutex_t mutex;

char **cell;
position cursor = {LEFT, TOP};
position end;

#define INTERVAL 1000
void *fall(void *null)
{
	char *top_row;
	short line;

	(void)null;
	while (true)
	{
		usleep(INTERVAL * 1000);
		pthread_mutex_lock(&mutex);
		free(cell[end.y]);
		for (short y = end.y; TOP < y; y--)
		{
			cell[y] = cell[y - 1];
			sprintf(cell[y], "%2d", y + 1);
		}
		cell[TOP] = calloc(end.x + 1, 1);
		sprintf(cell[TOP], " 1");
		for (short x = LEFT; x <= end.x; x++)
			cell[TOP][x] = max(' ', rand() % '~');
		render();
		pthread_mutex_unlock(&mutex);
	}
}

void fill()
{
	const int x_unit = glyph[BODY][' ']->w;
	const int y_unit = glyph[BODY][' ']->h;
	const int x_lim = width - (padding + x_unit);
	const int y_lim = height - padding;
	SDL_Rect origin = {padding, padding};
	short x, y, line;
	char index;
	bool selected;

	cell = malloc(256 * sizeof(char *));
	for (y = 0; origin.y < y_lim; origin.y += y_unit, y++)
	{
		line = y + 1;
		cell[y] = malloc(256);
		sprintf(cell[y], "%2d ", line);
		for (x = 0, origin.x = padding; origin.x < x_lim; origin.x += x_unit, x++)
			if (LEFT <= x)
				cell[y][x] = max(' ', rand() % '~');
		cell[y][x] = 0;
	}
	cell[y] = NULL;
	end.x = --x;
	end.y = --y;
	pthread_mutex_init(&mutex, NULL);
	//pthread_create(&thread, NULL, fall, NULL);
}

#define NEXT(CELL) CELL[y][x < end.x ? x+1 : x]
void render()
{
	const int x_unit = glyph[BODY][' ']->w;
	const int y_unit = glyph[BODY][' ']->h;
	SDL_Rect origin = {padding, padding};
	short x, y;
	char character, color = BODY;
	bool selected;

	for (y = 0; y <= end.y; origin.y += y_unit, y++)
		for (x = 0, origin.x = padding; x <= end.x; origin.x += x_unit, x++)
		{
			selected = cursor.x == x && cursor.y == y;
			character = cell[y][x];
			if (character == ' ')
				color = NEXT(cell)%4;
			if (x < 3)
				SDL_BlitSurface(selected ? inverted[character] : glyph[PLAIN][character], NULL, screen, &origin);
			else
				SDL_BlitSurface(selected ? inverted[character] : glyph[color][character], NULL, screen, &origin);
		}
	SDL_UpdateWindowSurface(window);
}

#define NONE -1
#define DONE previous_key = NONE; break
#define MINUS -1
#define PLUS 1
#define DISTANCE(X) (number ? X*number : X)
void handle_key_input(SDL_Event *event)
{
	static int previous_key = NONE;
	const int key = *event->text.text;
	static short number = 0;

	pthread_mutex_lock(&mutex);
	if (isdigit(key) && (number || key != '0') && !strchr("fF", previous_key))
	{
		number *= 10;
		number += key - '0';
		number = min(number, end.x);
		pthread_mutex_unlock(&mutex);
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
		case ' ':
			space(number); DONE;
		case 'h': 
			move(DISTANCE(MINUS), 0, RELATIVE); DONE;
		case 'j': 
			move(0, DISTANCE(PLUS), RELATIVE); DONE;
		case 'k': 
			move(0, DISTANCE(MINUS), RELATIVE); DONE;
		case 'l': 
			move(DISTANCE(PLUS), 0, RELATIVE); DONE;
		case 'w':
			word(number); DONE;
		case 'W':
			WORD(number); DONE;
		case 'e':
			word_end(number); DONE;
		case 'E':
			WORD_END(number); DONE;
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
	pthread_mutex_unlock(&mutex);
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
		inverted[c] = SDL_CreateRGBSurface(0, glyph[PLAIN][c]->w, glyph[PLAIN][c]->h, 32, 255, 255, 255, 255);
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
}
