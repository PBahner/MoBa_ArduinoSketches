#include <Arduino.h>
#include <SocketIOclient.h>  // library from https://github.com/Links2004/arduinoWebSockets
#include <ArduinoJson.h>
#include <WiFi.h>
#include <MCP23017.h>  // library from RobTillaart https://github.com/RobTillaart/MCP23017_RT
#include "CredentialManager.h"
#include "Credentials.h"
#include "Buttons.h"
#include "turnoutLeds.h"

void onSwitchButtonsDown(uint8_t, uint8_t[], uint8_t);
void onTrackButtonsUp(uint8_t, uint8_t[], uint8_t);
void onTrackButtonsDown(uint8_t, uint8_t[], uint8_t);
void createSocketIOEvent(String, uint8_t[], uint8_t);
void onSocketIOEvent(socketIOmessageType_t, uint8_t *, size_t);

bool TEST_MODE;

Buttons buttons(0x38, 0x39); // (0x20, 0x21)
MCP23017 Expander1(0x20);

uint8_t const tLEDs_length = 7;
TurnoutLeds tLEDs[] = {TurnoutLeds(0, &Expander1, 10, 11),
                       TurnoutLeds(1, &Expander1, 12, 13),
                       TurnoutLeds(2, &Expander1, 9, 8),
                       TurnoutLeds(3, &Expander1, 0, 1),
                       TurnoutLeds(4, &Expander1, 3, 2),
                       TurnoutLeds(5, &Expander1, 4, 5),
                       TurnoutLeds(6, &Expander1, 6, 7)};

SocketIOclient socketIO;
WiFiClient client;

const char *host;
const int port = 5000; // Socket.IO Port Address
const char path[] = "/socket.io/?EIO=4"; // Socket.IO Base Path
    
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

  for (int i = 0; i<tLEDs_length; i++) {
    tLEDs[i].begin();
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

  // initialize socketio events
  socketIO.onEvent(onSocketIOEvent);
  socketIO.begin(targetCredential.host, port, path);
}


void loop() {
  buttons.listen();
  socketIO.loop();
}


void onSwitchButtonsDown(uint8_t controlPanel, uint8_t switches[], uint8_t lengthOfSwitches) {
  //Serial.println("Helloooooo");
  if (controlPanel == 5) {
    for (int i=0; i<lengthOfSwitches; i++) {
      if (switches[i] == 0) {
        uint8_t data[1] = {switches[i]+6};
        createSocketIOEvent("change_turnouts", data, lengthOfSwitches);
        return;
      }
    }
  } else {
    createSocketIOEvent("change_turnouts", switches, lengthOfSwitches);
  }
}


void onTrackButtonsDown(uint8_t controlPanel, uint8_t tracks[], uint8_t lengthOfTracks) {
  if (controlPanel == 4) {
    for (int i=0; i<lengthOfTracks; i++) {
      if (tracks[i] == 0) {
        uint8_t data[1] = {tracks[i]+6};
        createSocketIOEvent("track_interruptions_on", data, lengthOfTracks);
        return;
      }
    }
  } else {
    createSocketIOEvent("track_interruptions_on", tracks, lengthOfTracks);
  }
}

void onTrackButtonsUp(uint8_t controlPanel, uint8_t tracks[], uint8_t lengthOfTracks) {
  if (controlPanel == 4) {
    for (int i=0; i<lengthOfTracks; i++) {
      if (tracks[i] == 0) {
        uint8_t data[1] = {tracks[i]+6};
        createSocketIOEvent("track_interruptions_off", data, lengthOfTracks);
        return;
      }
    }
  } else {
    createSocketIOEvent("track_interruptions_off", tracks, lengthOfTracks);
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
    case sIOtype_EVENT: {
      Serial.printf("[IOc] Event: %s\n", payload);

      const int capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(2);
      StaticJsonDocument<500> doc;
      DeserializationError error = deserializeJson(doc, payload);
      if (error) {
        Serial.print("fehler: ");
        Serial.println(error.f_str());
      }

      const char* event = doc[0];
      String sEvent = String(event);
      Serial.print("event: "); Serial.println(sEvent);
      JsonObject dataObject = doc[1]["data"];
      if(sEvent == "init_switch_positions" or sEvent == "update_switch_positions") {
        for (int i = 0; i<tLEDs_length; i++) {
          tLEDs[i].update(dataObject);
        }
      }
      break;
    }
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
