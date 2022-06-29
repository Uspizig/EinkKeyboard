//works with ota as well
//works with C3 ESP3 as well
//Install the following libs before usage:
/*
 * Install Extra->SoftSPI in GxEPD2 before USAGE!!!: https://github.com/ZinggJM/GxEPD2/tree/master/extras/sw_spi
 * Webserial: https://github.com/ayushsharma82/WebSerial
 * Async WebServer https://github.com/me-no-dev/ESPAsyncWebServer
 * Fast LED https://github.com/FastLED/FastLED
 * WifiManager: https://github.com/tzapu/WiFiManager
 * Does not work with S3 without modifications in C:\Users\xxxx\AppData\Local\Arduino15\packages\esp32\hardware\esp32\2.0.3-RC1\cores\esp32\esp32-hal-misc.c Line 71
 */

/*
This Sketch tests basic functionality of the eink Keyboard
  A) USB Keyboard 
  B) Wifi Manager: to Enter Your Credentials and Keyboard Shortcuts
  C) All Keyboard Buttons
  D) RGB LEDs
  E) WiFi
  F) Bluetooth
  G) EInk Display
  H) Joystick
  I) SD Card Storage
  J) Wifi Manager
DISABLED: B) Web Serial: Enables a Debugging Console as a web Server

Missing: B) Wifi Manager if no valid Wifi Config is found a Wifi Server is started if GPIO3 is pressed during startup


Problems:
- OTA Update only works directly after RESET
*/


/*
 * Design of Wifi Manager input fields:
 *  # Wifi Passwort
 *  # API Key für Wetter/Google Script
 *  # 9 Keys +Shift + CTRL Checkbox
 *  # 10 LED Colors in #RGB + 10 LED Effekte Dropdown
 * 
 * - KeyBoard Polling Function
 *  #Returns: Returns which variable was pressed as an INT Variable 11, 33, 32
 * 
 * - KeyBoard Action 
 *  # Variables: Int 11, 22; LED Color, LED effect
 * 
 * - EINK Update
 *  # Show some values of the buttons
 *  # Übergabewert: Symbole an Position 1 - 9
 * 
 * - WEB Polling
 *  # Poll some API Data out of the WEB and write it to an Array of Strings
 *  
 *  - BLE Connectivity
 *  # Connects Keyboard as a BLE Keyboard
 */
    
//USB
#include "USB.h"
#include "USBHIDKeyboard.h"
USBHIDKeyboard Keyboard;


//BrownOut Disable
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
//Task Watchdog
#include <esp_task_wdt.h>

//LEDS
#include "FastLED.h"
#include "FastLED_RGBW.h"

// ----------------------------
// Library Defines - Need to be defined before library import
// ----------------------------

//#define ESP_DRD_USE_SPIFFS true

// ----------------------------
// Standard Libraries - Already Installed if you have ESP32 set up
// ----------------------------

#include <WiFi.h>
#include <FS.h>
#include <SPIFFS.h>

#include "OTA.h" //Ota Update
#include "credentials.h" //Pin Definitions


//Web Serial
//#include <WebSerial.h>
//AsyncWebServer WebSerialserver(80);

//Joystick Variablen
int SensorWertx = 0;   // Festlegen der Variablen
int SensorWerty = 0;   // Festlegen der Variablen 




//LED Definitions
#define NUM_LEDS ANZAHL_LEDS
#define DATA_PIN RGB_LED_DATA_PIN
CRGBW leds[NUM_LEDS];
CRGB *ledsRGB = (CRGB *) &leds[0];
const uint8_t brightness = 128;
uint8_t i=0;
uint8_t hue, hue2 = 0;
long lastMsg = 0;
long lastMsg2 = 0;
boolean button_pressed =1;
boolean zielwert_erreicht =0;

//Definitionen für Keyboard
uint8_t keyboard_val = 0;


//Eink Definitions
//Install Extra SoftSPI in GxEPD2 before USAGE!!!
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
//#include <Fonts/FreeSansBold12pt7b.h>
//#include <Fonts/FreeSerifBold12pt7b.h>

const GFXfont* Schrift = &FreeMonoBold12pt7b;
const GFXfont* Schrift2 = &FreeMonoBold9pt7b;
const GFXfont* Schrift3 = &FreeMonoBold18pt7b; 
const GFXfont* Schrift4 = &FreeMono9pt7b; 
//const GFXfont* Schrift5 = &FreeSansBold12pt7b; 
//const GFXfont* Schrift6 = &FreeSerifBold12pt7b;
#include <SPI.h> 

//SD Card
#include "FS.h"
#include "SD.h"

