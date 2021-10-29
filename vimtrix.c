#include "vimtrix.h"

const int padding = 10;
const int width = 800;
const int height = 800;

SDL_Event event;
SDL_Window *window;
SDL_Renderer *renderer;

TTF_Font *font;
#define BODY 0
#define RED 1
#define GREEN 2
#define BLUE 3
#define PLAIN 4
#define WHITE 4
#define BLACK 5
#define INVERTED 5
SDL_Color color[6] = {
 {200,220,255},
 {255,80,220},
 {100,255,140},
 {0,220,255},
 {255,255,255},
 {10,20,40},
};
SDL_Surface *surface[6][128];
SDL_Texture *glyph[6][128];
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
	const int x_unit = surface[BODY][' ']->w;
	const int y_unit = surface[BODY][' ']->h;
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
	const int x_unit = surface[BODY][' ']->w;
	const int y_unit = surface[BODY][' ']->h;
	SDL_Rect origin = {padding, padding, x_unit, y_unit};
	short x, y;
	char c, color = BODY;
	bool selected;

	SDL_RenderClear(renderer);
	for (y = 0; y <= end.y; origin.y += y_unit, y++)
		for (x = 0, origin.x = padding; x <= end.x; origin.x += x_unit, x++)
		{
			selected = cursor.x == x && cursor.y == y;
			c = cell[y][x];
			if (c == ' ')
				color = NEXT(cell)%4;
			if (x < 3)
				SDL_RenderCopy(renderer, glyph[WHITE][c], NULL, &origin);
			else
				SDL_RenderCopy(renderer, glyph[selected ? INVERTED : color][c], NULL, &origin);
		}
	SDL_RenderPresent(renderer);
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

void assign_texture(char kind, char c)
{
	char text[] = {c, 0};

	if (kind == INVERTED)
		surface[kind][c] = TTF_RenderText_Shaded(font, text, color[BLACK], color[WHITE]);
	else
		surface[kind][c] = TTF_RenderText_Shaded(font, text, color[kind], color[BLACK]);
	glyph[kind][c] = SDL_CreateTextureFromSurface(renderer, surface[kind][c]);
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

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	TTF_Init();
	window = SDL_CreateWindow("vimtrix", CENTER, CENTER, width/2, height/2, SDL_WINDOW_METAL | SDL_WINDOW_ALLOW_HIGHDPI);
	renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_SetRenderDrawColor(renderer, color[BLACK].r, color[BLACK].g, color[BLACK].b, 255);
	font = TTF_OpenFont("JetBrainsMono-Regular.ttf", 16);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, width, height);
	for (char c = ' '; c <= '~'; c++)
		for (char kind = BODY; kind <= INVERTED; kind++)
			assign_texture(kind, c);
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
