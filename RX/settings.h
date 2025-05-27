//Uncomment your board below - Remove the "//"" from in front of the "#define" - YOU MUST EDIT THIS
//When choosing a board in Arduino IDE use bluepad32 for TX and regular esp32 for RX
//
//#define Radiomaster_Ranger_Nano_ELRS_RCTX //2.4ghz elrs rctx, up to 1W, flash bin via elrs web interface and confirm popup, ESP32-D0WDQ6 V3(select board ESP32-WROOM-DA)/sx1281, sbus output: nano module bay pins on back of unit - looking at the rear of the module: far left pin(signal/data) is sbus out and second from the left pin (ground) is ground
#define Radiomaster_ER6_ELRS_RCRX //2.4ghz elrs rcrx, up to 100mW , flash bin via elrs web interface and confirm popup, select esp32 pico d4 board/sx1281, sbus output: use a servo cable on Ch1: pwm signal(white) is sbus out and black is ground
//#define Jumper_AION_Nano_RCTX //2.4ghz elrs rctx, up to 500mW, flash bin via elrs web interface and confirm popup, esp32 pico d4(select esp32 pico d4 and hold down button when connecting via usb to upload, I also had to flash bootloader for some reason)/sx1281(e28-2g4m28s), sbus output: nano module bay pins on back of unit - looking at the rear of the module: far left pin(signal/data) is sbus out and second from the left pin (ground) is ground - must be powered via nano port vbat(2s) on pin 3rd from the left, otherwise power via usb will cause a brownout
//#define Lilygo_T3_S3_SX1280_WITH_PA //2.4ghz board
//#define Lilygo_T3_S3_SX1280_WITHOUT_PA //2.4ghz board
//#define GENERIC_MODULE //User configured
//#define Xiao_ESP32S3_and_WIO_SX1262 //915mhz board, WARNING Do not use: Need to implement fhss and additional code to be compliant in most regions
//#define Heltec_WiFi_LoRa_32_V3 //915mhz board, WARNING Do not use: Need to implement fhss and additional code to be compliant in most regions

//OTAUPDATE Creates a WIFI Access Point for OTA Updates via http://192.168.4.1/update
//YOU MAY EDIT THE SSID AND PASSWORD FOR YOUR ACCESS POINT
const char* ssid = "LORA-RX";
const char* password = "theroboverse";
AsyncWebServer server(80);
//ENDOTAUPDATE

//Uncomment to enable debug messages via serial monitor
#define DEBUG_ENABLED

//All of the main settings below must be the same on the RX/TX for the rc link to work correctly except for the chosen hardware/pinout
//Smaller bandwidth and larger spreading factor result in longer range but slower packet rate
//Larger bandwidth and smaller spreading factor result in shorter range but faster packet rate

//Define Packet Length in Bytes - Larger packet length allows higher resolution joysticks and includes l3/r3 at the expense of slower packet rate
#define PACKET_LENGTH     6 //8bit Resolution Joysticks, All buttons except L3/R3
//#define PACKET_LENGTH     7 //8bit Resolution Joysticks, All buttons including L3/R3
//#define PACKET_LENGTH     8 //10bit Resolution Joysticks, All buttons including L3/R3

//Define deadzone for joystick
#if PACKET_LENGTH < 8 //8bit joystick deadzone
  #define DEADZONE          8 //for 8 bit resolution typical .05 deadzone would be figured using 128*.05 = 6.4, but I am bumping it up to 8
#elif PACKET_LENGTH == 8 //10bit joystick deadzone
  #define DEADZONE          32 //for 10 bit resolution typical .05 deadzone would be figured using 512*.05 = 25.6, but I am bumping it up to 32
#endif