//GxEPD2_BW<GxEPD2_290_T5, GxEPD2_290_T5::HEIGHT> gfx(GxEPD2_290_T5(/*CS=5*/ E_INK_PIN_SPI_CS, /*DC=*/ E_INK_PIN_SPI_DC, /*RST=*/ E_INK_PIN_SPI_RST, /*BUSY=*/ E_INK_PIN_SPI_BUSY));
//GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> gfx(GxEPD2_290(E_INK_PIN_SPI_CS, E_INK_PIN_SPI_DC, E_INK_PIN_SPI_RST, E_INK_PIN_SPI_BUSY));

//geht
GxEPD2_BW<GxEPD2_270, GxEPD2_270::HEIGHT> gfx(GxEPD2_270(E_INK_PIN_SPI_CS, E_INK_PIN_SPI_DC, E_INK_PIN_SPI_RST, E_INK_PIN_SPI_BUSY));
//GxEPD2_BW<GxEPD2_290_T94_V2, GxEPD2_290_T94_V2::HEIGHT> gfx(GxEPD2_290_T94_V2(/*CS=5*/ E_INK_PIN_SPI_CS, /*DC=*/ E_INK_PIN_SPI_DC, /*RST=*/ E_INK_PIN_SPI_RST, /*BUSY=*/ E_INK_PIN_SPI_BUSY));//Charge Juli 2021 geht
//GxEPD2_BW<GxEPD2_290_T94, GxEPD2_290_T94::HEIGHT> gfx(GxEPD2_290_T94(/*CS=5*/ E_INK_PIN_SPI_CS, /*DC=*/ E_INK_PIN_SPI_DC, /*RST=*/ E_INK_PIN_SPI_RST, /*BUSY=*/ E_INK_PIN_SPI_BUSY));//Charge Juli 2021
/*------------------------------------------ EINK DISPLAY TEIL--------------------------------------------------------------*/
bool LargeIcon =  true;
bool SmallIcon =  false;
int fail_counter = 0;



// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------

#include <WiFiManager.h>
// Captive portal for configuring the WiFi

// Can be installed from the library manager (Search for "WifiManager", install the Alhpa version)
// https://github.com/tzapu/WiFiManager

//#include <ESP_DoubleResetDetector.h>
// A library for checking if the reset button has been pressed twice
// Can be used to enable config mode
// Can be installed from the library manager (Search for "ESP_DoubleResetDetector")
//https://github.com/khoih-prog/ESP_DoubleResetDetector

#include <ArduinoJson.h>
// ArduinoJson is used for parsing and creating the config file.
// Search for "Arduino Json" in the Arduino Library manager
// https://github.com/bblanchon/ArduinoJson

// -------------------------------------
// -------   Other Config   ------
// -------------------------------------

//const int PIN_LED = 2;

#define JSON_CONFIG_FILE "/sample_config.json"

// Number of seconds after reset during which a
// subseqent reset will be considered a double reset.
//#define DRD_TIMEOUT 10

// RTC Memory Address for the DoubleResetDetector to use
//#define DRD_ADDRESS 0

/* -----------------------------
Arichtektu:
- 1 API Key
- 1 Location
- 9 Tasten [1 char]
- 9 Zusatztasten (Checkbox)
- 9 Farben in int 0-255 = HSV
*/

//DoubleResetDetector *drd;

//flag for saving data
bool shouldSaveConfig = false;

char testString[50] = "default APIKey-value";

char Button11[5] = "A";
char Button12[5] = "B";
char Button13[5] = "C";

char Button21[5] = "D";
char Button22[5] = "E";
char Button23[5] = "F";

char Button31[5] = "G";
char Button32[5] = "H";
char Button33[5] = "I";

int testNumber = 1500;
int testNumber2 = 1500;

int LEDColor11 = 150;
int LEDColor12 = 160;
int LEDColor13 = 170;

int LEDColor21 = 180;
int LEDColor22 = 190;
int LEDColor23 = 200;

int LEDColor31 = 210;
int LEDColor32 = 220;
int LEDColor33 = 230;

bool testBool = true;

bool Checker11 =  false;
bool Checker12 =  false;
bool Checker13 =  false;

bool Checker21 =  false;
bool Checker22 =  false;
bool Checker23 =  false;

bool Checker31 =  false;
bool Checker32 =  false;
bool Checker33 =  false;

int day = 3; // 0 == Monday etc
char *daysOfWeek[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};

