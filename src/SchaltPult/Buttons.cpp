#include <Arduino.h>
#include "Buttons.h"

Buttons::Buttons(uint8_t out, uint8_t in) {
  addrOut = out;
  addrIn = in;
}

void Buttons::begin() {
  pcfOut.begin(255);
  pcfIn.begin();

  pcfOut.setAddress(addrOut);
  pcfIn.setAddress(addrIn);
  
  previousButtonStates = getStates();
}

void Buttons::onButtonsDown(uint8_t controlPanel, void (&buttonDownFunction)(uint8_t, uint8_t[], uint8_t)) {
  eventFunctions.btnDownFunctions[controlPanel] = &buttonDownFunction;
}

void Buttons::listen() {
  const buttonArray buttonStates = getStates();
  checkForEvents(buttonStates);
  
  previousButtonStates = buttonStates;
}

void Buttons::checkForEvents(buttonArray buttonStates) {
  for(int r=0; r<ROWS; r++) {
    // abbrechen, wenn On-Event-Funktion nicht existiert
    if(!eventFunctions.btnDownFunctions[r]) {continue;}
    // Array, in dem alle gedrückten Taster gespeichert werden   
    uint8_t buttonsDown[ROWS] = {NULL};
    uint8_t arrayPos = 0;    
    for(int c=0; c<COLS; c++) {
      const bool raisingEdge = buttonStates.array[r][c] == true and previousButtonStates.array[r][c] == false;
      if(raisingEdge) {
        buttonsDown[arrayPos] = c;
        arrayPos++;
      }
    }
    if(arrayPos > 0) {  // existiert ein Wert in der Liste
      eventFunctions.btnDownFunctions[r](r, buttonsDown, arrayPos);
    }
  }
}

buttonArray Buttons::getStates() {
  buttonArray tempButtonStates;
  //Serial.println("\n\n\n===");
  // iterate all rows
  for(int r=0; r<ROWS; r++) {
    // power off row (row is used now, connected to GND)
    pcfOut.write(r, LOW);
    // iterate all columns and read state
    for(int c=0; c<COLS; c++) {
      tempButtonStates.array[r][c] = !pcfIn.read(c);
      //Serial.print(tempButtonStates.array[r][c]);
    }
    //Serial.println("");
    // power on row (row not used anymore)
    // connected to 5V but blocked by diode
    pcfOut.write(r, HIGH);
    delayMicroseconds(50);
  }
  return tempButtonStates;
}