//2.4ghz modules settings sx1281
#if defined(Radiomaster_ER6_ELRS_RCRX) || defined(Radiomaster_Ranger_Nano_ELRS_RCTX) || defined(Jumper_AION_Nano_RCTX)
  #define USING_SX1281
  // Radio Configuration for 2.4ghz modules - YOU MAY EDIT THIS
  #define FREQ                2401.0
  #define BANDWIDTH           812.5 //options: 812.5bw/11 406.25bw/10sf 203.125bw/9sf
  #define SPREADING_FACTOR    11 //options: 812.5bw/11 406.25bw/10sf 203.125bw/9sf
  #define CODING_RATE_4X      5
  #define SYNC_WORD           0x12
  #define OUTPUT_POWER_DBM    3 //Set Transmit Power: WARNING! For boards with PA do not change output power or you may burn up the PA
  #define PREAMBLE_LENGTH     6
  //
  //Pin configurations - DO NOT EDIT THIS
  #if defined(Radiomaster_ER6_ELRS_RCRX)
    #define CUSTOM_SPI_PINS
    #define CUSTOM_MISO       33
    #define CUSTOM_MOSI       32
    #define CUSTOM_CS         27
    #define CUSTOM_SCK        25
    #define CUSTOM_SPI_FREQ   74880
    #define CUSTOM_SPI_BITORDER MSBFIRST
    #define CUSTOM_SPI_MODE   SPI_MODE0
    #define PIN_NSS           27 //CS PIN
    #define PIN_DIO1          37 //DIO1 PIN
    #define PIN_NRST          13 //RST PIN
    #define PIN_BUSY          36 //BUSY PIN
    #define SERIAL_TX_PIN     14 //Serial TX Pin 14 Ch1 PWM Signal - use a servo cable on Ch1: pwm signal(white) is sbus out and black is ground
    #define SERIAL_RX_PIN     12 //Serial RX Pin Pin 12 Ch2 PWM Signal
    HardwareSerial &SERIAL_INTERFACE = Serial1; //Set Serial Interface
    #define RADIO_TX_PIN      26 //Enable RF Switch
    #define RADIO_RX_PIN      RADIOLIB_NC //Enable RF Switch
    //Override TX Power setting above for Radiomaster_ER6_ELRS_RCRX
    #define OUTPUT_POWER_DBM  3 //WARNING! For this module do not change output power above 3 or you may burn up the PA
    #define MANUAL_GAIN       12 //LNA Value
    #define ANT_CTRL          5 //Antenna control pin
    #define ANT_CTRL_HILO     HIGH //or LOW
  #endif
  //Pin configurations - DO NOT EDIT THIS
  #if defined(Radiomaster_Ranger_Nano_ELRS_RCTX)
    #define CUSTOM_SPI_PINS
    #define CUSTOM_MISO       19
    #define CUSTOM_MOSI       23
    #define CUSTOM_CS         4
    #define CUSTOM_SCK        18
    #define CUSTOM_SPI_FREQ   74880
    #define CUSTOM_SPI_BITORDER MSBFIRST
    #define CUSTOM_SPI_MODE   SPI_MODE0
    #define PIN_NSS           4 //CS PIN
    #define PIN_DIO1          21 //DIO1 PIN
    #define PIN_NRST          5 //RST PIN
    #define PIN_BUSY          22 //BUSY PIN
    #define SERIAL_TX_PIN     13 //Serial TX Pin for SBUS Output
    #define SERIAL_RX_PIN     13 //Serial RX Pin
    HardwareSerial &SERIAL_INTERFACE = Serial1; //Set Serial Interface
    #define RADIO_TX_PIN      33 //Enable RF Switch
    #define RADIO_RX_PIN      32 //Enable RF Switch
    #define FAN_ENABLE        27 //Fan enable pin
    //Override TX Power setting above for Radiomaster_ER6_ELRS_RCRX
    #define OUTPUT_POWER_DBM  2 //WARNING! For this module do not change output power above 6 or you may burn up the PA
  #endif
  //Pin configurations - DO NOT EDIT THIS
  #if defined(Jumper_AION_Nano_RCTX)
    #define CUSTOM_SPI_PINS
    #define CUSTOM_MISO       19
    #define CUSTOM_MOSI       23
    #define CUSTOM_CS         5
    #define CUSTOM_SCK        18
    #define CUSTOM_SPI_FREQ   74880
    #define CUSTOM_SPI_BITORDER MSBFIRST
    #define CUSTOM_SPI_MODE   SPI_MODE0
    #define PIN_NSS           5 //CS PIN
    #define PIN_DIO1          4 //DIO1 PIN
    #define PIN_NRST          14 //RST PIN
    #define PIN_BUSY          21 //BUSY PIN
    #define SERIAL_TX_PIN     13 //Serial TX Pin for SBUS Output
    #define SERIAL_RX_PIN     13 //Serial RX Pin
    HardwareSerial &SERIAL_INTERFACE = Serial1; //Set Serial Interface
    #define RADIO_TX_PIN      26 //Enable RF Switch
    #define RADIO_RX_PIN      27 //Enable RF Switch
    //Override TX Power setting above for Radiomaster_ER6_ELRS_RCRX
    #define OUTPUT_POWER_DBM  2 //WARNING! For this module do not change output power above 5 or you may burn up the PA
    #define MANUAL_GAIN       12 //LNA Value
  #endif
