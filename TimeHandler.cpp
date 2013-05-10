/*
 * 16 * 16 LED matrix
 * april 2012
 * op414
 * http://op414.net
 * License : CC BY-NC-SA http://creativecommons.org/licenses/by-nc-sa/3.0/
 * ---------
 * TimeHandler.cpp : Implements the TimeHandler class which is used to interract with the Chronodot RTC (DS3231)
 */

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include "WProgram.h"
#endif

#include <Wire.h>

#include "TimeHandler.h"
#include "Display.h"
#include "InputHandler.h"
#include "settings.h"

// Constructor
TimeHandler::TimeHandler(Display *disp, InputHandler *inputs)
{
  m_hours = 0U;
  m_mins = 0U;
  m_secs = 0U;
  m_DOM = 0U;
  m_DOW = 0U;
  m_month = 0U;
  m_year = 0U;

  m_binaryMode = false;
  m_lastRTCCheck = 0UL;
  m_timeAdjustment = NO;

  m_disp = disp;
  m_inputs = inputs;
}

// Used to get the time from the chronodot
void TimeHandler::getRTCTime()
{
  Wire.beginTransmission(CHRONODOT_ADDR);
  Wire.write((byte)0x00); // Start at register 0
  Wire.endTransmission();
  Wire.requestFrom(CHRONODOT_ADDR, 7); // Request seven bytes (seconds, minutes, hours, dow, dom, month, year)

  while(Wire.available()) // Wait for the data to come
  {
    // Read the data
    m_secs = (unsigned int)bcdToDec(Wire.read());
    m_mins = (unsigned int)bcdToDec(Wire.read());
    m_hours = (unsigned int)bcdToDec(Wire.read());
    m_DOW = (unsigned int)bcdToDec(Wire.read());
    m_DOM = (unsigned int)bcdToDec(Wire.read());
    m_month = (unsigned int)bcdToDec(Wire.read());
    m_year = (unsigned int)bcdToDec(Wire.read());
  }
}

// Used to update the time.
boolean TimeHandler::updateTime()
{
  if((millis() - m_lastRTCCheck) >= RTC_CHECK_INTERVAL || m_lastRTCCheck == 0)
  {
    getRTCTime();
    m_lastRTCCheck = millis();

    return true;
  }
  else
    return false;
}

// Used to initialize the RTC (Chronodot)
void TimeHandler::initializeRTC()
{
  Wire.begin(); // Initialize the Wire library

  // Clear /EOSC bit
  Wire.beginTransmission(CHRONODOT_ADDR);
  Wire.write((byte)0x0E); // Select register
  Wire.write((byte)B00011100); // Write register bitmap, bit 7 is /EOSC
  Wire.endTransmission();
}

// Used to display the current time
void TimeHandler::displayTime()
{
  if(!m_binaryMode) // Normal mode
  {
    displayHours();
    displayMinutes();

    // Seconds
    for(int i = 0 ; i < 60 ; i++)
    {
      if(i <= 15) // top
        m_disp->setLed(i, 0, (boolean) i <= m_secs);
      else if(i <= 30) // right
        m_disp->setLed(15, i - 15, (boolean) i <= m_secs);
      else if(i <= 45) // bottom
        m_disp->setLed(45 - i, 15, (boolean) i <= m_secs);
      else // left
      m_disp->setLed(0, 60 - i, (boolean) i <= m_secs);
    }

    m_disp->setLed(15, 0, true);
    m_disp->setLed(15, 15, true);
    m_disp->setLed(0, 15, true);
  }
  else // Binary mode
  {
    displayBinaryTime();
  }
}

// Used to change time display mode
void TimeHandler::changeTimeDisplayMode()
{
  m_binaryMode = !m_binaryMode;
}

// Used to display the current date (DOM, DOW, month & year)
void TimeHandler::displayDate()
{
  displayDOM();
  displayMonth();

  displayDOW();

  displayYear();
}

// Helpers functions
byte TimeHandler::decToBcd(byte val)
{
  return ((val/10*16) + (val%10));
}

byte TimeHandler::bcdToDec(byte val)
{
  return ((val/16*10) + (val%16));
}

// Used to write time to the RTC
void TimeHandler::setRTCTime()
{
  Wire.beginTransmission(CHRONODOT_ADDR);
  Wire.write((byte)0x00); // Start at register 0x00 (seconnds)

  // Write the data
  Wire.write(decToBcd((byte)m_secs));
  Wire.write(decToBcd((byte)m_mins));
  Wire.write(decToBcd((byte)m_hours));
  Wire.write(decToBcd((byte)m_DOW));
  Wire.write(decToBcd((byte)m_DOM));
  Wire.write(decToBcd((byte)m_month));
  Wire.write(decToBcd((byte)m_year));

  Wire.endTransmission();
}