void saveConfigFile()
{
  Serial.println(F("Saving config"));
  StaticJsonDocument<1024> json;
  json["testString"] = testString;
  json["testNumber"] = testNumber;
  json["testBool"] = testBool;
  json["day"] = day;

  json["Button11"] =  Button11;
  json["Button12"] =  Button12;
  json["Button13"] =  Button13;

  json["Button21"] =  Button21;
  json["Button22"] =  Button22;
  json["Button23"] =  Button23;
  
  json["Button31"] =  Button31;
  json["Button32"] =  Button32;
  json["Button33"] =  Button33;

  json["LEDColor11"] =  LEDColor11;
  json["LEDColor12"] =  LEDColor12;
  json["LEDColor13"] =  LEDColor13;

  json["LEDColor21"] =  LEDColor21;
  json["LEDColor22"] =  LEDColor22;
  json["LEDColor23"] =  LEDColor23;

  json["LEDColor31"] =  LEDColor31;
  json["LEDColor32"] =  LEDColor32;
  json["LEDColor33"] =  LEDColor33;

  json["Checker11"] =  Checker11;
  json["Checker12"] =  Checker12;
  json["Checker13"] =  Checker13;

  json["Checker21"] =  Checker21;
  json["Checker22"] =  Checker22;
  json["Checker23"] =  Checker23;

  json["Checker31"] =  Checker31;
  json["Checker32"] =  Checker32;
  json["Checker33"] =  Checker33;

  
  File configFile = SPIFFS.open(JSON_CONFIG_FILE, "w");
  if (!configFile)
  {
    Serial.println("failed to open config file for writing");
  }

  serializeJsonPretty(json, Serial);
  if (serializeJson(json, configFile) == 0)
  {
    Serial.println(F("Failed to write to file"));
  }
  configFile.close();
}

bool loadConfigFile()
{
  //clean FS, for testing
  // SPIFFS.format();

  //read configuration from FS json
  Serial.println("mounting FS...");

  // May need to make it begin(true) first time you are using SPIFFS
  // NOTE: This might not be a good way to do this! begin(true) reformats the spiffs
  // it will only get called if it fails to mount, which probably means it needs to be
  // formatted, but maybe dont use this if you have something important saved on spiffs
  // that can't be replaced.
  if (SPIFFS.begin(false) || SPIFFS.begin(true))
  {
    Serial.println("mounted file system");
    if (SPIFFS.exists(JSON_CONFIG_FILE))
    {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open(JSON_CONFIG_FILE, "r");
      if (configFile)
      {
        Serial.println("opened config file");
        StaticJsonDocument<1024> json;
        DeserializationError error = deserializeJson(json, configFile);
        serializeJsonPretty(json, Serial);
        if (!error)
        {
          Serial.println("\nparsed json");

          strcpy(testString, json["testString"]);
          testNumber = json["testNumber"].as<int>();
          testBool = json["testBool"].as<bool>();
          day = json["day"].as<int>();

          strcpy(Button11 ,json["Button11"]);
          strcpy(Button12 ,json["Button12"]);
          strcpy(Button13 ,json["Button13"]);
          strcpy(Button21 ,json["Button21"]);
          strcpy(Button22 ,json["Button22"]);
          strcpy(Button23 ,json["Button23"]);
          strcpy(Button31 ,json["Button31"]);
          strcpy(Button32 ,json["Button32"]);
          strcpy(Button33 ,json["Button33"]);

          LEDColor11 = json["LEDColor11"].as<int>();
          LEDColor12 = json["LEDColor12"].as<int>();
          LEDColor13 = json["LEDColor13"].as<int>();
          LEDColor21 = json["LEDColor21"].as<int>();
          LEDColor22 = json["LEDColor22"].as<int>();
          LEDColor23 = json["LEDColor23"].as<int>();
          LEDColor31 = json["LEDColor31"].as<int>();
          LEDColor32 = json["LEDColor32"].as<int>();
          LEDColor33 = json["LEDColor33"].as<int>();

          Checker11 = json["Checker11"].as<bool>();
          Checker12 = json["Checker12"].as<bool>();
          Checker13 = json["Checker13"].as<bool>();
          Checker21 = json["Checker21"].as<bool>();
          Checker22 = json["Checker22"].as<bool>();
          Checker23 = json["Checker23"].as<bool>();
          Checker31 = json["Checker31"].as<bool>();
          Checker32 = json["Checker32"].as<bool>();
          Checker33 = json["Checker33"].as<bool>();

          return true;
        }
        else
        {
          Serial.println("failed to load json config");
        }
      }
    }
  }
  else
  {
    Serial.println("failed to mount FS");
  }
  //end read
  return false;
}

