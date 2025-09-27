#include <Arduino.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <MCP23017.h>  // library from RobTillaart https://github.com/RobTillaart/MCP23017_RT
#include "CredentialManager.h"
#include "Credentials.h"
#include "Buttons.h"
#include "turnoutLeds.h"
#include "SignalLed.h"

void onSwitchButtonsDown(uint8_t, uint8_t[], uint8_t);
void onTrackButtonsUp(uint8_t, uint8_t[], uint8_t);
void onTrackButtonsDown(uint8_t, uint8_t[], uint8_t);
void createWebsocketEvent(String, uint8_t[], uint8_t);
void onWebSocketEvent(WStype_t type, uint8_t * payload, size_t length);

bool TEST_MODE;

Buttons buttons(0x38, 0x39); // (0x20, 0x21)
MCP23017 Expander1(0x20);
MCP23017 Expander2(0x21);
MCP23017 Expander3(0x22);

uint8_t const tLEDs_length = 7;
TurnoutLeds tLEDs[] = {TurnoutLeds(0, &Expander1, 10, 11),
                       TurnoutLeds(1, &Expander1, 12, 13),
                       TurnoutLeds(2, &Expander1, 9, 8),
                       TurnoutLeds(3, &Expander1, 0, 1),
                       TurnoutLeds(4, &Expander1, 3, 2),
                       TurnoutLeds(5, &Expander1, 4, 5),
                       TurnoutLeds(6, &Expander1, 6, 7)};

uint8_t const sLEDs_length = 9;
SignalLED sLEDs[] = {SignalLED(3, 2, &Expander2, 9),
                     SignalLED(4, 2, &Expander2, 8),
                     SignalLED(5, 2, &Expander2, 7),
                     SignalLED(6, 1, &Expander3, 0),
                     SignalLED(10, 1, &Expander3, 1),
                     SignalLED(7, 1, &Expander3, 2),
                     SignalLED(8, 1, &Expander3, 4),
                     SignalLED(2, 1, &Expander3, 5),
                     SignalLED(1, 1, &Expander3, 6)};

WebSocketsClient webSocket;
WiFiClient client;

const char *host;
const int port = 5000; // Socket.IO Port Address
const char path[] = "/"; // Socket.IO Base Path
    
unsigned long lastBtnEvent;

const Credential findAvailableSSID(Credential credentials[], int numberOfSSIDs) {
  while (true) {
    // Start Wi-Fi scanning
    int numNetworks = WiFi.scanNetworks(false, true); // Scan for networks

    // Check for each SSID in the provided list
    for (int i = 0; i < numNetworks; i++) {
        String foundSSID = WiFi.SSID(i); // Get the SSID of the found network
        // Check against the given SSIDs
        for (int j = 0; j < numberOfSSIDs; j++) {
            if (foundSSID.equals(credentials[j].ssid)) {
                Serial.print("Found SSID: ");
                Serial.println(foundSSID);
                return credentials[j]; // Return the first found SSID
            }
        }
    }
  }
}

void tryConnectWiFi(const char* ssid, const char* password) {
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}

void setup() {
  Serial.begin(115200);
  buttons.begin();

  Expander1.begin();
  Expander1.pinMode8(0, 0x00);
  Expander1.pinMode8(1, 0x00);
  Expander1.write16(0x00);

  Expander2.begin();
  Expander2.pinMode8(0, 0x00);
  Expander2.pinMode8(1, 0x00);
  Expander2.write16(0x00);

  Expander3.begin();
  Expander3.pinMode8(0, 0x00);
  Expander3.pinMode8(1, 0x00);
  Expander3.write16(0x00);

  for (int i = 0; i<tLEDs_length; i++) {
    tLEDs[i].begin();
  }
  for (int i = 0; i<sLEDs_length; i++) {
    sLEDs[i].begin();
  }

  // initialize button listeners
  buttons.onButtonsDown(5, onSwitchButtonsDown);
  buttons.onButtonsDown(2, onSwitchButtonsDown);
  buttons.onButtonsDown(1, onTrackButtonsDown);
  buttons.onButtonsUp(1, onTrackButtonsUp);
  buttons.onButtonsDown(4, onTrackButtonsDown);
  buttons.onButtonsUp(4, onTrackButtonsUp);

  Credential credentials[] = {
    CredentialManager::productionCredential,
    CredentialManager::testCredential
  };

  const Credential targetCredential = findAvailableSSID(credentials, sizeof(credentials) / sizeof(credentials[0]));

  WiFi.begin(targetCredential.ssid, targetCredential.pass);
  tryConnectWiFi(targetCredential.ssid, targetCredential.pass);

  if (targetCredential.ssid == CredentialManager::productionCredential.ssid) {
    TEST_MODE = false;
  } else {
    TEST_MODE = true;
  }

  Serial.println("IP adress: ");
  Serial.println(WiFi.localIP());

  // Initialize WebSocket
  webSocket.begin(targetCredential.host, port, path);
  webSocket.onEvent(onWebSocketEvent);
  webSocket.setReconnectInterval(5000); // auto-reconnect after 5s
}


