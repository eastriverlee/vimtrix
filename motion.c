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
	if (cursor.x == end.x && cursor.y == end.y)
		return (false);
	cursor.x++;
	if (end.x < cursor.x && cursor.y <= end.y)
	{
		cursor.x = 0;
		cursor.y++;
	}
	return (cursor.y <= end.y);
}

bool backward()
{
	if (!cursor.x && !cursor.y)
		return (false);
	cursor.x--;
	if (cursor.x < 0 && 0 <= cursor.y)
	{
		cursor.x = end.x;
		cursor.y--;
	}
	return (0 <= cursor.y);
}

void skip_spaces(bool (*skip)(void))
{
	while (isspace(cell[cursor.y][cursor.x]) && skip())
		continue;
	cursor.y = clamp(0, cursor.y, end.y);
}

void word()
{
	const position original = cursor;
	int (*isskippable)(int);

	isskippable = isalnum_(cell[cursor.y][cursor.x]) ? isalnum_ : isother;
	while (isskippable(cell[cursor.y][cursor.x]) && forward())
		continue;
	skip_spaces(forward);
	play(original.x != cursor.x || original.y != cursor.y ? JUMP : BLOCK);
	render();
}

void WORD()
{
	const position original = cursor;

	while (!isspace(cell[cursor.y][cursor.x]) && forward())
		continue;
	skip_spaces(forward);
	play(original.x != cursor.x || original.y != cursor.y ? JUMP : BLOCK);
	render();
}

void back()
{
	const position original = cursor;
	int (*isskippable)(int);

	cursor.x--;
	skip_spaces(backward);
	isskippable = isalnum_(cell[cursor.y][cursor.x]) ? isalnum_ : isother;
	while (0 < cursor.x && isskippable(cell[cursor.y][cursor.x]) && backward())
		continue;
	cursor.x++;
	play(original.x != cursor.x || original.y != cursor.y ? JUMP : BLOCK);
	render();
}

void BACK()
{
	const position original = cursor;

	cursor.x--;
	skip_spaces(backward);
	while (0 < cursor.x && !isspace(cell[cursor.y][cursor.x]) && backward())
		continue;
	cursor.x++;
	play(original.x != cursor.x || original.y != cursor.y ? JUMP : BLOCK);
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
		defalt: return (0);
	}
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
	else
		cursor = original;
}

void first_non_space()
{
	cursor.x = 0;
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
			cursor.x = 0;
			while (isspace(cell[cursor.y][cursor.x]))
				cursor.x++;
		}
	}
	cursor.x = clamp(0, cursor.x, end.x);
	cursor.y = clamp(0, cursor.y, end.y);
	if (original.x != cursor.x || original.y != cursor.y)
		play(relative ? MOVE : JUMP);
	else
		play(BLOCK);
	render();
}

void find(int letter, bool after)
{
	position search = cursor;
	bool found = false;

	if (after)
		while (!found && ++search.x <= end.x)
			found = letter == cell[search.y][search.x];
	else
		while (!found && 0 <= --search.x)
			found = letter == cell[search.y][search.x];
	if (found)
	{
		cursor = search;
		render();
	}
}
