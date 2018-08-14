/**
 * @file     ui.cxx
 * @author   Simon Lövgren, 2018
 * 
 * @brief    User interface implementation. Based on Benjamin Vedder's NRF Esk8 Remote.
 */

/**
 * ****************************************************************************
 * INCLUDES
 * ****************************************************************************
 */
#include <Arduino.h>
#include <U8g2lib.h>
#include <stdio.h>
#include <string.h>

#include "ui.h"

#include "../Settings/Settings.h"
#include "../VescData/VescData.h"

// Iffy include of config for other module
#include "../Config/SettingsCfg.h"

/**
 * ****************************************************************************
 * DEFINES
 * ****************************************************************************
 */

/**
 * @define DISPLAY_BUFFER_SIZE
 * @brief  Size of text buffer for display output.
 */
#define DISPLAY_BUFFER_SIZE             20

/**
 * @define DATA_ROTATION_MS
 * @brief  Speed at which real time data rotates/changes.
 */
#define DATA_ROTATION_MS              4000

/**
 * ****************************************************************************
 * TYPEDEFS
 * ****************************************************************************
 */

/**
 * UI variables
 */
typedef struct UIVariables_t
{
    tScreen currentScreen;
    char            displayBuffer[ DISPLAY_BUFFER_SIZE ];
    /* Settings specific data */
    unsigned short  currentSetting;
    bool            changeSelectedSetting;
    /* Animation */

    /* Module status */
    bool            initialized;
    bool            started;
} tUIVariables;


/**
 * ****************************************************************************
 * PROTOTYPES
 * ****************************************************************************
 */

/**
 * @brief         Draw settings number.
 * @param[in] -
 * @return        -
 */
static void drawSettingNumber();

/**
 * @brief         Draw page.
 * @param[in] -
 * @return        -
 */
static void drawPage();

/**
 * @brief         Draw throttle gague.
 * @param[in] -
 * @return        -
 */
static void drawThrottle();

/**
 * @brief         Draw NRF24 signal status.
 * @param[in] -
 * @return        -
 */
static void drawSignal();

/**
 * @brief         Draw battery level.
 * @param[in] -
 * @return        -
 */
static void drawBatteryLevel();

/**
 * ****************************************************************************
 * PRIVATE VARIABLES
 * ****************************************************************************
 */

/**
 * UI Variables
 */
static tUIVariables ui = {
    .initialized = false,
    .started     = false
};

/**
 *    Defining the type of display used (128x32)
 */
static U8G2_SSD1306_128X32_UNIVISION_1_HW_I2C u8g2( U8G2_R0, U8X8_PIN_NONE );

/**
 * Logotype
 */
static const unsigned char logo_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x80, 0x3c, 0x01, 0xe0, 0x00, 0x07, 0x70, 0x18, 0x0e, 0x30, 0x18, 0x0c, 0x98, 0x99, 0x19, 0x80, 0xff, 0x01, 0x04, 0xc3, 0x20, 0x0c, 0x99, 0x30, 0xec, 0xa5, 0x37, 0xec, 0xa5, 0x37, 0x0c, 0x99, 0x30, 0x04, 0xc3, 0x20, 0x80, 0xff, 0x01, 0x98, 0x99, 0x19, 0x30, 0x18, 0x0c, 0x70, 0x18, 0x0e, 0xe0, 0x00, 0x07, 0x80, 0x3c, 0x01, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/**
 * Transmitting "icon"
 */
static const char signal_transmitting_bits[] = {
    0x18, 0x00, 0x0c, 0x00, 0xc6, 0x00, 0x66, 0x00, 0x23, 0x06, 0x33, 0x0f,
    0x33, 0x0f, 0x23, 0x06, 0x66, 0x00, 0xc6, 0x00, 0x0c, 0x00, 0x18, 0x00
};

/**
 * Connected to receiver symbol.
 */
static const char signal_connected_bits[] = {
    0x18, 0x00, 0x0c, 0x00, 0xc6, 0x00, 0x66, 0x00, 0x23, 0x06, 0x33, 0x09,
    0x33, 0x09, 0x23, 0x06, 0x66, 0x00, 0xc6, 0x00, 0x0c, 0x00, 0x18, 0x00
};

/**
 * Not connected to receiver symbol.
 */
static const char signal_noconnection_bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x09,
    0x00, 0x09, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};



/**
 * ****************************************************************************
 * INTERFACE FUNCTIONS
 * ****************************************************************************
 */

/* ----------------------------------------------------------------------------
 * Function
 * --------------------------------------------------------------------------*/
void ui_init( void )
{
    if ( !ui.initialized )
    {
        ui.currentScreen            = SCREEN_SPLASH;
        ui.currentSetting           = 0;
        ui.changeSelectedSetting    = false;
        memset( ui.displayBuffer, '\0', sizeof( ui.displayBuffer ) );

        // Set initialized
        ui.initialized = true;
    }
}

