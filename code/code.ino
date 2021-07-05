// PRECOMPILER DIRECTIVES
//#define ENABLE_SERVO
//#define ENABLE_RFID
#define ENABLE_OLED
//#define ENABLE_JOYSTICK

// IMPORTED LIBRARIES
#include <Servo.h>
#ifdef ENABLE_OLED
  #include <Wire.h>
  #include <Adafruit_GFX.h>
  #include <Adafruit_SSD1306.h>
#endif

// CONFIGURABLE CONSTANTS
// PIN ASSIGNMENTS
#ifdef ENABLE_SERVO
  const int SERVO_X_PIN = 9;
  const int SERVO_Y_PIN = 10;
#endif
const int LED_STATE_LOCKED_PIN = 5;
const int LED_STATE_UNLOCKED_PIN = 4;
#ifndef ENABLE_RFID
  const int UNLOCK_BUTTON_PIN = 2;
#endif

const int EMERGENCY_BUTTON_PIN = 3;

// MACHINE PARAMETERS
#ifdef ENABLE_SERVO
  const int SERVO_X_MIN_ANGLE = 0;
  const int SERVO_X_MAX_ANGLE = 140;
  const int SERVO_Y_MIN_ANGLE = 10;
  const int SERVO_Y_MAX_ANGLE = 140;
  const int SERVO_ANGLE_SPEED = 1;
#endif
#ifdef ENABLE_OLED
  const int OLED_SCREEN_WIDTH = 128;
  const int OLED_SCREEN_HEIGHT = 32;
#endif

// ENUMS
enum MachineState {
  MACHINE_LOCKED,
  MACHINE_UNLOCKED,
  MACHINE_EMERGENCY,
  MACHINE_FAILURE
};

// GLOBAL STATE
int xPos = 0;
int yPos = 0;
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
#ifdef ENABLE_OLED
  Adafruit_SSD1306 oled(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, -1);
#endif

// Arduino-mandated setup function. Runs once.
void setup() {
  // Initialize pin modes.
  pinMode(LED_STATE_LOCKED_PIN, OUTPUT);
  pinMode(LED_STATE_UNLOCKED_PIN, OUTPUT);
  #ifndef ENABLE_RFID
    pinMode(UNLOCK_BUTTON_PIN, INPUT_PULLUP);
  #endif
  pinMode(EMERGENCY_BUTTON_PIN, INPUT_PULLUP);
  #ifdef ENABLE_OLED
    // TODO: Should probably catch this error.
    oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(WHITE);
    oled.setCursor(0, 10);
    oled.println("starting up...");
    oled.display();
  #endif
  // Machine should start in a locked state.
  LockMachine();
}

// Arduino-mandated loop function. Repeats forever.
void loop() {
  // Putting this here because we should be able to enter emergency state at any time.
  if (digitalRead(EMERGENCY_BUTTON_PIN) == LOW) {
      LockMachine();
      machineState = MACHINE_EMERGENCY;
    }
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

void ReadJoystickInputs() {
  #ifndef ENABLE_JOYSTICK
  xPos++;
  if (xPos > 180) {
    xPos = 0;
  }
  yPos++;
  if (yPos > 180) {
    yPos = 0;
  }
  #endif
  #ifdef ENABLE_JOYSTICK
    // TODO: Read joystick inputs.
  #endif
}

void DrawUnlockedScreen() {
  #ifdef ENABLE_OLED
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(WHITE);
    oled.setCursor(0,10);
    oled.println("UNLOCKED");
    // Display current voltage.
    oled.print(analogRead(A0) * 5 / 1023);
    oled.println("V");
    // Draw current position component.
    oled.drawRect(OLED_SCREEN_WIDTH - OLED_SCREEN_HEIGHT, 0, OLED_SCREEN_HEIGHT, OLED_SCREEN_HEIGHT, WHITE);
    int drawCursorX = NormalizeValue(0, 180, xPos, OLED_SCREEN_WIDTH - OLED_SCREEN_HEIGHT, OLED_SCREEN_WIDTH - 1);
    int drawCursorY = NormalizeValue(0, 180, yPos, 1, OLED_SCREEN_HEIGHT - 1);
    oled.fillRect(drawCursorX, drawCursorY, 1, 1, WHITE);
    oled.display();
  #endif
}

// Returns a safe version of a value (e.g. if the value is 110, but the maximum safe value is 100, 100 is returned).
int SafeValue(int input, int minimum, int maximum) {
  return min(maximum, max(minimum, input));
}

void DoStateUnlockedFunctions() {
  DrawUnlockedScreen();
  ReadJoystickInputs();
  // This is a temporary test of the servo code. Ignore this.
  #ifdef ENABLE_SERVO
  xServo.write(SafeValue(xPos, SERVO_X_MIN_ANGLE, SERVO_X_MAX_ANGLE);
  yServo.write(SafeValue(yPos, SERVO_Y_MIN_ANGLE, SERVO_Y_MAX_ANGLE);
  #endif
  #ifndef ENABLE_RFID
    // Check if the lock/unlock button has been pressed.
    if (digitalRead(UNLOCK_BUTTON_PIN) == LOW) {
      LockMachine();
      delay(1000);
      return;
    }
  #endif
}

void DoStateLockedFunctions() {
  #ifdef ENABLE_OLED
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(WHITE);
    oled.setCursor(0,10);
    oled.println("MACHINE LOCKED");
    #ifdef ENABLE_RFID
      oled.println("use card to unlock");
    #endif
    #ifndef ENABLE_RFID
      oled.println("press unlock button");
    #endif
    oled.display();
  #endif
  #ifndef ENABLE_RFID
    // Check if the lock/unlock button has been pressed.
    if (digitalRead(UNLOCK_BUTTON_PIN) == LOW) {
      UnlockMachine();
      delay(1000);
      return;
    }
  #endif
}

void DoStateEmergencyFunctions() {
  digitalWrite(LED_STATE_UNLOCKED_PIN, LOW);
  digitalWrite(LED_STATE_LOCKED_PIN, HIGH);
  #ifdef ENABLE_OLED
    oled.clearDisplay();
    oled.setTextSize(2);
    oled.setTextColor(WHITE);
    oled.setCursor(0,0);
    oled.println("EMERGENCY");
    oled.display();
  #endif
  delay(250);
  digitalWrite(LED_STATE_LOCKED_PIN, LOW);
  #ifdef ENABLE_OLED
    oled.clearDisplay();
    oled.display();
  #endif
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

// Normalizes a value from one range to another. Does not perform boundary validation.
int NormalizeValue(float inBottom, float inTop, float value, int outBottom, int outTop) {
  inTop -= inBottom;
  value -= inBottom;
  int tempZeroedMaxValue = outTop - outBottom;
  int result = int((value * tempZeroedMaxValue) / inTop);
  return result + outBottom;
}
