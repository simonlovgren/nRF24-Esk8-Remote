/**
 * @file     ui.hxx
 * @author   Simon Lövgren, 2018
 * 
 * @brief    User interface header file. Based on Benjamin Vedder's NRF Esk8 Remote.
 */

#ifndef _UI_H_
#define _UI_H_

/**
 * ****************************************************************************
 * INCLUDES
 * ****************************************************************************
 */

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

/**
 * Screen types available.
 */
typedef enum Screen_t
{
    SCREEN_SPLASH,
    SCREEN_TITLE,
    SCREEN_SETTINGS,
    SCREEN_MAIN,
    /* This should always be last, as it is the number of available screens. */
    SCREEN_COUNT // Not an actual selectable screen
} tScreen;

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
void ui_init();

/**
 * @brief         start UI module.
 * @param[in] -
 * @return        -
 */
void ui_start();

/**
 * @brief         Draw settings menu screen.
 * @param[in] -
 * @return        -
 */
void ui_drawSettingsMenu();

/**
 * @brief         Update the main display.
 * @param[in] -
 * @return        -
 */
void ui_updateMainDisplay();

/**
 * @brief         Draw the start screen.
 * @param[in] -
 * @return        -
 */
void ui_drawStartScreen();

/**
 * @brief         Draw the title screen.
 * @param[in] title Title to draw on screen.
 * @return        -
 */
void ui_drawTitleScreen( char* title );


#endif     // _UI_H_
