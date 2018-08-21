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

AquaContext *aqua_initialize(int width, int height, float frames_per_cycle, float frames_per_day);
void aqua_free(AquaContext *context);
void aqua_update_ripple(AquaContext *context);
void aqua_add_source(AquaContext *context, AquaSource *source);
void aqua_add_random_source(AquaContext *context);
void aqua_update_sources(AquaContext *context);
void aqua_get_buffer_size(AquaContext *context, int *width, int *height);
void aqua_get_frame_size(AquaContext *context, int *width, int *height);
void aqua_get_frame(AquaContext *context, unsigned char *buffer);
AquaLightMap *aqua_generate_light_map(AquaContext *context, int num_lights, AquaPoint *lights);
void aqua_light_map_get_light_for_pixel(AquaLightMap *light_map, int *width, int *height, int *buffer);
void aqua_light_map_get_light_pixel_count(AquaLightMap *light_map, int *num_lights, int *buffer);
void aqua_light_map_get_light_brightness(AquaLightMap *light_map, int *num_lights, unsigned char *buffer);
void aqua_light_map_render(AquaLightMap *light_map, AquaContext *context);
void aqua_free_light_map(AquaLightMap *light_map);
void aqua_advance_sky(AquaContext *context);
void aqua_get_current_sky_palette(AquaContext *context, AquaColour *palette);
void aqua_get_light_colours_from_brightnesses(AquaContext *context, AquaLightMap *light_map, AquaColour *light_colours);

