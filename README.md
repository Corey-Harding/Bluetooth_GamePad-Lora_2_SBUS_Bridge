# Bluetooth_GamePad-Lora_2_SBUS_Bridge
Use a xbox (or similar bluetooth) controller over LoRa to control a rc device that accepts sbus input.  Example project controls a Unitree Go2 Dog using two 2.4G ELRS RC Modules. (Radiomaster ER6, Ranger Nano, Jumper Aion Nano, or LilyGo T3 S3 2.4)
```
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
```
