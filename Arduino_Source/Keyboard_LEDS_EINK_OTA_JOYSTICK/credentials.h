#define mySSID "xxxxxxxxxxxx"
#define myPASSWORD "xxxxxxxxxxxx"
#define thingspeak_server "api.thingspeak.com"
#define thingspeak_api_key "xxxxxxxxxxxxxx"

//#define keyboard1 // ESP32 Wroom
#define keyboard2 // ESP32-S38N1 WROOM

#ifdef keyboard1
    //ESP32 KeyboardBoard
    //#define mqtt_client_on
	#define ANZAHL_LEDS 1 
    #define RGB_LED_DATA_PIN 23
    #define WS2812_LEDS
    #define updaterateLED 70 
	#define ausschalten 18000 
	#define weiss_helligkeit 200
    
    
    
    #define BUTTON_DISABLED 0
    #define BUTTON_ENABLED 1
    #define timeSeconds 500
    #define updaterate_leds 2000
    #define max_counter 255
    #define min_counter 0
	
	//Joystick
	#define X1_PIN 17
  #define Y1_PIN 18
  #define X2_PIN 15
  #define Y2_PIN 2
 
	
	//Joystick ESP32 Wroom
	#define ROW_NUM     3 
	#define COLUMN_NUM  3 
	#define ROW1 34
	#define ROW2 35
	#define ROW3 39 
	#define COL1 22
	#define COL2 32
	#define COL3 13
	
	//Eink KEYBOARD -> ESP32 WROOM
	  #define E_INK_PIN_SPI_BUSY 4
	  #define E_INK_PIN_SPI_CS   18
	  #define E_INK_PIN_SPI_RST  16//21
	  #define E_INK_PIN_SPI_DC   17
	  #define E_INK_PIN_SPI_SCK  19
	  #define E_INK_PIN_SPI_DIN  21
	  #define E_INK_PIN_SPI_MISO 33//n/A
	  #define Large  5
	  #define Small  4
	  #define xanpassung -30


#endif


#ifdef keyboard2
    //ESP32-S3 KeyboardBoard
    //#define mqtt_client_on
  //#define WEB_DEBUG_SERIAL 1
  #define ANZAHL_LEDS 4 
    #define RGB_LED_DATA_PIN 10
    #define WS2812_LEDS
    #define updaterateLED 70 
  #define ausschalten 18000 
  #define weiss_helligkeit 200
  #define KEYPRESS_DETECTION_VALUE 2000  
    
    
    #define BUTTON_DISABLED 0
    #define BUTTON_ENABLED 1
    #define timeSeconds 500
    #define updaterate_leds 2000
    #define max_counter 255
    #define min_counter 0
  
  //Joystick ESP32-S3 Wroom
  #define X1_PIN 17
  #define Y1_PIN 18
  #define X2_PIN 1
  #define Y2_PIN 2
  
  //Keyboard PINS ESP32-S3
  #define ROW_NUM     3 
  #define COLUMN_NUM  3 
  #define ROW1 6
  #define ROW2 4
  #define ROW3 7
  #define COL1 15
  #define COL2 5
  #define COL3 9
  
  //Eink KEYBOARD -> ESP32-S3 WROOM
    #define E_INK_PIN_SPI_BUSY 35
    #define E_INK_PIN_SPI_CS   39
    #define E_INK_PIN_SPI_RST  36//21
    #define E_INK_PIN_SPI_DC   37
    #define E_INK_PIN_SPI_SCK  40
    #define E_INK_PIN_SPI_DIN  42
    #define E_INK_PIN_SPI_MISO 41//n/A
    #define Large  5
    #define Small  4
    #define xanpassung -30


//SD Card
   #define SD_CARD_PIN_DAT2 10
   #define SD_CARD_PIN_DAT3 11
   #define SD_CARD_PIN_CMD 13
   #define SD_CARD_PIN_CLK 14
   #define SD_CARD_PIN_DAT0 47
   #define SD_CARD_PIN_DAT1 48
#endif
