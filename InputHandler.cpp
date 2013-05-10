/*
 * 16 * 16 LED matrix
 * april 2012
 * op414
 * http://op414.net
 * License : CC BY-NC-SA http://creativecommons.org/licenses/by-nc-sa/3.0/
 * ---------
 * InputHandler.cpp : implements the InputHandle class, which is used to debounce the buttons inputs.
 */
 
#if defined(ARDUINO) && ARDUINO >= 100
  #include <Arduino.h>
#else
  #include "WProgram.h"
#endif

#include "InputHandler.h"
#include "settings.h"

// Constructor
InputHandler::InputHandler()
{
  for(int i = 0 ; i < 3 ; i++)
  {
    pinMode(m_buttonPins[i], INPUT); // Set the button pins to inputs
    
    // Initialize the arrays
    m_buttonState[i] = false;
    m_lastButtonState[i] = false;
    m_lastButtonReading[i] = false;
    m_debounceStartTime[i] = 0UL;
    m_lastButtonChangeTime[i] = 0UL;
  }
}

// Allows to store the three pins in an array
const int InputHandler::m_buttonPins[3] = {PIN_MODE, PIN_PLUS, PIN_MINUS};

// Used to update the value of the buttons
void InputHandler::updateButtonsStates()
{
  for(int i = 0 ; i < 3 ; i++)
  {
    boolean reading = digitalRead(m_buttonPins[i]);
    
    if(reading != m_lastButtonReading[i]) // Bouncing
      m_debounceStartTime[i] = millis();
      
    if(reading == m_lastButtonReading[i] && millis() - m_debounceStartTime[i] >= DEBOUNCE_DELAY) // Not bouncing anymore
    {
      m_lastButtonState[i] = m_buttonState[i];
      m_buttonState[i] = reading;
      
      if(m_lastButtonState[i] != m_buttonState[i]) // Button changed state
      {
        m_lastButtonChangeTime[i] = millis();
      }
    }
    
    m_lastButtonReading[i] = reading;
  }
}

// Used to get the debounced reading of a button
boolean InputHandler::getButtonState(int button)
{
  return m_buttonState[button];
}

// Used to get the last button state
boolean InputHandler::getLastButtonState(int button)
{
  return m_lastButtonState[button];
}

// Used to get a single button press (the button must be released between each push)
boolean InputHandler::getSinglePress(int button)
{
  return m_buttonState[button] == HIGH && m_lastButtonState[button] == LOW ? true : false;
}

// Used to got the time since the button is in the current position
unsigned long InputHandler::getLastChangeTime(int button)
{
  return m_lastButtonChangeTime[button];
}
