// PRECOMPILER DIRECTIVES
#define ENABLE_SERVO
#define ENABLE_RFID
#define ENABLE_OLED
#define ENABLE_JOYSTICK

// IMPORTED LIBRARIES
#include <Servo.h>
#ifdef ENABLE_OLED
  #include <Wire.h>
  #include <Adafruit_GFX.h>
  #include <Adafruit_SSD1306.h>
#endif
#ifdef ENABLE_RFID
  #include <SPI.h>
  #include <MFRC522.h>
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
#ifdef ENABLE_JOYSTICK
  const int JOYSTICK_X_PIN = A1;
  const int JOYSTICK_Y_PIN = A2;
  const int JOYSTICK_Z_PIN = 6;
#endif
#ifdef ENABLE_RFID
  const int RFID_SS_PIN = 8;
  const int RFID_RST_PIN = 7;
#endif
const int SOLAR_PANEL_READ_PIN = A0;
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
#ifdef ENABLE_RFID
  const String RFID_VALID_CARD = "C3 A5 2C 40";
#endif
#ifdef ENABLE_JOYSTICK
  // The maximum delta of xPos or yPos based on input. Joystick input will be scaled to this value.
  const int MAX_INPUT_SPEED = 5;
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
#ifdef ENABLE_RFID
  MFRC522 mfrc522(RFID_SS_PIN, RFID_RST_PIN);
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
  #ifdef ENABLE_JOYSTICK
    pinMode(JOYSTICK_Z_PIN, INPUT);
  #endif
  #ifdef ENABLE_RFID
    SPI.begin();
    mfrc522.PCD_Init();
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
  // NormalizeValue(float inBottom, float inTop, float value, int outBottom, int outTop)
  #ifdef ENABLE_JOYSTICK
    xPos += NormalizeValue(0, 1023, analogRead(JOYSTICK_X_PIN), 0, MAX_INPUT_SPEED);
    yPos += NormalizeValue(0, 1023, analogRead(JOYSTICK_Y_PIN), 0, MAX_INPUT_SPEED);
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

void SetServosToCurrentPosition() {
  #ifdef ENABLE_SERVO
    xServo.write(SafeValue(xPos, SERVO_X_MIN_ANGLE, SERVO_X_MAX_ANGLE));
    yServo.write(SafeValue(yPos, SERVO_Y_MIN_ANGLE, SERVO_Y_MAX_ANGLE));
  #endif
}

void DoStateUnlockedFunctions() {
  DrawUnlockedScreen();
  ReadJoystickInputs();
  SetServosToCurrentPosition();
  if (ValidChangeLockAction()) {
    UnlockMachine();
    delay(1000);
    return;
  }
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
  if (ValidChangeLockAction()) {
    UnlockMachine();
    delay(1000);
    return;
  }
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

bool ValidChangeLockAction() {
  #ifndef ENABLE_RFID
    // Check if the lock/unlock button has been pressed.
    if (digitalRead(UNLOCK_BUTTON_PIN) == LOW) {
      return true;
    } else {
      return false;
    }
  #endif
  #ifdef ENABLE_RFID
    if ( ! mfrc522.PICC_IsNewCardPresent())
    {
      // TODO: Possibly show an error here?
      return false;
    }
    if ( ! mfrc522.PICC_ReadCardSerial())
    {
      // TODO: Possibly show an error here?
      return false;
    }
    String content= "";
    byte letter;
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    content.toUpperCase();
    if (content.substring(1) == RFID_VALID_CARD)
    {
      // TODO: Show access granted message?
      return true;
    }
    else   {
      // TODO: Show access denied message?
      return false;
    }
  #endif
}
