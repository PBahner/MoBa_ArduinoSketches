#include <Arduino.h>
#include <SPI.h>
#include <mcp2515.h>

struct can_frame canMsg1;
struct can_frame canMsg2;
struct can_frame canMsg3;
struct can_frame canMsg4;
MCP2515 mcp2515(10);

void setup() {
  canMsg1.can_id  = 10;
  canMsg1.can_dlc = 3;
  canMsg1.data[0] = 0 + (2 << 4);
  canMsg1.data[1] = 0x20;

  canMsg2.can_id  = 10;
  canMsg2.can_dlc = 3;
  canMsg2.data[0] = 0 + (2 << 4);
  canMsg2.data[1] = 0x20;
  canMsg2.data[2] = 10;

  canMsg3.can_id  = 10;
  canMsg3.can_dlc = 3;
  canMsg3.data[0] = 0 + (1 << 4);
  canMsg3.data[1] = 3;
  canMsg3.data[2] = 125;

  canMsg4.can_id  = 10;
  canMsg4.can_dlc = 3;
  canMsg4.data[0] = 0 + (1 << 4);
  canMsg4.data[1] = 3;
  canMsg4.data[2] = 255;

  while (!Serial);
  Serial.begin(115200);
  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_125KBPS);
  mcp2515.setNormalMode();
  
  Serial.println("Example: Write via CAN");
}

void loop() {
  mcp2515.sendMessage(&canMsg1);
  mcp2515.sendMessage(&canMsg3);
  delay(1000);
  mcp2515.sendMessage(&canMsg2);
  mcp2515.sendMessage(&canMsg4);

  Serial.println("Messages sent");
  
  delay(1000);
}
