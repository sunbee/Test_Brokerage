#ifndef SENSOR_ARRAY_H
#define SENSOR_ARRAY_H

/*
For each sensor or device in the array,
1. Include the library (e.g. Adafruit's)
2. Create an instance of the sensor class
3. Declare the start_me method prototype
4. Declare the get method prototype
5. Declare an optional last value.
Then implement the prototypes in the cpp file,
accessing the instance with this->me.
*/

#include "PinNumbers.h"
#include <DHT.h>

#define DHT_TYPE DHT22

#include <DallasTemperature.h>
#include "Temperature.h"

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

        void start_dht22();
        float _last_dht_RH;
        float _last_dht_temperature;
        float get_dht_humidity(bool=true);
        float get_dht_temperature(enum temperatureUnit, bool=true);

        void start_ds18b20(uint8_t);
        float get_ds18b20_temperature(enum temperatureUnit);

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
        DHT _dht = DHT(PIN_DHT22, DHT_TYPE);

        Temperature _temperature;
        OneWire _oneWire = OneWire(0);
        DallasTemperature _ds18b20 = DallasTemperature();

        Adafruit_SSD1306 _display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

        int _last_waterLevel;
        Adafruit_MCP3008 _mcp;

        uint32_t _last_tsl_luminosity;
        Adafruit_TSL2591 _tsl = Adafruit_TSL2591(2591);
};
#endif

/*
Obtain sensor readings to publish to MQTT broker.
This section has the instructions to use the the MCP3008 
10-bit analog-to-digital converter for adding upto 
8 analog sensors to a nodemcu, circumventng the limitation 
of a single analog pin. We have retained the potentiometer 
wired to A0 for test comparison.
*/

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

/*
Obtain temperature readings from the ds18b20 "Dallas" temperature sensor
over the one-wire protocol. This protocol requires only a digital pin and
we have used pin D3 (GPIO0) on the nodemcu.
*/

/*
Obtain temperature and relative humidity from the DHT22 sensor 
over the one-wire protocol. This is a finicky sensor and may return 
nan-values until ready. Provision is made to report -99.99 
when this happens. 
*/