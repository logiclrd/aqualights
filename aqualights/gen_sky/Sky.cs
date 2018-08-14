using System;

class Sky
{
	public Colour[] Colours = new Colour[256];
	public double Brightness;

	public Sky(byte[] palette)
	{
		for (int i = 0; i < 256; i++)
			this.Colours[i] = new Colour(palette, i * 3);

		CalculateBrightness();
	}

	public void CalculateBrightness()
	{
		double averageBrightness = 0.0;
		double maximumBrightness = 0.0;

		for (int i = 0; i < 256; i++)
		{
			double thisColourBrightness = this.Colours[i].Brightness;

			averageBrightness += thisColourBrightness;
			maximumBrightness = Math.Max(maximumBrightness, thisColourBrightness);
		}

		averageBrightness /= 256.0;

		this.Brightness = averageBrightness + maximumBrightness;
	}

	public override string ToString()
	{
		return string.Join(", ", this.Colours);
	}
}
