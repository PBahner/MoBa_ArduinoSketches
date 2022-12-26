#ifndef BUTTONS_H
#define BUTTONS_H

#include <PCF8574.h>

#define ROWS 8
#define COLS 8

struct buttonArray {
  bool array[ROWS][COLS];
};
struct eventFunctionsArray {
  void (*btnDownFunctions[ROWS])(uint8_t, uint8_t[], uint8_t);
  eventFunctionsArray() {
    for(int r=0; r<ROWS; r++) {
      btnDownFunctions[r] = NULL;
    }
  }
};

class Buttons {
  public:

    Buttons(uint8_t, uint8_t);
    void listen();
    void onButtonsDown(uint8_t controlPanel, void (&buttonDownFunction)(uint8_t, uint8_t[], uint8_t));

  private:

    PCF8574 pcfOut;
    PCF8574 pcfIn;
    buttonArray previousButtonStates;
    eventFunctionsArray eventFunctions;
    
    buttonArray getStates();
    void checkForEvents(buttonArray);
    
};
#endif
