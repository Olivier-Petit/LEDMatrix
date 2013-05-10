/*
 * 16 * 16 LED matrix
 * Created : may 2012
 * op414
 * http://op414.net
 * License : CC BY-NC-SA http://creativecommons.org/licenses/by-nc-sa/3.0/
 * ---------
 * SettingsHandler.cpp : Implements the SettingsHandler class, which is used to save and read settings
 */

#if defined(ARDUINO) && ARDUINO >= 100
  #include <Arduino.h>
#else
  #include "WProgram.h"
#endif

#include <EEPROM.h>

#include "SettingsHandler.h"

#include "settings.h"
#include "Display.h"
#include "InputHandler.h"

// Constructor
SettingsHandler::SettingsHandler(Display *disp, InputHandler *inputs)
{
  m_disp = disp;
  m_inputs = inputs;
  
  for(int i = 0 ; i++ ; i < 5)
  {
    m_settings[i] = 0;
  }
  
  /*EEPROM.write(0, 30);
  EEPROM.write(1, 30);
  EEPROM.write(2, 30);
  EEPROM.write(3, 30);
  EEPROM.write(4, B00000000);*/
}

// Used to get the settings from the EEPROM
void SettingsHandler::read()
{
  for(int i = 0 ; i < 5 ; i++)
  {
    m_settings[i] = EEPROM.read(i);
  }
}

// Used to read the mode durations
void SettingsHandler::getModeDurations(unsigned int modeDurations[])
{
  for(int i = 0 ; i < 4 ; i++)
  {
    modeDurations[i] = (unsigned int)(m_settings[i] * 1000UL);
  }
}

// Used to get the booleans settings
boolean SettingsHandler::getBooleanSetting(int setting)
{
  return m_settings[4] & (B00000001 << setting) ? true : false;
}

// Used to write the settings to the EEPROM memory
void SettingsHandler::save()
{
  for(int i = 0 ; i < 5 ; i++)
  {
    EEPROM.write(i, m_settings[i]);
  }
}

