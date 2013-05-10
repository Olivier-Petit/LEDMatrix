/*
 * 16 * 16 LED matrix
 * Created : april 2012
 * Updated : may 2012
 * op414
 * http://op414.net
 * License : CC BY-NC-SA http://creativecommons.org/licenses/by-nc-sa/3.0/
 * ---------
 * Display.cpp : Implements the Display class which is used to interract with the display (through MAX7219 chips).
 */

#if defined(ARDUINO) && ARDUINO >= 100
  #include <Arduino.h>
#else
  #include "WProgram.h"
#endif

#include "Display.h"
#include "InputHandler.h"
#include "settings.h"

#include <SPI.h>

// Constructor
Display::Display(InputHandler *inputs)
{
  m_inputs = inputs;
  
  m_adjustingBrightness = false;
  
  // Setting-up the Load pin
  pinMode(PIN_LOAD, OUTPUT);
  digitalWrite(PIN_LOAD, HIGH);
  
  // Clear the buffer
  clear();
  
  // Clear the backbuffer
  for(int i = 0 ; i < 4 ; i++)
  {
    for(int j = 0 ; j < 8 ; j++)
    {
      m_backBuffer[i][j] = B00000000;
    }
  }
  
  // Setting-up the SPI library
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV128);
  SPI.setDataMode(SPI_MODE0);
  
  // We do not want to use any decoding option
  sendAll(MAX_REG_DECODEMODE, 0x00);
  
  // Setting the brightness
  m_brightness = -1; // Auto mode
  m_lastBrightnessUpdate = 0UL;
  m_lastBrightnessValue = 0;
  setBrightness(0x08);
  
  // Setting the scan limit to its maximal (we're using all outputs)
  sendAll(MAX_REG_SCANLIMIT, 0x07);
  
  // Making sure that the test mode is disabled
  setTestMode(false);
  
  // Clearing the display
  for(int i = 1 ; i <= 8 ; i++)
  {
    sendAll(i, 0x00);
  }
  
  // Leaving shutdown mode
  sendAll(MAX_REG_SHUTDOWN, 0x01);
}

