#include <SocketIOclient.h>
#include <ArduinoJson.h>
#include "WlanPass.h"
#include "Buttons.h"
#include "WiFi.h"
#include "MCP23017.h" // library from RobTillaart https://github.com/RobTillaart/MCP23017_RT

Buttons buttons(0x38, 0x39); // (0x20, 0x21)
MCP23017 Expander1(0x20);

SocketIOclient socketIO;
WiFiClient client;
const char host[] = "192.168.10.130";
const int port = 5000; // Socket.IO Port Address
const char path[] = "/socket.io/?EIO=4"; // Socket.IO Base Path
    
unsigned long lastBtnEvent;

void setup() {
  Serial.begin(115200);
  Expander1.begin();

  Expander1.pinMode8(0, 0x00);
  Expander1.pinMode8(1, 0x00);

  // initialize button listeners 
  buttons.onButtonsDown(1, onSwitchButtonsDown);
  buttons.onButtonsDown(2, onSwitchButtonsDown);

  // connect to WiFi
  Serial.print("Connecting to ");
  Serial.println(WlanPass::ssid);
  WiFi.begin(WlanPass::ssid, WlanPass::pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("IP adress: ");
  Serial.println(WiFi.localIP());

  // initialize socketio events
  socketIO.onEvent(onSocketIOEvent);
  socketIO.begin(host, port, path);
}


void loop() {
  buttons.listen();
  socketIO.loop();
  Expander1.write16(0xFF00);
  delay(500);
  Expander1.write16(0x00);
  Serial.println(Expander1.lastError());
  delay(500);
}


void onSwitchButtonsDown(uint8_t controlPanel, uint8_t switches[], uint8_t lengthOfSwitches) {
  //Serial.println("Helloooooo");
  if (controlPanel == 1) {
    for (int i=0; i<lengthOfSwitches; i++) {
      if (switches[i] == 0) {
        uint8_t data[1] = {switches[i]+6};
        createSocketIOEvent("change_switches", data, lengthOfSwitches);
        return;
      }
    }
  } else {
    createSocketIOEvent("change_switches", switches, lengthOfSwitches);
  }
}


void createSocketIOEvent(String eventName, uint8_t data[], uint8_t length) {
  StaticJsonDocument<1024> doc;
  doc.add(eventName);
  
  JsonArray dataArray = doc[1].createNestedArray("data");
  for(int i=0; i<length; i++) {
    dataArray.add(data[i]);
  }
  
  String output;
  serializeJson(doc, output);
  socketIO.sendEVENT(output);
  Serial.print("[sendEvent] ");
  Serial.println(output);   
}


void onSocketIOEvent(socketIOmessageType_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case sIOtype_DISCONNECT:
      Serial.printf("[IOc] Disconnected!\n");
      break;
    case sIOtype_CONNECT:
      Serial.printf("[IOc] Connected to url: %s\n", payload);
      // join default namespace (no auto join in Socket.IO V3)
      socketIO.send(sIOtype_CONNECT, "/");
      break;
    case sIOtype_EVENT:
      Serial.printf("[IOc] Event: %s\n", payload);
      // Send event
      //socketIO.send(sIOtype_ACK, output);
      break;
    case sIOtype_ACK:
      Serial.printf("[IOc] get ack: %u\n", length);
      break;
    case sIOtype_ERROR:
      Serial.printf("[IOc] get error: %u\n", length);
      break;
    case sIOtype_BINARY_EVENT:
      Serial.printf("[IOc] get binary: %u\n", length);
      break;
    case sIOtype_BINARY_ACK:
      Serial.printf("[IOc] get binary ack: %u\n", length);
      break;    
  }
}