// Used to adjust the time. Returns : 0 : display buffer not modified, 1 : display buffer modified, 2 : adjusting time finished
int TimeHandler::adjustTime()
{
  if(m_timeAdjustment == NO) // First call
  {
    m_timeAdjustment = DOM;

    // Display day of month
    displayDOM();

    return 1;
  }
  else if(m_timeAdjustment == DOM) // Adjusting the day
  {
    boolean displayModified = false;
    
    if(m_inputs->getSinglePress(PLUS))
    {
      m_DOM = (m_DOM == 31U) ? 1U : m_DOM + 1U;
      displayModified = true;
    }
    else if(m_inputs->getSinglePress(MINUS))
    {
      m_DOM = (m_DOM == 1U) ? 31U : m_DOM - 1U;
      displayModified = true;
    }
    else if(m_inputs->getSinglePress(MODE)) // Go to month adjustment
    {
      m_timeAdjustment = MONTH;
      
      displayMonth();
      displayModified = true;
    }
    
    if(displayModified)
      displayDOM();
    
    return displayModified ? 1 : 0;
  }
  else if(m_timeAdjustment == MONTH)
  {
    boolean displayModified = false;
    
    if(m_inputs->getSinglePress(PLUS))
    {
      m_month = (m_month == 12U) ? 1U : m_month + 1U;
      displayModified = true;
    }
    else if(m_inputs->getSinglePress(MINUS))
    {
      m_month = (m_month == 1U) ? 12U : m_month - 1U;
      displayModified = true;
    }
    else if(m_inputs->getSinglePress(MODE)) // Go to DOW adjustment
    {
      m_timeAdjustment = DOW;
      
      displayDOW();
      displayModified = true;
    }
    
    if(displayModified)
      displayMonth();
    
    return displayModified ? 1 : 0;
  }
  else if(m_timeAdjustment == DOW)
  {
    boolean displayModified = false;
    
    if(m_inputs->getSinglePress(PLUS))
    {
      m_DOW = (m_DOW == 7U) ? 1U : m_DOW + 1U;
      displayModified = true;
    }
    else if(m_inputs->getSinglePress(MINUS))
    {
      m_DOW = (m_DOW == 1U) ? 7U : m_DOW - 1U;
      displayModified = true;
    }
    else if(m_inputs->getSinglePress(MODE)) // Go to year adjustment
    {
      m_timeAdjustment = YEAR;
      
      displayYear();
      displayModified = true;
    }
    
    if(displayModified)
      displayDOW();
    
    return displayModified ? 1 : 0;
  }
  else if(m_timeAdjustment == YEAR)
  {
    boolean displayModified = false;
    
    if(m_inputs->getSinglePress(PLUS))
    {
      m_year = (m_year == 99U) ? 0U : m_year + 1U;
      displayModified = true;
    }
    else if(m_inputs->getSinglePress(MINUS))
    {
      m_year = (m_year == 0U) ? 99U : m_year - 1U;
      displayModified = true;
    }
    else if(m_inputs->getSinglePress(MODE)) // Go to hours adjustment
    {
      // Verify date integrity
      boolean dateOK = true;
      
      // February
      boolean leapYear = (m_year % 4) == 0 ? true : false; // This is not exactly the real rule, but It's valid until 2099
      if(m_month == 2U && (m_DOM == 30U || m_DOM == 31 || (m_DOM == 29U && leapYear == false)))
        dateOK = false;
        
      // Check for number of days
      if(m_DOM == 31U && (m_month == 4U || m_month == 6U || m_month == 9U || m_month == 11U))
        dateOK = false;
      
      if(dateOK == false) // Go back to begin
      {
        m_disp->clear();
        m_timeAdjustment = NO;
        
        return 1;
      }
      
      m_timeAdjustment = HOURS;
      
      m_disp->clear();
      
      // Make cool looking corners
      m_disp->setLed(0, 0, true); m_disp->setLed(0, 1, true); m_disp->setLed(1,0, true);
      m_disp->setLed(15, 0, true); m_disp->setLed(14, 0, true); m_disp->setLed(15, 1, true);
      m_disp->setLed(0, 15, true); m_disp->setLed(0, 14, true); m_disp->setLed(1, 15, true);
      m_disp->setLed(15, 15, true); m_disp->setLed(15, 14, true); m_disp->setLed(14, 15, true);
      
      displayHours();
      displayModified = true;
    }
    
    if(displayModified && m_timeAdjustment == YEAR)
      displayYear();
    
    return displayModified ? 1 : 0;
  }
  else if(m_timeAdjustment == HOURS) // Adjusting the hours
  {
    boolean displayModified = false;

    if(m_inputs->getSinglePress(PLUS))
    {
      m_hours = (m_hours == 23U) ? 0U : m_hours + 1U;
      displayModified = true;
    }
    else if(m_inputs->getSinglePress(MINUS))
    {
      m_hours = (m_hours == 0U) ? 23U : m_hours - 1U;
      displayModified = true;
    }
    else if(m_inputs->getSinglePress(MODE)) // Go to minutes adjustment
    {
      m_timeAdjustment = MINS;

      displayMinutes();
      displayModified = true;
    }

    if(displayModified) // Display hours
      displayHours();

    return displayModified ? 1 : 0;
  }
  else if(m_timeAdjustment == MINS) // Adjusting the minutes
  {
    boolean displayModified = false;

    if(m_inputs->getSinglePress(PLUS))
    {
      m_mins = (m_mins == 59U) ? 0U : m_mins + 1U;
      displayModified = true;
    }
    else if(m_inputs->getSinglePress(MINUS))
    {
      m_mins = (m_mins == 0U) ? 59U : m_mins - 1U;
      displayModified = true;
    }
    else if(m_inputs->getSinglePress(MODE)) // Time adjustment finished
    {
      m_timeAdjustment = NO;

      m_secs = 0U;
      setRTCTime();

      return 2;
    }

    if(displayModified) // Display minutes
      displayMinutes();

    return displayModified ? 1 : 0;
  }
}

