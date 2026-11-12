#include "simple_logger.h"

#include <SDL.h>

#include "time.h"

typedef struct
{
	Uint64 ticks_per_second;
	Uint64 ticks_since_start;
	float real_delta;
	float scaled_delta;
	float time_scale;
	float target_time_scale;

	//maybe smothen time scaleing eventually
	//float time_scale_transition_speed;
}TimeSystem;

extern int __DEBUG;

static TimeSystem time_system = { 0 };

void time_system_close();


void time_system_init()
{
	time_system.ticks_per_second = SDL_GetPerformanceFrequency();
	time_system.ticks_since_start = SDL_GetPerformanceCounter();
	time_system.time_scale = 1;
	time_system.target_time_scale = 1;

	if (__DEBUG)slog("time system initialized");
	atexit(time_system_close);
}
void time_system_close()
{
	memset(&time_system, 0, sizeof(TimeSystem));
	if (__DEBUG)slog("mouse system closed");
}
float time_get_real_delta()
{
	return time_system.real_delta;
}
float time_get_scaled_delta()
{
	return time_system.scaled_delta;
}
void time_set_time_scale(float time_scale)
{
	if (time_scale < 0)
	{
		slog("cannot make the time scale negative");
		return;
	}
	time_system.target_time_scale = time_scale;
}
float time_get_time_scale()
{
	return time_system.time_scale;
}

void time_frame_update()
{
	Uint64 ticks_since_start;
	Uint64 ticks_since_last_frame;

	ticks_since_start = SDL_GetPerformanceCounter();
	ticks_since_last_frame = ticks_since_start - time_system.ticks_since_start;
	time_system.ticks_since_start = ticks_since_start;

	if (time_system.ticks_per_second > 0)
	{
		time_system.real_delta = (float) ticks_since_last_frame / (float) time_system.ticks_per_second;
	}
	else
	{
		slog("ticks_per_second is 0 somehow");
		time_system.real_delta = 0;
	}

	//clamp delta time if big lag
	if (time_system.real_delta > 0.1)
	{
		time_system.real_delta = 0.1;
	}

	if (time_system.time_scale != time_system.target_time_scale)
	{
		time_system.time_scale = time_system.target_time_scale;
	}

	time_system.scaled_delta = time_system.real_delta * time_system.time_scale;
}
