using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace debugui
{
	public class AquaLib
	{
		[DllImport("aquadll", CallingConvention = CallingConvention.Cdecl)]
		public static extern IntPtr aqua_initialize(int width, int height, float frames_per_cycle);
		[DllImport("aquadll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void aqua_update_ripple(IntPtr context);
		[DllImport("aquadll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void aqua_add_source(IntPtr context, ref AquaSource source);
		[DllImport("aquadll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void aqua_update_sources(IntPtr context);
		[DllImport("aquadll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void aqua_get_buffer_size(IntPtr context, out int width, out int height);
		[DllImport("aquadll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void aqua_get_frame_size(IntPtr context, out int width, out int height);
		[DllImport("aquadll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void aqua_get_frame(IntPtr context, byte[] buffer);
	}
}
