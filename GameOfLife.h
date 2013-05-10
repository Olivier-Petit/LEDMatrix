/*
 * 16 * 16 LED matrix
 * april 2012
 * op414
 * http://op414.net
 * License : CC BY-NC-SA http://creativecommons.org/licenses/by-nc-sa/3.0/
 * ---------
 * GameOfLife.h : GameOfLife class definition.
 */
 
#ifndef DEF_GAMEOFLIFE
#define DEF_GAMEOFLIFE

#define GOL_MAX_ITERATION 95

#include "Display.h"

class GameOfLife
{
  public:
    GameOfLife(Display *disp); 
    void getNextStep();
    void initialize();
    void autoReset();
    void resetStepCounter();
    boolean autoNextStep();
  
  private:
    Display *m_disp;
    unsigned int m_step;
    unsigned long m_lastUpdateTime;
};

#endif
