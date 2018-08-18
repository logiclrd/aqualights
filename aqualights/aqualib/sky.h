#pragma once

#pragma pack(4)

struct AquaColour
{
	unsigned char r, g, b;
};

typedef struct AquaColour AquaColour;

struct AquaSky
{
	AquaColour sky_colour[256];
};

extern const AquaSky sky[];
extern const int sky_count;
