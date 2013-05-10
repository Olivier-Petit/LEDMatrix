/*
 * 16 * 16 LED matrix
 * Created : may 2012
 * op414
 * http://op414.net
 * License : CC BY-NC-SA http://creativecommons.org/licenses/by-nc-sa/3.0/
 * ---------
 * SettingsHandler.h : SettingsHandler class definition
 */
 
#ifndef DEF_SETTINGSHANDLER
#define DEF_SETTINGSHANDLER

#include "settings.h"
#include "Display.h"
#include "InputHandler.h"

/* ========== SETTINGS STORAGE ==========
 *
 * Byte 0 : GOL mode duration (seconds)
 * Byte 1 : Time mode duration (seconds)
 * Byte 2 : Date mode duration (seconds)
 * Byte 3 : Temperature mode duration (seconds)
 *
 * Byte 4 :
 *   Bit 0 (LSB) : Auto mode change (disabled : 0 / enabled : 1)
 *   Bit 1 : Clock display mode (normal : 0 / binary : 1)
 *   Bit 2 : Auto clock display mode change (disabled : 0 / enabled : 1)
 *
 */
 

// Boolean settings bits definitions 
#define SETTING_AUTO_MODE_CHANGE                0
#define SETTING_CLOCK_DISPLAY_MODE              1
#define SETTING_AUTO_CLOCK_DISPLAY_MODE_CHANGE  3

class SettingsHandler
{
  public:
    SettingsHandler(Display *disp, InputHandler *inputs);
    void read();
    void getModeDurations(unsigned int modeDurations[]);
    boolean getBooleanSetting(int setting);
    
    void save();
  
  private:
    byte m_settings[5];
    
    Display *m_disp;
    InputHandler *m_inputs;
};

#endif
 