/* ----------------------------------------------------------------------------
 * Function
 * --------------------------------------------------------------------------*/
void ui_start( void )
{
    if ( !ui.started )
    {
        u8g2.begin(); // Start display driver

        // Set started
        ui.started = true;
    }
}

/* ----------------------------------------------------------------------------
 * Function
 * --------------------------------------------------------------------------*/
void ui_drawStartScreen( char* title )
{
    u8g2.firstPage();
    do
    {
        u8g2.drawXBM( 4, 4, 24, 24, logo_bits );
        strncpy( ( char * ) &ui.displayBuffer, title, DISPLAY_BUFFER_SIZE );
        ui.displayBuffer[ DISPLAY_BUFFER_SIZE - 1 ] = '\0'; // Always set last char in buffer to NULL
        u8g2.setFont( u8g2_font_helvR10_tr );
        u8g2.drawStr( 34, 22, ui.displayBuffer );
    }
    while ( u8g2.nextPage() );

    // Delay to display startup screen properly
    // TODO: Move delay to main execution
    delay( 1500 );
}

/* ----------------------------------------------------------------------------
 * Function
 * --------------------------------------------------------------------------*/
void ui_drawTitleScreen( char* title )
{
    u8g2.firstPage();

    do
    {
        strncpy( ( char * ) &ui.displayBuffer, title, DISPLAY_BUFFER_SIZE );
        ui.displayBuffer[ DISPLAY_BUFFER_SIZE - 1 ] = '\0'; // Always set last char in buffer to NULL
        u8g2.setFont( u8g2_font_helvR10_tr );
        u8g2.drawStr( 12, 20, ui.displayBuffer );
    }
    while ( u8g2.nextPage() );

    // Delay to display title screen properly
    // TODO: Move delay to main execution
    delay( 1500 );
}

/* ----------------------------------------------------------------------------
 * Function
 * --------------------------------------------------------------------------*/
void ui_updateMainDisplay()
{
    u8g2.firstPage();
    do
    {
        switch( ui.currentScreen )
        {
            case SCREEN_SETTINGS:
                drawSettingsMenu();
                drawSettingNumber();
            break;
            case SCREEN_MAIN:
                drawThrottle();
                drawPage();
                drawBatteryLevel();
                drawSignal();
            break;
            default:
                // No default. What to do here?
            break;
        }
    }
    while ( u8g2.nextPage() );
}

/**
 * ****************************************************************************
 * PRIVATE FUNCTIONS
 * ****************************************************************************
 */

/* ----------------------------------------------------------------------------
 * Function
 * --------------------------------------------------------------------------*/
void drawSettingNumber()
{
    // Position on OLED
    int x = 2;
    int y = 10;

    // Draw current setting number box
    u8g2.drawRFrame( x + 102, y - 10, 22, 32, 4 );

    // Draw current setting number
    snprintf( ( char* )&ui.displayBuffer, DISPLAY_BUFFER_SIZE, "%d", ui.currentSetting + 1 );
    ui.displayBuffer[ DISPLAY_BUFFER_SIZE - 1 ] = '\0'; // Always set last char in buffer to NULL


    u8g2.setFont( u8g2_font_profont22_tn );
    u8g2.drawStr( x + 108, 22, ui.displayBuffer );
}

/* ----------------------------------------------------------------------------
 * Function
 * --------------------------------------------------------------------------*/
void drawSettingsMenu()
{
    // Position on OLED
    int x = 0;
    int y = 10;

    // Get setting from config table
    tSetting* setting = &setting_settings[ ui.currentSetting ];

    // Draw setting title
    strncpy( ( char* )&ui.displayBuffer, setting->name, 20 );
    ui.displayBuffer[ DISPLAY_BUFFER_SIZE - 1 ] = '\0'; // Always set last char in buffer to NULL
    u8g2.setFont( u8g2_font_profont12_tr );
    u8g2.drawStr( x, y, ui.displayBuffer );

    // Draw settings value
    int val = settings_getSetting( ui.currentSetting );
    snprintf( ( char* )&ui.displayBuffer, DISPLAY_BUFFER_SIZE, "%d %s", val, setting->unit );
    ui.displayBuffer[ DISPLAY_BUFFER_SIZE - 1 ] = '\0'; // Always set last char in buffer to NULL
    u8g2.setFont( u8g2_font_10x20_tr );

    if ( ui.changeSelectedSetting )
    {
        u8g2.drawStr( x + 10, y + 20, ui.displayBuffer );
    }
    else
    {
        u8g2.drawStr( x, y + 20, ui.displayBuffer );
    }
}

/* ----------------------------------------------------------------------------
 * Function
 * --------------------------------------------------------------------------*/
