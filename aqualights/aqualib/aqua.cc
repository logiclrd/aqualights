#include "aqua.h"
#include "math.h"

AquaContext *aqua_initialize(int width, int height, float frames_per_cycle)
{
	AquaContext *context = new AquaContext();

	context->window_width = width;
	context->window_height = height;

	int border = width / 9;

	if (border < 20)
		border = 20;

	context->window_offset_x = border;
	context->window_offset_y = border;

	context->grid_size_x = context->window_width + context->window_offset_x * 2;
	context->grid_size_y = context->window_height + context->window_offset_y * 2;

	context->gw = context->grid_size_x;

	context->window_right = context->window_offset_x + context->window_width - 1;
	context->window_bottom = context->window_offset_y + context->window_height - 1;

	context->grid_size_xy = context->grid_size_x * context->grid_size_y;

	context->func = new float[context->grid_size_xy];
	context->funci = new float[context->grid_size_xy];
	context->damp = new float[context->grid_size_xy];

	for (int i = 0; i < context->grid_size_xy; i++)
	{
		context->func[i] = 0.0f;;
		context->funci[i] = 0.0f;
		context->damp[i] = 1.0f;
	}

	for (int i = 0; i != context->window_offset_x; i++)
		for (int j = 0; j != context->grid_size_y; j++)
			context->damp[i + j * context->gw] =
			context->damp[context->grid_size_x - 1 - i + context->gw * j] = (.999f - (context->window_offset_x - i) * .002f);

	for (int i = 0; i != context->window_offset_y; i++)
		for (int j = 0; j != context->grid_size_x; j++)
			context->damp[j + context->gw * i] =
			context->damp[j + (context->grid_size_y - 1 - i) * context->gw] = (.999f - (context->window_offset_y - i) * .002f);

	context->time_factor = 6.283185f / frames_per_cycle;

	return context;
}

void aqua_update_ripple(AquaContext *context)
{
	int mxx = context->grid_size_x - 1;
	int mxy = context->grid_size_y - 1;

	int jstart;
	int jend;
	int jinc;

	if (context->move_down)
	{
		jstart = 1;
		jend = mxy;
		jinc = 1;
		context->move_down = false;
	}
	else
	{
		jstart = mxy - 1;
		jend = 0;
		jinc = -1;
		context->move_down = true;
	}

	bool move_right = context->move_down;

	float sinhalfth = sinf(0.125f);
	float sinth = sinf(0.25f);
	float scaleo = 1 - sqrtf(4 * sinhalfth * sinhalfth - sinth * sinth);

	for (int j = jstart; j != jend; j += jinc)
	{
		int istart;
		int iend;
		int iinc;

		if (move_right)
		{
			iinc = 1;
			istart = 1;
			iend = mxx;

			move_right = false;
		}
		else
		{
			iinc = -1;
			istart = mxx - 1;
			iend = 0;

			move_right = true;
		}

		int gi = j * context->gw + istart;
		int giEnd = j * context->gw + iend;

		for (; gi != giEnd; gi += iinc) {
			float previ = context->func[gi - 1];
			float nexti = context->func[gi + 1];
			float prevj = context->func[gi - context->gw];
			float nextj = context->func[gi + context->gw];

			float basis = (nexti + previ + nextj + prevj) * .25f;
			
			float a = context->func[gi] - basis;
			float b = context->funci[gi];

			float damp = context->damp[gi];

			if (damp != 1)
			{
				a *= damp;
				b *= damp;
			}

			context->func[gi] = basis + a * scaleo - b * sinth;
			context->funci[gi] = b * scaleo + a * sinth;
		}
	}
}

void aqua_add_source(AquaContext *context, AquaSource *source)
{
	AquaSource *link = new AquaSource(*source);

	link->fade_ratio = 1.0f / (link->duration * 0.25f);

	link->next = context->first_source;
	context->first_source = link;

	if (source->next)
		aqua_add_source(context, source->next);
}

void aqua_update_sources(AquaContext *context)
{
	AquaSource **ref = &context->first_source;

	for (AquaSource *source = context->first_source; source != NULL; ref = &source->next, source = source->next)
	{
top_of_loop:
		source->local_time++;

		if (source->local_time > source->duration)
		{
			*ref = source->next;
			delete source;
			source = *ref;

			if (source == NULL)
				break;

			goto top_of_loop;
		}

		float lifetime_intensity = 1.0;

		if (source->local_time < source->duration / 4)
			lifetime_intensity = source->local_time * source->fade_ratio;
		if (source->local_time >= source->duration * 3 / 4)
			lifetime_intensity = (source->duration - source->local_time) * source->fade_ratio;

		float intensity = source->intensity * lifetime_intensity * sinf(source->local_time * context->time_factor);

		int start_y = (int)ceilf(source->y - source->radius);
		int end_y = (int)floorf(source->y + source->radius);

		if ((start_y < context->grid_size_y) && (end_y >= 0))
		{
			if (start_y < 0)
				start_y = 0;
			if (end_y >= context->grid_size_y)
				end_y = context->grid_size_y - 1;

			for (int y = start_y; y <= end_y; y++)
			{
				float sector_radius = fabsf(y - source->y);

				if (sector_radius >= source->radius)
					continue;

				float sector_width = sqrtf(source->radius * source->radius - sector_radius * sector_radius);

				int start_x = (int)ceilf(source->x - sector_width);
				int end_x = (int)floorf(source->x + sector_width);

				if ((start_x < context->grid_size_x) && (end_x >= 0))
				{
					if (start_x < 0)
						start_x = 0;
					if (end_x >= context->grid_size_x)
						end_x = context->grid_size_x - 1;

					for (int x = start_x, d = y * context->gw + x; x <= end_x; x++, d++)
					{
						context->func[d] = intensity;
						context->funci[d] = 0;
					}
				}
			}
		}

		source->x += source->dx;
		source->y += source->dy;
	}
}

void aqua_get_buffer_size(AquaContext *context, int *width, int *height)
{
	*width = context->grid_size_x;
	*height = context->grid_size_y;
}

void aqua_get_frame_size(AquaContext *context, int *width, int *height)
{
	*width = context->window_width;
	*height = context->window_height;
}

void aqua_get_frame(AquaContext *context, unsigned char *buffer)
{
	int d = context->window_offset_y * context->gw + context->window_offset_x;
	int d_row_delta = context->grid_size_x - context->window_width;

	int o = 0;

	for (int y = 0; y < context->window_height; y++, d += d_row_delta)
		for (int x = 0; x < context->window_width; x++, d++, o++)
		{
			int pixel_value = (int)((context->func[d] + 1) * 128);

			if (pixel_value < 0)
				pixel_value = 0;
			if (pixel_value > 255)
				pixel_value = 255;

			buffer[o] = (unsigned char)pixel_value;
		}
}

// TODO: voronoi mapping of light positions
// TODO: convert brightness to palette
// TODO: palette rotation functions
//       => long day/night cycle
//       => short, stochastic sun/shade cycle during day
// TODO: create sources randomly
