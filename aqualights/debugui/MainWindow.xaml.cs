using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;

namespace debugui
{
	/// <summary>
	/// Interaction logic for MainWindow.xaml
	/// </summary>
	public partial class MainWindow : Window
	{
		public MainWindow()
		{
			InitializeComponent();
		}

		IntPtr _context;
		BinaryWriter _videoWriter = null;

		private void cmdInitialize_Click(object sender, RoutedEventArgs e)
		{
			_context = AquaLib.aqua_initialize(100, 75, 20, 10 /* frames per second */ * 3600 /* seconds per hour */ * 4 /* hours */);
		}

		DispatcherTimer tmrAdvance;

		private void tbAdvance_Click(object sender, RoutedEventArgs e)
		{
			if (tmrAdvance == null)
			{
				tmrAdvance = new DispatcherTimer(DispatcherPriority.Send);
				tmrAdvance.IsEnabled = false;
				tmrAdvance.Interval = TimeSpan.FromMilliseconds(20);
				tmrAdvance.Tick += tmrAdvance_Tick;
			}

			tmrAdvance.IsEnabled = tbAdvance.IsChecked ?? false;
		}

		DateTime _nextAddSource = DateTime.MinValue;

		private void tmrAdvance_Tick(object sender, EventArgs e)
		{
			if ((tbAddSource.IsChecked ?? false) && (_nextAddSource < DateTime.UtcNow))
			{
				_nextAddSource = DateTime.UtcNow.AddSeconds(1.0);
				AquaLib.aqua_add_random_source(_context);
			}

			AquaLib.aqua_update_ripple(_context);
			AquaLib.aqua_update_sources(_context);

			AquaLib.aqua_advance_sky(_context);

			var skyPaletteNative = new AquaColour[256];

			AquaLib.aqua_get_current_sky_palette(_context, skyPaletteNative);

			var skyPalette = new Color[256];
			var skyPalettePbgra32 = new int[256];

			for (int i = 0; i < 256; i++)
			{
				skyPalette[i] = Color.FromArgb(255, skyPaletteNative[i].r, skyPaletteNative[i].g, skyPaletteNative[i].b);
				skyPalettePbgra32[i] = ToPbgra32(skyPalette[i]);
			}

			imgSkyPalette.Source = BitmapSource.Create(
				1,
				256,
				96.0,
				96.0,
				PixelFormats.Pbgra32,
				null,
				skyPalettePbgra32,
				4);

			AquaLib.aqua_get_frame_size(_context, out int width, out int height);

			byte[] imageBuffer = new byte[width * height];

			bool showLights = (tbRenderLights.IsChecked ?? false) && (_lightMap != IntPtr.Zero) && (_lightMapBuffer.Length == imageBuffer.Length);

			if (!showLights)
				AquaLib.aqua_get_frame(_context, imageBuffer);
			else
			{
				AquaLib.aqua_light_map_render(_lightMap, _context);

				AquaLib.aqua_light_map_get_light_brightness(_lightMap, out int numLights, null);
				byte[] lightBrightness = new byte[numLights];
				AquaLib.aqua_light_map_get_light_brightness(_lightMap, out numLights, lightBrightness);

				for (int i = 0; i < _lightMapBuffer.Length; i++)
				{
					int lightIndex = _lightMapBuffer[i];

					if (lightIndex < 0)
						imageBuffer[i] = 0;
					else
						imageBuffer[i] = lightBrightness[lightIndex];
				}
			}

			int[] imageBufferColour = new int[width * height];

			for (int i = 0; i < imageBufferColour.Length; i++)
				imageBufferColour[i] = skyPalettePbgra32[imageBuffer[i]];

			imgDisplay.Source = BitmapSource.Create(
				width,
				height,
				96.0,
				96.0,
				PixelFormats.Pbgra32,
				null,
				imageBufferColour,
				width * 4);

			if (tbCreateVideo.IsChecked ?? false)
			{
				for (int i = 0; i < imageBufferColour.Length; i++)
					_videoWriter.Write(imageBufferColour[i]);

				_videoWriter.Flush();
			}
		}

		private int ToPbgra32(Color colour)
		{
			float[] channels = new float[] { colour.ScR, colour.ScG, colour.ScB, colour.ScA };

			channels[0] *= channels[3];
			channels[1] *= channels[3];
			channels[2] *= channels[3];

			int accumulator = 0;

			for (int i = 0; i < 4; i++)
			{
				int channelInt = (int)Math.Floor(256.0f * channels[i]);

				if (channelInt < 0)
					channelInt = 0;
				if (channelInt > 255)
					channelInt = 255;

				channelInt <<= (i * 8);

				accumulator |= channelInt;
			}

			return accumulator;
		}

		Random _rnd = new Random();

		private void tbAddSource_Click(object sender, RoutedEventArgs e)
		{
			if (tbAddSource.IsChecked ?? false)
			{
				if (!Keyboard.Modifiers.HasFlag(ModifierKeys.Control))
				{
					AquaLib.aqua_add_random_source(_context);
					tbAddSource.IsChecked = false;
				}
			}
		}

