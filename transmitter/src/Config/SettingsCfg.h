/**
 * @file   settings.h
 * @author Simon Lövgren, 2018
 * 
 * @brief  User interface header file. Based on Benjamin Vedder's NRF Esk8 Remote.
 */

#ifndef _SETTINGSCFG_H_
#define _SETTINGSCFG_H_

#include "../Settings/Settings.h"

tSetting setting_settings[] = {
    /*  Name,           Unit,     Value,    Range min,     Range max*/
    {"Trigger",         "",       0,        0,             3        },
    {"Battery type",    "",       0,        0,             1        },
    {"Battery cells",   "S",      10,       0,             12       },
    {"Motor poles",     "",       14,       0,             250      },
    {"Motor pulley",    "T",      14,       0,             250      },
    {"Wheel pulley",    "T",      36,       0,             250      },
    {"Wheel diameter",  "mm",     80,       0,             250      },
    {"UART data",       "",       1,        0,             1        },
    {"Throttle min",    "",       0,        0,             1023     },
    {"Throttle center", "",       512,      0,             1023     },
    {"Throttle max",    "",       1023,     0,             1023     }
};

#endif     // _SETTINGS_H_