#endif

//2.4ghz modules settings sx1280
#if defined(Lilygo_T3_S3_SX1280_WITH_PA) || defined(Lilygo_T3_S3_SX1280_WITHOUT_PA)
  #define USING_SX1280
  // Radio Configuration for 2.4ghz modules - YOU MAY EDIT THIS
  #define FREQ                2450.0
  #define BANDWIDTH           812.5 //options: 812.5bw/11 406.25bw/10sf 203.125bw/9sf
  #define SPREADING_FACTOR    11 //options: 812.5bw/11 406.25bw/10sf 203.125bw/9sf
  #define CODING_RATE_4X      5
  #define SYNC_WORD           0x12
  #define OUTPUT_POWER_DBM    3 //Set Transmit Power: WARNING! For board Lilygo_T3_S3_SX1280_WITH_PA do not change output power above 3dbm or you will burn up the PA
  #define PREAMBLE_LENGTH     6
  //
  //Pin configurations - DO NOT EDIT THIS
  #if defined(Lilygo_T3_S3_SX1280_WITH_PA) || defined(Lilygo_T3_S3_SX1280_WITHOUT_PA)
    #define PIN_NSS           7 //CS PIN
    #define PIN_DIO1          9 //DIO1 PIN
    #define PIN_NRST          8 //RST PIN
    #define PIN_BUSY          36 //BUSY PIN
    #define SERIAL_TX_PIN     46 //Serial TX Pin for SBUS Output
    #define SERIAL_RX_PIN     42 //Serial RX Pin
    HardwareSerial &SERIAL_INTERFACE = Serial1; //Set Serial Interface
  #endif
  //
  //Set RF Switch if Lilygo_T3_S3_SX1280_WITH_PA - DO NOT EDIT THIS
  #if defined(Lilygo_T3_S3_SX1280_WITH_PA)
    #define RADIO_TX_PIN      10 //Enable RF Switch
    #define RADIO_RX_PIN      21 //Enable RF Switch
    //Override TX Power setting above for Lilygo_T3_S3_SX1280_WITH_PA
    #define OUTPUT_POWER_DBM  3 //WARNING! For this module do not change output power above 3dbm or you will burn up the PA
  #endif
#endif

//915mhz modules settings
#if defined(Xiao_ESP32S3_and_WIO_SX1262) || defined(Heltec_WiFi_LoRa_32_V3)
  #define USING_SX1262
  // Radio Configuration for 915mhz module - YOU MAY EDIT THIS
  #define FREQ                915.0
  #define BANDWIDTH           125
  #define SPREADING_FACTOR    8
  #define CODING_RATE_4X      5
  #define SYNC_WORD           0x12
  #define OUTPUT_POWER_DBM    -9 //Set Transmit Power: -9 to 22dbm
  #define PREAMBLE_LENGTH     6
  //
  //Pin configurations -DO NOT EDIT THIS
  #if defined(Xiao_ESP32S3_and_WIO_SX1262)
    #define PIN_NSS           41 //CS PIN
    #define PIN_DIO1          39 //DIO1 PIN
    #define PIN_NRST          42 //RST PIN
    #define PIN_BUSY          40 //BUSY PIN
    #define SERIAL_TX_PIN     9 //Serial TX Pin for SBUS Output Pin9/D10
    #define SERIAL_RX_PIN     8 //Serial RX Pin
    HardwareSerial &SERIAL_INTERFACE = Serial1; //Set Serial Interface
  #endif
  #if defined(Heltec_WiFi_LoRa_32_V3)
    #define PIN_NSS           8 //CS PIN
    #define PIN_DIO1          14 //DIO1 PIN
    #define PIN_NRST          12 //RST PIN
    #define PIN_BUSY          13 //BUSY PIN
    #define SERIAL_TX_PIN     45 //Serial TX Pin for SBUS Output
    #define SERIAL_RX_PIN     46 //Serial RX Pin
    HardwareSerial &SERIAL_INTERFACE = Serial1; //Set Serial Interface
  #endif
