/**
 * @file   main.cpp
 * @author Benjamin Vedder, 2017
 * @author Simon Lövgren, 2018
 * 
 * @brief  Main entry point for nRF24 Esk8 Remote Transmitter.
 */

/**
 * ****************************************************************************
 * INCLUDES
 * ****************************************************************************
 */
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <SPI.h>
#include <EEPROM.h>
#include <RF24.h>
#include "VescUart.h"

#include "Ui/Ui.h"
#include "Settings/Settings.h"


/**
 * ****************************************************************************
 * DEFINES
 * ****************************************************************************
 */

// #define DEBUG

#ifdef DEBUG
    #define DEBUG_PRINT(x)    Serial.println (x)
    #include "printf.h"
#else
    #define DEBUG_PRINT(x)
#endif


/**
 * ****************************************************************************
 * TYPEDEFS
 * ****************************************************************************
 */



// Defining struct to hold stats 
struct stats {
    float maxSpeed;
    long maxRpm;
    float minVoltage;
    float maxVoltage;
};

// Defining struct to hold setting values while remote is turned on.
struct settings {
    byte triggerMode;
    byte batteryType;
    byte batteryCells;
    byte motorPoles;
    byte motorPulley;
    byte wheelPulley;
    byte wheelDiameter;
    bool useUart;
    int minHallValue;
    int centerHallValue;
    int maxHallValue;
};


/**
 * ****************************************************************************
 * PRIVATE VARIABLES
 * ****************************************************************************
 */

// Defining variables for speed and distance calculation
float gearRatio;
float ratioRpmSpeed;
float ratioPulseDistance;

const byte numOfSettings = 11;

String settingPages[numOfSettings][2] = {

};

// Setting rules format: default, min, max.
int settingRules[numOfSettings][3] {
    {0, 0, 3}, // 0 Killswitch, 1 cruise & 2 data toggle
    {0, 0, 1}, // 0 Li-ion & 1 LiPo
    {10, 0, 12},
    {14, 0, 250},
    {15, 0, 250},
    {40, 0, 250},
    {83, 0, 250},
    {1, 0, 1}, // Yes or no
    {0, 0, 1023},
    {512, 0, 1023},
    {1023, 0, 1023}
};

struct vescValues data;
struct settings remoteSettings;

// Pin defination
const byte triggerPin = 2;
const int chargeMeasurePin = 8;
const int batteryMeasurePin = A1;
const int hallSensorPin = A0;

// Battery monitering
const float minVoltage = 3.2;
const float maxVoltage = 4.1;
const float refVoltage = 5.0; // Set to 4.5V if you are testing connected to USB, otherwise 5V (or the supply voltage)

// Defining variables for Hall Effect throttle.
short hallMeasurement, throttle;
byte hallCenterMargin = 4;

// Defining variables for NRF24 communication
bool connected = false;
short failCount;
const uint64_t pipe = 0xE8E8F0F0E1LL; // If you change the pipe, you will need to update it on the receiver to.
unsigned long lastTransmission;


// Instantiating RF24 object for NRF24 communication
RF24 radio(9, 10);

// Defining variables for Settings menu
bool changeSettings = false;
bool changeSelectedSetting = false;

bool settingsLoopFlag = false;
bool settingsChangeFlag = false;
bool settingsChangeValueFlag = false;


/**
 * ****************************************************************************
 *    PROTOTYPES
 * ****************************************************************************
 */

void controlSettingsMenu();
void setDefaultEEPROMSettings();
void loadEEPROMSettings();
void updateEEPROMSettings();
void calculateRatios();
int getSettingValue(int index);
void setSettingValue(int index, int value);
bool inRange(int val, int minimum, int maximum);
boolean triggerActive();
void transmitToVesc();
void calculateThrottlePosition();
int batteryLevel();
float batteryVoltage();



/**
 * ****************************************************************************
 * INTERFACE FUNCTIONS
 * ****************************************************************************
 */

void setup() {
    // setDefaultEEPROMSettings(); // Call this function if you want to reset settings
    
    #ifdef DEBUG
        Serial.begin(9600);
    #endif
    
    loadEEPROMSettings();

    pinMode(triggerPin, INPUT_PULLUP);
    pinMode(hallSensorPin, INPUT);
    pinMode(batteryMeasurePin, INPUT);

    ui_drawStartScreen();

    if (triggerActive()) {
        changeSettings = true;
        ui_drawTitleScreen( "Remote Settings" );
    }

    // Start radio communication
    radio.begin();
    radio.setPALevel(RF24_PA_MAX);
    radio.enableAckPayload();
    radio.enableDynamicPayloads();
    radio.openWritingPipe(pipe);

    #ifdef DEBUG
        printf_begin();
        radio.printDetails();
    #endif
}

