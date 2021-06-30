// IMPORTED LIBRARIES
#include <Servo.h>

// PRECOMPILER DIRECTIVES
#define ENABLE_SERVO

// CONFIGURABLE CONSTANTS
// PIN ASSIGNMENTS
const int SERVO_X_PIN = 9;
const int SERVO_Y_PIN = 10;

// MACHINE PARAMETERS
const int SERVO_X_MIN_ANGLE = 0;
const int SERVO_X_MAX_ANGLE = 140;
const int SERVO_Y_MIN_ANGLE = 10;
const int SERVO_Y_MAX_ANGLE = 140;
const int SERVO_ANGLE_SPEED = 1;

// ENUMS
enum machine_state {
  MACHINE_LOCKED,
  MACHINE_UNLOCKED,
  MACHINE_EMERGENCY,
  MACHINE_FAILURE
};

// GLOBAL STATE
int pos = 0;
int servoXAngle;
int servoYAngle;
machine_state machineState;

// GLOBAL OBJECTS
Servo xServo;
Servo yServo;

// Arduino-mandated setup function. Runs once.
void setup() {
  machineState = MACHINE_LOCKED;
  
}

// Arduino-mandated loop function. Repeats forever.
void loop() {
  if (machineState == MACHINE_UNLOCKED) {
    for (pos = SERVO_X_MIN_ANGLE; pos <= SERVO_X_MAX_ANGLE; pos += SERVO_ANGLE_SPEED) {
      #ifdef ENABLE_SERVO
        xServo.write(pos);
        yServo.write(pos);
      #endif
      delay(15);
    }
    for (pos = SERVO_X_MAX_ANGLE; pos >= SERVO_X_MIN_ANGLE; pos -= SERVO_ANGLE_SPEED) {
      #ifdef ENABLE_SERVO
        xServo.write(pos);
        yServo.write(pos);
      #endif
      delay(15);
    }
  }
}

void UnlockMachine() {
  #ifdef ENABLE_SERVO
    xServo.attach(SERVO_X_PIN);
    yServo.attach(SERVO_Y_PIN);
  #endif
  
  machineState = MACHINE_UNLOCKED;
}

void LockMachine() {
  #ifdef ENABLE_SERVO
    xServo.detach(SERVO_X_PIN);
    yServo.detach(SERVO_Y_PIN);
  #endif

  machineState = MACHINE_LOCKED;
}