//callback notifying us of the need to save config
void saveConfigCallback()
{
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

// This gets called when the config mode is launced, might
// be useful to update a display with this info.
void configModeCallback(WiFiManager *myWiFiManager)
{
  Serial.println("Entered Conf Mode");

  Serial.print("Config SSID: ");
  Serial.println(myWiFiManager->getConfigPortalSSID());

  Serial.print("Config IP Address: ");
  Serial.println(WiFi.softAPIP());
}

// Custom HTML WiFiManagerParameter don't support getValue directly
String getCustomParamValue(WiFiManager *myWiFiManager, String name)
{
  String value;

  int numArgs = myWiFiManager->server->args();
  for (int i = 0; i < numArgs; i++) {
    Serial.println(myWiFiManager->server->arg(i));
  }
  if (myWiFiManager->server->hasArg(name))
  {
    value = myWiFiManager->server->arg(name);
  }
  return value;
}

void setup()
{
  Serial.begin(115200);
  Serial.println("starting up");
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);//BrownOut Disable
  esp_task_wdt_init(60, false); //Task Watchdog set to 30sec
  
  LED_Setup(); //start the LEDS
  rainbowLoop();
  FastLED.show();
  Serial.println("LED on");
  
  Serial.print("Dateiname: ");
  Serial.println(__FILE__); //https://forum.arduino.cc/t/which-program-am-i-running/410688/18
  
  //pinMode(PIN_LED, OUTPUT);

  //SD Start
  SPI.begin(SD_CARD_PIN_CLK, SD_CARD_PIN_DAT0, SD_CARD_PIN_CMD); //CLK, MISO, MOSI
  if(!SD.begin(SD_CARD_PIN_DAT3, SPI, 4000000)){ //Chip Select
    Serial.println("Card Mount Failed");
  }

  uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }

  

  bool forceConfig = false;