void loop() {
  buttons.listen();
  socketIO.loop();
}


void onSwitchButtonsDown(uint8_t controlPanel, uint8_t switches[], uint8_t lengthOfSwitches) {
  if (controlPanel == 5) {
    for (int i=0; i<lengthOfSwitches; i++) {
      if (switches[i] == 0) {
        uint8_t data[1] = {switches[i]+6};
        createWebsocketEvent("change_turnouts", data, lengthOfSwitches);
        return;
      }
    }
  } else {
    createWebsocketEvent("change_turnouts", switches, lengthOfSwitches);
  }
}


void onTrackButtonsDown(uint8_t controlPanel, uint8_t tracks[], uint8_t lengthOfTracks) {
  if (controlPanel == 4) {
    for (int i=0; i<lengthOfTracks; i++) {
      if (tracks[i] == 0) {
        uint8_t data[1] = {tracks[i]+6};
        createWebsocketEvent("track_interruptions_on", data, lengthOfTracks);
        return;
      }
    }
  } else {
    createWebsocketEvent("track_interruptions_on", tracks, lengthOfTracks);
  }
}

void onTrackButtonsUp(uint8_t controlPanel, uint8_t tracks[], uint8_t lengthOfTracks) {
  if (controlPanel == 4) {
    for (int i=0; i<lengthOfTracks; i++) {
      if (tracks[i] == 0) {
        uint8_t data[1] = {tracks[i]+6};
        createWebsocketEvent("track_interruptions_off", data, lengthOfTracks);
        return;
      }
    }
  } else {
    createWebsocketEvent("track_interruptions_off", tracks, lengthOfTracks);
  }
}


void createWebsocketEvent(String eventName, uint8_t data[], uint8_t length) {
  StaticJsonDocument<1024> doc;
  doc["msg_id"] = eventName;
  
  JsonArray dataArray = doc.createNestedArray("data");
  for(int i=0; i<length; i++) {
    dataArray.add(data[i]);
  }
  
  String output;
  serializeJson(doc, output);
  webSocket.sendTXT(output);
  // Serial.print("[sendEvent] ");
  // Serial.println(output);   
}


void onWebSocketEvent(WStype_t  type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[WS] Disconnected!\n");
      break;
    
    case WStype_CONNECTED:
      Serial.printf("[WS] Connected to url: %s\n", payload);
      break;
    
    case WStype_TEXT: {
      // Serial.printf("[WS] Event: %s\n", payload);

      StaticJsonDocument<1024> doc;
      DeserializationError error = deserializeJson(doc, payload);
      if (error) {
        Serial.print("[WS] JSON parse error: ");
        Serial.println(error.f_str());
        return;
      }

      const char* msg_id = doc["msg_id"];
      JsonObject data = doc["data"].as<JsonObject>();

      if (strcmp(msg_id, "init_switch_positions") == 0 ||
          strcmp(msg_id, "update_switch_positions") == 0) {
        for (int i = 0; i < tLEDs_length; i++) {
          tLEDs[i].update(data);
        }
      } else if (strcmp(msg_id, "distribute_track_interruptions") == 0) {
        for (int i = 0; i < sLEDs_length; i++) {
          sLEDs[i].update(data);
        }
      }
      break;
    }
    
    case WStype_ERROR:
      Serial.printf("[WS] get error: %u\n", length);
      break;  
  }
}