		List<AquaPoint> _lights = new List<AquaPoint>();
		IntPtr _lightMap;
		int[] _lightMapBuffer;

		private void tbConfigureLights_Click(object sender, RoutedEventArgs e)
		{
			if (tbConfigureLights.IsChecked ?? false)
			{
				if (_context == IntPtr.Zero)
				{
					tbConfigureLights.IsChecked = false;
					return;
				}

				if (_lightMap != IntPtr.Zero)
					AquaLib.aqua_free_light_map(_lightMap);

				_lights.Clear();
				_lightMap = IntPtr.Zero;
				_lightMapBuffer = null;
				pLights.Data = new PathGeometry();
				imgLightMap.Source = null;

				if (Keyboard.Modifiers.HasFlag(ModifierKeys.Control))
				{
					const int NumLights = 300;

					AquaLib.aqua_get_frame_size(_context, out int width, out int height);

					double totalArea = width * height * 2;
					double areaPerLight = totalArea / NumLights;

					double squareSide = Math.Sqrt(areaPerLight);

					double x = squareSide * 0.25;
					double y = squareSide * 0.5;

					double minX = squareSide * 0.25;
					double maxX = width - squareSide * 0.25;
					int dx = 1;

					double xScale = imgDisplay.ActualWidth / width;
					double yScale = imgDisplay.ActualHeight / height;

					for (int i = 0; i < NumLights; i++)
					{
						var scaledPosition = new AquaPoint(x, y);
						var physicalPosition = new Point(x * xScale, y * yScale);

						AddLight(physicalPosition, scaledPosition);

						x += dx * (squareSide * 0.5);

						if ((dx > 0) && (x > maxX))
						{
							x = 2 * maxX - x;
							y += squareSide;
							dx = -1;
						}
						else if ((dx < 0) && (x < minX))
						{
							x = 2 * minX - x;
							y += squareSide;
							dx = +1;
						}
					}

					tbConfigureLights.IsChecked = false;
				}
			}

			if (!(tbConfigureLights.IsChecked ?? false))
			{
				if (_lights.Count > 0)
				{
					try
					{
						_lightMap = AquaLib.aqua_generate_light_map(_context, _lights.Count, _lights.ToArray());

						AquaLib.aqua_light_map_get_light_for_pixel(_lightMap, out int width, out int height, null);
						_lightMapBuffer = new int[width * height];
						AquaLib.aqua_light_map_get_light_for_pixel(_lightMap, out width, out height, _lightMapBuffer);

						AquaLib.aqua_light_map_get_light_pixel_count(_lightMap, out int numLights, null);

						int[] randomColor = new int[numLights];

						int opaque = unchecked((int)0xFF000000);

						for (int i = 0; i < numLights; i++)
							randomColor[i] = opaque | _rnd.Next(0, 16777216);

						int[] imageBuffer = new int[width * height];

						for (int i = 0; i < imageBuffer.Length; i++)
							if (_lightMapBuffer[i] >= 0)
								imageBuffer[i] = randomColor[_lightMapBuffer[i]];

						imgLightMap.Source = BitmapSource.Create(
							width,
							height,
							96.0,
							96.0,
							PixelFormats.Pbgra32,
							null,
							imageBuffer,
							width * 4);
					}
					catch
					{
						if (_lightMap != IntPtr.Zero)
							AquaLib.aqua_free_light_map(_lightMap);

						_lightMap = IntPtr.Zero;

						throw;
					}
				}
			}
		}

		private void imgDisplay_MouseDown(object sender, MouseButtonEventArgs e)
		{
			if (tbConfigureLights.IsChecked ?? false)
			{
				AquaLib.aqua_get_frame_size(_context, out int width, out int height);

				var physicalPosition = e.GetPosition(imgDisplay);

				var scaledPosition = new AquaPoint(
					physicalPosition.X * width / imgDisplay.ActualWidth,
					physicalPosition.Y * height / imgDisplay.ActualHeight);

				AddLight(physicalPosition, scaledPosition);
			}
		}

		void AddLight(Point physicalPosition, AquaPoint scaledPosition)
		{
			_lights.Add(scaledPosition);

			var geometry = pLights.Data as PathGeometry;

			var figure = new PathFigure();

			figure.StartPoint = physicalPosition;
			figure.Segments.Add(new LineSegment(physicalPosition, isStroked: true));

			geometry.Figures.Add(figure);
		}

		private void tbShowLights_Click(object sender, RoutedEventArgs e)
		{
			pLights.Visibility = (tbShowLights.IsChecked ?? false) ? Visibility.Visible : Visibility.Collapsed;
		}

		private void tbRenderLights_Click(object sender, RoutedEventArgs e)
		{
			imgLightMap.Visibility = (tbRenderLights.IsChecked ?? false) ? Visibility.Hidden : Visibility.Visible;
		}

		private void tbCreateVideo_Click(object sender, RoutedEventArgs e)
		{
			if ((tbCreateVideo.IsChecked ?? false) && (_videoWriter == null))
				_videoWriter = new BinaryWriter(File.OpenWrite("video.bin"));
		}
	}
}