// Seven segment digit table (GFEDCBA notation)
const byte Display::m_sevenSegDigit[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
const int Display::m_sevenSegTemplate[7][2][3] = 
  {{{0, 1, 2}, {0, 0, 0}}, {{2, 2, 2}, {0, 1, 2}}, {{2, 2, 2}, {2, 3, 4}}, {{0, 1, 2}, {4, 4, 4}}, {{0, 0, 0}, {2, 3, 4}}, {{0, 0, 0}, {0, 1, 2}}, {{0, 1, 2}, {2, 2, 2}}};

// Send the same command to each driver
void Display::sendAll(byte reg, byte val)
{
  digitalWrite(PIN_LOAD, LOW);
  
  for(int i = 0 ; i < 4 ; i++)
  {
    SPI.transfer(reg);
    SPI.transfer(val);
  }
  
  digitalWrite(PIN_LOAD, HIGH);
}

// Used to modify the brightness of the display
void Display::setBrightness(byte val)
{
  sendAll(MAX_REG_INTENSITY, val);
}

// Used to test the display
void Display::setTestMode(boolean testMode)
{
  if(testMode)
    sendAll(MAX_REG_DISPLAYTEST, 0x01);
  else
    sendAll(MAX_REG_DISPLAYTEST, 0x00);
}

// Used to clear the buffer
void Display::clear()
{
  for(int i = 0 ; i < 4 ; i++)
  {
    for(int j = 0 ; j < 8 ; j++)
    {
        m_buffer[i][j] = B00000000;
    }
  }
}

// Used to send the content of the buffer to the controllers
void Display::display()
{
  int modifiedRegAmount[4] = {0};
  int maxModifiedRegAmount = 0;
  int lastDigitUpdated[4] = {0}; // Stores the last digit updated
  
  // Find the modified registers for each driver
  for(int i = 0 ; i < 4 ; i++)
  {
    for(int j = 0 ; j < 8 ; j++)
    {
      if(m_buffer[i][j] != m_backBuffer[i][j])
        modifiedRegAmount[i]++;
    }
    
    maxModifiedRegAmount = max(modifiedRegAmount[i], maxModifiedRegAmount);
  }
  
  #ifdef SERIAL_DEBUG
    Serial.println("==== Sending " + String(maxModifiedRegAmount) + " commands ====");
    Serial.println("--- Modified registers ---");
    
    for(int i = 0 ; i < 4 ; i++)
    {
      Serial.println("Driver #" + String(i));
      
      for(int j = 0 ; j < 8 ; j++)
      {
        if(m_buffer[i][j] != m_backBuffer[i][j])
          Serial.println("  DIG" + String(j) + " : YES");
        else
          Serial.println("  DIG" + String(j) + " : NO");
      }
      
      Serial.println();
    }
    Serial.println();
  #endif
  
  // Send the commands
  for(int i = 0 ; i < maxModifiedRegAmount ; i++)
  {
    #ifdef SERIAL_DEBUG
       Serial.println("--- Sending #" + String(i) + " ---");
    #endif
      
    digitalWrite(PIN_LOAD, LOW);
    for(int j = 3 ; j >= 0 ; j--) // We have to send the commands in reverse order
    { 
      // Find the digit to update
      int k = lastDigitUpdated[j];
      while(m_buffer[j][k] == m_backBuffer[j][k] && k < 8)
      {
        k++;
      }
      
      // Update it
      if(k == 8) // We have updated all the registers ; send no-op code
      {
        #ifdef SERIAL_DEBUG
          Serial.println("Driver #" + String(j) + " : No operation");
        #endif
      
        SPI.transfer(MAX_REG_NOOP);
        SPI.transfer(MAX_REG_NOOP);
      }
      else // We do not have updated all registers
      {
        lastDigitUpdated[j] = k; // Save the last digit updated
        m_backBuffer[j][k] = m_buffer[j][k]; // Copy the buffer into the backbuffer
        
        #ifdef SERIAL_DEBUG
          Serial.print("Driver #" + String(j) + ", DIG" + String(k) + " : ");
          Serial.println(m_buffer[j][k], BIN);
        #endif
        
        // Send command
        SPI.transfer(k + 1); // Register (DIG0 = REG #1, DIG1 = REG #2, ..., DIG 7 = REG #8)
        SPI.transfer(m_buffer[j][k]); // Value
      }
    }
    digitalWrite(PIN_LOAD, HIGH); // Load da shit
  }
}

// Used to set an led in the buffer
void Display::setLed(int x, int y, boolean val)
{
  // Find position
  int driver = (x / 8) + ((y / 8) * 2); // x_d = x/8 | y_d = y/8 || i_d = x_d + y_d * 2
  int dig = y % 8;
  int seg = 7 - (x % 8); // DP = 7, A = 6, ... , G = 0
  
  // Backup the digit (one byte/register)
  byte oldVal = m_buffer[driver][dig];
  
  // Update buffer
  if(val) // Turn it on
    m_buffer[driver][dig] |= B00000001 << seg;
  else // Turn it off
    m_buffer[driver][dig] &= ~(B00000001 << seg);
}

// Used to check the state of an led in the buffer
boolean Display::testLed(int x, int y)
{
  // Find position
  int driver = (x / 8) + ((y / 8) * 2); // x_d = x/8 | y_d = y/8 || i_d = x_d + y_d * 2
  int dig = y % 8;
  int seg = 7 - (x % 8); // DP = 7, A = 6, ... , G = 0
  
  // Check the buffer
  return (m_buffer[driver][dig] & (B00000001 << seg)) ? true : false;
}

// Used to display a seven segment digit
void Display::setDigit(int x, int y, int digit)
{
  // Clear the digit area
  for(int i = 0 ; i < 3 ; i++)
  {
    for(int j = 0 ; j < 5 ; j++)
    {
      setLed(x + i, y + j, false);
    }
  }
  
  // Insert the digit into the buffer
  for(int i = 0 ; i < 7 ; i++)
  {
    if(m_sevenSegDigit[digit] & (B00000001 << i))
    {
      for(int j = 0 ; j < 3; j++)
        setLed(x + m_sevenSegTemplate[i][0][j], y + m_sevenSegTemplate[i][1][j], true);
    }
  }
}

// Used to display a test pattern
void Display::testPattern()
{
  for(int i = 0 ; i < 16 ; i++)
  {
    clear();
    for(int j = 0 ; j < 16 ; j++)
    {
      setLed(i, j, true);
      setLed(j, i, true);
    }
    display();
    delay(45);
  }
  clear();
}

// Used to know if no LED is no
boolean Display::empty()
{
  for(int i = 0 ; i < 4 ; i++)
  {
    for(int j = 0 ; j < 8 ; j++)
    {
      if(m_buffer[i][j] != 0U)
        return false;
    }
  }
  
  return true;
}

// Used to adjust the brightness. Returns : 0 : display buffer not modified, 1 : display buffer modified, 2 : adjusting time finished
int Display::adjustBrightness()
{
  boolean modified = false;
  
  if(!m_adjustingBrightness) // First call
  {
    m_adjustingBrightness = true;
    
    // Make cool looking corners
    setLed(0, 0, true); setLed(0, 1, true); setLed(1,0, true);
    setLed(15, 0, true); setLed(14, 0, true); setLed(15, 1, true);
    setLed(0, 15, true); setLed(0, 14, true); setLed(1, 15, true);
    setLed(15, 15, true); setLed(15, 14, true); setLed(14, 15, true);
    
    // Display the brightness
    if(m_brightness != -1) // Not in auto mode
    {
      setDigit(4, 5, m_brightness / 10);
      setDigit(9, 5, m_brightness - (m_brightness / 10) * 10);
    }
    
    return 1;
  }
  else if(m_inputs->getSinglePress(PLUS)) // Increment
  {
    m_brightness = (m_brightness == 0x0F) ? -1 : m_brightness + 1;
    modified = true; 
  }
  else if(m_inputs->getSinglePress(MINUS)) // Decrement
  {
    m_brightness = (m_brightness == - 1) ? 0x0F : m_brightness - 1;
    modified = true;
  }
  else if(m_inputs->getSinglePress(MODE)) // Setting finished
  {
    m_adjustingBrightness = false;
    
    return 2;
  }
  
  if(modified)
  {
    if(m_brightness != -1) // Not in auto mode
    {
      setDigit(4, 5, m_brightness / 10);
      setDigit(9, 5, m_brightness - (m_brightness / 10) * 10);
      
      setBrightness(m_brightness);
    }
    else // In auto mode
    {
      // Clear where the digits were
      for(int i = 4 ; i < 12 ; i++)
      {
        for(int j = 5 ; j < 10 ; j++)
        {
          setLed(i, j, false);
        }
      }
    }
    
    return 1;
  }
  
  return 0;
}

// Used to updadte the brightness
void Display::updateBrightness()
{
  int reading = analogRead(PIN_PHOTOCELL);
  
  if(m_brightness == -1 && millis() - m_lastBrightnessUpdate >= BRIGHTNESS_UPDATE_INTERVAL && abs(reading - m_lastBrightnessValue) > BRIGHTNESS_UPDATE_THRESHOLD)
  {
    setBrightness((byte)constrain(map(reading, 0, 900, 0x00, 0x0F), 0x00, 0x0F));
    m_lastBrightnessValue = reading;
  }
}

// ---------- Debuging functions ---------------

// Used to print the state of the buffer
#ifdef DEBUG
void Display::printBuffer()
{
  // Print drivers 0 and 1
  for(int i = 0 ; i < 8 ; i++)
  {
    // Print driver 0
    for(int j = 7 ; j >= 0 ; j--)
    {
      if(m_buffer[0][i] & (B00000001 << j)) // Led is on
        Serial.print("X ");
      else // Led is off
        Serial.print(". ");
    }
    
    Serial.print("+ ");
    
    // Print driver 1
    for(int j = 7 ; j >= 0 ; j--)
    {
      if(m_buffer[1][i] & (B00000001 << j)) // Led is on
        Serial.print("X ");
      else // Led is off
        Serial.print(". ");
    }
    
    Serial.print("\n"); // Line is finished
  }
  
  Serial.println("+ + + + + + + + + + + + + + + + + ");
  
  // Print drivers 2 and 3
  for(int i = 0 ; i < 8 ; i++)
  {
    // Print driver 2
    for(int j = 7 ; j >= 0 ; j--)
    {
      if(m_buffer[2][i] & (B00000001 << j)) // Led is on
        Serial.print("X ");
      else // Led is off
        Serial.print(". ");
    }
    
    Serial.print("+ ");
    
    // Print driver 3
    for(int j = 7 ; j >= 0 ; j--)
    {
      if(m_buffer[3][i] & (B00000001 << j)) // Led is on
        Serial.print("X ");
      else // Led is off
        Serial.print(". ");
    }
    
    Serial.print("\n"); // Line is finished
  }
}

// Used to print the brightness
void Display::printBrightness()
{
  Serial.println(m_brightness, HEX);
}
#endif
