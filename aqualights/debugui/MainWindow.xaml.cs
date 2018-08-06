using System;
using System.Collections.Generic;
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

		private void cmdInitialize_Click(object sender, RoutedEventArgs e)
		{
			_context = AquaLib.aqua_initialize(100, 75, 20);
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

		private void tmrAdvance_Tick(object sender, EventArgs e)
		{
			AquaLib.aqua_update_ripple(_context);
			AquaLib.aqua_update_sources(_context);

			AquaLib.aqua_get_frame_size(_context, out int width, out int height);

			byte[] imageBuffer = new byte[width * height];

			bool showLights = (tbShowLights.IsChecked ?? false) && (_lightMap != IntPtr.Zero) && (_lightMapBuffer.Length == imageBuffer.Length);

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

			imgDisplay.Source = BitmapSource.Create(
				width,
				height,
				96.0,
				96.0,
				PixelFormats.Gray8,
				null,
				imageBuffer,
				width);
		}

		Random _rnd = new Random();

		private void cmdAddSource_Click(object sender, RoutedEventArgs e)
		{
			AquaLib.aqua_add_random_source(_context);
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
			}
			else
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

		private void tbShowLights_Click(object sender, RoutedEventArgs e)
		{
			imgLightMap.Visibility = (tbShowLights.IsChecked ?? false) ? Visibility.Hidden : Visibility.Visible;
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

				_lights.Add(scaledPosition);

				var geometry = pLights.Data as PathGeometry;

				var figure = new PathFigure();

				figure.StartPoint = physicalPosition;
				figure.Segments.Add(new LineSegment(physicalPosition, isStroked: true));

				geometry.Figures.Add(figure);
			}
		}
	}
}
