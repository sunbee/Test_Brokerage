#include "Temperature.h"

float Temperature::convertCtoF(float c) 
{
  return (c * 1.8) + 32.0;
}

float Temperature::convertFtoC(float f)
{
  return (f - 32.0) / 1.8;
}