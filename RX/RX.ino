//WARNING!
//The provided code is a proof of concept only!
//
//DO NOT flash this code onto a device which operates on a frequency in a region with duty cycle restrictions/dwell time restrictions/air time restrictions
//
//Take the time to understand the laws and regulations in your area
//It is your responsibility to ensure you are in compliance
//
//The provided code needs to be ported to FHSS before it could even be close to being compliant with any air time related restrictions
//or ported to a device which operates on a frequency that does not have the same type of restrictions.
//Perhaps the provided 2.4ghz examples may have less restrictions than the 915mhz examples?
//I do not believe you can transmit using the 915mhz examples in any region. So please do your research and comply!
//
//As it currently stands this code will not comply with duty cycle restrictions/dwell time restrictions/air time restrictions and needs further work before actual use
//The author(s) take no responsibility for your use or misuse of this code.
//It is up to the end user to research, know, and comply with all of the laws and regulations in the region in which they are operating in.
//The end user, and only the end user, is responsible for the end user's actions.
//
//Again this is a proof of concept example only and not meant for actual use.  You have been warned!
//
//XBOX BT to SBUS related Code is forked from: https://github.com/mechzrobotics/Unitree_GO2_SBUS
//LORA RX code based on example from: https://github.com/jgromes/RadioLib/blob/master/examples/SX126x/SX126x_Receive_Interrupt/SX126x_Receive_Interrupt.ino
//LORA TX code based on example from: https://github.com/jgromes/RadioLib/blob/master/examples/SX126x/SX126x_Transmit_Interrupt/SX126x_Transmit_Interrupt.ino
//Encoding/Decoding of LORA Packets containing XBOX Axis/Button States by Corey Harding
//
//Compatible Boards:
//2.4ghz
//Radiomaster_Ranger_Nano_ELRS_RCTX (1W)
//Radiomaster_ER6_ELRS_RCRX (100mW)
//Jumper_AION_Nano_RCTX (500mW)
//Lilygo_T3_S3_SX1280_WITH_PA
//Lilygo_T3_S3_SX1280_WITHOUT_PA
//915mhz
//SeedStudio Xiao ESP32S3 plus WIO SX1262 //Do not use
//Heltec WiFi LoRa 32 v3 (sx1262) //Do not use
//
//Required 3rd party board manager urls:
//Official ESP32 package: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
//"Bluepad32 + ESP32" package: https://raw.githubusercontent.com/ricardoquesada/esp32-arduino-lib-builder/master/bluepad32_files/package_esp32_bluepad32_index.json
//
//Required libraries:
//https://github.com/ESP32Async/AsyncTCP (Currently: 3.4.1)
//https://github.com/ESP32Async/ESPAsyncWebServer (Currently: 3.7.7)
//https://github.com/ayushsharma82/ElegantOTA (Currently: 3.1.7)
//https://github.com/jgromes/RadioLib (Currently: 7.1.2)
//https://github.com/bolderflight/sbus (Currently: 8.1.4)

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include <RadioLib.h>
#include "sbus.h"
#include "settings.h" //Modify settings.h file to suite your hardware
#include "ConvertLoraPacketToSBUS.h"

//PacketRate-loop counter
long lastMillis = 0;
long PacketRatehz = 0;

// flag to indicate that a packet was received
volatile bool receivedFlag = false;

// this function is called when a complete packet
// is received by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
#if defined(ESP8266) || defined(ESP32)
  ICACHE_RAM_ATTR
#endif
void setFlag(void) {
  // we got a packet, set the flag
  receivedFlag = true;
}

