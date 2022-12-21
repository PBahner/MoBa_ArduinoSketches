#ifndef SWITCHSERVO_H
#define SWITCHSERVO_H

#include <Servo.h>

#define SERVO_SPEED 1000

class SwitchServo: public Servo {
  private:
    bool targetSwitchState;
    bool currentSwitchState = false;
    int middle = 1500;
    uint8_t potiPin;
    int totalDeflection;
    int halfDeflection;
    void setDeflection(int d) {
      totalDeflection = d;
      halfDeflection = d/2;
  ***REMOVED***
    void setPotiPin(uint8_t pin) {
      potiPin = pin;
      int diff = map(analogRead(potiPin), 0, 1023, -200, 200);
      middle = 1500 + diff;
      Serial.print(pin);
      Serial.print(": ");
      Serial.println(middle);
  ***REMOVED***
    void moveSwitchServoMicrosecondsUp() {  // increase micros
      for(int micros=middle-halfDeflection; micros<middle+halfDeflection; micros++) {
        writeMicroseconds(micros);
        delayMicroseconds(SERVO_SPEED);
    ***REMOVED***
  ***REMOVED***

    void moveSwitchServoMicrosecondsDown() {  // decrease micros
      for(int micros=middle+halfDeflection; micros>middle-halfDeflection; micros--) {
        writeMicroseconds(micros);
        delayMicroseconds(SERVO_SPEED);
    ***REMOVED***
  ***REMOVED***
  

  public:
    void init(int servoPin, uint8_t potiPin, int totalDeflection=600) {
      attach(servoPin);
      setPotiPin(potiPin);
      setDeflection(totalDeflection);
  ***REMOVED***

    void moveSwitchServo(bool target) {
      if(target != targetSwitchState) {
        if(target) {
          moveSwitchServoMicrosecondsDown();
      ***REMOVED*** else {
          moveSwitchServoMicrosecondsUp();
      ***REMOVED***
        targetSwitchState = target;
    ***REMOVED***
  ***REMOVED***
    
    void updateCurrentSwitchState(bool current) {
      currentSwitchState = current;
  ***REMOVED***
***REMOVED***;
#endif
