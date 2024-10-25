#ifndef SWITCHSERVO_H
#define SWITCHSERVO_H

#include <Servo.h>

#define SERVO_SPEED 1000

class SwitchServo: public Servo {
  private:
    bool plusInitialized = false;
    bool minusInitialized = false;
    bool currentStatePlus = false;
    bool currentStateMinus = false;
    bool servoDirection = false;
    uint8_t servoPin;
    bool withFeedback = true;
    int middle = 1500;
    uint8_t potiPin;
    int totalDeflection;
    int halfDeflection;

    void setDeflection(int d) {
      totalDeflection = d;
      halfDeflection = d/2;
    }

    void setPotiPin(uint8_t pin) {
      potiPin = pin;
      int diff = map(analogRead(potiPin), 0, 1023, -200, 200);
      middle = 1500 + diff;
      Serial.print(pin);
      Serial.print(": ");
      Serial.println(middle);
    }

    void attachAt() {
      if (attached()) {return;}
      if (!plusInitialized or !minusInitialized) {return;}
      if ((currentStateMinus and !currentStatePlus) ^ servoDirection) {
        writeMicroseconds(middle - halfDeflection);
      } else {
        writeMicroseconds(middle + halfDeflection);
      }
      attach(servoPin);
    }

    void moveSwitchServoMicrosecondsUp() {  // increase micros
      for(int micros=middle-halfDeflection; micros<middle+halfDeflection; micros++) {
        writeMicroseconds(micros);
        delayMicroseconds(SERVO_SPEED);
      }
    }

    void moveSwitchServoMicrosecondsDown() {  // decrease micros
      for(int micros=middle+halfDeflection; micros>middle-halfDeflection; micros--) {
        writeMicroseconds(micros);
        delayMicroseconds(SERVO_SPEED);
      }
    }
  

  public:
    void init(int servoPin, uint8_t potiPin, int totalDeflection, bool direction, bool withFeedback=false) {
      // attach(servoPin);
      this->servoPin = servoPin;
      setPotiPin(potiPin);
      setDeflection(totalDeflection);
      if(!withFeedback) {
        currentStateMinus = false;
        currentStatePlus = true;
      }
      servoDirection = direction;
    }

    void moveSwitchServo(bool target) {  // test erforderlich
      if (!attached()) {return;}
      target = target ^ servoDirection;  // apply servo direction with xor
      int8_t currentSwitchState = (2 - currentStateMinus - currentStatePlus * 2) ^ servoDirection;
      if((!(target == currentSwitchState)) and ((plusInitialized and minusInitialized) or !withFeedback)) {
        if(target) {
          Serial.println("move up");
          moveSwitchServoMicrosecondsDown();
        } else {
          Serial.println("move down");
          moveSwitchServoMicrosecondsUp();
        }
        if (!withFeedback) {
          currentStateMinus = !currentStateMinus;
          currentStatePlus = !currentStatePlus;
        }
      }
    }
    
    void updateCurrentStatePlus(bool currentPlus) {
      currentStatePlus = currentPlus;
      plusInitialized = true;
      if(minusInitialized) {
        attachAt();
      }
    }

    void updateCurrentStateMinus(bool currentMinus) {
      currentStateMinus = currentMinus;
      minusInitialized = true;
      if(plusInitialized) {
        attachAt();
      }
    }
};
#endif
