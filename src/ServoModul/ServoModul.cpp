#include "Arduino.h"
#include "SwitchServo.h"
#include <mcp2515.h>

void receiveEventCAN();
void updateTargetSwitchStates();
void initializeSwitchReferences();
void updateCurrentSwitchStates();

#define MODULE_ID 0

#define DLC_MSG_TARGET_SWITCH_STATES 1
#define ID_MSG_TARGET_SWITCH_STATES 1

MCP2515 mcp2515(10);
struct can_frame canMsgReceived;
struct can_frame canMsgRequestInputRefs;

bool inputRefsInitialized = false;

struct InputReference {
  uint8_t input_module;
  uint8_t input_type;
  uint8_t input_address;
  uint8_t input_pin;
};

struct Switch {
  Switch(SwitchServo s, uint8_t i, bool d = false) {
    servo = s;
    id = i;
    direction = d;
  }
  SwitchServo servo;
  uint8_t id;
  bool direction;
  InputReference inputRefPlus;
  InputReference inputRefMinus;
};

#if MODULE_ID == 0
  #define MSG_DATA_BYTE 0
  Switch switches[5] = {{SwitchServo(), 6, false},  // Switch 6
                        {SwitchServo(), 5, false},  // Switch 5
                        {SwitchServo(), 2, false},  // Switch 2
                        {SwitchServo(), 1, true},  // Switch 1
                        {SwitchServo(), 0, false}};  // Switch 0
  const int deflection[5] = {400, 400, 600, 600, 800};
  const bool withFeedback = true;

#elif MODULE_ID == 1
  #define MSG_DATA_BYTE 1
  Switch switches[2] = {{SwitchServo(), 3},
                        {SwitchServo(), 4}};
  const int deflection[5] = {400, 600};
  const bool withFeedback = false;

#endif

const uint8_t servoPins[8] = {3, 4, 5, 6, 7, 8, 9};
const uint8_t potiPins[8] = {A7, A6, A5, A4, A3, A2, A1};

void setup() {
  Serial.begin(115200);

  // status-LED
  pinMode(A0, OUTPUT);

  // initialize switch servos
  for(int i=0; i<sizeof(switches)/sizeof(Switch); i++) {
    switches[i].servo.init(servoPins[i], potiPins[i], deflection[i], switches[i].direction, withFeedback);
  }

  // setup CAN
  mcp2515.reset();
  mcp2515.setBitrate(CAN_125KBPS);
  mcp2515.setNormalMode();

  // create CAN-msg for requesting input references
  canMsgRequestInputRefs.can_dlc = sizeof(switches)/sizeof(Switch);
  canMsgRequestInputRefs.can_id = 3;
  // ToDo: only not initialized switches
  for(int i=0; i<sizeof(switches)/sizeof(Switch); i++) {
    canMsgRequestInputRefs.data[i] = switches[i].id;
  }
}

void loop() {
  // blink status-LED
  if(millis()%500 < 250) {
    digitalWrite(A0, HIGH);
  } else {
    digitalWrite(A0, LOW);
  }

  // request can input references for switch feedback every second
  // ToDo: request only, if not ref available
  if(millis()%2000 == 0) {
    mcp2515.sendMessage(&canMsgRequestInputRefs);
  }

  // check CAN-Bus for messages
  if (mcp2515.readMessage(&canMsgReceived) == MCP2515::ERROR_OK) {
    //if (canMsgReceived.can_id == ID_MSG_TARGET_SWITCH_STATES)
      receiveEventCAN();
  }
}

void receiveEventCAN() {
  // Serial.print(canMsgReceived.can_id); // print ID
  // Serial.print(" "); 
  // Serial.print(canMsgReceived.can_dlc); // print DLC
  // Serial.print(" ");
  // for(int i=0; i<canMsgReceived.can_dlc; i++){
  //   Serial.print(canMsgReceived.data[i]);
  //   Serial.print(" ");
  // }
  // Serial.println("empfangen");
  switch(canMsgReceived.can_id) {
    case 1: updateTargetSwitchStates(); /*Serial.println("Weiche");*/ break;
    case 2: initializeSwitchReferences(); break;
    case 11: updateCurrentSwitchStates(); break;
  }
}

void updateTargetSwitchStates() {
  for(int i=0; i<sizeof(switches)/sizeof(Switch); i++) {
    bool state = bitRead(canMsgReceived.data[MSG_DATA_BYTE], i);
    Serial.print("move: "); Serial.print(switches[i].id); Serial.print("state: "); Serial.println(state);
    switches[i].servo.moveSwitchServo(state);
  }
}

void initializeSwitchReferences() {  // funktionalität bewiesen 22.12.2023
  // Serial.println("initSwitchRef");
  uint8_t switchId = canMsgReceived.data[3] >> 1;
  boolean switchState = bitRead(canMsgReceived.data[3], 0);

  uint8_t switchListIndex;
  bool switchIdExists = false;
  for (int i=0; i<sizeof(switches)/sizeof(Switch); i++) {
    if (switchId == switches[i].id) {
      switchListIndex = i;
      switchIdExists = true;
      break;
    }
  }
  if (!switchIdExists) {return;}

  uint8_t moduleId = canMsgReceived.data[0] & 0x0F;
  uint8_t inputType = canMsgReceived.data[0] >> 4;
  InputReference ref = {moduleId, inputType, canMsgReceived.data[1], canMsgReceived.data[2]};

  switch (switchState) {
    case false: switches[switchListIndex].inputRefPlus = ref; break;
    case true: switches[switchListIndex].inputRefMinus = ref; break;
  }
}

void updateCurrentSwitchStates() {
  uint8_t moduleId = canMsgReceived.data[0] & 0x0F;
  uint8_t inputType = canMsgReceived.data[0] >> 4;
  uint8_t inputId = canMsgReceived.data[1];  // ic-id or pin-id
  uint8_t inputData = canMsgReceived.data[2];

  for(int i=0; i<sizeof(switches)/sizeof(Switch); i++) {  // funktionalität bewiesen 23.12.
    if (
        switches[i].inputRefPlus.input_module == moduleId and
        switches[i].inputRefPlus.input_type == inputType and
        switches[i].inputRefPlus.input_address == inputId) {
      bool switchState = bitRead(inputData, switches[i].inputRefPlus.input_pin);
      // Serial.print("update plus "); Serial.print(switchState);
      // Serial.print(" "); Serial.print(inputData);
      // Serial.print(" switch: "); Serial.println(switches[i].id);
      switches[i].servo.updateCurrentStatePlus(switchState);
    }
    if (
        switches[i].inputRefMinus.input_module == moduleId and
        switches[i].inputRefMinus.input_type == inputType and
        switches[i].inputRefMinus.input_address == inputId) {
      bool switchState = bitRead(inputData, switches[i].inputRefMinus.input_pin);
      // Serial.print("update minus "); Serial.print(switchState);
      // Serial.print(" "); Serial.println(inputData);
      switches[i].servo.updateCurrentStateMinus(switchState);
    }
  }
}
