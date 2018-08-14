/**
 * @file   settings.h
 * @author Simon Lövgren, 2018
 * 
 * @brief  User interface header file. Based on Benjamin Vedder's NRF Esk8 Remote.
 */

#ifndef _SETTINGS_H_
#define _SETTINGS_H_

/**
 * ****************************************************************************
 * INCLUDES
 * ****************************************************************************
 */

#include <Arduino.h>

/**
 * ****************************************************************************
 * DEFINES
 * ****************************************************************************
 */

/**
 * ****************************************************************************
 * TYPEDEFS
 * ****************************************************************************
 */

typedef struct settings_t
{
    char* name;
    char* unit;
    int   value;
    int   min;
    int   max;
} tSetting;

/**
 * ****************************************************************************
 * FUNCTIONS
 * ****************************************************************************
*/

/**
 * @brief         Initialize UI module.
 * @param[in] -
 * @return        -
 */
void settings_init();

/**
 * @brief         start UI module.
 * @param[in] -
 * @return        -
 */
void settings_start();

/**
 * @brief         Retreives value of setting.
 * @param[in]     setting
 *                Setting to retreive value of.
 * @return        Value of setting.
 */
int settings_getSetting( unsigned int setting );


#endif     // _SETTINGS_H_