void drawPage()
{
    // Continuous variables
    static unsigned long lastDataRotation = 0;
    static int displayData                = 0;

    int decimals;
    float value;
    String suffix;
    String prefix;

    int first, last;

    int x = 0;
    int y = 16;

    // Rotate the realtime data each 4s.
    if ( ( millis() - lastDataRotation ) >= DATA_ROTATION_MS )
    {
        lastDataRotation = millis();
        displayData++;

        if ( displayData > 2 )
        {
            displayData = 0;
        }
    }

    switch ( displayData )
    {
        case 0:
            value = ratioRpmSpeed * data.rpm;
            suffix = "KMH";
            prefix = "SPEED";
            decimals = 1;
            break;
        case 1:
            value = ratioPulseDistance * data.tachometerAbs;
            suffix = "KM";
            prefix = "DISTANCE";
            decimals = 2;
            break;
        case 2:
            value = data.inpVoltage;
            suffix = "V";
            prefix = "BATTERY";
            decimals = 1;
            break;
    }

    // Display prefix (title)
    displayString = prefix;
    displayString.toCharArray( displayBuffer, 10 );
    u8g2.setFont( u8g2_font_profont12_tr );
    u8g2.drawStr( x, y - 1, displayBuffer );

    // Split up the float value: a number, b decimals.
    first = abs( floor( value ) );
    last = value * pow( 10, 3 ) - first * pow( 10, 3 );

    // Add leading zero
    if ( first <= 9 )
    {
        displayString = "0" + ( String )first;
    }
    else
    {
        displayString = ( String )first;
    }

    // Display numbers
    displayString.toCharArray( displayBuffer, 10 );
    u8g2.setFont( u8g2_font_logisoso22_tn );
    u8g2.drawStr( x + 55, y + 13, displayBuffer );

    // Display decimals
    displayString = "." + ( String )last;
    displayString.toCharArray( displayBuffer, decimals + 2 );
    u8g2.setFont( u8g2_font_profont12_tr );
    u8g2.drawStr( x + 86, y - 1, displayBuffer );

    // Display suffix
    displayString = suffix;
    displayString.toCharArray( displayBuffer, 10 );
    u8g2.setFont( u8g2_font_profont12_tr );
    u8g2.drawStr( x + 86 + 2, y + 13, displayBuffer );
}

/* ----------------------------------------------------------------------------
 * Function
 * --------------------------------------------------------------------------*/
void drawThrottle()
{
    int x = 0;
    int y = 18;

    // Draw throttle
    u8g2.drawHLine( x, y, 52 );
    u8g2.drawVLine( x, y, 10 );
    u8g2.drawVLine( x + 52, y, 10 );
    u8g2.drawHLine( x, y + 10, 5 );
    u8g2.drawHLine( x + 52 - 4, y + 10, 5 );

    if (throttle >= 127)
    {
        int width = map( throttle, 127, 255, 0, 49 );
        for ( int i = 0; i < width; i++ )
        {
            //if( (i % 2) == 0){
            u8g2.drawVLine( x + i + 2, y + 2, 7 );
            //}
        }
    }
    else
    {
        int width = map( throttle, 0, 126, 49, 0 );
        for ( int i = 0; i < width; i++ )
        {
            //if( (i % 2) == 0){
            u8g2.drawVLine( x + 50 - i, y + 2, 7 );
            //}
        }
    }
}

/* ----------------------------------------------------------------------------
 * Function
 * --------------------------------------------------------------------------*/
void drawSignal()
{
    // Position on OLED
    int x = 114;
    int y = 17;

    if ( connected == true )
    {
        if ( triggerActive() )
        {
            u8g2.drawXBM( x, y, 12, 12, signal_transmitting_bits );
        }
        else
        {
            u8g2.drawXBM( x, y, 12, 12, signal_connected_bits );
        }
    }
    else
    {
        if ( millis() - lastSignalBlink > 500 )
        {
            signalBlink = !signalBlink;
            lastSignalBlink = millis();
        }

        if (signalBlink == true)
        {
            u8g2.drawXBM( x, y, 12, 12, signal_connected_bits );
        }
        else
        {
            u8g2.drawXBM(x, y, 12, 12, signal_noconnection_bits);
        }
    }
}

/* ----------------------------------------------------------------------------
 * Function
 * --------------------------------------------------------------------------*/
void drawBatteryLevel()
{
    int level = batteryLevel();

    // Position on OLED
    int x = 108;
    int y = 4;

    u8g2.drawFrame( x + 2, y, 18, 9 );
    u8g2.drawBox( x, y + 2, 2, 5 );

    for ( int i = 0; i < 5; i++ )
    {
        int p = round( ( 100 / 5 ) * i );
        if ( p <= level )
        {
            u8g2.drawBox( x + 4 + ( 3 * i ), y + 2, 2, 5 );
        }
    }
}

