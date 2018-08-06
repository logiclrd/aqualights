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

			AquaLib.aqua_get_frame(_context, imageBuffer);

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

		private void tbLights_Click(object sender, RoutedEventArgs e)
		{
			if (tbLights.IsChecked ?? false)
			{
				if (_context == IntPtr.Zero)
				{
					tbLights.IsChecked = false;
					return;
				}

				_lights.Clear();
				pLights.Data = new PathGeometry();
				imgLightMap.Source = null;
			}
			else
			{
				if (_lights.Count > 0)
				{
					var lightMap = IntPtr.Zero;

					try
					{
						lightMap = AquaLib.aqua_generate_light_map(_context, _lights.Count, _lights.ToArray());

						AquaLib.aqua_light_map_get_light_for_pixel(lightMap, out int width, out int height, null);
						var mapBuffer = new short[width * height];
						AquaLib.aqua_light_map_get_light_for_pixel(lightMap, out width, out height, mapBuffer);

						AquaLib.aqua_light_map_get_light_pixel_count(lightMap, out int numLights, null);

						int[] randomColor = new int[numLights];

						int opaque = unchecked((int)0xFF000000);

						for (int i = 0; i < numLights; i++)
							randomColor[i] = opaque | _rnd.Next(0, 16777216);

						int[] imageBuffer = new int[width * height];

						for (int i = 0; i < imageBuffer.Length; i++)
							if (mapBuffer[i] >= 0)
								imageBuffer[i] = randomColor[mapBuffer[i]];

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
					finally
					{
						if (lightMap != IntPtr.Zero)
							AquaLib.aqua_free_light_map(lightMap);
					}
				}
			}
		}

		private void imgDisplay_MouseDown(object sender, MouseButtonEventArgs e)
		{
			if (tbLights.IsChecked ?? false)
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