void setup() {
  #if defined(DEBUG_ENABLED)
    Serial.begin(115200);
  #endif

  //OTAUPDATE
  WiFi.softAP(ssid, password);
  #if defined(DEBUG_ENABLED)
    Serial.println("");
    Serial.println("SSID: ");
    Serial.println(ssid);
    Serial.println("Password: ");
    Serial.println(password);
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  #endif

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! I am ESP32.");
  });

  server.begin();
  #if defined(DEBUG_ENABLED)
    Serial.println("HTTP server started");
  #endif

  ElegantOTA.begin(&server);    // Start ElegantOTA
  //ENDOTAUPDATE

  //Turn on fans if available
  #if defined(FAN_ENABLE)
    pinMode(FAN_ENABLE, OUTPUT);
    digitalWrite(FAN_ENABLE, HIGH);
  #endif

  //Antenna Control
  #if defined(ANT_CTRL)
    pinMode(ANT_CTRL, OUTPUT);
    digitalWrite(ANT_CTRL, ANT_CTRL_HILO);
  #endif

  //Set Custom SPI Pins
  #if defined(CUSTOM_SPI_PINS)
    spi.begin(CUSTOM_SCK, CUSTOM_MISO, CUSTOM_MOSI, CUSTOM_CS);
  #endif
  #if defined(LILYGO_T3_S3)
      //SPI BUS SHARED WITH SD CARD
      pinMode(CUSTOM_CS, OUTPUT);
      digitalWrite(CUSTOM_CS, HIGH);
      pinMode(PIN_NRST, OUTPUT);
      digitalWrite(PIN_NRST, HIGH);
  #endif

  // initialize radio with default settings
  #if defined(DEBUG_ENABLED)
    Serial.println(F("Initializing ... "));
  #endif
  int state = radio.begin(FREQ, BANDWIDTH, SPREADING_FACTOR, CODING_RATE_4X, SYNC_WORD, OUTPUT_POWER_DBM, PREAMBLE_LENGTH);
  if (state == RADIOLIB_ERR_NONE) {
    #if defined(DEBUG_ENABLED)
      Serial.println(F("success!"));
    #endif
  } else {
    #if defined(DEBUG_ENABLED)
      Serial.println(F("failed, code "));
      Serial.println(state);
    #endif
    while (true) { delay(10); }
  }

  //Enable RF switch if required by module
  #if defined(RADIO_RX_PIN) || defined(RADIO_TX_PIN)
      //Set RX/TX Enable Pins
      radio.setRfSwitchPins(RADIO_RX_PIN, RADIO_TX_PIN);
  #endif

  radio.implicitHeader(PACKET_LENGTH);
  radio.setCRC(0);

  //Setup manual gain
  #if defined(MANUAL_GAIN)
      //Set RX/TX Enable Pins
      radio.setGainControl(MANUAL_GAIN);
  #endif

  // set the function that will be called
  // when new packet is received
  radio.setPacketReceivedAction(setFlag);

  // start listening for LoRa packets
  #if defined(DEBUG_ENABLED)
    Serial.println(F("Starting to listen ... "));
  #endif
  state = radio.startReceive();
  if (state == RADIOLIB_ERR_NONE) {
    #if defined(DEBUG_ENABLED)
      Serial.println(F("success!"));
    #endif
  } else {
    #if defined(DEBUG_ENABLED)
      Serial.println(F("failed, code "));
      Serial.println(state);
    #endif
    while (true) { delay(10); }
  }

  /* Begin the SBUS communication */
  sbus_tx.Begin();

}

void loop() {
  //PacketRate in "hz" timer
  long currentMillis = millis();

  //OTAUPDATE
  ElegantOTA.loop();
  //ENDOTAUPDATE

  // check if the flag is set
  if(receivedFlag) {
    // reset flag
    receivedFlag = false;

    // you can read received data as an Arduino String
    //String dataPacketStr;
    //int state = radio.readData(dataPacketStr);

    byte dataPacket[PACKET_LENGTH];
    int state = radio.readData(dataPacket, PACKET_LENGTH);

    if (state == RADIOLIB_ERR_NONE) {
      // print data of the packet
      #if defined(DEBUG_ENABLED)
        Serial.println(F("Data:\t"));
      //Serial.println(dataPacketStr);
      #endif

      //Send controller data to SBUS Conversion function to be Converted to PWM values and TX via SBUS
      
      ConvertLoraPacketToSBUS(dataPacket);

      PacketRatehz++; //increase PacketRate variable
    } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
      // packet was received, but is malformed
      #if defined(DEBUG_ENABLED)
        Serial.println(F("CRC error!"));
      #endif

    } else {
      // some other error occurred
      #if defined(DEBUG_ENABLED)
        Serial.println(F("failed, code "));
        Serial.println(state);
      #endif

    }
    #if defined(USING_SX1280) || defined(USING_SX1281)
      // put module back to listen mode
      radio.startReceive();
    #endif
  }
      //PacketRate-hz
  if(currentMillis - lastMillis > 1000){
    #if defined(DEBUG_ENABLED)
      Serial.println("PacketRate in hz: "+String(PacketRatehz));
    #endif
    lastMillis = currentMillis;
    PacketRatehz = 0;
  }
  vTaskDelay(1);
  delayMicroseconds(600);
}
