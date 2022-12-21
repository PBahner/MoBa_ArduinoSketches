#include "SwitchServo.h"
#include <mcp2515.h>

#define MODULE_ID 1

#define DLC_MSG_TARGET_SWITCH_STATES 1
#define ID_MSG_TARGET_SWITCH_STATES 1

MCP2515 mcp2515(10);
struct can_frame canMsgReceived;

struct Switch {
  SwitchServo servo;
  uint8_t id;
***REMOVED***;

#if MODULE_ID == 0
  #define MSG_DATA_BYTE 0
  Switch switches[5] = {{SwitchServo(), 0***REMOVED***,
                        {SwitchServo(), 1***REMOVED***,
                        {SwitchServo(), 2***REMOVED***,
                        {SwitchServo(), 3***REMOVED***,
                        {SwitchServo(), 4***REMOVED******REMOVED***;
  const int deflection[5] = {400, 400, 600, 600, 800***REMOVED***;
#elif MODULE_ID == 1
  #define MSG_DATA_BYTE 1
  Switch switches[2] = {{SwitchServo(), 0***REMOVED***,
                        {SwitchServo(), 1***REMOVED******REMOVED***;
  const int deflection[5] = {400, 600***REMOVED***;
#endif

const uint8_t servoPins[8] = {3, 4, 5, 6, 7, 8, 9***REMOVED***;
const uint8_t potiPins[8] = {A7, A6, A5, A4, A3, A2, A1***REMOVED***;

***REMOVED***
  Serial.begin(9600);

  // status-LED
  pinMode(A0, OUTPUT);

  // initialize switch servos
  for(int i=0; i<sizeof(switches)/sizeof(Switch); i++) {
    switches[i].servo.init(servoPins[i], potiPins[i], deflection[i]);
***REMOVED***

  // setup CAN
  mcp2515.reset();
  mcp2515.setBitrate(CAN_125KBPS);
  mcp2515.setNormalMode();
***REMOVED***

***REMOVED***
  // blink status-LED
  if(millis()%500 < 250) {
    digitalWrite(A0, HIGH);
***REMOVED*** else {
    digitalWrite(A0, LOW);
***REMOVED***

  // check CAN-Bus for messages
  if (mcp2515.readMessage(&canMsgReceived) == MCP2515::ERROR_OK) {
    //if (canMsgReceived.can_id == ID_MSG_TARGET_SWITCH_STATES)
      receiveEventCAN();
***REMOVED***
***REMOVED***

void receiveEventCAN() {
  Serial.print(canMsgReceived.can_id); // print ID
  Serial.print(" "); 
  Serial.print(canMsgReceived.can_dlc); // print DLC
  Serial.print(" ");
  for(int i=0; i<canMsgReceived.can_dlc; i++){
    Serial.print(canMsgReceived.data[i]); 
***REMOVED***
  Serial.println(" empfangen");
  switch(canMsgReceived.can_id) {
    case 1: updateSwitchStates(); Serial.println("Weiche"); break;
***REMOVED***
***REMOVED***

void updateSwitchStates() {
  for(int i=0; i<sizeof(switches)/sizeof(Switch); i++) {
    bool state = bitRead(canMsgReceived.data[MSG_DATA_BYTE], i);
    Serial.println("move");
    switches[i].servo.moveSwitchServo(state);
***REMOVED***
***REMOVED***
