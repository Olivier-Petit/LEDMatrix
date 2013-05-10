/*
 * 16 * 16 LED matrix
 * april 2012
 * op414
 * http://op414.net
 * License : CC BY-NC-SA http://creativecommons.org/licenses/by-nc-sa/3.0/
 * ---------
 * TimeHandler.h : TimeHandler class definition.
 */

#ifndef DEF_TIMEHANDLER
#define DEF_TIMEHANDLER

#define CHRONODOT_ADDR 0x68

#include "Display.h"
#include "InputHandler.h"
#include "settings.h"

#define RTC_CHECK_INTERVAL 300UL

#define MODE_DMY // Comment for MDY mode

// Lambda enumeration for the date/time adjustment
enum{NO, DOM, MONTH, DOW, YEAR, HOURS, MINS};

class TimeHandler
{
  public:
    TimeHandler(Display *disp, InputHandler *inputs);
    void displayTime();
    void displayDate();
    void initializeRTC();
    boolean updateTime();
    void changeTimeDisplayMode();
    int adjustTime();
    
    #ifdef DEBUG
      void printTime();
    #endif
  
  private:
    unsigned int m_hours;
    unsigned int m_mins;
    unsigned int m_secs;
    unsigned int m_DOM;
    unsigned int m_DOW;
    unsigned int m_month;
    unsigned int m_year;
    
    boolean m_binaryMode;
    unsigned long m_lastRTCCheck;
    int m_timeAdjustment;
    
    Display *m_disp;
    InputHandler *m_inputs;
    
    byte decToBcd(byte val);
    byte bcdToDec(byte val);
    void setRTCTime();
    void getRTCTime();
    
    void displayDOM();
    void displayDOW();
    void displayMonth();
    void displayYear();
    void displayHours();
    void displayMinutes();
    void displayBinaryTime();
};

#endif
