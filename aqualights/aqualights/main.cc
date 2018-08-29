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
#define LED_BRIGHTNESS 255 /* Set to 0 for darkest and 255 for brightest */
#define LED_CHANNEL    0   /* set to '1' for GPIOs 13, 19, 41, 45 or 53 */

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

  for (int i=0; i < 256; i++)
    _led_palette[i] = LEDColour(_light_palette[i].r, _light_palette[i].g, _light_palette[i].b);

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

void *generate_dummy_light_positions(AquaContext *context, AquaPoint *light_positions, int num_lights)
{
  int width, height;

  aqua_get_frame_size(context, &width, &height);

  double total_area = width * height * 2;
  double area_per_light = total_area / num_lights;

  double square_side = sqrt(area_per_light);

  double x = square_side * 0.25;
  double y = square_side * 0.5;

  double min_x = square_side * 0.25;
  double max_x = width - square_side * 0.25;
  int dx = 1;

  for (int i = 0; i < num_lights; i++)
  {
    light_positions[i] = AquaPoint((float)x, (float)y);

    x += dx * (square_side * 0.5);

    if ((dx > 0) && (x > max_x))
    {
      x = 2 * max_x - x;
      y += square_side;
      dx = -1;
    }
    else if ((dx < 0) && (x < min_x))
    {
      x = 2 * min_x - x;
      y += square_side;
      dx = +1;
    }
  }
}

Adafruit_NeoPixel *_leds = NULL;

void clear_leds_atexit()
{
  printf("Running atexit...\n");
 
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

void install_atexit()
{
  atexit(clear_leds_atexit);

  struct sigaction int_action = { 0 };

  int_action.sa_handler = exit;

  sigaction(SIGINT, &int_action, NULL);
}

int main()
{
  srand(time(NULL));

  AquaContext *context = aqua_initialize(100, 75, 20, 10 /* frames per second */ * 3600 /* seconds per hour */ * 4 /* hours */);

  AquaPoint light_positions[LED_COUNT];

  generate_dummy_light_positions(context, light_positions, LED_COUNT);

  AquaLightMap *light_map = aqua_generate_light_map(context, LED_COUNT, light_positions);

  _leds = new Adafruit_NeoPixel(LED_COUNT, LED_PIN, LED_FREQ_HZ, LED_DMA, LED_INVERT, LED_BRIGHTNESS, LED_CHANNEL);

  install_atexit();

  struct timespec before, after;

  clock_gettime(CLOCK_MONOTONIC, &before);

#define MAX_ITERS 80 /* FPS */ * 15 /* seconds */

  for (int i=0; i < MAX_ITERS; i++)
  {
    advance(context);
    show_lights(context, light_map, _leds);
  }

  clock_gettime(CLOCK_MONOTONIC, &after);

  double time_before = before.tv_sec + before.tv_nsec * 0.000000001;
  double time_after = after.tv_sec + after.tv_nsec * 0.000000001;

  double time_elapsed = time_after - time_before;

  printf("Elapsed: %.3f seconds\n", time_elapsed);
  printf("Iterations per second: %.3f\n", MAX_ITERS / time_elapsed);

  aqua_free(context);
}

