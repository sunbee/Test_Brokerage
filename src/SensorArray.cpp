/*
Copyright of Sanjay R. Bhatikar
*/
#include "SensorArray.h"

SensorArray::SensorArray() {

};

void SensorArray::start_ds18b20(uint8_t temperaturePin) {
    this->_oneWire.begin(temperaturePin);
    this->_ds18b20.setOneWire(&this->_oneWire);
    this->_ds18b20.begin();
}

float SensorArray::get_ds18b20_temperature(enum temperatureUnit unit) {
    float returnValue = 0.0;

    this->_ds18b20.requestTemperatures();
    returnValue = this->_ds18b20.getTempCByIndex(0);
    if (unit == FAHRENHEIT) {
        returnValue = this->_temperature.convertCtoF(returnValue);
    }
    return returnValue;
}

void SensorArray::start_display() {
    _display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    _display.clearDisplay();

}

void SensorArray::displayMessage(String mess) {
  /*
  Use in the callback for MQTT subscriber.
  The callback is 'mosquittoDo()'.
  */
  this->_display.clearDisplay();
  this->_display.setTextSize(1.5);
  this->_display.setTextColor(WHITE);
  this->_display.setCursor(0,0);
  this->_display.println(mess);
  this->_display.display();
}

void SensorArray::start_mcp() {
    this->_mcp.begin();
}

int SensorArray::get_mcp_waterLevel(uint8_t channel_waterLevel, bool liveReading) {
    if (liveReading) {
        this->_last_waterLevel = this->_mcp.readADC(channel_waterLevel);
    }
    return this->_last_waterLevel;
}

void SensorArray::start_tsl() {
    if (this->_tsl.begin()) {
        Serial.println("Sensor armed!");
    } else {
        Serial.println("Found no lux sensor!");
    };

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
    Ignore until we have figured out how to derive this result
    from 32-bit full luminosity. Per documentation, the 32-bit
    full luminosity has low word for IR and high word for full 
    spectrum. This would indicate shifting or masking bits to 
    compute 16-bit IR, Full or Visible spectrum result. 
    */
   if (liveReading) {
       this->_last_tsl_luminosity = this->_tsl.getFullLuminosity();
   }
   return this->_tsl.getLuminosity(TSL2591_INFRARED);
}

uint16_t SensorArray::get_tsl_fullSpectrum(bool liveReading) {
    /*
    Take a live reading, otherwise report last read value.
    Ignore until we have figured out how to derive this result
    from 32-bit full luminosity.
    */
   if (liveReading) {
       this->_last_tsl_luminosity = this->_tsl.getFullLuminosity();
   }
   return this->_tsl.getLuminosity(TSL2591_FULLSPECTRUM);
}

uint16_t SensorArray::get_tsl_visibleLight(bool liveReading) {
    /*
    Take a live reading, otherwise report last read value.
    Ignore until we have figured out how to derive this result
    from 32-bit full luminosity.
    */
   if (liveReading) {
       this->get_tsl_fullSpectrum(liveReading) - this->get_tsl_IR(liveReading);
   }
    return this->_tsl.getLuminosity(TSL2591_VISIBLE);
}

float SensorArray::get_tsl_lux(bool liveReading) {
    if (liveReading) {
        this->_last_tsl_luminosity = this->_tsl.getFullLuminosity();
    }
    return this->_tsl.calculateLux(this->get_tsl_fullSpectrum(liveReading), this->get_tsl_IR(liveReading));
}
