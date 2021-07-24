/*
Copyright of Sanjay R. Bhatikar
*/
#include "SensorArray.h"

SensorArray::SensorArray() {

};

void SensorArray::start_tsl() {
    /*
    Configure the gain for the TSL2591 sensor:
    TSL2591_GAIN_LOW: Sets the gain to 1x (bright light)
    TSL2591_GAIN_MED: Sets the gain to 25x (general purpose)
    TSL2591_GAIN_HIGH: Sets the gain to 428x (low light)
    TSL2591_GAIN_MAX: Sets the gain to 9876x (extremely low light)
    */
    this->_tsl.setGain(TSL2591_GAIN_MED);

    /*
    Configure the integration time for TSL2591 sensor:
    TSL2591_INTEGRATIONTIME_100MS
    TSL2591_INTEGRATIONTIME_200MS
    TSL2591_INTEGRATIONTIME_300MS
    TSL2591_INTEGRATIONTIME_400MS
    TSL2591_INTEGRATIONTIME_500MS
    TSL2591_INTEGRATIONTIME_600MS
    */
    this->_tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);

    this->_tsl.begin();
};

uint32_t SensorArray::get_tsl_luminosity(bool liveReading) {
    /*
    Take a live reading, otherwise report last read value.
    */
   if (liveReading) {
       this->_last_tsl_luminosity = this->_tsl.getFullLuminosity();
   }
   return this->_last_tsl_luminosity;

}

uint16_t SensorArray::get_tsl_IR(bool liveReading) {
    /*
    Take a live reading, otherwise report last read value.
    */
   if (liveReading) {
       this->_last_tsl_luminosity = this->_tsl.getFullLuminosity();
   }
   return this->_last_tsl_luminosity >> 16;
}

uint16_t SensorArray::get_tsl_fullSpectrum(bool liveReading) {
    /*
    Take a live reading, otherwise report last read value.
    */
   if (liveReading) {
       this->_last_tsl_luminosity = this->_tsl.getFullLuminosity();
   }
   return this->_last_tsl_luminosity & 0xFFFF;
}

uint16_t SensorArray::get_tsl_visibleLight(bool liveReading) {
    /*
    Take a live reading, otherwise report last read value.
    */
    return this->get_tsl_fullSpectrum(liveReading) - this->get_tsl_IR(liveReading);
}


