/*
 * 16 * 16 LED matrix
 * april 2012
 * op414
 * http://op414.net
 * License : CC BY-NC-SA http://creativecommons.org/licenses/by-nc-sa/3.0/
 * ---------
 * InputHandler.h : InputHandler class definition.
 */
 
#ifndef DEF_INPUTHANDLER
#define DEF_INPUTHANDLER

#define DEBOUNCE_DELAY 10


// Lambda enumeration to make the code more readable
enum{MODE = 0, PLUS = 1, MINUS = 2};

class InputHandler
{
  public:
    InputHandler();
    void updateButtonsStates();
    boolean getButtonState(int button);
    boolean getLastButtonState(int button);
    boolean getSinglePress(int button);
    unsigned long getLastChangeTime(int button);
  
  private:
    boolean m_buttonState[3];
    boolean m_lastButtonState[3];
    boolean m_lastButtonReading[3];
    unsigned long m_debounceStartTime[3];
    unsigned long m_lastButtonChangeTime[3];
    
    static const int m_buttonPins[3];
    
};

#endif
