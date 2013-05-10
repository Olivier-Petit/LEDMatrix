/*
 * 16 * 16 LED matrix
 * april 2012
 * op414
 * http://op414.net
 * License : CC BY-NC-SA http://creativecommons.org/licenses/by-nc-sa/3.0/
 * ---------
 * TempSensor.cpp : TempSensor class definition.
 */

#ifndef DEF_TEMPSENSOR
#define DEF_TEMPSENSOR

#include <OneWire.h>

#include "Display.h"

#define DS18B20_START_CONVERSION 0x44
#define DS18B20_READ_SCRATCHPAD 0xBE

#define TEMP_CONVERSION_DELAY 1000UL
#define TEMP_CHECK_INTERVAL 5000UL

class TempSensor
{
  public:
    TempSensor(Display *disp);
    boolean updateTemp();
    void displayTemp();
    
    #ifdef DEBUG
      void printTemp();
    #endif
  
  private:
    int m_tempWhole;
    int m_tempFract;
    
    Display *m_disp;
    OneWire m_sensor;
    boolean m_conversionAsked;
    unsigned long m_conversionStartTime;
    
    void startConversion();
};

#endif
