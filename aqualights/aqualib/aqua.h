#pragma once

#include "sky.h"

struct AquaSource
{
	float x, y;
	float dx, dy;
	bool is_stationary;

	float radius;
	float intensity;

	int local_time, duration;

	float fade_ratio;

	struct AquaSource *next;
};

typedef struct AquaSource AquaSource;

struct AquaContext
{
	int window_width, window_height;
	int window_offset_x, window_offset_y;
	int window_right, window_bottom;
	int grid_size_x, grid_size_y;
	int grid_size_xy;
	int gw;

	bool move_down;

	float *func;
	float *funci;
	float *damp;

	float frames_per_cycle;
	float time_factor;

	float rand_factor;

	AquaSource *first_source;

	const AquaSky *from_sky;
	const AquaSky *to_sky;
	int to_sky_index;
	float sky_fade_t;
	float sky_fade_dt;
	float sky_day_night_t;
	float sky_day_night_dt;
	float sky_sun_shade_t;
	float sky_sun_shade_dt;
	AquaSky current_sky;
};

typedef struct AquaContext AquaContext;

struct AquaPoint
{
	float x, y;

	AquaPoint()
	{
		x = 0;
		y = 0;
	}

	AquaPoint(float x, float y)
	{
		this->x = x;
		this->y = y;
	}

	template <typename DuckPoint>
	AquaPoint(const DuckPoint &other)
	{
		x = other.x;
		y = other.y;
	}
};

typedef struct AquaPoint AquaPoint;

struct AquaLightMap
{
	int width, height;
	int *light_for_pixel;

	int num_lights;
	int *light_pixel_count;
	unsigned char *light_brightness;
};

typedef struct AquaLightMap AquaLightMap;