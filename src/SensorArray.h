#ifndef SENSOR_ARRAY_H
#define SENSOR_ARRAY_H

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2591.h>

class SensorArray 
{
    public:
        SensorArray();
        void start_tsl();
        uint16_t get_tsl_visibleLight(bool = false);
        uint16_t get_tsl_IR(bool = false);
        uint16_t get_tsl_fullSpectrum(bool = false);
        uint32_t get_tsl_luminosity(bool = true);

    private:
        uint32_t _last_tsl_luminosity;
        Adafruit_TSL2591 _tsl = Adafruit_TSL2591(2591);

};
#endif