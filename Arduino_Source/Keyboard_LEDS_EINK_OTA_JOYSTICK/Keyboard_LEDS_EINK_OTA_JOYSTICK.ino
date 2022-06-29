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
  B) Web Serial: Enables a Debugging Console as a web Server
  C) All Keyboard Buttons
  D) RGB LEDs
  E) WiFi
  F) Bluetooth
  G) EInk Display
  H) Joystick
  I) SD Card Storage


Missing: !!! Wifi Manager if no valid Wifi Config is found a Wifi Server is started if GPIO13 is pressed
!!!!Select USB CDC On Boot Enabled -> then maybe serial does not work any more.
//Upload Mode???

Probleme:
- OTA Update geht nur direkt nach Reset.
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

#include "OTA.h" //Ota Update
#include "credentials.h" //Pin Definitions

//LEDS
#include "FastLED.h"
#include "FastLED_RGBW.h"

//Web Serial
#include <WebSerial.h>
AsyncWebServer WebSerialserver(80);

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

//SD Karte
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

  
// Initiate Wifi with ADC2 on ESP32 workaround bellow:
#include "soc/sens_reg.h"    // needed for manipulating ADC2 control register
uint32_t adc_register;


// End initiation of Wifi with ADC2 on ESP32 workaround.



void setup() {
  Serial.begin(115200);
  //adc_register = READ_PERI_REG(SENS_SAR_READ_CTRL2_REG); // Wifi with ADC2 on ESP32 workaround.
  LED_Setup();
  setupOTA("ESP32S3KEYBOARD");
  Serial.print("OTA Rennt: ");
  Serial.println(__FILE__); //https://forum.arduino.cc/t/which-program-am-i-running/410688/18
  
  //WebSerial is accessible at your ESP's <IPAddress>/webserial URL
  WebSerial.begin(&WebSerialserver);
  WebSerialserver.begin();
  Serial.println("WebSerial is running");
  WebSerial.println("WebSerial is running"); 
  rainbowLoop();
  FastLED.show();
  Serial.println("LED an");
  
  //Display_Start(); //Initalisiert das Display
  //Serial.println("Display an");
   
   //SD Karte
   /*
   #define SD_CARD_PIN_DAT2 10
   #define SD_CARD_PIN_DAT3 11 = CS
   #define SD_CARD_PIN_CMD 13 = MOSI
   #define SD_CARD_PIN_CLK 14 = CLK 
   #define SD_CARD_PIN_DAT0 47 = MISO
   #define SD_CARD_PIN_DAT1 48
  */
  
  SPI.begin(SD_CARD_PIN_CLK, SD_CARD_PIN_DAT0, SD_CARD_PIN_CMD); //CLK, MISO, MOSI
  if(!SD.begin(SD_CARD_PIN_DAT3, SPI, 4000000)){ //Chip Select
    WebSerial.println("Card Mount Failed");
  }

  uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
        WebSerial.println("No SD card attached");
        return;
    }

    WebSerial.print("SD Card Type: ");
    if(cardType == CARD_MMC){
        WebSerial.println("MMC");
    } else if(cardType == CARD_SD){
        WebSerial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        WebSerial.println("SDHC");
    } else {
        WebSerial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    //WebSerial.print("SD Card Size: ");WebSerial.println(cardSize);

    //Keyboard.begin();
    //USB.begin();
    WebSerial.print("Keyboard started");
  
  delay(500);
  Serial.println("setup done");
}



void loop() {
   ArduinoOTA.handle();
   //Joystick_Read();
   
   delay(150);
   
   
   keyboard_val = key_test();
   if (keyboard_val != 0){
    WebSerial.print("Key: "); WebSerial.println(keyboard_val);
    keyboard_val = 0;
    //Keyboard.press('a');
    //delay(100);
    //Keyboard.releaseAll();
    //Keyboard.write(KEY_RETURN);
   }
   
}

