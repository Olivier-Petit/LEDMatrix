/*
 * 16 * 16 LED matrix
 * april 2012
 * op414
 * http://op414.net
 * License : CC BY-NC-SA http://creativecommons.org/licenses/by-nc-sa/3.0/
 * ---------
 * GameOfLife.cpp : Implements the GameOfLife class (Connway's game of life).
 */
 
#if defined(ARDUINO) && ARDUINO >= 100
  #include <Arduino.h>
#else
  #include "WProgram.h"
#endif

#include "GameOfLife.h"
#include "Display.h"
#include "settings.h"

// Constructor
GameOfLife::GameOfLife(Display *disp)
{
  m_disp = disp;
  m_step = 0U;
  m_lastUpdateTime = 0UL;
}

// Used to go one generation forward
void GameOfLife::getNextStep()
{
  boolean backupWorld[16][16] = {{false}};
  
  // Increment step counter
  m_step++;
  
  // Backup current state
  for(int i = 0 ; i < 16 ; i++)
  {
    for(int j = 0 ; j < 16 ; j++)
    {
      backupWorld[i][j] = m_disp->testLed(i, j);
    }
  }
  
  // Update cells
  for(int x = 0 ; x < 16 ; x++)
  {
    for(int y = 0 ; y < 16 ; y++)
    {
      // Count the number of neighbors
      int neighbors = 0;
      
      if(x > 0 && y > 0 && backupWorld[x-1][y-1] == true) // top left
        neighbors++;
      
      if(y > 0 && backupWorld[x][y-1] == true) // top
        neighbors++;
        
      if(x < 15 && y > 0 && backupWorld[x+1][y-1] == true) // top right
        neighbors++;
        
      if(x < 15 && backupWorld[x+1][y] == true) // right
        neighbors++;
        
      if(x < 15 && y < 15 && backupWorld[x+1][y+1] == true) // bottom right
        neighbors++;
      
      if(y < 15 && backupWorld[x][y+1] == true) // bottom
        neighbors++;
        
      if(x > 0 && y < 15 && backupWorld[x-1][y+1] == true) // bottom left
        neighbors++;
        
      if(x > 0 && backupWorld[x-1][y] == true) // left
        neighbors++;
        
        
      // Update the cell accordingly
      if(backupWorld[x][y] == true && (neighbors > 3 || neighbors < 2)) // cell is alive and has too few or too much neighbors -> turn it off
        m_disp->setLed(x, y, false);
      else if(backupWorld[x][y] == false && neighbors == 3) // cell is dead and has 3 neighbors -> turn it on
        m_disp->setLed(x, y, true);
    }
  }
}

// Used to automatically go to the next step
boolean GameOfLife::autoNextStep()
{
  int potVal = analogRead(PIN_POT);
  
  if(millis() - m_lastUpdateTime >= (unsigned long)(potVal < 512 ? map(potVal, 0, 511, 10, 200) : map(potVal, 512, 1023, 200, 5000)) || m_lastUpdateTime == 0) // Update needed
  {
    getNextStep();
    m_lastUpdateTime = millis();
    
    return true;
  }
  else // Nothing to do
    return false;
}

// Used to get a random initialization
void GameOfLife::initialize()
{
  m_step = 0U;
  
  for(int i = 0 ; i < 16 ; i++)
  {
    for(int j = 0 ; j < 16 ; j++)
    {
      if(random(2) == 0)
        m_disp->setLed(i, j, true);
      else
        m_disp->setLed(i, j, false);
    }
  }
}

// Used to check if a reset is needed
void GameOfLife::autoReset()
{
  if(m_step > GOL_MAX_ITERATION || m_disp->empty())
    initialize();
}

// Used to reset the step counter
void GameOfLife::resetStepCounter()
{
  m_step = 0U;
}
