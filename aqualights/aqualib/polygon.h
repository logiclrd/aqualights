#pragma once

#include "aqua.h"

#include <algorithm>
#include <vector>

template <typename TPixel>
void fill_polygon(const std::vector<AquaPoint> &points, TPixel *buffer, int buffer_width, int buffer_height, TPixel fill_colour)
{
	// Public-domain code by Darel Rex Finley, 2007
	// Adapted to C++ by Jonathan Gilbert
	std::vector<int> node_x;

	// Loop through the rows of the image.
	for (int pixel_y = 0; pixel_y < buffer_height; pixel_y++)
	{
		// Build a list of nodes.
		node_x.clear();

		for (std::vector<AquaPoint>::size_type i = 0, j = points.size() - 1; i < points.size(); j = i, i++)
		{
			if ((points[i].y < pixel_y && points[j].y >= pixel_y)
			 || (points[j].y < pixel_y && points[i].y >= pixel_y))
			{
				float intersection_x = points[i].x + (pixel_y - points[i].y) * (points[j].x - points[i].x) / (points[j].y - points[i].y);

				node_x.push_back((int)intersection_x);
			}
		}

		// Sort the nodes.
		std::sort(node_x.begin(), node_x.end());

		// Fill the pixels between node pairs.
		for (std::vector<int>::size_type i = 0; i < node_x.size(); i += 2)
		{
			if (node_x[i] >= buffer_width)
				break;

			if (node_x[i + 1] > 0)
			{
				if (node_x[i] < 0)
					node_x[i] = 0;
				if (node_x[i + 1] > buffer_width)
					node_x[i + 1] = buffer_width;

				for (int pixel_x = node_x[i], d = pixel_y * buffer_width + pixel_x; pixel_x < node_x[i + 1]; pixel_x++, d++)
					buffer[d] = fill_colour;
			}
		}
	}
}
