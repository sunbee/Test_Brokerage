#ifndef SENSOR_ARRAY_H
#define SENSOR_ARRAY_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#include <Adafruit_MCP3008.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2591.h>


class SensorArray 
{
    public:
        SensorArray();

        void start_display();
        void displayMessage(String="Namaste ji!");

        void start_mcp();
        int get_mcp_waterLevel(uint8_t=0, bool=true);

        void start_tsl();
        uint16_t get_tsl_visibleLight(bool=false);
        uint16_t get_tsl_IR(bool=false);
        uint16_t get_tsl_fullSpectrum(bool=false);
        uint32_t get_tsl_luminosity(bool=true);
        float get_tsl_lux(bool=true);
    private:
        Adafruit_SSD1306 _display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

        int _last_waterLevel;
        Adafruit_MCP3008 _mcp;

        uint32_t _last_tsl_luminosity;
        Adafruit_TSL2591 _tsl = Adafruit_TSL2591(2591);

};
#endif

/*
Obtain analog values from sensors via MCP3008 10-bit analog-to-digital
convertor (ADC). The connections must be as follows:
MCP <> ESP8266
15 <> 3.3V
14 <> 3.3V
13 <> GND
12 <> D5 (CLK)
11 <> D6 (MISO)
10 <> D7 (MOSI)
9  <> D8 (CS)
8  <> GND

Pin numbering on the MCP3008 is 0-15 with 2 rows of 8 pins on either side 
is as follows: 
a.) Pins numbered 0-7 in one row are analog input channels.
b.) Pins numbered 8-15 in the row across are power and communications bus.
c.) Pins 0 and 15 are at the notch on either side. 
*/

/*
Display a message received by subscriber to OLED.
This section has the Adafruit libraries and setup instructions
in order to use the OLED display.
*/
