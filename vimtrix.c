#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <termios.h>

typedef struct dimension
{
	int width;
	int height;
} dimension;

typedef struct position
{
	int x;
	int y;
} position;

typedef struct letter
{
	int x;
	int y;
	char character;
	bool reversed;
} letter;

typedef letter ** grid;

grid screen;
position cursor;
dimension size;

void measure(dimension *size)
{
	struct winsize window;

	ioctl(STDOUT_FILENO, TIOCGWINSZ, &window);
	size->height = window.ws_row;
	size->width = window.ws_col;
}

#define ECHOFLAGS (ECHO | ECHOE | ECHOK | ECHONL)
void setup(grid *screen, dimension size)
{
	static struct termios terminal;

	tcgetattr(STDOUT_FILENO, &terminal);
	terminal.c_lflag &= ~ICANON;
	terminal.c_lflag &= ~ECHOFLAGS;
	tcsetattr(STDOUT_FILENO, TCSANOW, &terminal);
	*screen = malloc(sizeof(letter *) * size.height);
	for (int i = 0; i < size.height; i++)
	{
		(*screen)[i] = malloc(sizeof(letter) * size.width);
		for (int j = 0; j < size.width; j++)
		{
			//(*screen)[i][j].character = rand() % 96 + 31;
			(*screen)[i][j].character = j % 2 ? 'a' : ' ';
			(*screen)[i][j].reversed = false;
		}
	}
}

int min(int a, int b)
{
	return (a < b ? a : b);
}

int max(int a, int b)
{
	return (a > b ? a : b);
}

int clamp(int n, int minimum, int maximum)
{
	return (max(min(maximum, n), minimum));
}

#define NONE "\033[0m"
#define REVERSED "\033[7m"

void print_letter(letter letter)
{
	printf("%s%c%s", letter.reversed ? REVERSED : "", letter.character, NONE);
}

void move(int x, int y)
{
	x = clamp(x, 0, size.width - 1);
	y = clamp(y, 0, size.height - 1);
	printf("\033[%d;%dH", y + 1, x + 1);
	cursor.x = x;
	cursor.y = y;
}

void render(grid screen, dimension size)
{
	const int eof = size.height - 1;
	for (int y = 0; y < size.height; y++)
	{
		for (int x = 0; x < size.width; x++)
			print_letter(screen[y][x]);
		if (y != eof)
			putchar('\n');
	}
}

#define BEGINNING (y == 0 && x == 0)
void BACK(int x, int y)
{
	const int end_of_line = size.width - 1;

	while (!BEGINNING && screen[y][x].character != ' ')
		x == 0 ? x = end_of_line, y-- : x--;
	while (!BEGINNING && screen[y][x].character == ' ')
		x == 0 ? x = end_of_line, y-- : x--;
	while (!BEGINNING && screen[y][x].character != ' ')
		x == 0 ? x = end_of_line, y-- : x--;
	//if (!(BEGINNING && screen[0][0].character == ' '))
		x == end_of_line ? x = 0, y++ : x++;
	move(x, y);
}

#define END (y == last_line && x == end_of_line)
void WORD(int x, int y)
{
	const int end_of_line = size.width - 1;
	const int last_line = size.height - 1;

	while (!END && screen[y][x].character != ' ')
		x == end_of_line ? x = 0, y++ : x++;
	while (!END && screen[y][x].character == ' ')
		x == end_of_line ? x = 0, y++ : x++;
	move(x, y);
}

int main()
{
	int input;

	if (isatty(STDOUT_FILENO))
	{
		measure(&size);
		setup(&screen, size);
		render(screen, size);
		move(cursor.x, cursor.y);
		while (true)
		{
			input = getchar();
			switch (input)
			{
				case 'h':
					move(cursor.x - 1, cursor.y); break;
				case 'j':
					move(cursor.x, cursor.y + 1); break;
				case 'k':
					move(cursor.x, cursor.y - 1); break;
				case 'l':
					move(cursor.x + 1, cursor.y); break;
				case 'g':
					if (getchar() == 'g')
						move(0, 0); break;
				case 'B':
					BACK(cursor.x, cursor.y); break;
				case 'G':
					move(0, size.height); break;
				case 'W':
					WORD(cursor.x, cursor.y); break;
				default: break;
			}
		}
	}
}
