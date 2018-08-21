// Adafruit NeoPixel library port to the rpi_ws281x library.
// Author: Tony DiCola (tony@tonydicola.com), Jeremy Garff (jer@jers.net)
// Port to C++ by logic <logic@deltaq.org>

#include <cstdint>

#include "ws2811.h"

int LEDColour(int red, int green, int blue);

class Adafruit_NeoPixel
{
  ws2811_t _leds;
  ws2811_channel_t *_channel;
public:
  Adafruit_NeoPixel(int num, int pin, int freq_hz = 800000, int dma = 10, bool invert = false, int brightness = 255, int channel = 0, int strip_type = WS2811_STRIP_RGB);

  void show();
  ws2811_channel_t &channel();
};
