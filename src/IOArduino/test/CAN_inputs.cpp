#include <Arduino.h>
#include <SPI.h>
#include <mcp2515.h>

struct can_frame canMsgReceived;
struct can_frame canMsgRequestInputs1;
struct can_frame canMsgRequestInputs2;
MCP2515 mcp2515(10);
unsigned long timeStamp = 0;

void receiveEventCAN();

void setup() {
  canMsgRequestInputs1.can_id  = 12;
  canMsgRequestInputs1.can_dlc = 2;
  canMsgRequestInputs1.data[0] = 0 + (2 << 4);
  canMsgRequestInputs1.data[1] = 0x39;

  canMsgRequestInputs2.can_id  = 12;
  canMsgRequestInputs2.can_dlc = 2;
  canMsgRequestInputs2.data[0] = 0 + (1 << 4);
  canMsgRequestInputs2.data[1] = A0;

  while (!Serial);
  Serial.begin(9600);
  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_125KBPS);
  mcp2515.setNormalMode();
  
  Serial.println("Example: Read via CAN");
}

void loop() {
  if (millis() >= timeStamp + 1000) {
    mcp2515.sendMessage(&canMsgRequestInputs1);
    mcp2515.sendMessage(&canMsgRequestInputs2);
    Serial.println("Requests sent");
    timeStamp = millis();
  }
  if (mcp2515.readMessage(&canMsgReceived) == MCP2515::ERROR_OK) {
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
}
