#include <Arduino.h>
#include <mcp2515.h>
#include <PCF8574.h>

void receiveEventCAN();
void updateOutputs();
void updateInputs();

#define MODULE_ID 0

#define ID_MSG_TARGET_OUTPUT_STATES 10
#define ID_MSG_CURRENT_INPUT_STATES 11

MCP2515 mcp2515(10);
struct can_frame canMsgReceived;
struct can_frame canMsgInputs;

PCF8574 PCFInterface;

void setup() {

  // initialize PCF-I2C-Bus
  PCFInterface.begin();

  // setup CAN
  mcp2515.reset();
  mcp2515.setBitrate(CAN_125KBPS);
  mcp2515.setNormalMode();

  canMsgInputs.can_dlc = 3;
  canMsgInputs.can_id = 11;

  PCFInterface.setAddress(0x38);
  PCFInterface.write(0, LOW);
  delay(500);
  PCFInterface.write(0, HIGH);
  delay(500);
  PCFInterface.setAddress(0x20);
  PCFInterface.write(0, LOW);
  delay(500);
  PCFInterface.write(0, HIGH);
  delay(500); 
}

void loop() {
  // check CAN-Bus for messages
  if (mcp2515.readMessage(&canMsgReceived) == MCP2515::ERROR_OK) {
    //if (canMsgReceived.can_id == ID_MSG_TARGET_SWITCH_STATES)
    receiveEventCAN();
  }
  // PCFInterface.setAddress(0x20);
  //PCFInterface.write8(255);
  // PCFInterface.write(mcp2515.readMessage(&canMsgReceived), LOW);
  //PCFInterface.write(mcp2515.getStatus()%8, LOW);
  delay(10);
}

void receiveEventCAN() {
  // PCFInterface.setAddress(0x20);
  // PCFInterface.write8(canMsgReceived.can_id);
  // return;
  switch(canMsgReceived.can_id) {
    case 10: updateOutputs(); break;
    case 12: updateInputs(); break; // request
  }
}

void updateOutputs() {
  uint8_t moduleId = canMsgReceived.data[0] & 0x0F;
  uint8_t outputType = canMsgReceived.data[0] >> 4;
  if(moduleId != MODULE_ID) { return; }

  uint8_t outputId = canMsgReceived.data[1];
  uint8_t outputData = canMsgReceived.data[2];

  switch(outputType) {
    case 0: // GPIO-digital-output
      pinMode(outputId, OUTPUT);
      digitalWrite(outputId, outputData);
      break;
    case 1: // GPIO-analog-output
      pinMode(outputId, OUTPUT);
      analogWrite(outputId, outputData);
      break;
    case 2: // PCF8574(A)-digital-ouput
      PCFInterface.setAddress(outputId);
      PCFInterface.write8(~outputData);
      break;
  }
}

void updateInputs() {
  uint8_t moduleId = canMsgReceived.data[0] & 0x0F;
  uint8_t inputType = canMsgReceived.data[0] >> 4;
  if(moduleId != MODULE_ID) { return; }

  uint8_t inputId = canMsgReceived.data[1];  // IC or Pin ID
  canMsgInputs.data[0] = canMsgReceived.data[0]; // copy module-id and output-type
  canMsgInputs.data[1] = inputId;  // copy ic-id or pin-id

  switch (inputType) {
    case 0:  // GPIO-digital-input
      pinMode(inputId, INPUT);
      canMsgInputs.data[2] = digitalRead(inputId);
      mcp2515.sendMessage(&canMsgInputs);
      break;
    case 1:  // GPIO-analog-input
      pinMode(inputId, INPUT);
      canMsgInputs.data[2] = analogRead(inputId);
      mcp2515.sendMessage(&canMsgInputs);
      break;
    case 2:  // PCF8574(A)-digital-input
      PCFInterface.setAddress(inputId);
      canMsgInputs.data[2] = ~PCFInterface.read8();
      mcp2515.sendMessage(&canMsgInputs);
      break;
  }
}