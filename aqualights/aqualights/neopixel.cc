// Adafruit NeoPixel library port to the rpi_ws281x library.
// Author: Tony DiCola (tony@tonydicola.com), Jeremy Garff (jer@jers.net)
// Port to C++ by logic <logic@deltaq.org>

#include <cstdint>

#include "ws2811.h"

#include <iostream>
#include <cstring>
#include <sstream>

#include "neopixel.h"

using namespace std;

int LEDColour(int red, int green, int blue)
{
  return (red << 16) | (green << 8) | blue;
}

Adafruit_NeoPixel::Adafruit_NeoPixel(int num, int pin, int freq_hz, int dma, bool invert, int brightness, int channel, int strip_type)
{
  // Create ws2811_t structure and fill in parameters.
  memset(&_leds, 0, sizeof(_leds));

  // Initialize the channels to zero
  for (int channum = 0; channum < 2; channum++)
  {
    ws2811_channel_t &chan = _leds.channel[channum];

    chan.count = 0;
    chan.gpionum = 0;
    chan.invert = 0;
    chan.brightness = 0;
  }

  // Initialize the channel in use
  _channel = &_leds.channel[channel];
  _channel->count = num;
  _channel->gpionum = pin;
  _channel->invert = invert ? 1 : 0;
  _channel->brightness = brightness;
  _channel->strip_type = strip_type;

  // Initialize the controller
  _leds.freq = freq_hz;
  _leds.dmanum = dma;

  // Initialize library, must be called once before other functions are called.
  ws2811_return_t resp = ws2811_init(&_leds);

  if (resp != WS2811_SUCCESS)
  {
    const char *message = ws2811_get_return_t_str(resp);

    stringstream buffer;

    buffer << "ws2811_init failed with code " << resp << " (" << message << ")";

    throw buffer.str().c_str();
  }
}

void Adafruit_NeoPixel::show()
{
  ws2811_return_t resp = ws2811_render(&_leds);

  if (resp != WS2811_SUCCESS)
  {
    const char *message = ws2811_get_return_t_str(resp);

    stringstream buffer;

    buffer << "ws2811_render failed with code " << resp << " (" << message << ")";

    throw buffer.str().c_str();
  }
}

ws2811_channel_t &Adafruit_NeoPixel::channel()
{
  return *_channel;
}