void loop() {
    
    calculateThrottlePosition();

    if (changeSettings == true) {
        // Use throttle and trigger to change settings
        controlSettingsMenu();
    }
    else
    {
        // Use throttle and trigger to drive motors
        if (triggerActive())
        {
            throttle = throttle;
        }
        else
        {
            // 127 is the middle position - no throttle and no brake/reverse
            throttle = 127;
        }
        // Transmit to receiver
        transmitToVesc();
    }

    // Call function to update display and LED
    ui_updateMainDisplay();
}


/**
 * ****************************************************************************
 * PRIVATE FUNCTIONS
 * ****************************************************************************
 */

void controlSettingsMenu() {
    if (triggerActive()) {
        if (settingsChangeFlag == false) {

            // Save settings to EEPROM
            if (changeSelectedSetting == true) {
                updateEEPROMSettings();
            }

            changeSelectedSetting = !changeSelectedSetting;
            settingsChangeFlag = true;
        }
    } else {
        settingsChangeFlag = false;
    }

    if (hallMeasurement >= (remoteSettings.maxHallValue - 150) && settingsLoopFlag == false) {
        // Up
        if (changeSelectedSetting == true) {
            int val = getSettingValue(currentSetting) + 1;

            if (inRange(val, settingRules[currentSetting][1], settingRules[currentSetting][2])) {
                setSettingValue(currentSetting, val);
                settingsLoopFlag = true;
            }
        } else {
            if (currentSetting != 0) {
                currentSetting--;
                settingsLoopFlag = true;
            }
        }
    }
    else if (hallMeasurement <= (remoteSettings.minHallValue + 150) && settingsLoopFlag == false) {
        // Down
        if (changeSelectedSetting == true) {
            int val = getSettingValue(currentSetting) - 1;

            if (inRange(val, settingRules[currentSetting][1], settingRules[currentSetting][2])) {
                setSettingValue(currentSetting, val);
                settingsLoopFlag = true;
            }
        } else {
            if (currentSetting < (numOfSettings - 1)) {
                currentSetting++;
                settingsLoopFlag = true;
            }
        }
    } else if (inRange(hallMeasurement, remoteSettings.centerHallValue - 50, remoteSettings.centerHallValue + 50)) {
        settingsLoopFlag = false;
    }
}

void setDefaultEEPROMSettings() {
    for (int i = 0; i < numOfSettings; i++) {
        setSettingValue(i, settingRules[i][0]);
    }

    updateEEPROMSettings();
}

void loadEEPROMSettings() {
    // Load settings from EEPROM to custom struct
    EEPROM.get(0, remoteSettings);

    bool rewriteSettings = false;

    // Loop through all settings to check if everything is fine
    for (int i = 0; i < numOfSettings; i++) {
        int val = getSettingValue(i);

        if (! inRange(val, settingRules[i][1], settingRules[i][2])) {
            // Setting is damaged or never written. Rewrite default.
            rewriteSettings = true;
            setSettingValue(i, settingRules[i][0]);
        }
    }

    if (rewriteSettings == true) {
        updateEEPROMSettings();
    } else {
        // Calculate constants
        calculateRatios();
    }
}

// Write settings to the EEPROM then exiting settings menu.
void updateEEPROMSettings() {
    EEPROM.put(0, remoteSettings);
    calculateRatios();
}

// Update values used to calculate speed and distance travelled.
void calculateRatios() {
    gearRatio = (float)remoteSettings.motorPulley / (float)remoteSettings.wheelPulley;

    ratioRpmSpeed = (gearRatio * 60 * (float)remoteSettings.wheelDiameter * 3.14156) / (((float)remoteSettings.motorPoles / 2) * 1000000); // ERPM to Km/h

    ratioPulseDistance = (gearRatio * (float)remoteSettings.wheelDiameter * 3.14156) / (((float)remoteSettings.motorPoles * 3) * 1000000); // Pulses to km travelled
}