/*
  drd = new DoubleResetDetector(DRD_TIMEOUT, DRD_ADDRESS);
  if (drd->detectDoubleReset())
  {
    Serial.println(F("Forcing config mode as there was a Double reset detected"));
    forceConfig = true;
  }
*/
  //Erkennung von Setup über PIN
  pinMode(3, INPUT);
  int setup_pin = 0;
  setup_pin = digitalRead(3);
  if (setup_pin){forceConfig = true;} //TEST USPIZIG
  else{forceConfig = false;}
  
  bool spiffsSetup = loadConfigFile();
  if (!spiffsSetup)
  {
    Serial.println(F("Forcing config mode as there is no saved config"));
    forceConfig = true;
  }

  //WiFi.disconnect();
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  
  delay(10);
  
  //Here erase setting always USPIZIG
   //wm.resetSettings(); // wipe settings

  WiFiManager wm;

  //wm.resetSettings(); // wipe settings
  //set config save notify callback
  wm.setSaveConfigCallback(saveConfigCallback);
  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wm.setAPCallback(configModeCallback);

  //--- additional Configs params ---

  // Text box (String)
  WiFiManagerParameter custom_text_box("key_text", "Enter your ApiKey here", testString, 50); // 50 == max length
  
  WiFiManagerParameter Button11_text_box("key_text_Button11", "Shortcut Button11", Button11, 5); // 5 == max length
  WiFiManagerParameter Button12_text_box("key_text_Button12", "Shortcut Button12", Button12, 5); // 5 == max length
  WiFiManagerParameter Button13_text_box("key_text_Button13", "Shortcut Button13", Button13, 5); // 5 == max length

  WiFiManagerParameter Button21_text_box("key_text_Button21", "Shortcut Button21", Button21, 5); // 5 == max length
  WiFiManagerParameter Button22_text_box("key_text_Button22", "Shortcut Button22", Button22, 5); // 5 == max length
  WiFiManagerParameter Button23_text_box("key_text_Button23", "Shortcut Button23", Button23, 5); // 5 == max length

  WiFiManagerParameter Button31_text_box("key_text_Button31", "Shortcut Button31", Button31, 5); // 5 == max length
  WiFiManagerParameter Button32_text_box("key_text_Button32", "Shortcut Button32", Button32, 5); // 5 == max length
  WiFiManagerParameter Button33_text_box("key_text_Button33", "Shortcut Button33", Button33, 5); // 5 == max length

  // Text box (Number)
  char convertedValue[6];
  char convertedValue2[6];
  char convertedValueLEDColor11[6];
  char convertedValueLEDColor12[6];
  char convertedValueLEDColor13[6];
  char convertedValueLEDColor21[6];
  char convertedValueLEDColor22[6];
  char convertedValueLEDColor23[6];
  char convertedValueLEDColor31[6];
  char convertedValueLEDColor32[6];
  char convertedValueLEDColor33[6];
  sprintf(convertedValue, "%d", testNumber); // Need to convert to string to display a default value.
  sprintf(convertedValue2, "%d", testNumber); // Need to convert to string to display a default value.
  sprintf(convertedValueLEDColor11, "%d", LEDColor11); // Need to convert to string to display a default value.
  sprintf(convertedValueLEDColor12, "%d", LEDColor12); // Need to convert to string to display a default value.
  sprintf(convertedValueLEDColor13, "%d", LEDColor13); // Need to convert to string to display a default value.
  sprintf(convertedValueLEDColor21, "%d", LEDColor21); // Need to convert to string to display a default value.
  sprintf(convertedValueLEDColor22, "%d", LEDColor22); // Need to convert to string to display a default value.
  sprintf(convertedValueLEDColor23, "%d", LEDColor23); // Need to convert to string to display a default value.
  sprintf(convertedValueLEDColor31, "%d", LEDColor31); // Need to convert to string to display a default value.
  sprintf(convertedValueLEDColor32, "%d", LEDColor32); // Need to convert to string to display a default value.
  sprintf(convertedValueLEDColor33, "%d", LEDColor33); // Need to convert to string to display a default value.

  WiFiManagerParameter custom_text_box_num("key_num", "Enter your number here", convertedValue, 7); // 7 == max length
  WiFiManagerParameter custom_text_box_num2("key_num2", "Bitte Zahl eingeben", convertedValue2, 7); // 7 == max length uspizig

  WiFiManagerParameter custom_text_box_ValueLEDColor11("LED11", "Farbe HSV LED11", convertedValueLEDColor11, 7); // 7 == max length uspizig
  WiFiManagerParameter custom_text_box_ValueLEDColor12("LED12", "Farbe HSV LED12", convertedValueLEDColor12, 7); // 7 == max length uspizig
  WiFiManagerParameter custom_text_box_ValueLEDColor13("LED13", "Farbe HSV LED13", convertedValueLEDColor13, 7); // 7 == max length uspizig

  WiFiManagerParameter custom_text_box_ValueLEDColor21("LED21", "Farbe HSV LED21", convertedValueLEDColor21, 7); // 7 == max length uspizig
  WiFiManagerParameter custom_text_box_ValueLEDColor22("LED22", "Farbe HSV LED22", convertedValueLEDColor22, 7); // 7 == max length uspizig
  WiFiManagerParameter custom_text_box_ValueLEDColor23("LED23", "Farbe HSV LED32", convertedValueLEDColor23, 7); // 7 == max length uspizig

  WiFiManagerParameter custom_text_box_ValueLEDColor31("LED31", "Farbe HSV LED31", convertedValueLEDColor31, 7); // 7 == max length uspizig
  WiFiManagerParameter custom_text_box_ValueLEDColor32("LED32", "Farbe HSV LED32", convertedValueLEDColor32, 7); // 7 == max length uspizig
  WiFiManagerParameter custom_text_box_ValueLEDColor33("LED33", "Farbe HSV LED33", convertedValueLEDColor33, 7); // 7 == max length uspizig

  //Check Box
  char *customHtml;
  if (testBool)
  {
    customHtml = "type=\"checkbox\" checked";
  }
  else
  {
    customHtml = "type=\"checkbox\"";
  }
  WiFiManagerParameter custom_checkbox("key_bool", "Checkbox", "T", 2, customHtml); 
  // The "t" isn't really important, but if the
  // box is checked the value for this field will
  // be "t", so we can check for that.

  WiFiManagerParameter custom_checkbox_Checker11("key_bool_11", "SHIFT+CTRL 11", "T", 2, customHtml); 
  WiFiManagerParameter custom_checkbox_Checker12("key_bool_12", "SHIFT+CTRL 12", "T", 2, customHtml); 
  WiFiManagerParameter custom_checkbox_Checker13("key_bool_13", "SHIFT+CTRL 13", "T", 2, customHtml); 
  WiFiManagerParameter custom_checkbox_Checker21("key_bool_21", "SHIFT+CTRL 21", "T", 2, customHtml); 
  WiFiManagerParameter custom_checkbox_Checker22("key_bool_22", "SHIFT+CTRL 22", "T", 2, customHtml); 
  WiFiManagerParameter custom_checkbox_Checker23("key_bool_23", "SHIFT+CTRL 23", "T", 2, customHtml); 
  WiFiManagerParameter custom_checkbox_Checker31("key_bool_31", "SHIFT+CTRL 31", "T", 2, customHtml); 
  WiFiManagerParameter custom_checkbox_Checker32("key_bool_32", "SHIFT+CTRL 32", "T", 2, customHtml); 
  WiFiManagerParameter custom_checkbox_Checker33("key_bool_33", "SHIFT+CTRL 33", "T", 2, customHtml); 
  

  //Select menu (custom HTML)
  // The custom html options do not get handled the same way as other standard ones

  const char *day_select_str = R"(
  <br/><label for='day'>Custom Field Label</label>
  <select name="dayOfWeek" id="day" onchange="document.getElementById('key_custom').value = this.value">
    <option value="0">Monday</option>
    <option value="1">Tuesday</option>
    <option value="2">Wednesday</option>
    <option value="3">Thursday</option>
    <option value="4">Friday</option>
    <option value="5">Saturday</option>
    <option value="6">Sunday</option>
  </select>
  <script>
    document.getElementById('day').value = "%d";
    document.querySelector("[for='key_custom']").hidden = true;
    document.getElementById('key_custom').hidden = true;
  </script>
  )";

  char bufferStr[700];
  // The sprintf is so we can input the value of the current selected day
  // If you dont need to do that, then just pass the const char* straight in.
  sprintf(bufferStr, day_select_str, day);

  Serial.print(bufferStr);

  WiFiManagerParameter custom_field(bufferStr);


  // Hidden field to get the data
  sprintf(convertedValue, "%d", day); // Need to convert to string to display a default value.

  WiFiManagerParameter custom_hidden("key_custom", "Will be hidden", convertedValue, 2);

  //add all your parameters here
  wm.addParameter(&custom_text_box);
  
  wm.addParameter(&Button11_text_box);
  wm.addParameter(&Button12_text_box);
  wm.addParameter(&Button13_text_box);
  wm.addParameter(&Button21_text_box);
  wm.addParameter(&Button22_text_box);
  wm.addParameter(&Button23_text_box);
  wm.addParameter(&Button31_text_box);
  wm.addParameter(&Button32_text_box);
  wm.addParameter(&Button33_text_box);
  
  wm.addParameter(&custom_text_box_num);
  wm.addParameter(&custom_text_box_num2);

  wm.addParameter(&custom_text_box_ValueLEDColor11);
  wm.addParameter(&custom_text_box_ValueLEDColor12);
  wm.addParameter(&custom_text_box_ValueLEDColor13);
  wm.addParameter(&custom_text_box_ValueLEDColor21);
  wm.addParameter(&custom_text_box_ValueLEDColor22);
  wm.addParameter(&custom_text_box_ValueLEDColor23);
  wm.addParameter(&custom_text_box_ValueLEDColor31);
  wm.addParameter(&custom_text_box_ValueLEDColor32);
  wm.addParameter(&custom_text_box_ValueLEDColor33);
  
  wm.addParameter(&custom_checkbox);
  wm.addParameter(&custom_checkbox_Checker11);
  wm.addParameter(&custom_checkbox_Checker12);
  wm.addParameter(&custom_checkbox_Checker13);
  wm.addParameter(&custom_checkbox_Checker21);
  wm.addParameter(&custom_checkbox_Checker22);
  wm.addParameter(&custom_checkbox_Checker23);
  wm.addParameter(&custom_checkbox_Checker31);
  wm.addParameter(&custom_checkbox_Checker32);
  wm.addParameter(&custom_checkbox_Checker33);
  
  wm.addParameter(&custom_hidden); // Needs to be added before the javascript that hides it
  wm.addParameter(&custom_field);

  Serial.println("hello");

  //digitalWrite(PIN_LED, LOW);
  if (forceConfig)
  {
    if (!wm.startConfigPortal("WifiTetris", "clock123"))
    {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.restart();
      delay(5000);
    }
  }
  else
  {
    if (!wm.autoConnect("WifiTetris", "clock123"))
    {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      // if we still have not connected restart and try all over again
      ESP.restart();
      delay(5000);
    }
  }

  // If we get here, we are connected to the WiFi
  //digitalWrite(PIN_LED, HIGH);

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  //setupOTA("ESP32KEYBOARD");

  //save the custom parameters to FS
  if (shouldSaveConfig)
  {
    // Lets deal with the user config values

    // Copy the string value
    strncpy(testString, custom_text_box.getValue(), sizeof(testString));
    Serial.print("testString: ");
    Serial.println(testString);

    strncpy(Button11, Button11_text_box.getValue(), sizeof(Button11));
    strncpy(Button12, Button12_text_box.getValue(), sizeof(Button12));
    strncpy(Button13, Button13_text_box.getValue(), sizeof(Button13));

    strncpy(Button21, Button21_text_box.getValue(), sizeof(Button21));
    strncpy(Button22, Button22_text_box.getValue(), sizeof(Button22));
    strncpy(Button23, Button23_text_box.getValue(), sizeof(Button23));

    strncpy(Button31, Button31_text_box.getValue(), sizeof(Button31));
    strncpy(Button32, Button32_text_box.getValue(), sizeof(Button32));
    strncpy(Button33, Button33_text_box.getValue(), sizeof(Button33));

    //Convert the number value
    testNumber = atoi(custom_text_box_num.getValue());
    Serial.print("testNumber: ");
    Serial.println(testNumber);

    //Convert the Nummer 2 Werte
    testNumber = atoi(custom_text_box_num2.getValue());
    Serial.print("testNumber: ");
    Serial.println(testNumber);

    LEDColor11 = atoi(custom_text_box_ValueLEDColor11.getValue());
    LEDColor12 = atoi(custom_text_box_ValueLEDColor12.getValue());
    LEDColor13 = atoi(custom_text_box_ValueLEDColor13.getValue());
    LEDColor21 = atoi(custom_text_box_ValueLEDColor21.getValue());
    LEDColor22 = atoi(custom_text_box_ValueLEDColor22.getValue());
    LEDColor23 = atoi(custom_text_box_ValueLEDColor23.getValue());
    LEDColor31 = atoi(custom_text_box_ValueLEDColor31.getValue());
    LEDColor32 = atoi(custom_text_box_ValueLEDColor32.getValue());
    LEDColor33 = atoi(custom_text_box_ValueLEDColor33.getValue());
    

    //Handle the bool value
    testBool = (strncmp(custom_checkbox.getValue(), "T", 1) == 0);

    Checker11 = (strncmp(custom_checkbox_Checker11.getValue(), "T", 1) == 0);
    Checker12 = (strncmp(custom_checkbox_Checker12.getValue(), "T", 1) == 0);
    Checker13 = (strncmp(custom_checkbox_Checker13.getValue(), "T", 1) == 0);
    Checker21 = (strncmp(custom_checkbox_Checker21.getValue(), "T", 1) == 0);
    Checker22 = (strncmp(custom_checkbox_Checker22.getValue(), "T", 1) == 0);
    Checker23 = (strncmp(custom_checkbox_Checker23.getValue(), "T", 1) == 0);
    Checker31 = (strncmp(custom_checkbox_Checker31.getValue(), "T", 1) == 0);
    Checker32 = (strncmp(custom_checkbox_Checker32.getValue(), "T", 1) == 0);
    Checker33 = (strncmp(custom_checkbox_Checker33.getValue(), "T", 1) == 0);
    
    Serial.print("testBool: ");
    if (testBool)
    {
      Serial.println("true");
    }
    else
    {
      Serial.println("false");
    }

    //The custom one
    day = atoi(custom_hidden.getValue());
    Serial.print("Selected Day: ");
    Serial.println(daysOfWeek[day]);


    saveConfigFile();
  }
  Display_Start(); //Initalisiert das Display
  Keyboard.begin();
  USB.begin();
  Serial.println("USB fertig");
  Serial.println("setup fertig");
}

