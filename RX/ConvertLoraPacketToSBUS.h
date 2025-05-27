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
//Encoding/Decoding of LORA Packets containing XBOX Axis/Button States by Corey Harding

/* SBUS object, writing SBUS */
//SBUS Serial, RX Pin, TX Pin
//Values for Xiao Esp32s3, Pin 43(D6) is TX
bfs::SbusTx sbus_tx(&SERIAL_INTERFACE, SERIAL_RX_PIN, SERIAL_TX_PIN, true);
/* SBUS data */
bfs::SbusData data;

unsigned long StartTime = 0;
unsigned long CurrentTime = 0;
unsigned long ElapsedTime = 0;

struct SBUS_DATA{
  int32_t CH1;
  int32_t CH2;
  int32_t CH3;
  int32_t CH4;
  int32_t CH5;
  int32_t CH6;
  int32_t CH7;
  int32_t CH8;
};

// Button State Flags:
bool A_Flag = 0;
bool B_Flag = 0;
bool X_Flag = 0;
bool Y_Flag = 0;
bool L1_Flag = 0;
bool L2_Flag = 0;
bool R1_Flag = 0;
bool R2_Flag = 0;
bool D_UP_Flag = 0;
bool D_DOWN_Flag = 0;
bool D_LEFT_Flag = 0;
bool D_RIGHT_Flag = 0;
bool START_Flag = 0;
bool SELECT_Flag = 0;
bool XBOX_Flag = 0;
bool SHARE_Flag = 0;

// DOG State Flags: //Unitree Go2 Specific
bool HIGH_STAND_Flag = 1;
bool OBSTACLE_Flag = 0;
int8_t WALK_FLAG = 0;

void writeSBUS(SBUS_DATA my_data){
  data.ch[0] = my_data.CH1;
  data.ch[1] = my_data.CH2;
  data.ch[2] = my_data.CH3;
  data.ch[3] = my_data.CH4;
  data.ch[4] = my_data.CH5; // CH5 default 992
  data.ch[5] = my_data.CH6; // CH6
  data.ch[6] = my_data.CH7; // CH7
  data.ch[7] = my_data.CH8; // CH8
  data.ch[8] = 1690; //CH9 static PWM value as shown in Unitree Go2 SBUS Documentation
  data.ch[9] = 154; //CH10 static PWM value as shown in Unitree Go2 SBUS Documentation
  //Output SBUS Data
  sbus_tx.data(data);
  sbus_tx.Write();
  #if defined(DEBUG_ENABLED)
    Serial.println(
    "Ch1: "+String(data.ch[0])+" ,\
    Ch2: "+String(data.ch[1])+" ,\
    Ch3: "+String(data.ch[2])+" ,\
    Ch4: "+String(data.ch[3])+" ,\
    Ch5: "+String(data.ch[4])+" ,\
    Ch6: "+String(data.ch[5])+" ,\
    Ch7: "+String(data.ch[6])+" ,\
    Ch8: "+String(data.ch[7])
    );
  #endif
}

//based on readAxis by johnwasser:https://forum.arduino.cc/t/adding-a-dead-zone-to-a-analog-imput-on-arduino-leonardo/662508/5
int32_t readAxis(int16_t Axis_Packet, int AXIS_LOW_PACKET, int AXIS_HIGH_PACKET, int AXIS_LOW_PWM, int AXIS_HIGH_PWM, int AXIS_INVERTED) {
  const int DEADZONE_LOW  = 0-DEADZONE;
  const int DEADZONE_HIGH = 0+DEADZONE;
  const int AXIS_MID_PWM = ( (AXIS_LOW_PWM/2) + (AXIS_HIGH_PWM/2) );
  if (AXIS_INVERTED == 1) {
    int LOW_SWAP = AXIS_HIGH_PACKET;
    int HIGH_SWAP = AXIS_LOW_PACKET;
    AXIS_HIGH_PACKET = HIGH_SWAP;
    AXIS_LOW_PACKET = LOW_SWAP;
  }
  if (Axis_Packet < DEADZONE_LOW) {
    return map(Axis_Packet , AXIS_LOW_PACKET, AXIS_HIGH_PACKET, AXIS_LOW_PWM, AXIS_HIGH_PWM);
  }
  else if (Axis_Packet > DEADZONE_HIGH){
    return map(Axis_Packet, AXIS_LOW_PACKET, AXIS_HIGH_PACKET, AXIS_LOW_PWM, AXIS_HIGH_PWM);
  }
  else {
    return AXIS_MID_PWM;
  }
}

