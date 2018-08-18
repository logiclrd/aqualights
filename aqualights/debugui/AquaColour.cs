using System;
using System.Runtime.InteropServices;

namespace debugui
{
	[StructLayout(LayoutKind.Sequential, Pack = 4)]
	public struct AquaColour
	{
		public byte r;
		public byte g;
		public byte b;
	}
}
