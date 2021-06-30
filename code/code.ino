// IMPORTED LIBRARIES
#include <Servo.h>

// PRECOMPILER DIRECTIVES
//#define ENABLE_SERVO
//#define ENABLE_RFID

// CONFIGURABLE CONSTANTS
// PIN ASSIGNMENTS
#ifdef ENABLE_SERVO
  const int SERVO_X_PIN = 9;
  const int SERVO_Y_PIN = 10;
#endif
const int LED_STATE_LOCKED_PIN = 3;
const int LED_STATE_UNLOCKED_PIN = 2;
#ifndef ENABLE_RFID
  const int UNLOCK_BUTTON_PIN = 4;
#endif

// MACHINE PARAMETERS
#ifdef ENABLE_SERVO
  const int SERVO_X_MIN_ANGLE = 0;
  const int SERVO_X_MAX_ANGLE = 140;
  const int SERVO_Y_MIN_ANGLE = 10;
  const int SERVO_Y_MAX_ANGLE = 140;
  const int SERVO_ANGLE_SPEED = 1;
#endif

// ENUMS
enum MachineState {
  MACHINE_LOCKED,
  MACHINE_UNLOCKED,
  MACHINE_EMERGENCY,
  MACHINE_FAILURE
};

// GLOBAL STATE
int pos = 0;
#ifdef ENABLE_SERVO
  int servoXAngle;
  int servoYAngle;
#endif
MachineState machineState;

// GLOBAL OBJECTS
#ifdef ENABLE_SERVO
  Servo xServo;
  Servo yServo;
#endif

// Arduino-mandated setup function. Runs once.
void setup() {
  // Initialize pin modes.
  pinMode(LED_STATE_LOCKED_PIN, OUTPUT);
  pinMode(LED_STATE_UNLOCKED_PIN, OUTPUT);
  #ifndef ENABLE_RFID
    pinMode(UNLOCK_BUTTON_PIN, INPUT);
  #endif
  // Machine should start in a locked state.
  LockMachine();
}

// Arduino-mandated loop function. Repeats forever.
void loop() {
  switch (machineState) {
    case MACHINE_LOCKED:
      DoStateLockedFunctions();
      break;
    case MACHINE_UNLOCKED:
      DoStateUnlockedFunctions();
      break;
    case MACHINE_FAILURE:
      DoStateEmergencyFunctions();
      break;
    case MACHINE_EMERGENCY:
      DoStateEmergencyFunctions();
      break;
    default:
      DoStateEmergencyFunctions();
  }
}

void DoStateUnlockedFunctions() {
  // This is a temporary test of the servo code. Ignore this.
  #ifdef ENABLE_SERVO
  for (pos = SERVO_X_MIN_ANGLE; pos <= SERVO_X_MAX_ANGLE; pos += SERVO_ANGLE_SPEED) {
    xServo.write(pos);
    yServo.write(pos);
    delay(15);
  }
  for (pos = SERVO_X_MAX_ANGLE; pos >= SERVO_X_MIN_ANGLE; pos -= SERVO_ANGLE_SPEED) {
    xServo.write(pos);
    yServo.write(pos);
    delay(15);
  }
  #endif
  #ifndef ENABLE_RFID
    // Check if the lock/unlock button has been pressed.
    if (digitalRead(UNLOCK_BUTTON_PIN) == HIGH) {
      LockMachine();
      delay(1000);
      return;
    }
  #endif
}

void DoStateLockedFunctions() {
  #ifndef ENABLE_RFID
    // Check if the lock/unlock button has been pressed.
    if (digitalRead(UNLOCK_BUTTON_PIN) == HIGH) {
      UnlockMachine();
      delay(1000);
      return;
    }
  #endif
}

void DoStateEmergencyFunctions() {
  digitalWrite(LED_STATE_UNLOCKED_PIN, LOW);
  digitalWrite(LED_STATE_LOCKED_PIN, HIGH);
  delay(250);
  digitalWrite(LED_STATE_LOCKED_PIN, LOW);
  delay(250);
}

// Sets the machine in an unlocked state. Unlocks the servos and sets the status lights.
void UnlockMachine() {
  // Set status lights.
  digitalWrite(LED_STATE_UNLOCKED_PIN, HIGH);
  digitalWrite(LED_STATE_LOCKED_PIN, LOW);
  
  #ifdef ENABLE_SERVO
    xServo.attach(SERVO_X_PIN);
    yServo.attach(SERVO_Y_PIN);
  #endif
  
  machineState = MACHINE_UNLOCKED;
}

// Sets the machine in a locked state. Disconnects the servos and sets the status lights.
void LockMachine() {
  // Set status lights.
  digitalWrite(LED_STATE_UNLOCKED_PIN, LOW);
  digitalWrite(LED_STATE_LOCKED_PIN, HIGH);
  
  #ifdef ENABLE_SERVO
    xServo.detach();
    yServo.detach();
  #endif

  machineState = MACHINE_LOCKED;
}
