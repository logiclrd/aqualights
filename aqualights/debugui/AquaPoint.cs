using System;
using System.Runtime.InteropServices;

namespace debugui
{
	[StructLayout(LayoutKind.Sequential)]
	public struct AquaPoint
	{
		public float X, Y;

		public AquaPoint(float x, float y)
		{
			X = x;
			Y = y;
		}

		public AquaPoint(double x, double y)
		{
			X = (float)x;
			Y = (float)y;
		}
	}
}
