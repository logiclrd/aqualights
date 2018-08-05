#pragma once

struct AquaSource
{
	float x, y;
	float dx, dy;

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

	float time_factor;

	AquaSource *first_source;
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
	short *light_for_pixel;

	int num_lights;
	short *light_pixel_count;
};

typedef struct AquaLightMap AquaLightMap;