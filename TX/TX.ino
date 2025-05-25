//WARNING!
//The below code is a proof of concept only!
//
//DO NOT flash this code onto a device which is operated in a region with duty cycle restrictions/dwell time restrictions/air time restrictions
//
//The below code is intended to be ported to either FHSS with the ability to become more compliant with air time related restrictions
//or ported to a device which operates on a frequency that does not have the same type of restrictions. 2.4ghz Lora?
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
//Radiomaster_Ranger_Nano_ELRS_RCTX
//Radiomaster_ER6_ELRS_RCRX
//Lilygo_T3_S3_SX1280_WITH_PA
//Lilygo_T3_S3_SX1280_WITHOUT_PA
//915mhz
//SeedStudio Xiao ESP32S3 plus WIO SX1262
//Heltec WiFi LoRa 32 v3 (sx1262)
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

#include <Bluepad32.h>

//OTAUPDATE
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include <RadioLib.h>

#include "settings.h" //Modify settings.h file to suite your hardware

//PacketRate-loop counter
long lastMillis = 0;
long PacketRatehz = 0;

// save transmission state between loops
int transmissionState = RADIOLIB_ERR_NONE;

// flag to indicate that a packet was sent
volatile bool transmittedFlag = false;

//Zero out initial controller state
int8_t Lx = 0x00; 
int8_t Ly = 0x00;
int8_t Rx = 0x00;  
int8_t Ry = 0x00;
uint8_t combinedButtons = 0x00;
uint8_t buttonsState = 0x00;

// this function is called when a complete packet
// is transmitted by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
#if defined(ESP8266) || defined(ESP32)
  ICACHE_RAM_ATTR
#endif
void setFlag(void) {
  // we sent a packet, set the flag
  transmittedFlag = true;
}

ControllerPtr myControllers[BP32_MAX_GAMEPADS];

// This callback gets called any time a new gamepad is connected.
// Up to 4 gamepads can be connected at the same time.
void onConnectedController(ControllerPtr ctl) {
    bool foundEmptySlot = false;
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == nullptr) {
            #if defined(DEBUG_ENABLED)
              Serial.printf("CALLBACK: Controller is connected, index=%d\n", i);
            #endif
            // Additionally, you can get certain gamepad properties like:
            // Model, VID, PID, BTAddr, flags, etc.
            ControllerProperties properties = ctl->getProperties();
            #if defined(DEBUG_ENABLED)
              Serial.printf("Controller model: %s, VID=0x%04x, PID=0x%04x\n", ctl->getModelName().c_str(), properties.vendor_id, properties.product_id);
            #endif
            myControllers[i] = ctl;
            foundEmptySlot = true;
            break;
        }
    }
    if (!foundEmptySlot) {
      #if defined(DEBUG_ENABLED)
        Serial.println("CALLBACK: Controller connected, but could not found empty slot");
      #endif
    }
}

void onDisconnectedController(ControllerPtr ctl) {
    bool foundController = false;

    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == ctl) {
            #if defined(DEBUG_ENABLED)
              Serial.printf("CALLBACK: Controller disconnected from index=%d\n", i);
            #endif
            myControllers[i] = nullptr;
            foundController = true;
            break;
        }
    }

    if (!foundController) {
      #if defined(DEBUG_ENABLED)
        Serial.println("CALLBACK: Controller disconnected, but not found in myControllers");
      #endif
    }
}

