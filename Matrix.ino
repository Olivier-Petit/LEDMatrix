/*
 * 16 * 16 LED matrix
 * Created : april 2012
 * Updated : may 2012
 * op414
 * http://op414.net
 * License : CC BY-NC-SA http://creativecommons.org/licenses/by-nc-sa/3.0/
 * ---------
 * Matrix.ino : main file ; contains setup() and loop() functions.
 */

#include <SPI.h>
#include <Wire.h>
#include <OneWire.h>
#include <EEPROM.h>

#include "Display.h"
#include "GameOfLife.h"
#include "TimeHandler.h"
#include "InputHandler.h"
#include "TempSensor.h"
#include "SettingsHandler.h"

#define CONTINUOUS_PRESS_THRESHOLD 1000UL

InputHandler inputs;
Display disp(&inputs);
GameOfLife gol(&disp);
TimeHandler time(&disp, &inputs);
TempSensor temp(&disp);
SettingsHandler settings(&disp, &inputs);

// Lambda enumaration for the mode selector
enum{GOL = 0, TIME = 1 , DATE = 2, TEMP = 3, SETTINGS, TIME_ADJUST, BRIGHTNESS_ADJUST}; // GOL = GameOfLife

int mode = GOL;
boolean autoModeChange = false;
unsigned long lastModeChange = 0UL;
unsigned int modeDuration[4] = {0};

void setup()
{
  randomSeed(analogRead(PIN_RAND));

  #ifdef DEBUG
    Serial.begin(9600);
  #endif
  
  time.initializeRTC();
  disp.testPattern();
  
  // Read the saved settings
  settings.read();
  settings.getModeDurations(modeDuration);
  autoModeChange = settings.getBooleanSetting(SETTING_AUTO_MODE_CHANGE);
}

void loop()
{
  // Refresh the inputs states
  inputs.updateButtonsStates();
  
  // Update the brightness of the display
  disp.updateBrightness();
  
  // Auto mode change activation/desactivation
  if((inputs.getSinglePress(PLUS) && inputs.getButtonState(MINUS) == HIGH) || (inputs.getSinglePress(MINUS) && inputs.getButtonState(PLUS) == HIGH))
  {
    if(autoModeChange)
    {
      autoModeChange = false;
    }
    else
    {
      autoModeChange = true;
      lastModeChange = millis(); // We do not want to change mode imediately
    }
  }
  
  // Go to settings mode
  if(mode != SETTINGS && inputs.getButtonState(MODE) == HIGH && millis() - inputs.getLastChangeTime(MODE) >= CONTINUOUS_PRESS_THRESHOLD)
  {
    mode = SETTINGS;
    
    disp.clear();
    
    // Make cool looking corners
    disp.setLed(0, 0, true); disp.setLed(0, 1, true); disp.setLed(1,0, true);
    disp.setLed(15, 0, true); disp.setLed(14, 0, true); disp.setLed(15, 1, true);
    disp.setLed(0, 15, true); disp.setLed(0, 14, true); disp.setLed(1, 15, true);
    disp.setLed(15, 15, true); disp.setLed(15, 14, true); disp.setLed(14, 15, true);
    
    // Time
    // Plus sign
    disp.setLed(2, 3, true);
    disp.setLed(1, 4, true); disp.setLed(2, 4, true); disp.setLed(3, 4, true);
    disp.setLed(2, 5, true);
    
    // Digits
    disp.setDigit(6, 2, 8);
    disp.setDigit(12, 2, 8);
    
    // Separator
    disp.setLed(10, 3, true); disp.setLed(10, 5, true);
    
    // Brightness
    // Minus sign
    disp.setLed(1, 12, true); disp.setLed(2, 12, true); disp.setLed(3, 12, true);
    
    // Scale
    for(int i = 6 ; i < 14 ; i++)
      disp.setLed(i, 13, true);
    
    for(int i = 8 ; i < 14 ; i++)
      disp.setLed(i, 12, true);
    
    for(int i = 10 ; i < 14 ; i++)
      disp.setLed(i, 11, true);
    
    for(int i = 12 ; i < 14 ; i++)
      disp.setLed(i, 10, true);

    disp.display();
  }
  
  // ------------------------------- CHANGE MODE --------------------------------------------
  
  // Change mode
  if(mode != SETTINGS && mode != TIME_ADJUST && mode != BRIGHTNESS_ADJUST && (inputs.getSinglePress(MODE) || (autoModeChange == true && millis() - lastModeChange >= modeDuration[mode])))
  {
    if(mode == GOL)
    {
      mode = TIME;
      
      time.updateTime();
      
      disp.clear();
      time.displayTime();
      disp.display();
    }
    else if(mode == TIME)
    {
      mode = DATE;
      
      time.updateTime();
      
      disp.clear();
      time.displayDate();
      disp.display();
    }
    else if(mode == DATE)
    {
      mode = TEMP;
      
      temp.updateTemp();
      
      disp.clear();
      temp.displayTemp();
      disp.display();
    }
    else if(mode == TEMP)
    {
      mode = GOL;
      
      // We keep the live cells (= the temp digits) as a base for the GOL
      gol.resetStepCounter();
      gol.getNextStep();
      disp.display();
    }
    
    // If in autoModeChange, backup the time of the change
    if(autoModeChange)
      lastModeChange = millis();
  }
  
  // -------------------------- NORMAL MODES -------------------------------------
  
  // Refresh the display according to the current mode
  if(mode == GOL)
  {
    // Manual reset
    if(inputs.getSinglePress(PLUS))
    {
        gol.initialize();
        disp.display();
    }
        
    if(gol.autoNextStep())
    {
      gol.autoReset(); // Auto reset (no more live cells or too much steps)
      disp.display();
    }
  }
  else if(mode == TIME)
  {
    // Display mode change
    if(inputs.getSinglePress(PLUS))
    {
      time.changeTimeDisplayMode();
      disp.clear();
      time.displayTime();
      disp.display();
    }
    
    if(time.updateTime())
    {
      time.displayTime();
      disp.display();
    }
  }
  else if(mode == DATE)
  {
    if(time.updateTime())
    {
      time.displayDate();
      disp.display();
    }
  }
  else if(mode == TEMP)
  {
    if(temp.updateTemp())
    {
      temp.displayTemp();
      disp.display();
    }
  }
  // ----------------------------- SETTINGS MODES ---------------------------------
  else if(mode == SETTINGS)
  {
    if(inputs.getSinglePress(PLUS)) // Go to time adjusment
    {
      mode = TIME_ADJUST;
      disp.clear();
    }
    else if(inputs.getSinglePress(MINUS)) // Go to brightness adjustment
    {
      mode = BRIGHTNESS_ADJUST;
      disp.clear();
    }
  }
  else if(mode == TIME_ADJUST)
  {
    int response = time.adjustTime();
    
    if(response == 1) // Time was modified by a user action -> refresh display
    {
      disp.display();
    }
    else if(response == 2) // Time adjustment finished
    {
      disp.display();
      
      lastModeChange = millis(); // We want to stay in time mode for a moment if in auto mode change
      mode = TIME;
    }
  }
  else if(mode == BRIGHTNESS_ADJUST)
  {
    int response = disp.adjustBrightness();
    
    if(response == 1) // Brightness modified by a user action -> refresh display
    {
      disp.display();
    }
    else if(response == 2) // Brightness adjustment finshed -> go to GOL mode
    {
      gol.initialize();
      disp.display();
      
      lastModeChange == millis(); // We want to stay in GOL mode for a moment if in auto mode change
      mode = GOL;
    }
  }
}