// Displaying functions
void TimeHandler::displayDOM()
{
#ifdef MODE_DMY
  m_disp->setDigit(0, 0, m_DOM / 10);
  m_disp->setDigit(4, 0, m_DOM - (m_DOM / 10) * 10);
#else
  m_disp->setDigit(9, 0, m_DOM / 10);
  m_disp->setDigit(13, 0, m_DOM - (m_DOM / 10) * 10);
#endif
}

void TimeHandler::displayDOW()
{
  for(int i = 1 ; i < 8 ; i++)
  {
    m_disp->setLed(i * 2 - 1, 7, i <= m_DOW);
    m_disp->setLed(i * 2 - 1, 8, i <= m_DOW);
  }
}

void TimeHandler::displayMonth()
{
#ifdef MODE_DMY
  m_disp->setDigit(9, 0, m_month / 10);
  m_disp->setDigit(13, 0, m_month - (m_month / 10) * 10);
#else
  m_disp->setDigit(0, 0, m_month / 10);
  m_disp->setDigit(4, 0, m_month - (m_month / 10) * 10);
#endif
}

void TimeHandler::displayYear()
{
  m_disp->setDigit(0, 11, 2);
  m_disp->setDigit(4, 11, 0);

  m_disp->setDigit(9, 11, m_year / 10);
  m_disp->setDigit(13, 11, m_year - (m_year / 10) * 10);
}

void TimeHandler::displayHours()
{
  m_disp->setDigit(2, 2, m_hours / 10);
  m_disp->setDigit(6, 2, m_hours - ((m_hours / 10) * 10));
}

void TimeHandler::displayMinutes()
{
  m_disp->setDigit(7, 9, m_mins / 10);
  m_disp->setDigit(11, 9, m_mins - ((m_mins / 10) * 10));
}

void TimeHandler::displayBinaryTime()
{
  for(int i = 0 ; i <= 6 ; i++)
  {
    // Hours
    m_disp->setLed(2, 15 - (i * 2), m_hours & (B00000001 << i));
    m_disp->setLed(3, 15 - (i * 2), m_hours & (B00000001 << i));
    m_disp->setLed(2, 14 - (i * 2), m_hours & (B00000001 << i));
    m_disp->setLed(3, 14 - (i * 2), m_hours & (B00000001 << i));

    // Minutes
    m_disp->setLed(7, 15 - (i * 2), m_mins & (B00000001 << i));
    m_disp->setLed(8, 15 - (i * 2), m_mins & (B00000001 << i));
    m_disp->setLed(7, 14 - (i * 2), m_mins & (B00000001 << i));
    m_disp->setLed(8, 14 - (i * 2), m_mins & (B00000001 << i));

    // Seconds
    m_disp->setLed(12, 15 - (i * 2), m_secs & (B00000001 << i));
    m_disp->setLed(13, 15 - (i * 2), m_secs & (B00000001 << i));
    m_disp->setLed(12, 14 - (i * 2), m_secs & (B00000001 << i));
    m_disp->setLed(13, 14 - (i * 2), m_secs & (B00000001 << i));
  }
}

// --------- Debuging functions ---------
#ifdef DEBUG

// Used to print the current time to the serial port
void TimeHandler::printTime() 
{
#ifdef MODE_DMY
  Serial.println(String(m_DOM) + "/" + String(m_month) + "/" + String(m_year) + " (DOW : " + String(m_DOW) + ")");
#else
  Serial.println(String(m_month) + "/" + String(m_DOM) + "/" + String(m_year) + " (DOW : " + String(m_DOW) + ")");
#endif

  Serial.println(String(m_hours) + ":" + String(m_mins) + ":" + String(m_secs));
}
#endif

