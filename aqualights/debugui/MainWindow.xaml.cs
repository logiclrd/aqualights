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
			_context = AquaLib.aqua_initialize(100, 75, 10);
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
			AquaLib.aqua_get_buffer_size(_context, out int width, out int height);

			int direction = _rnd.Next(2);
			int delta = 1 - direction * 2;

			var source = new AquaSource();

			source.X = direction * width;
			source.Y = _rnd.Next(height);
			source.DX = delta;
			source.DY = (float)(_rnd.NextDouble() - 0.5) * 0.1f;
			source.Radius = (float)(_rnd.NextDouble() + 1);
			source.Intensity = (float)_rnd.NextDouble() * 0.75f + 0.25f;
			source.Duration = _rnd.Next(width / 5, width);
			source.X = source.X + delta * _rnd.Next(0, width - source.Duration);

			AquaLib.aqua_add_source(_context, ref source);
		}
	}
}