void LED_Setup(){
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);//BrownOut Disable
  esp_task_wdt_init(30, false); //Task Watchdog set to 30sec
  FastLED.addLeds<WS2812B, DATA_PIN, RGB>(ledsRGB, getRGBWsize(NUM_LEDS));
  FastLED.setBrightness(brightness);
  FastLED.show();
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
      KeyBoard[i][0] = analogRead(ROW1); 
      KeyBoard[i][1] = analogRead(ROW2); 
      KeyBoard[i][2] = analogRead(ROW3); 
      digitalWrite(colPins[i], HIGH);  
    }
    
    
    //DebugAusgabe
     for (int i = 0; i < nCol; i++){
      for (int b = 0; b < nRow; b++){
        int a = i+1; int c = b+1;
        if (KeyBoard[i][b] < KEYPRESS_DETECTION_VALUE) 
        {
         #ifdef WEB_DEBUG_SERIAL
          WebSerial.print(str + a + c +":" + KeyBoard[i][b] +" \n");
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
       WebSerial.println(stringer + "Joy x:" + SensorWertx +" y:" + SensorWerty); 
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
  #ifdef Anzeige_42
    gfx.drawFastHLine(0, 115, 200, GxEPD_BLACK); //Linie horizontal Teil Breite bis Text
    gfx.drawFastVLine(200, 0, gfx.height(), GxEPD_BLACK); //Linie Vertical Volle Höhe
  #else  
    gfx.drawFastHLine(0, 115, gfx.width(), GxEPD_BLACK); //Linie horizontal Volle Breite
  #endif
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


//OLD ONLY FOR TESTING DEBUG
void key_test2(void){
  //Keypads like this work by toggling column pins output low sequentially, reading the state of each row pin for each column.
  //https://www.esp32.com/viewtopic.php?t=18989
  int val1 = 0, val2 = 0, val3 = 0;
  pinMode(COL1, OUTPUT);
  pinMode(COL2, OUTPUT);
  pinMode(COL3, OUTPUT);
  pinMode(ROW1, INPUT);
  pinMode(ROW2, INPUT);
  pinMode(ROW3, INPUT);
  
  digitalWrite(COL1, LOW);  
  Serial.print("COL 1: "); 
  val1 = analogRead(ROW1); Serial.print(" 11:"); Serial.print(val1);
  val2 = analogRead(ROW2); Serial.print(" 12:"); Serial.print(val2);
  val3 = analogRead(ROW3); Serial.print(" 13:"); Serial.println(val3);
  if (val1 < 2000 && val2 >2000 && val3 >2000) WebSerial.println("Taste11!");
  if (val1 > 2000 && val2 <2000 && val3 >2000) WebSerial.println("Taste12!");
  if (val1 > 2000 && val2 >2000 && val3 <2000) WebSerial.println("Taste13!");
  digitalWrite(COL1, HIGH);

  digitalWrite(COL2, LOW);  
  Serial.print("COL 2: "); 
  val1 = analogRead(ROW1); Serial.print(" 21:"); Serial.print(val1);
  val2 = analogRead(ROW2); Serial.print(" 22:"); Serial.print(val2);
  val3 = analogRead(ROW3); Serial.print(" 23:"); Serial.println(val3);
  if (val1 < 2000 && val2 >2000 && val3 >2000) WebSerial.println("Taste21!");
  if (val1 > 2000 && val2 <2000 && val3 >2000) WebSerial.println("Taste22!");
  if (val1 > 2000 && val2 >2000 && val3 <2000) WebSerial.println("Taste23!");
  digitalWrite(COL2, HIGH);

  digitalWrite(COL3, LOW);  
  Serial.print("COL 3: "); 
  val1 = analogRead(ROW1); Serial.print(" 31:"); Serial.print(val1);
  val2 = analogRead(ROW2); Serial.print(" 32:"); Serial.print(val2);
  val3 = analogRead(ROW3); Serial.print(" 33:"); Serial.println(val3);
  if (val1 < 2000 && val2 >2000 && val3 >2000) WebSerial.println("Taste31!");
  if (val1 > 2000 && val2 <2000 && val3 >2000) WebSerial.println("Taste32!");
  if (val1 > 2000 && val2 >2000 && val3 <2000) WebSerial.println("Taste33!");
  digitalWrite(COL3, HIGH);
  Serial.println(" ");

}
