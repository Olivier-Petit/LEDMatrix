/*
 * 16 * 16 LED matrix
 * Created : april 2012
 * Updated : may 2012
 * op414
 * http://op414.net
 * License : CC BY-NC-SA http://creativecommons.org/licenses/by-nc-sa/3.0/
 * ---------
 * Display.h : Definition of the Display class.
 */

#ifndef DEF_DISPLAY
#define DEF_DISPLAY

#include "InputHandler.h"
#include "settings.h"

// Definition of MAX7219 registers
#define MAX_REG_NOOP           0x00
#define MAX_REG_DECODEMODE     0x09
#define MAX_REG_INTENSITY      0x0A
#define MAX_REG_SCANLIMIT      0x0B
#define MAX_REG_SHUTDOWN       0x0C
#define MAX_REG_DISPLAYTEST    0x0F

#define BRIGHTNESS_UPDATE_INTERVAL 50UL
#define BRIGHTNESS_UPDATE_THRESHOLD 10

class Display
{
  public:
    Display(InputHandler *inputs);
    void setTestMode(boolean testMode);
    void clear();
    void display();
    void setLed(int x, int y, boolean val);
    boolean testLed(int x, int y);
    void setDigit(int x, int y, int digit);
    void testPattern();
    boolean empty();
    int adjustBrightness();
    void updateBrightness();
    
    #ifdef DEBUG
      void printBuffer();
      void printBrightness();
    #endif
  
  private:
    byte m_buffer[4][8];
    byte m_backBuffer[4][8];
    int m_brightness; // -1 = auto
    unsigned long m_lastBrightnessUpdate;
    int m_lastBrightnessValue;
    static const byte m_sevenSegDigit[10];
    static const int m_sevenSegTemplate[7][2][3];
    
    int m_adjustingBrightness;
    
    InputHandler *m_inputs;
  
    void sendAll(byte reg, byte val);
    void setBrightness(byte val);
};

#endif