void loop()
{
  Joystick_Read();
  delay(300);
  //drd->loop();
   //ArduinoOTA.handle();
   
   keyboard_val = key_test();
   //Display_Keyboard_Values();
   if (keyboard_val != 0){
    Serial.print("Key: "); Serial.println(keyboard_val);
    press_my_keys();
   }
   //Display_Joystick_Values();
}

void press_my_keys(void){
  switch (keyboard_val){
    case 11: if(Checker11){Keyboard.press(KEY_LEFT_CTRL); Keyboard.press(KEY_LEFT_SHIFT); Keyboard.print(Button11);}else{Keyboard.print(Button11);} break;
    case 12: if(Checker12){Keyboard.press(KEY_LEFT_CTRL); Keyboard.press(KEY_LEFT_SHIFT); Keyboard.print(Button12);}else{Keyboard.print(Button12);} break;
    case 13: if(Checker13){Keyboard.press(KEY_LEFT_CTRL); Keyboard.press(KEY_LEFT_SHIFT); Keyboard.print(Button13);}else{Keyboard.print(Button13);} break;
    case 21: if(Checker21){Keyboard.press(KEY_LEFT_CTRL); Keyboard.press(KEY_LEFT_SHIFT); Keyboard.print(Button21);}else{Keyboard.print(Button21);} break;
    case 22: if(Checker22){Keyboard.press(KEY_LEFT_GUI); Keyboard.print(Button22);}else{Keyboard.print(Button22);} break;
    case 23: if(Checker23){Keyboard.press(KEY_LEFT_CTRL); Keyboard.press(KEY_LEFT_SHIFT); Keyboard.print(Button23);}else{Keyboard.print(Button23);} break;
    case 31: if(Checker31){Keyboard.press(KEY_LEFT_CTRL); Keyboard.press(KEY_LEFT_SHIFT); Keyboard.print(Button31);}else{Keyboard.print(Button31);} break;
    case 32: if(Checker32){Keyboard.press(KEY_LEFT_CTRL); Keyboard.press(KEY_LEFT_SHIFT); Keyboard.print(Button32);}else{Keyboard.print(Button32);} break;
    case 33: if(Checker33){Keyboard.press(KEY_LEFT_CTRL); Keyboard.press(KEY_LEFT_SHIFT); Keyboard.print(Button33);}else{Keyboard.print(Button33);} break;
    default:  break;
  }
  delay(50); 
  Keyboard.releaseAll();
  keyboard_val = 0;
  /*Keyboard.press('b');
  delay(100);
  Keyboard.releaseAll();
  Keyboard.write(KEY_RETURN);*/
}


