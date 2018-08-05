using System;
using System.Runtime.InteropServices;

namespace debugui
{
	[StructLayout(LayoutKind.Sequential)]
	public struct AquaSource
	{
		public float X, Y;
		public float DX, DY;

		public float Radius;
		public float Intensity;

		public int LocalTime, Duration;

		public float FadeRatio;

		public IntPtr Next;
	}
}