#endif

//Additional define for user to add their own module
#if defined(GENERIC_MODULE) //Custom user module, generic esp32s3 dev board?
  //
  #define USING_SX1280 //or comment and uncomment below
  //#define USING_SX1262 
  //
  // Radio Configuration for 2.4ghz modules - YOU MAY EDIT THIS
  #define FREQ                2450.0
  #define BANDWIDTH           812.5 //options: bw:812.5/sf:11_or_12(slower) bw:406.25/sf:10_or_11(slower) bw:203.125/sf:9_or_10(slower)
  #define SPREADING_FACTOR    11 //options: bw:812.5/sf:11_or_12(slower) bw:406.25/sf:10_or_11(slower) bw:203.125/sf:9_or_10(slower)
  #define CODING_RATE_4X      5
  #define SYNC_WORD           0x12
  #define OUTPUT_POWER_DBM    3 //Set Transmit Power: WARNING! For this Lilygo_T3_S3_SX1280_WITH_PA do not change output power above 3dbm or you will burn up the PA
  #define PREAMBLE_LENGTH     6
  //
  //Pin configurations - EDIT THIS FOR GENERIC
    #define PIN_NSS           41 //CS PIN
    #define PIN_DIO1          39 //DIO1 PIN
    #define PIN_NRST          42 //RST PIN
    #define PIN_BUSY          40  //BUSY PIN
    #define SERIAL_TX_PIN     17 //Serial TX Pin for SBUS Output
    #define SERIAL_RX_PIN     18 //Serial RX Pin
    HardwareSerial &SERIAL_INTERFACE = Serial1; //Set Serial Interface
    //#define RADIO_TX_PIN      10 //Enable RF Switch
    //#define RADIO_RX_PIN      21 //Enable RF Switch
#endif

#if defined(CUSTOM_SPI_PINS)
  SPIClass spi(VSPI);
  SPISettings spiSettings(CUSTOM_SPI_FREQ, CUSTOM_SPI_BITORDER, CUSTOM_SPI_MODE);
#endif

//Set radio type - DO NOT EDIT THIS
#if defined(CUSTOM_SPI_PINS)
  #if defined(USING_SX1280) //2.4ghz
    SX1280 radio = new Module(PIN_NSS, PIN_DIO1, PIN_NRST, PIN_BUSY, spi, spiSettings);
  #endif
  #if defined(USING_SX1281) //2.4ghz
    SX1281 radio = new Module(PIN_NSS, PIN_DIO1, PIN_NRST, PIN_BUSY, spi, spiSettings);
  #endif
  #if defined(USING_SX1262) //915mhz
    SX1262 radio = new Module(PIN_NSS, PIN_DIO1, PIN_NRST, PIN_BUSY, spi, spiSettings);
  #endif
#else
  #if defined(USING_SX1280) //2.4ghz
    SX1280 radio = new Module(PIN_NSS, PIN_DIO1, PIN_NRST, PIN_BUSY);
  #endif
  #if defined(USING_SX1281) //2.4ghz
    SX1281 radio = new Module(PIN_NSS, PIN_DIO1, PIN_NRST, PIN_BUSY);
  #endif
  #if defined(USING_SX1262) //915mhz
    SX1262 radio = new Module(PIN_NSS, PIN_DIO1, PIN_NRST, PIN_BUSY);
  #endif
#endif