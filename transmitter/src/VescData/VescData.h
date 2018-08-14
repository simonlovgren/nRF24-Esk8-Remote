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

// Defining struct to hold UART data.
typedef struct vescValues_t {
    float ampHours;
    float inpVoltage;
    long  rpm;
    long  tachometerAbs;
} tVescValues;

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
void vescdata_init();

/**
 * @brief         start UI module.
 * @param[in] -
 * @return        -
 */
void vescdata_start();

/**
 * @brief         Get VESC values.
 * @param[out]    data
 *                Pointer to where VESC values are to be stored.
 * @return        TRUE if OK, FALSE otherwise.
 */
bool vescdata_get( tVescValues* data );

/**
 * @brief         Get input voltage.
 * @param[in]     -
 * @return        Input voltage.
 */
float vescdata_getInputVoltage();

/**
 * @brief         Get RPM.
 * @param[in]     -
 * @return        RPM.
 */
long vescdata_getRpm();

/**
 * @brief         Get absolute tachometer count.
 * @param[in]     -
 * @return        Tachometer count.
 */
long vescdata_getTachometerCount();

/**
 * @brief         Get Amp hours.
 * @param[in]     -
 * @return        Amp hours.
 */
float vescdata_getAmpHours();

/**
 * @brief         Set VESC values.
 * @param[in]     data
 *                VESC values to set.
 * @return        TRUE if OK, FALSE otherwise.
 */
bool vescdata_set( tVescValues* data );

#endif     // _SETTINGS_H_
