// Include ChibiOS library for AVR microcontrollers
#include <ChibiOS_AVR.h> // Note: This is a port of ChibiOS to Arduino
// Include CAN libraries
#include <mcp2515.h>

MCP2515 mcp2515(10);

// Function prototypes
void UART_Init(void);
void CAN_Init(void);


// Stack size to be assigned to the thread
// More info http://chibios.sourceforge.net/docs3/rt/group__threads.html
static THD_WORKING_AREA(waTh1, 100); // Working area 1 of 100 bytes
static THD_WORKING_AREA(waTh2, 200); // Working area 2 of 200 bytes
static THD_WORKING_AREA(waTh3, 200); // Working area 3 of 200 bytes

// Structure to pass to threads
struct threadData{
  int _blinkTime;
  int _lightPin;
};

// Global variables to be used with CAN messages
char myChar;
char toggleSTM32LED;

// Blink LED thread function
static THD_FUNCTION (blinkerThread, arg){
  //setup thread vars
  threadData *thisData  = (threadData*)arg;
  int lightPin          = thisData->_lightPin;
  int blinkTime         = thisData->_blinkTime;

  //set the LED pinMode
  pinMode(lightPin, OUTPUT);

  while(1){
    //blink
    digitalWrite(lightPin, HIGH);
    chThdSleep(blinkTime);
    
    digitalWrite(lightPin, LOW);
    chThdSleep(blinkTime);
  }
}

// Switch STM32 LED thread
static THD_FUNCTION (ledSwitchThread, arg){
  threadData *thisData  = (threadData*)arg;
  int lightPin          = thisData->_lightPin;
  struct can_frame canMsg;
  
  pinMode(lightPin, OUTPUT);
  digitalWrite(lightPin, HIGH);
  
  while(1){

    if (Serial.available()){
      myChar = Serial.read();

      if(myChar == 'b'){
        Serial.println("Sending command to turn on STM LED");
        toggleSTM32LED = 0x10;
      }

      if(myChar == 'n'){
        Serial.println("Sending command to turn off STM LED");
        toggleSTM32LED = 0x00;
      }
    }

    // Check if we have STM32 messages on CAN
    if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) { // Check for any messages

        if(canMsg.can_id == 0x101U || canMsg.can_id == 0x103U) { // Check the ID of message
          Serial.print("ID: ");
          Serial.print(canMsg.can_id,HEX);
          Serial.print(", ");
          Serial.print("DLC: ");
          Serial.print(canMsg.can_dlc,DEC);
          Serial.print(", ");
          Serial.print("Data: ");

          for(int i=0; i < canMsg.can_dlc; i++){ 
            Serial.print(canMsg.data[i],HEX);
            Serial.print(" ");
          }
          Serial.println("Message received!");
        }
    }

    // Execute every 500ms
    chThdSleep(500);
  }
}

// Send CAN message thread
static THD_FUNCTION (messageSendThread, arg){

  struct can_frame canMsg;
  
  while(1){

    canMsg.can_id             = 0x631; //formatted in HEX
    canMsg.can_dlc            = 8; //formatted in DEC

    if(toggleSTM32LED == 0x10){
      canMsg.data[0] = 0x50;
    }
    else{
      canMsg.data[0] = 0x40;
    }
      
    canMsg.data[1] = 0x05;
    canMsg.data[2] = 0x30;
    canMsg.data[3] = 0xFA; //formatted in HEX
    canMsg.data[4] = 0xBE;
    canMsg.data[5] = 0x40;
    canMsg.data[6] = 0x30;
    canMsg.data[7] = 0x22;
    
    mcp2515.sendMessage(&canMsg);
    
    Serial.println("CAN message was sent");
    
    chThdSleep(5500);
  }
}

//-------------- UART INIT -----------
void UART_Init(void){

  // Init serial communication at 9600 baud
  Serial.begin(9600);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  }
  
  Serial.println("Serial is running!");

  // Just a delay
  delay(300);
}


//-------------- CAN INIT -----------
void CAN_Init(void){

  Serial.println("Attempting to start CAN Module");

  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS,MCP_8MHZ);
  mcp2515.setNormalMode();
}

//------------- ChibiOS SETUP -------------------------------
void chSetup(){

  // Create thread data set 1
  threadData set1;
  set1._lightPin = 7; // D7 PIN
  set1._blinkTime = 300;
  
  // Create thread data set 2
  threadData set2;
  set2._lightPin = 8; // D8 PIN


  // Schedule blinkerThread 
  chThdCreateStatic(waTh1, sizeof(waTh1), NORMALPRIO, blinkerThread, (void*)&set1);

  // Schedule ledSwitchThread
  chThdCreateStatic(waTh2, sizeof(waTh2), NORMALPRIO, ledSwitchThread, (void*)&set2);

  // Schedule messageSendThread
  chThdCreateStatic(waTh3, sizeof(waTh3), NORMALPRIO, messageSendThread, NULL);
  
  // IDLE
  while(1){

    chThdSleep(10000);
  }
}

//--------------------- Arduino SETUP ----------------------------
void setup(){

  UART_Init();
  CAN_Init();
  
  // initialize and start ChibiOS
  chBegin(chSetup); // Spawn point of ChibiOS
  
  // should not return
  while(1);
}

//-------------------- Arduino LOOP ----------------------------
void loop(){
  /* not used */
}
