int directionPin = 12;
int pwmPin = 3;
int brakePin = 9;

//uncomment if using channel B, and remove above definitions
//int directionPin = 13;
//int pwmPin = 11;
//int brakePin = 8;

//boolean to switch direction
bool directionState;

***REMOVED***
  
//define pins
pinMode(directionPin, OUTPUT);
pinMode(pwmPin, OUTPUT);
pinMode(brakePin, OUTPUT);

***REMOVED***

***REMOVED***

  //change direction every loop()
  directionState = !directionState;

  //write a low state to the direction pin (13)
  if(directionState == false){
    digitalWrite(directionPin, LOW);
***REMOVED***
  //write a high state to the direction pin (13)
  else{
    digitalWrite(directionPin, HIGH);
***REMOVED***

  //release breaks
  digitalWrite(brakePin, LOW);
  //set work duty for the motor
  analogWrite(pwmPin, 255);

  delay(3000);

  for (byte speed=255; speed>=0; speed--) {
    analogWrite(pwmPin, speed);
    delay(20);
***REMOVED***

  //activate breaks
  digitalWrite(brakePin, HIGH);
  //set work duty for the motor to 0 (off)
  analogWrite(pwmPin, 0);

  delay(2000);
***REMOVED***
