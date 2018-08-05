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