// Get settings value by index (usefull when iterating through settings).
int getSettingValue(int index) {
    int value;
    switch (index) {
        case 0: value = remoteSettings.triggerMode;         break;
        case 1: value = remoteSettings.batteryType;         break;
        case 2: value = remoteSettings.batteryCells;        break;
        case 3: value = remoteSettings.motorPoles;            break;
        case 4: value = remoteSettings.motorPulley;         break;
        case 5: value = remoteSettings.wheelPulley;         break;
        case 6: value = remoteSettings.wheelDiameter;     break;
        case 7: value = remoteSettings.useUart;                 break;
        case 8: value = remoteSettings.minHallValue;        break;
        case 9: value = remoteSettings.centerHallValue; break;
        case 10: value = remoteSettings.maxHallValue;     break;
    }
    return value;
}

// Set a value of a specific setting by index.
void setSettingValue(int index, int value) {
    switch (index) {
        case 0: remoteSettings.triggerMode = value;         break;
        case 1: remoteSettings.batteryType = value;         break;
        case 2: remoteSettings.batteryCells = value;        break;
        case 3: remoteSettings.motorPoles = value;            break;
        case 4: remoteSettings.motorPulley = value;         break;
        case 5: remoteSettings.wheelPulley = value;         break;
        case 6: remoteSettings.wheelDiameter = value;     break;
        case 7: remoteSettings.useUart = value;                 break;
        case 8: remoteSettings.minHallValue = value;        break;
        case 9: remoteSettings.centerHallValue = value; break;
        case 10: remoteSettings.maxHallValue = value;     break;
    }
}

// Check if an integer is within a min and max value
bool inRange(int val, int minimum, int maximum) {
    return ((minimum <= val) && (val <= maximum));
}

// Return true if trigger is activated, false otherwice
boolean triggerActive() {
    if (digitalRead(triggerPin) == LOW)
        return true;
    else
        return false;
}

// Function used to transmit the throttle value, and receive the VESC realtime data.
void transmitToVesc() {
    // Transmit once every 50 millisecond
    if (millis() - lastTransmission >= 50) {

        lastTransmission = millis();

        boolean sendSuccess = false;
        // Transmit the speed value (0-255).
        sendSuccess = radio.write(&throttle, sizeof(throttle));

        // Listen for an acknowledgement reponse (return of VESC data).
        while (radio.isAckPayloadAvailable()) {
            radio.read(&data, sizeof(data));
        }

        if (sendSuccess == true)
        {
            // Transmission was a succes
            failCount = 0;
            sendSuccess = false;

            DEBUG_PRINT("Transmission succes");
        } else {
            // Transmission was not a succes
            failCount++;

            DEBUG_PRINT("Failed transmission");
        }

        // If lost more than 5 transmissions, we can assume that connection is lost.
        if (failCount < 5) {
            connected = true;
        } else {
            connected = false;
        }
    }
}

void calculateThrottlePosition() {
    // Hall sensor reading can be noisy, lets make an average reading.
    int total = 0;
    for (int i = 0; i < 10; i++) {
        total += analogRead(hallSensorPin);
    }
    hallMeasurement = total / 10;

    DEBUG_PRINT( (String)hallMeasurement );
    
    if (hallMeasurement >= remoteSettings.centerHallValue) {
        throttle = constrain(map(hallMeasurement, remoteSettings.centerHallValue, remoteSettings.maxHallValue, 127, 255), 127, 255);
    } else {
        throttle = constrain(map(hallMeasurement, remoteSettings.minHallValue, remoteSettings.centerHallValue, 0, 127), 0, 127);
    }

    // removeing center noise
    if (abs(throttle - 127) < hallCenterMargin) {
        throttle = 127;
    }
}

// Function used to indicate the remotes battery level.
int batteryLevel() {
    float voltage = batteryVoltage();

    if (voltage <= minVoltage) {
        return 0;
    } else if (voltage >= maxVoltage) {
        return 100;
    } else {
        return (voltage - minVoltage) * 100 / (maxVoltage - minVoltage);
    }
}

// Function to calculate and return the remotes battery voltage.
float batteryVoltage() {
    float batteryVoltage = 0.0;
    int total = 0;

    for (int i = 0; i < 10; i++) {
        total += analogRead(batteryMeasurePin);
    }

    batteryVoltage = (refVoltage / 1024.0) * ((float)total / 10.0);

    return batteryVoltage;
}