void ConvertLoraPacketToSBUS(byte dataPacket[]) {

  //Extract joystick axis values from Packet (-128 to 127)
  int8_t LxPacket = dataPacket[0];
  int8_t LyPacket = dataPacket[1];
  int8_t RxPacket = dataPacket[2];
  int8_t RyPacket = dataPacket[3];

  #if PACKET_LENGTH == 8 //10bit resolution joystick
    int8_t Axis10bitPacket = dataPacket[7];
    //Reconstruct Packets into 10bit resolution joystick data (-512 to 511)
    int16_t Lx10 = LxPacket<<2|bitRead(Axis10bitPacket,7)<<1|bitRead(Axis10bitPacket,6);
    int16_t Ly10 = LyPacket<<2|bitRead(Axis10bitPacket,5)<<1|bitRead(Axis10bitPacket,4);
    int16_t Rx10 = RxPacket<<2|bitRead(Axis10bitPacket,3)<<1|bitRead(Axis10bitPacket,2);
    int16_t Ry10 = RyPacket<<2|bitRead(Axis10bitPacket,1)<<1|bitRead(Axis10bitPacket,0);
    //Map Joystick Axis to SBUS Values
    //Explanation: readAxis(Axis_Packet, LOWEST_PACKET_VALUE_USED, HIGHEST_PACKET_VALUE_USED, LOWEST_PWM_VALUE_USED, HIGHEST_PWM_VALUE_USED, AXIS_INVERTED)
    int32_t Lx = readAxis(Lx10, -512, 511, 432, 1552, 0);
    int32_t Ly = readAxis(Ly10, -512, 511, 192, 1792, 1); //This value is inverted on Unitree Go2, this makes joystick not inverted
    int32_t Rx = readAxis(Rx10, -512, 511, 432, 1552, 0);
    int32_t Ry = readAxis(Ry10, -512, 511, 432, 1552, 0);
  #else //8bit resolution joystick
    //Map Joystick Axis to SBUS Values
    //Explanation: readAxis(Axis_Packet, LOWEST_PACKET_VALUE_USED, HIGHEST_PACKET_VALUE_USED, LOWEST_PWM_VALUE_USED, HIGHEST_PWM_VALUE_USED, AXIS_INVERTED)
    int32_t Lx = readAxis(LxPacket, -128, 127, 432, 1552, 0);
    int32_t Ly = readAxis(LyPacket, -128, 127, 192, 1792, 1); //This value is inverted on Unitree Go2, this makes joystick not inverted
    int32_t Rx = readAxis(RxPacket, -128, 127, 432, 1552, 0);
    int32_t Ry = readAxis(RyPacket, -128, 127, 432, 1552, 0);
  #endif

  //Extract Button States from Packet
  uint8_t Buttons_Byte1 = dataPacket[4];
  uint8_t Buttons_Byte2 = dataPacket[5];

  //Extract DPAD Button States from Buttons_Byte1
  bool DPAD_UP, DPAD_DOWN, DPAD_RIGHT, DPAD_LEFT;
  DPAD_UP = (Buttons_Byte1 & 0b00010000) != 0;
  DPAD_DOWN = (Buttons_Byte1 & 0b00100000) != 0;
  DPAD_RIGHT = (Buttons_Byte1 & 0b01000000) != 0;
  DPAD_LEFT = (Buttons_Byte1 & 0b10000000) != 0;

  //Extract Misc Button States from Buttons_Byte1
  bool XBOX, START, SELECT, SHARE;
  XBOX = (Buttons_Byte1 & 0b00000001) != 0;
  SELECT = (Buttons_Byte1 & 0b00000010) != 0;
  START = (Buttons_Byte1 & 0b00000100) != 0;
  SHARE = (Buttons_Byte1 & 0b00001000) != 0;

  //Extract All Button States from Buttons_Byte2
  bool A, B, X, Y, L1, L2, R1, R2;
  A = (Buttons_Byte2 & 0b00000001) != 0;
  B = (Buttons_Byte2 & 0b00000010) != 0;
  X = (Buttons_Byte2 & 0b00000100) != 0;
  Y = (Buttons_Byte2 & 0b00001000) != 0;
  L1 = (Buttons_Byte2 & 0b00010000) != 0;
  R1 = (Buttons_Byte2 & 0b00100000) != 0;
  L2 = (Buttons_Byte2 & 0b01000000) != 0;
  R2 = (Buttons_Byte2 & 0b10000000) != 0;

  #if PACKET_LENGTH >=7
    //Extract All Button States from Buttons_Byte3
    uint8_t Buttons_Byte3 = dataPacket[6];
    bool L3, R3;
    L3 = (Buttons_Byte3 & 0b00000001) != 0;
    R3 = (Buttons_Byte3 & 0b00000010) != 0;
  #endif

  //Show axis values and button states via the serial console
  #if defined(DEBUG_ENABLED)
    #if PACKET_LENGTH >= 7
      Serial.println(
        "Lx: "+String(Lx)+" ,\
        Ly: "+String(Ly)+" ,\
        Rx: "+String(Rx)+" ,\
        Ry: "+String(Ry)+" ,\
        DPAD_UP: "+String(DPAD_UP)+" ,\
        DPAD_DOWN: "+String(DPAD_DOWN)+" ,\
        DPAD_RIGHT: "+String(DPAD_RIGHT)+" ,\
        DPAD_LEFT: "+String(DPAD_LEFT)+" ,\
        XBOX: "+String(XBOX)+" ,\
        START: "+String(START)+" ,\
        SELECT: "+String(SELECT)+" ,\
        SHARE: "+String(SHARE)+" ,\
        A: "+String(A)+" ,\
        B: "+String(B)+" ,\
        X: "+String(X)+" ,\
        Y: "+String(Y)+" ,\
        L1: "+String(L1)+" ,\
        R1: "+String(R1)+" ,\
        L2: "+String(L2)+" ,\
        R2: "+String(R2)+" ,\
        L3: "+String(L3)+" ,\
        R3: "+String(R3)
      );
    #else
      Serial.println(
        "Lx: "+String(Lx)+" ,\
        Ly: "+String(Ly)+" ,\
        Rx: "+String(Rx)+" ,\
        Ry: "+String(Ry)+" ,\
        DPAD_UP: "+String(DPAD_UP)+" ,\
        DPAD_DOWN: "+String(DPAD_DOWN)+" ,\
        DPAD_RIGHT: "+String(DPAD_RIGHT)+" ,\
        DPAD_LEFT: "+String(DPAD_LEFT)+" ,\
        XBOX: "+String(XBOX)+" ,\
        START: "+String(START)+" ,\
        SELECT: "+String(SELECT)+" ,\
        SHARE: "+String(SHARE)+" ,\
        A: "+String(A)+" ,\
        B: "+String(B)+" ,\
        X: "+String(X)+" ,\
        Y: "+String(Y)+" ,\
        L1: "+String(L1)+" ,\
        R1: "+String(R1)+" ,\
        L2: "+String(L2)+" ,\
        R2: "+String(R2)
      );
    #endif
  #endif


  //Initialize all channels with PWM values specific to the Unitree Go2 Application

  // MODE FLAGS:
  int32_t MODE1 = 992;
  int32_t MODE2 = 192;
  int32_t MODE3 = 1792;

  int32_t _LOW = 192;
  int32_t _MID = 992;
  int32_t _HIGH = 1792;

  int32_t CH1_STATE = Rx;
  int32_t CH2_STATE = Ry;
  int32_t CH3_STATE = Ly;
  int32_t CH4_STATE = Lx;

  int32_t CH5_STATE = MODE1;
  int32_t CH6_STATE = _LOW;
  int32_t CH7_STATE = _LOW;
  int32_t CH8_STATE = _MID;

  // Store Values Here to write to SBUS TX
  SBUS_DATA tx_data;

  tx_data.CH1 = CH1_STATE;
  tx_data.CH2 = CH2_STATE;
  tx_data.CH3 = CH3_STATE;
  tx_data.CH4 = CH4_STATE;
  tx_data.CH5 = CH5_STATE;
  tx_data.CH6 = CH6_STATE;
  tx_data.CH7 = CH7_STATE;
  tx_data.CH8 = CH8_STATE;
  
  //The below code is various examples of mapping different Unitree Go2 8 channel SBUS Commands to various buttons on a XBOX Controller

  // START BUTTON - Unlock
  if(START == true){
    if(START_Flag == 0){
      CH5_STATE = MODE1;
      CH7_STATE = _LOW;
      tx_data.CH5 = CH5_STATE;
      tx_data.CH7 = CH7_STATE;

      writeSBUS(tx_data);
      delayMicroseconds(600);

      CH5_STATE = MODE1;
      CH7_STATE = _HIGH;
      tx_data.CH5 = CH5_STATE;
      tx_data.CH7 = CH7_STATE;

      writeSBUS(tx_data);
      delayMicroseconds(600);

      CH5_STATE = MODE1;
      CH7_STATE = _LOW;
      tx_data.CH5 = CH5_STATE;
      tx_data.CH7 = CH7_STATE;

      writeSBUS(tx_data);
      delayMicroseconds(600);

      START_Flag = 1;

      return;      
    }
    else{}
  }
  else if(START_Flag = 1 && START == 0){
    START_Flag = 0;
  }

  // SELECT BUTTON - Damping State
  if(SELECT == 1){
    if(SELECT_Flag == 0){
      CH5_STATE = MODE1;
      CH6_STATE = _LOW;
      tx_data.CH5 = CH5_STATE;
      tx_data.CH6 = CH6_STATE;

      writeSBUS(tx_data);
      delayMicroseconds(600);

      CH5_STATE = MODE1;
      CH6_STATE = _HIGH;
      tx_data.CH5 = CH5_STATE;
      tx_data.CH6 = CH6_STATE;

      writeSBUS(tx_data);
      delayMicroseconds(600);

      CH5_STATE = MODE1;
      CH6_STATE = _LOW;
      tx_data.CH5 = CH5_STATE;
      tx_data.CH6 = CH6_STATE;

      writeSBUS(tx_data);
      delayMicroseconds(600);

      SELECT_Flag = 1;

      return;      
    }
    else{}
  }
  else if(SELECT_Flag = 1 && SELECT == 0){
    SELECT_Flag = 0;
  }

  // DPAD BUTTONS
  // DPAD UP BUTTON - Fall Recover
  if(DPAD_UP == 1){

    if(D_UP_Flag == 0){
      CH5_STATE = MODE2;
      CH7_STATE = _LOW;
      tx_data.CH5 = CH5_STATE;
      tx_data.CH7 = CH7_STATE;

      writeSBUS(tx_data);
      delayMicroseconds(600);

      CH5_STATE = MODE2;
      CH7_STATE = _HIGH;
      tx_data.CH5 = CH5_STATE;
      tx_data.CH7 = CH7_STATE;

      writeSBUS(tx_data);
      delayMicroseconds(600);

      CH5_STATE = MODE2;
      CH7_STATE = _LOW;
      tx_data.CH5 = CH5_STATE;
      tx_data.CH7 = CH7_STATE;

      writeSBUS(tx_data);
      delayMicroseconds(600);

      D_UP_Flag = 1;

      return;
      }  
  }
  else if(DPAD_UP == 0 && D_UP_Flag == 1){
    D_UP_Flag = 0;
  }
  else{}

  // DPAD DOWN BUTTON - STAND Toggle
  if (DPAD_DOWN == true) {
    if(D_DOWN_Flag == 0){

      if(HIGH_STAND_Flag == 0){

        CH5_STATE = MODE1;
        CH8_STATE = _MID;
        tx_data.CH5 = CH5_STATE;
        tx_data.CH8 = CH8_STATE;

        writeSBUS(tx_data);
        delayMicroseconds(600);

        CH5_STATE = MODE1;
        CH8_STATE = _HIGH;
        tx_data.CH5 = CH5_STATE;
        tx_data.CH8 = CH8_STATE;

        writeSBUS(tx_data);
        delayMicroseconds(600);

        CH5_STATE = MODE1;
        CH8_STATE = _LOW;
        tx_data.CH5 = CH5_STATE;
        tx_data.CH8 = CH8_STATE;

        writeSBUS(tx_data);
        delayMicroseconds(600);

        D_DOWN_Flag = 1;
        HIGH_STAND_Flag = 1;

        return;

      }
      else if (HIGH_STAND_Flag == 1) {

        CH5_STATE = MODE1;
        CH8_STATE = _MID;
        tx_data.CH5 = CH5_STATE;
        tx_data.CH8 = CH8_STATE;

        writeSBUS(tx_data);
        delayMicroseconds(600);

        CH5_STATE = MODE1;
        CH8_STATE = _LOW;
        tx_data.CH5 = CH5_STATE;
        tx_data.CH8 = CH8_STATE;

        writeSBUS(tx_data);
        delayMicroseconds(600);

        CH5_STATE = MODE1;
        CH8_STATE = _MID;
        tx_data.CH5 = CH5_STATE;
        tx_data.CH8 = CH8_STATE;

        writeSBUS(tx_data);
        delayMicroseconds(600);

        D_DOWN_Flag = 1;
        HIGH_STAND_Flag = 0;

        return;

      }
            
    } 
  }
  else if(DPAD_DOWN == 0 && D_DOWN_Flag == 1){
    D_DOWN_Flag = 0;
  }
  else{}

  // DPAD RIGHT BUTTON - Light Toggle
  if (DPAD_RIGHT == true) {
    if(D_RIGHT_Flag == 0){

      CH5_STATE = MODE3;
      CH8_STATE = _MID;
      tx_data.CH5 = CH5_STATE;
      tx_data.CH8 = CH8_STATE;

      writeSBUS(tx_data);
      delayMicroseconds(600);

      CH5_STATE = MODE3;
      CH8_STATE = _LOW;
      tx_data.CH5 = CH5_STATE;
      tx_data.CH8 = CH8_STATE;

      writeSBUS(tx_data);
      delayMicroseconds(600);

      CH5_STATE = MODE3;
      CH8_STATE = _MID;
      tx_data.CH5 = CH5_STATE;
      tx_data.CH8 = CH8_STATE;

      writeSBUS(tx_data);
      delayMicroseconds(600);

      D_RIGHT_Flag= 1;

      return;
    } 
  }
  else if(DPAD_RIGHT == 0 && D_RIGHT_Flag == 1){
    D_RIGHT_Flag = 0;
  }
  else{}

  // DPAD LEFT BUTTON - Continous Movement
  if (DPAD_LEFT == 1) {
    if(D_LEFT_Flag == 0){

      CH5_STATE = MODE2;
      CH6_STATE = _LOW;
      tx_data.CH5 = CH5_STATE;
      tx_data.CH6 = CH6_STATE;

      writeSBUS(tx_data);
      delayMicroseconds(600);

      CH5_STATE = MODE2;
      CH6_STATE = _HIGH;
      tx_data.CH5 = CH5_STATE;
      tx_data.CH6 = CH6_STATE;

      writeSBUS(tx_data);
      delayMicroseconds(600);

      CH5_STATE = MODE2;
      CH6_STATE = _LOW;
      tx_data.CH5 = CH5_STATE;
      tx_data.CH6 = CH6_STATE;

      writeSBUS(tx_data);
      delayMicroseconds(600);

      D_LEFT_Flag= 1;

      return;
    } 
  }
  else if(DPAD_LEFT == 0 && D_LEFT_Flag == 1){
    D_LEFT_Flag = 0;
  }
  else{}

  // SHOULDER Buttons:
  // R1 Button - Walking Gait Toggle 
  if (R1 == true) {
    if(R1_Flag == 0){

      CH5_STATE = MODE3;
      CH8_STATE = _MID;
      tx_data.CH5 = CH5_STATE;
      tx_data.CH8 = CH8_STATE;

      writeSBUS(tx_data);
      delayMicroseconds(600);

      CH5_STATE = MODE3;
      CH8_STATE = _HIGH;
      tx_data.CH5 = CH5_STATE;
      tx_data.CH8 = CH8_STATE;

      writeSBUS(tx_data);
      delayMicroseconds(600);

      CH5_STATE = MODE3;
      CH8_STATE = _MID;
      tx_data.CH5 = CH5_STATE;
      tx_data.CH8 = CH8_STATE;

      writeSBUS(tx_data);
      delayMicroseconds(600);

      R1_Flag= 1;

      return;
    } 
  }
  else if(R1 == 0 && R1_Flag == 1){
    R1_Flag = 0;
  }
  else{}


  // FACE Buttons:
  // X BUTTON - Obstacle Avoidance Toggle
  if (X == 1) {
    if(X_Flag == 0){

      if(OBSTACLE_Flag == 0){

        CH5_STATE = MODE2;
        CH8_STATE = _MID;
        tx_data.CH5 = CH5_STATE;
        tx_data.CH8 = CH8_STATE;

        writeSBUS(tx_data);
        delayMicroseconds(600);

        CH5_STATE = MODE2;
        CH8_STATE = _LOW;
        tx_data.CH5 = CH5_STATE;
        tx_data.CH8 = CH8_STATE;

        writeSBUS(tx_data);
        delayMicroseconds(600);

        CH5_STATE = MODE2;
        CH8_STATE = _MID;
        tx_data.CH5 = CH5_STATE;
        tx_data.CH8 = CH8_STATE;

        writeSBUS(tx_data);
        delayMicroseconds(600);

        X_Flag = 1;
        OBSTACLE_Flag = 1;

        return;
      }
      else if (OBSTACLE_Flag == 1) {

        CH5_STATE = MODE2;
        CH8_STATE = _MID;
        tx_data.CH5 = CH5_STATE;
        tx_data.CH8 = CH8_STATE;

        writeSBUS(tx_data);
        delayMicroseconds(600);

        CH5_STATE = MODE2;
        CH8_STATE = _HIGH;
        tx_data.CH5 = CH5_STATE;
        tx_data.CH8 = CH8_STATE;

        writeSBUS(tx_data);
        delayMicroseconds(600);

        CH5_STATE = MODE2;
        CH8_STATE = _MID;
        tx_data.CH5 = CH5_STATE;
        tx_data.CH8 = CH8_STATE;

        writeSBUS(tx_data);
        delayMicroseconds(600);

        X_Flag = 1;
        OBSTACLE_Flag = 0;

        return;
      }
    } 
  }
  else if(X_Flag == 1 && X == 0){
    X_Flag = 0;
  }
  else{}

  // Y BUTTON - Dance
  if(Y == 1){
    if(Y_Flag == 0){
      CH5_STATE = MODE3;
      CH7_STATE = _LOW;
      tx_data.CH5 = CH5_STATE;
      tx_data.CH7 = CH7_STATE;

      writeSBUS(tx_data);
      delayMicroseconds(600);

      CH5_STATE = MODE3;
      CH7_STATE = _HIGH;
      tx_data.CH5 = CH5_STATE;
      tx_data.CH7 = CH7_STATE;

      writeSBUS(tx_data);
      delayMicroseconds(600);

      CH5_STATE = MODE3;
      CH7_STATE = _LOW;
      tx_data.CH5 = CH5_STATE;
      tx_data.CH7 = CH7_STATE;

      writeSBUS(tx_data);
      delayMicroseconds(600);

      Y_Flag = 1;

      return;      
    }
    else{}
  }
  else if(Y_Flag = 1 && Y == 0){
    Y_Flag = 0;
  }

  // A BUTTON - Jump
  if (A == 1) {
    if(A_Flag == 0){

      CH5_STATE = MODE3;
      CH6_STATE = _LOW;
      tx_data.CH5 = CH5_STATE;
      tx_data.CH6 = CH6_STATE;

      writeSBUS(tx_data);
      delayMicroseconds(600);

      CH5_STATE = MODE3;
      CH6_STATE = _HIGH;
      tx_data.CH5 = CH5_STATE;
      tx_data.CH6 = CH6_STATE;

      writeSBUS(tx_data);
      delayMicroseconds(600);

      CH5_STATE = MODE3;
      CH6_STATE = _LOW;
      tx_data.CH5 = CH5_STATE;
      tx_data.CH6 = CH6_STATE;

      writeSBUS(tx_data);
      delayMicroseconds(600);

      A_Flag= 1;

      return;
    } 
  }
  else if(A == 0 && A_Flag == 1){
    A_Flag = 0;
  }
  else{}

  data.ch[0] = CH1_STATE;
  data.ch[1] = CH2_STATE;
  data.ch[2] = CH3_STATE;
  data.ch[3] = CH4_STATE;
  data.ch[4] = CH5_STATE;
  data.ch[5] = CH6_STATE;
  data.ch[6] = CH7_STATE;
  data.ch[7] = CH8_STATE;
  data.ch[8] = 1690; //CH9 static PWM value as shown in Unitree Go2 SBUS Documentation
  data.ch[9] = 154; //CH10 static PWM value as shown in Unitree Go2 SBUS Documentation

  //Output SBUS Data
  sbus_tx.data(data);
  sbus_tx.Write();
  #if defined(DEBUG_ENABLED)
    Serial.println(
      "Ch1: "+String(data.ch[0])+" ,\
      Ch2: "+String(data.ch[1])+" ,\
      Ch3: "+String(data.ch[2])+" ,\
      Ch4: "+String(data.ch[3])+" ,\
      Ch5: "+String(data.ch[4])+" ,\
      Ch6: "+String(data.ch[5])+" ,\
      Ch7: "+String(data.ch[6])+" ,\
      Ch8: "+String(data.ch[7])
    );
  #endif
}