void LED_Setup(){
  FastLED.addLeds<WS2812B, DATA_PIN, RGB>(ledsRGB, getRGBWsize(NUM_LEDS));
  FastLED.setBrightness(brightness);
  FastLED.show();
}

void rainbow(){
  static uint8_t hue;
  for(int i = 0; i < NUM_LEDS; i++){
    leds[i] = CHSV((i * 256 / NUM_LEDS) + hue, 255, 55);
  }
  FastLED.show();
  hue++;
}
 
void rainbowLoop(){
  long millisIn = millis();
  long loopTime = 5000; // 5 seconds
 
  while(millis() < millisIn + loopTime){
    rainbow();
    delay(5);
  }
}



int key_test(void){
    String str;
    int rueckgabe = 0;
    int nRow = 3;
    int nCol = 3;
    int KeyBoard[nRow][nCol] = {
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0}
    };
    uint8_t colPins[3] = {COL1, COL2, COL3}; //connect to the column pinouts of the keypad
    pinMode(COL1, OUTPUT);
    pinMode(COL2, OUTPUT);
    pinMode(COL3, OUTPUT);
    pinMode(ROW1, INPUT);
    pinMode(ROW2, INPUT);
    pinMode(ROW3, INPUT);
    //Abfrage der Tasten
    for (int i = 0; i < nCol; i++){
      digitalWrite(colPins[i], LOW);  
      KeyBoard[i][0] = digitalRead(ROW1); 
      KeyBoard[i][1] = digitalRead(ROW2); 
      KeyBoard[i][2] = digitalRead(ROW3); 
      digitalWrite(colPins[i], HIGH);  
      /*Serial.println(i);
      Serial.println(KeyBoard[i][0]);
      Serial.println(KeyBoard[i][1]);
      Serial.println(KeyBoard[i][2]);
      */
    }
    
    
    //DebugAusgabe
    //#define WEB_DEBUG_SERIAL 
     for (int i = 0; i < nCol; i++){
      for (int b = 0; b < nRow; b++){
        int a = i+1; int c = b+1;
        if (KeyBoard[i][b] < KEYPRESS_DETECTION_VALUE){         
          #ifdef WEB_DEBUG_SERIAL
            Serial.print(str + a + c +":" + KeyBoard[i][b] +" \n");
          #endif
          rueckgabe = (a*10)+c;
        }
        }
      }
    return rueckgabe;
}


