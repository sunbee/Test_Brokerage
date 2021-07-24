#ifndef TEMPERATURE_H
#define TEMPERATURE_H

enum temperatureUnit {
    FAHRENHEIT,
    CELSIUS
};

class Temperature {
    public:
        float convertCtoF(float c);
        float convertFtoC(float f);
};
#endif