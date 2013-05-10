/*
 * 16 * 16 LED matrix
 * april 2012
 * op414
 * http://op414.net
 * License : CC BY-NC-SA http://creativecommons.org/licenses/by-nc-sa/3.0/
 * ---------
 * TempSensor.cpp : Implements the TempSensor class which is used to interract with the DS18B20 temperature sensor.
 */
 
#if defined(ARDUINO) && ARDUINO >= 100
  #include <Arduino.h>
#else
  #include "WProgram.h"
#endif

#include <OneWire.h>

#include "TempSensor.h"
#include "Display.h"
#include "settings.h"

// Constructor
TempSensor::TempSensor(Display *disp) : m_sensor(PIN_TEMP)
{
  m_disp = disp;
  m_tempWhole = 0;
  m_tempFract = 0;
  m_conversionAsked = false;
}

// Used to ask the sensor to start computing the temperature
void TempSensor::startConversion()
{
  m_sensor.reset(); // Reset bus
  m_sensor.skip(); // Skip the device selection (only one sensor is on the bus)
  m_sensor.write(DS18B20_START_CONVERSION); // Ask to start conversion
  
  m_conversionStartTime = millis();
}
  
// Used to update the temp
boolean TempSensor::updateTemp()
{
  if(millis() - m_conversionStartTime >= TEMP_CHECK_INTERVAL || m_conversionStartTime == 0) // Begin a new conversion
  {
    startConversion();
    m_conversionStartTime = millis();
    m_conversionAsked = true;
    
    return false; // Temp did not changed
  }
  else if(millis() - m_conversionStartTime >= TEMP_CONVERSION_DELAY && m_conversionAsked == true) // Read the temp
  {
    byte data[9];
    
    m_sensor.reset();
    m_sensor.skip();
    m_sensor.write(DS18B20_READ_SCRATCHPAD); // Ask to read scratchpad
    
    for(int i = 0 ; i < 9 ; i++) // Read 9 bytes
      data[i] = m_sensor.read();
    
    int lowByte = data[0];
    int highByte = data[1];
    
    int tReading = (highByte << 8) + lowByte; // Make a twelve bit int
    int signBit = tReading & 0x8000; // Test the most significant bit
    
    if(signBit) // Negative
      tReading = (tReading ^ 0xFFFF) + 1; // 2's comp
      
    int tc100 = (6 * tReading) + tReading / 4; // Multiply by (100 * 0.0625) or 6.25
    
    m_tempWhole = tc100 / 100; // Get the whole portion
    m_tempFract = tc100 % 100; // Get the fractionnal portion
    
    m_conversionAsked = false; // We do not want to read the scratchpad before asking for a new conversion
    
    return true; // Temp changed
  }
  
  return false; // Temp did no changed
}

// Used to display the temp
void TempSensor::displayTemp()
{
  m_disp->setDigit(1, 10, m_tempWhole / 10); // First digit
  m_disp->setDigit(5, 10, m_tempWhole - (m_tempWhole / 10) * 10); // Second digit
  m_disp->setLed(9, 14, true); // Dot
  m_disp->setDigit(11, 10, m_tempFract / 10); // fractionnal portion
  
  // Degre Symbol
  m_disp->setLed(10, 1, true);
  
  // C
  m_disp->setLed(12, 1, true); m_disp->setLed(13, 1, true); m_disp->setLed(14, 1, true);
  m_disp->setLed(12, 2, true);
  m_disp->setLed(12, 3, true);
  m_disp->setLed(12, 4, true);
  m_disp->setLed(12, 5, true); m_disp->setLed(13, 5, true); m_disp->setLed(14, 5, true);
}

// ---------- DEBUGING FUNCTIONS --------

// Used to print the temp
#ifdef DEBUG
void TempSensor::printTemp()
{
  Serial.println(String(m_tempWhole) + "." + ((m_tempFract < 10) ? "0" : "") + String(m_tempFract));
}
#endif
