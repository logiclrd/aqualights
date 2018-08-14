using System;

struct Colour
{
	public byte R, G, B;

	public Colour(byte[] palette, int offset)
	{
		R = palette[offset];
		G = palette[offset + 1];
		B = palette[offset + 2];
	}

	public double Brightness
	{
		get
		{
			return
				rY * sRGBInverseGamma(R) +
				gY * sRGBInverseGamma(G) +
				bY * sRGBInverseGamma(B);
		}
	}

	public override string ToString()
	{
		return R + ", " + G + ", " + B;
	}

	// sRGB luminance(Y) values
	const double rY = 0.212655;
	const double gY = 0.715158;
	const double bY = 0.072187;

	private double sRGBInverseGamma(int curveIntensity)
	{
		double normalizedIntensity = curveIntensity / 255.0;

		if (normalizedIntensity <= 0.04045)
			return normalizedIntensity / 12.92;
		else
			return Math.Pow((normalizedIntensity + 0.055) / 1.055, 2.4);
	}
}