void processGamepad(ControllerPtr ctl) {
    //PacketRate in "hz" timer
    long currentMillis = millis();

    // JoyStick States: Drop axis from 10bit resolution to 8bit resolution
    Lx = (ctl->axisX() >> 2); 
    Ly = (ctl->axisY() >> 2);
    Rx = (ctl->axisRX() >> 2);  
    Ry = (ctl->axisRY() >> 2);

    // Button States:
    uint8_t dpadState = ctl->dpad(); //4 bits
    uint8_t miscState = ctl->miscButtons(); //4 bits
    // Combine the above buttons
    combinedButtons = (dpadState<<4) | (miscState);
    buttonsState = ctl->buttons();

    //Show axis values and button states via the serial console
    #if defined(DEBUG_ENABLED)
      Serial.println(
        "Lx: "+String(Lx)+" ,\
        Ly: "+String(Ly)+" ,\
        Rx: "+String(Rx)+" ,\
        Ry: "+String(Ry)+" ,\
        DPAD: "+String(ctl->dpad())+" ,\
        XBOX: "+String(ctl->miscSystem())+" ,\
        START: "+String(ctl->miscStart())+" ,\
        SELECT: "+String(ctl->miscSelect())+" ,\
        SHARE: "+String(ctl->miscCapture())+" ,\
        A: "+String(ctl->a())+" ,\
        B: "+String(ctl->b())+" ,\
        X: "+String(ctl->x())+" ,\
        Y: "+String(ctl->y())+" ,\
        L1: "+String(ctl->l1())+" ,\
        R1: "+String(ctl->r1())+" ,\
        L2: "+String(ctl->l2())+" ,\
        R2: "+String(ctl->r2())
      );
    #endif

    //Pack data into a packet to be transmitted
    byte dataPacket[] = {Lx, Ly, Rx, Ry, combinedButtons, buttonsState};
    //Or send as a string(Not as efficient)
    //String dataPacketStr = String(String(Lx)+","+String(Ly)+","+String(Rx)+","+String(Ry)+","+String(dpadState)+","+String(miscState)+","+String(buttonsState));

    if(transmittedFlag) {
      // reset flag
      transmittedFlag = false;

      if (transmissionState == RADIOLIB_ERR_NONE) {
        // packet was successfully sent
        #if defined(DEBUG_ENABLED)
          Serial.println(F("transmission finished!"));
        #endif
        PacketRatehz++; //increase PacketRate variable
        // NOTE: when using interrupt-driven transmit method,
        //       it is not possible to automatically measure
        //       transmission data rate using getDataRate()

      } else {
        #if defined(DEBUG_ENABLED)
          Serial.println(F("failed, code "));
          Serial.println(transmissionState);
        #endif

      }

      // clean up after transmission is finished
      // this will ensure transmitter is disabled,
      // RF switch is powered down etc.
      radio.finishTransmit();

      // wait a second before transmitting again
      // send another one
      #if defined(DEBUG_ENABLED)
        Serial.println(F("Sending another packet ... "));
      #endif
      //transmissionState = radio.startTransmit(dataPacketStr); // Transmit as a string rather than a byte array
      transmissionState = radio.startTransmit(dataPacket, PACKET_LENGTH);
    }
    
    //PacketRate-hz
    if(currentMillis - lastMillis > 1000){
      #if defined(DEBUG_ENABLED)
        Serial.println("PacketRate in hz: "+String(PacketRatehz));
      #endif
      lastMillis = currentMillis;
      PacketRatehz = 0;
    }
}

void processControllers() {
    for (auto myController : myControllers) {
        //if (myController && myController->isConnected() && myController->hasData()) {
        if (myController && myController->isConnected()) {
            if (myController->isGamepad()) {
                processGamepad(myController);
            } 
            else {
              #if defined(DEBUG_ENABLED)
                Serial.println("Unsupported controller");
              #endif
            }
        }
    }
}

// Arduino setup function. Runs in CPU 1
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
    // when packet transmission is finished
    radio.setPacketSentAction(setFlag);

    // start transmitting the first packet
    #if defined(DEBUG_ENABLED)
      Serial.println(F("Sending first packet ... "));
    #endif
    
    // Initialize Transmitter by Sending the first packet containing all ZEROS
    byte ZERO_PACKET[PACKET_LENGTH];
    memset(ZERO_PACKET, '\0', sizeof ZERO_PACKET);
    transmissionState = radio.startTransmit(ZERO_PACKET, PACKET_LENGTH);

    //Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
    const uint8_t* addr = BP32.localBdAddress();
    //Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    // Setup the Bluepad32 callbacks
    BP32.setup(&onConnectedController, &onDisconnectedController);

    // "forgetBluetoothKeys()" should be called when the user performs
    // a "device factory reset", or similar.
    // Calling "forgetBluetoothKeys" in setup() just as an example.
    // Forgetting Bluetooth keys prevents "paired" gamepads to reconnect.
    // But it might also fix some connection / re-connection issues.
    BP32.forgetBluetoothKeys();

    // Enables mouse / touchpad support for gamepads that support them.
    // When enabled, controllers like DualSense and DualShock4 generate two connected devices:
    // - First one: the gamepad
    // - Second one, which is a "virtual device", is a mouse.
    // By default, it is disabled.
    BP32.enableVirtualDevice(false);
}

// Arduino loop function. Runs in CPU 1.
void loop() {

    //OTAUPDATE
    ElegantOTA.loop();
    //ENDOTAUPDATE

    // This call fetches all the controllers' data.
    // Call this function in your main loop.
    bool dataUpdated = BP32.update();

    processControllers();

    // The main loop must have some kind of "yield to lower priority task" event.
    // Otherwise, the watchdog will get triggered.
    // If your main loop doesn't have one, just add a simple `vTaskDelay(1)`.
    // Detailed info here:
    // https://stackoverflow.com/questions/66278271/task-watchdog-got-triggered-the-tasks-did-not-reset-the-watchdog-in-time

    vTaskDelay(1);
    delayMicroseconds(600);
}
