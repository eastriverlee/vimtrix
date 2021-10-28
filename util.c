#include "vimtrix.h"

int min(int a, int b)
{
	return (a < b ? a : b);
}

int max(int a, int b)
{
	return (a > b ? a : b);
}

int clamp(int minimum, int n, int maximum)
{
	return (max(minimum, min(n, maximum)));
}

void play(char sound)
{
	SDL_ClearQueuedAudio(device[sound]);
	SDL_QueueAudio(device[sound], buffer[sound], length[sound]);
	SDL_PauseAudioDevice(device[sound], 0);
}
