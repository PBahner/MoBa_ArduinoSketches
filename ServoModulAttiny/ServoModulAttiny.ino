// from: https://arduinodiy.wordpress.com/2015/05/20/servo-on-attiny13/
// from: https://circuitdigest.com/microcontroller-projects/programming-attiny13-with-arduino-uno-control-a-servo-motor
// from: https://www.makerguides.com/servo-arduino-tutorial/

#include <avr/io.h>
#define F_CPU 9600000

#define SERVO_SPEED 20
#define SERVO_PIN PB0
#define SET_LEFT_PIN PB1
#define SET_RIGHT_PIN PB2
#define POTI_PIN A2
#define FEEDBACK_PIN PB3
byte ANGLE_LEFT;
byte ANGLE_RIGHT;
byte ANGLE_MIDDLE = 65;
byte currentAngle;

void setup() {
  pinMode(SERVO_PIN, OUTPUT);  // set servo as output pin
  digitalWrite(SERVO_PIN, LOW);
  pinMode(SET_LEFT_PIN, INPUT);
  pinMode(SET_RIGHT_PIN, INPUT);
  pinMode(FEEDBACK_PIN, INPUT);
  // initialise center of Servo
  int compensation = analogRead(POTI_PIN);
  ANGLE_MIDDLE += map(compensation, 0, 511, -10, 10);
  ANGLE_LEFT = ANGLE_MIDDLE + 10;
  ANGLE_RIGHT = ANGLE_MIDDLE - 10;
  // get current Switch pos
  if (digitalRead(FEEDBACK_PIN)) {
    currentAngle = ANGLE_LEFT;
  } else {
    currentAngle = ANGLE_RIGHT;
  }
  
  delay(500);
  servoPulse(currentAngle);
}


void loop () {
  if(digitalRead(SET_RIGHT_PIN)) {
    moveServo(ANGLE_RIGHT);
    delay(200);
  }
  if (digitalRead(SET_LEFT_PIN)){
    moveServo(ANGLE_LEFT);
    delay(200);
  }
}


void moveServo (int desiredAngle) {
  if (currentAngle < desiredAngle) {
    for (byte angle=currentAngle; angle<=desiredAngle; angle++) {
      servoPulse(angle);
      delay(SERVO_SPEED);
    }
  } else if (currentAngle > desiredAngle) {
    for (byte angle=currentAngle; angle>=desiredAngle; angle--) {
      servoPulse(angle);
      delay(SERVO_SPEED);
    }
  }
  currentAngle = desiredAngle;
}


void servoPulse (int angle) {
  //pwm = (angle * 11) + 500;    // Convert angle to microseconds
  //pwm = 300+angle*(2500/180);
  int pwm = map(angle, 0, 120, 544, 2400);
  digitalWrite(SERVO_PIN, HIGH);
  delayMicroseconds(pwm);
  digitalWrite(SERVO_PIN, LOW);
  delay(20);                   // Refresh cycle of servo
}