void Joystick_Read(){
       //https://www.esp32.com/viewtopic.php?t=18989
       /*SensorWertx = analogRead(X1_PIN);  //  Hier : VRx - X-Achse
       SensorWerty = analogRead(Y1_PIN); // Hier : VRy - Y-Achse
       Serial.print("X1:");  Serial.print(SensorWertx, DEC); 
       Serial.print(" |Y1:");  Serial.print(SensorWerty, DEC);*/
       String stringer;
       SensorWertx = analogRead(X2_PIN);  //  Hier : VRx - X-Achse
       SensorWerty = analogRead(Y2_PIN); // Hier : VRy - Y-Achse
       //Serial.println(stringer + "Joy x:" + SensorWertx +" y:" + SensorWerty); 
}

//Initalisiert das E-Ink Display mit den wichtigsten Grundparametern
void Display_Start(void){
  int zahlen;
  gfx.epd2.init(E_INK_PIN_SPI_SCK, E_INK_PIN_SPI_DIN, 115200, true, 20, false); 
  delay(100);
  gfx.setRotation(0); gfx.fillScreen(GxEPD_WHITE); gfx.setTextColor(GxEPD_BLACK);
  gfx.fillCircle(15,70,50, GxEPD_BLACK);
  Serial.println("Circle");
  gfx.fillRect(0, 0, 120, 80, GxEPD_BLACK);
  Serial.println("RECT");
  Serial.println("Display setup done");
  gfx.display(false); 
  gfx.drawFastHLine(0, 115, gfx.width(), GxEPD_BLACK); //Linie horizontal Volle Breite
  gfx.setCursor(70, 90); gfx.setFont(Schrift2); 
  gfx.setRotation(0);
  gfx.print("Startup.");     
  gfx.print(zahlen);   
  zahlen++;  
  gfx.display(true);
  Serial.println("Text1");
  
  gfx.setCursor(10, 160); gfx.setFont(Schrift2);
  gfx.print("-Keyboard Test-");     
  //gfx.print(zahlen);   
  zahlen++;  
  gfx.display(true);
  Serial.println("Text2");
  gfx.setCursor(10, 180); gfx.setFont(Schrift2);
  gfx.print("E-Ink Display -");     
  //gfx.print(zahlen);   
  zahlen++;  
  gfx.display(true);
  Serial.println("Text3");
  gfx.setRotation(0); gfx.fillScreen(GxEPD_WHITE);
}

void Display_Joystick_Values(void){ 
  gfx.fillRect(50, 50, 80, 80, GxEPD_WHITE);
  gfx.setCursor(70, 90); 
  gfx.print(SensorWertx);
  gfx.setCursor(70, 120); 
  gfx.print(SensorWerty);
  gfx.display(true);
}


void Display_Keyboard_Values(void){ 
  gfx.fillRect(50, 50, 80, 80, GxEPD_WHITE);
  gfx.setCursor(70, 90); 
  gfx.print(keyboard_val);
  gfx.display(true);
}
