#include "vimtrix.h"

int isalnum_(int c)
{
	return (c == '_' || isalnum(c));
}

int isother(int c)
{
	return (!isalnum_(c) && !isspace(c));
}

bool forward()
{
	cursor.x++;
	if (end.x < cursor.x && cursor.y == end.y)
		return (false);
	if (end.x < cursor.x && cursor.y <= end.y)
	{
		cursor.y++;
		cursor.x = LEFT;
	}
	return (cursor.y <= end.y);
}

bool backward()
{
	cursor.x--;
	if (cursor.x < LEFT && cursor.y == TOP)
		return (false);
	if (cursor.x < LEFT && TOP <= cursor.y)
	{
		cursor.y--;
		cursor.x = end.x;
	}
	return (TOP <= cursor.y);
}

void space(short count)
{
	const position original = cursor;

	do forward(); while (0 < --count);
	play(MOVED ? JUMPED ? JUMP : MOVE : BLOCK);
	render();
}

void skip_spaces(bool (*skip)(void))
{
	while (isspace(cell[cursor.y][cursor.x]) && skip()) continue;
	cursor.y = clamp(TOP, cursor.y, end.y);
}

void word(short count)
{
	const position original = cursor;
	int (*isskippable)(int);

	do
	{
		isskippable = isalnum_(cell[cursor.y][cursor.x]) ? isalnum_ : isother;
		while (isskippable(cell[cursor.y][cursor.x]) && forward()) continue;
		skip_spaces(forward);
	} while (0 < --count);
	cursor.x = clamp(LEFT, cursor.x, end.x);
	play(MOVED ? JUMPED ? JUMP : MOVE : BLOCK);
	render();
}

void WORD(short count)
{
	const position original = cursor;

	do
	{
		while (!isspace(cell[cursor.y][cursor.x]) && forward()) continue;
		skip_spaces(forward);
	} while (0 < --count);
	cursor.x = clamp(LEFT, cursor.x, end.x);
	play(MOVED ? JUMPED ? JUMP : MOVE : BLOCK);
	render();
}

char next(char **cell)
{
	short x = cursor.x + 1;
	short y = cursor.y;

	if (end.x <= x)
	{
		x = LEFT;
		y++;
	}
	return (end.y < y ? 0 : cell[y][x]);
}

void word_end(short count)
{
	const position original = cursor;
	int (*isskippable)(int);

	do
	{
		forward();
		skip_spaces(forward);
		isskippable = isalnum_(cell[cursor.y][cursor.x]) ? isalnum_ : isother;
		while (isskippable(next(cell)) && forward()) continue;
	} while (0 < --count);
	cursor.x = clamp(LEFT, cursor.x, end.x);
	play(MOVED ? JUMPED ? JUMP : MOVE : BLOCK);
	render();
}

void WORD_END(short count)
{
	const position original = cursor;
	int (*isskippable)(int);

	do
	{
		forward();
		skip_spaces(forward);
		while (!isspace(next(cell)) && forward()) continue;
	} while (0 < --count);
	cursor.x = clamp(LEFT, cursor.x, end.x);
	play(MOVED ? JUMPED ? JUMP : MOVE : BLOCK);
	render();
}

void back(short count)
{
	const position original = cursor;
	int (*isskippable)(int);

	do if (backward())
	{
		skip_spaces(backward);
		isskippable = isalnum_(cell[cursor.y][cursor.x]) ? isalnum_ : isother;
		while (LEFT <= cursor.x && isskippable(cell[cursor.y][cursor.x]) && backward())
			continue;
		forward();
	} while (0 < --count);
	cursor.x = clamp(LEFT, cursor.x, end.x);
	play(MOVED ? JUMPED ? JUMP : MOVE : BLOCK);
	render();
}

void BACK(short count)
{
	const position original = cursor;

	do if (backward())
	{
		skip_spaces(backward);
		while (LEFT <= cursor.x && !isspace(cell[cursor.y][cursor.x]) && backward())
			continue;
		forward();
	} while (0 < --count);
	cursor.x = clamp(LEFT, cursor.x, end.x);
	play(MOVED ? JUMPED ? JUMP : MOVE : BLOCK);
	render();
}

char pair_of(char bracket)
{
	switch (bracket)
	{
		case '{': return ('}');
		case '}': return ('{');
		case '[': return (']');
		case ']': return ('[');
		case '(': return (')');
		case ')': return ('(');
		defalt: break;
	}
	return (0);
}

void match_pair()
{
	const position original = cursor;
	const char this = cell[cursor.y][cursor.x];
	const char pair = pair_of(this);
	bool (*skip)(void) = strchr("{[(", this) ? forward : backward;
	bool skip_next = false;
	bool searched = false;

	while (!searched && skip())
	{
		if (cell[cursor.y][cursor.x] == this)
			skip_next = true;
		searched = !skip_next && cell[cursor.y][cursor.x] == pair;
	}
	if (searched)
		render();
	else cursor = original;
	play(MOVED ? JUMPED ? JUMP : MOVE : BLOCK);
}

void first_non_space()
{
	cursor.x = LEFT;
	skip_spaces(forward);
	render();
}

void move(short x, short y, bool relative)
{
	position original = cursor;

	if (relative)
	{
		cursor.x += x;
		cursor.y += y;
	}
	else
	{
		if (x > STAY) cursor.x = x;
		if (y > STAY) cursor.y = y;
		if (x == START)
		{
			cursor.x = LEFT;
			while (isspace(cell[cursor.y][cursor.x]))
				cursor.x++;
		}
	}
	cursor.x = clamp(LEFT, cursor.x, end.x);
	cursor.y = clamp(TOP, cursor.y, end.y);
	play(MOVED ? JUMPED ? JUMP : MOVE : BLOCK);
	render();
}

void find(int letter, bool after)
{
	position original = cursor;
	position search = cursor;
	bool found = false;

	if (after)
		while (!found && ++cursor.x <= end.x)
			found = letter == cell[cursor.y][cursor.x];
	else
		while (!found && LEFT <= --cursor.x)
			found = letter == cell[cursor.y][cursor.x];
	if (found)
		render();
	else cursor = original;
	play(MOVED ? JUMPED ? JUMP : MOVE : BLOCK);
}
