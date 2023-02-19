#include "SwitchServo.h"
#include <mcp2515.h>

#define MODULE_ID 0

#define DLC_MSG_TARGET_SWITCH_STATES 1
#define ID_MSG_TARGET_SWITCH_STATES 1

MCP2515 mcp2515(10);
struct can_frame canMsgReceived;

struct Switch {
  SwitchServo servo;
  uint8_t id;
};

#if MODULE_ID == 0
  #define MSG_DATA_BYTE 0
  Switch switches[5] = {{SwitchServo(), 4},  // Switch 6
                        {SwitchServo(), 3},  // Switch 5
                        {SwitchServo(), 2},  // Switch 2
                        {SwitchServo(), 1},  // Switch 1
                        {SwitchServo(), 0}};  // Switch 0
  const int deflection[5] = {800, 600, 600, 400, 400};
#elif MODULE_ID == 1
  #define MSG_DATA_BYTE 1
  Switch switches[2] = {{SwitchServo(), 0},
                        {SwitchServo(), 1}};
  const int deflection[5] = {400, 600};
#endif

const uint8_t servoPins[8] = {3, 4, 5, 6, 7, 8, 9};
const uint8_t potiPins[8] = {A7, A6, A5, A4, A3, A2, A1};

void setup() {
  Serial.begin(9600);

  // status-LED
  pinMode(A0, OUTPUT);

  // initialize switch servos
  for(int i=0; i<sizeof(switches)/sizeof(Switch); i++) {
    switches[i].servo.init(servoPins[i], potiPins[i], deflection[i]);
  }

  // setup CAN
  mcp2515.reset();
  mcp2515.setBitrate(CAN_125KBPS);
  mcp2515.setNormalMode();
}

void loop() {
  // blink status-LED
  if(millis()%500 < 250) {
    digitalWrite(A0, HIGH);
  } else {
    digitalWrite(A0, LOW);
  }

  // check CAN-Bus for messages
  if (mcp2515.readMessage(&canMsgReceived) == MCP2515::ERROR_OK) {
    //if (canMsgReceived.can_id == ID_MSG_TARGET_SWITCH_STATES)
      receiveEventCAN();
  }
}

void receiveEventCAN() {
  Serial.print(canMsgReceived.can_id); // print ID
  Serial.print(" "); 
  Serial.print(canMsgReceived.can_dlc); // print DLC
  Serial.print(" ");
  for(int i=0; i<canMsgReceived.can_dlc; i++){
    Serial.print(canMsgReceived.data[i]); 
  }
  Serial.println(" empfangen");
  switch(canMsgReceived.can_id) {
    case 1: updateSwitchStates(); Serial.println("Weiche"); break;
  }
}

void updateSwitchStates() {
  for(int i=0; i<sizeof(switches)/sizeof(Switch); i++) {
    bool state = bitRead(canMsgReceived.data[MSG_DATA_BYTE], switches[i].id);
    Serial.println("move");
    switches[i].servo.moveSwitchServo(state);
  }
}
