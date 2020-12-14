#include "neopixel.h"
#include "aqua.h"

#include "ws2811.h"

#include <signal.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

// LED strip configuration:
#define LED_COUNT      300 /* Number of LED pixels. */
#define LED_PIN        18  /* GPIO pin connected to the pixels (18 uses PWM!). */
#define LED_FREQ_HZ    800000  /* Serial communicatior frequency in hertz */
#define LED_DMA        10  /* DMA channel to use for generating signal (try 10) */
#define LED_INVERT     false   /* True to invert the signal (when using NPN transistor level shift) */
#define LED_BRIGHTNESS 20 /* Set to 0 for darkest and 255 for brightest */
#define LED_CHANNEL    0   /* set to '1' for GPIOs 13, 19, 41, 45 or 53 */

// Turns out the voltage fades when the LED strip is at max brightness. Try to
// make the whole thing (255, 255, 255) and the LEDs are orange-red by the end
// of the strip. So, we scale the brightness down so that even at its brightest,
// the fade is avoided. Empirically, the fade is just starting to become apparent
// when the brightness hits 100. None of our palettes are all white, so a 50%
// fade, which gives a theoretical maximum of 128, is probably a safe bet most
// of the time.

#define PALETTE_NUMERATOR 1
#define PALETTE_DENOMINATOR 2

time_t last_source_added_time = 0;

bool should_add_source()
{
  int current_time = time(NULL);

  if (last_source_added_time == current_time)
    return false;

  last_source_added_time = current_time;

  return true;
}

void advance(AquaContext *context)
{
  if (should_add_source())
    aqua_add_random_source(context);

  aqua_update_ripple(context);
  aqua_update_sources(context);

  aqua_advance_sky(context);
}

unsigned char _light_brightness[LED_COUNT];
AquaColour _light_palette[256];
ws2811_led_t _led_palette[256];

void show_lights(AquaContext *context, AquaLightMap *light_map, Adafruit_NeoPixel *leds)
{
  aqua_get_current_sky_palette(context, &_light_palette[0]);

  #define P(x) ((x) * PALETTE_NUMERATOR / PALETTE_DENOMINATOR)

  for (int i=0; i < 256; i++)
    _led_palette[i] = LEDColour(P(_light_palette[i].r), P(_light_palette[i].g), P(_light_palette[i].b));

  #undef P

  aqua_light_map_render(light_map, context);

  int num_lights = LED_COUNT;

  aqua_light_map_get_light_brightness(light_map, &num_lights, &_light_brightness[0]);

  if (num_lights != LED_COUNT)
    throw "Unexpected number of lights from aqua_light_map_get_light_brightness";

  ws2811_channel_t &channel = leds->channel();

  for (int i=0; i < LED_COUNT; i++)
    channel.leds[i] = _led_palette[_light_brightness[i]];

  leds->show();
}

void *generate_light_positions(AquaContext *context, AquaPoint *light_positions, int num_lights)
{
  int width, height;

  aqua_get_frame_size(context, &width, &height);

  double cell_width = width / 24.0;
  double cell_height = height / 12.0;

  int index = 0;

  for (int row = 0; row < 12; row++)
  {
    int even_row = (row & 1) == 0;

    double x = even_row ? cell_width * 0.5 : width - cell_width * 0.5;
    double y = (row + 0.5) * cell_height;

    double dx = even_row ? cell_width : -cell_width;

    for (int column = 0; column < 25; column++)
    {
      light_positions[index++] = AquaPoint((float)x, (float)y);

      x += dx;

      if (x < 0)
        x = 0;
      if (x > width)
        x = width;
    }
  }
}

struct LoopStatistics
{
  struct timespec Before, After;
  int IterationCount;

  void Start()
  {
    clock_gettime(CLOCK_MONOTONIC, &Before);
    IterationCount = 0;
  }

  void Count()
  {
    IterationCount++;
  }

  void Finish()
  {
    clock_gettime(CLOCK_MONOTONIC, &After);
  }

  void PrintReport()
  {
    double time_before = Before.tv_sec + Before.tv_nsec * 0.000000001;
    double time_after = After.tv_sec + After.tv_nsec * 0.000000001;

    double time_elapsed = time_after - time_before;

    printf("Elapsed: %.3f seconds\n", time_elapsed);
    printf("Iterations per second: %.3f\n", IterationCount / time_elapsed);
  }
};

LoopStatistics _loop_statistics;
Adafruit_NeoPixel *_leds = NULL;
AquaContext *_context = NULL;

void clear_leds_atexit()
{
  if (_leds == NULL)
  {
    printf("=> No LED context\n");
    return;
  }

  ws2811_channel_t &channel = _leds->channel();

  for (int i=0; i < LED_COUNT; i++)
    channel.leds[i] = 0;

  printf("=> Sending (0,0,0) to LEDs\n");

  _leds->show();
}

void perform_atexit()
{
  printf("Running atexit...\n");

  clear_leds_atexit();

  _loop_statistics.Finish();
  _loop_statistics.PrintReport();

  if (_context != NULL)
    aqua_free(_context);
}

void install_atexit()
{
  atexit(perform_atexit);

  struct sigaction int_action = { 0 };

  int_action.sa_handler = exit;

  sigaction(SIGINT, &int_action, NULL);
}

int main()
{
  srand(time(NULL));

  _context = aqua_initialize(100, 75, 20, 10 /* frames per second */ * 3600 /* seconds per hour */ * 4 /* hours */);

  AquaPoint light_positions[LED_COUNT];

  generate_light_positions(_context, light_positions, LED_COUNT);

  AquaLightMap *light_map = aqua_generate_light_map(_context, LED_COUNT, light_positions);

  _leds = new Adafruit_NeoPixel(LED_COUNT, LED_PIN, LED_FREQ_HZ, LED_DMA, LED_INVERT, LED_BRIGHTNESS, LED_CHANNEL);

  install_atexit();

  _loop_statistics.Start();

  while (true)
  {
    advance(_context);
    show_lights(_context, light_map, _leds);

    _loop_statistics.Count();
  }
}

