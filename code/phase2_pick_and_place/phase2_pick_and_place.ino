/*
 * Phase 2 - Line follower + servo gripper (pick & place)
 * ElectroNova 2026, Team Error 404  |  Track: Robotics & Automation
 *
 * NOTE: The original hackathon servo code was lost. This sketch was
 * re-written afterwards for the same hardware. Tune the angles/timings
 * below on the real robot before relying on it.
 *
 * Behaviour
 *  1. Follow the black line (same 8-sensor logic as Phase 1).
 *  2. When ALL 8 sensors see black (a marker square / stop box) the robot stops.
 *  3. 1st marker -> PICK  the object (open gripper, lower arm, close, raise).
 *     2nd marker -> PLACE the object (lower arm, open gripper, raise, close).
 *  4. After the place, the robot stops for good (set STOP_AFTER_PLACE = false to keep going).
 *
 * Hardware
 *  Arduino Uno, L298N, 8-ch IR array, 2 servos (gripper + arm lift/tilt)
 *  Gripper servo -> D13 | Arm servo -> A3
 *  Servo power: use a separate 5-6 V supply (or the battery via a regulator),
 *  NOT the Arduino 5 V pin, and connect all grounds together.
 *  The Servo library uses Timer1, which disables PWM on D9/D10 - the motor
 *  PWM pins (D3, D11) are on Timer2 so they keep working.
 */
#include <Servo.h>

// ---------------- Pins ----------------
const int SENSOR_PINS[8] = {A0, A1, A2, 5, 6, 7, 8, 9};
const int ENA = 3, IN1 = 2, IN2 = 4;      // left motor
const int ENB = 11, IN3 = 10, IN4 = 12;   // right motor
const int GRIPPER_PIN = 13;
const int ARM_PIN     = A3;

// ---------------- Tunables ----------------
const int SPEED_FORWARD = 80;
const int SPEED_TURN    = 60;

// Servo angles (degrees) - CALIBRATE with servo_calibration.ino
const int GRIP_OPEN   = 90;
const int GRIP_CLOSED = 30;
const int ARM_UP      = 120;
const int ARM_DOWN    = 50;
const int SERVO_STEP_DELAY = 15;   // ms per degree (smaller = faster)

const unsigned long MARKER_HOLD_MS = 150;  // all-black must last this long
const unsigned long CLEAR_MARKER_MS = 450; // drive forward to leave the marker
const bool STOP_AFTER_PLACE = true;

// ---------------- State ----------------
Servo gripper, arm;
int gripAngle = GRIP_CLOSED;
int armAngle  = ARM_UP;
int s[8];
unsigned long markerSeenSince = 0;
enum Stage { GO_TO_PICK, GO_TO_PLACE, DONE };
Stage stage = GO_TO_PICK;

// ---------------- Setup / loop ----------------
// Function prototypes (so the sketch also builds in PlatformIO / VS Code)
void handleMarker();
void pickObject();
void placeObject();
void moveServo(Servo &sv, int &current, int target);
void readSensors();
bool match(int a, int b, int c, int d, int e, int f, int g, int h);
bool allBlack();
void followLine();
void forward();
void turnRight();
void turnLeft();
void motorStop();
void driveForward(unsigned long ms);

void setup() {
  for (int i = 0; i < 8; i++) pinMode(SENSOR_PINS[i], INPUT);
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  Serial.begin(9600);

  gripper.attach(GRIPPER_PIN);
  arm.attach(ARM_PIN);
  gripper.write(gripAngle);
  arm.write(armAngle);
  motorStop();
  delay(2000);  // time to place the robot on the track
}

void loop() {
  if (stage == DONE) { motorStop(); return; }

  readSensors();

  if (allBlack()) {
    if (markerSeenSince == 0) markerSeenSince = millis();
    if (millis() - markerSeenSince >= MARKER_HOLD_MS) {
      motorStop();
      handleMarker();
      markerSeenSince = 0;
    }
    return;           // keep current motor state until marker is confirmed
  }
  markerSeenSince = 0;
  followLine();
}

// ---------------- Pick & place ----------------
void handleMarker() {
  if (stage == GO_TO_PICK) {
    pickObject();
    stage = GO_TO_PLACE;
    driveForward(CLEAR_MARKER_MS);   // move off the marker square
  } else if (stage == GO_TO_PLACE) {
    placeObject();
    if (STOP_AFTER_PLACE) stage = DONE;
    else { stage = GO_TO_PICK; driveForward(CLEAR_MARKER_MS); }
  }
}

void pickObject() {
  moveServo(gripper, gripAngle, GRIP_OPEN);    delay(300);
  moveServo(arm,     armAngle,  ARM_DOWN);     delay(300);
  moveServo(gripper, gripAngle, GRIP_CLOSED);  delay(500);  // grab
  moveServo(arm,     armAngle,  ARM_UP);       delay(300);
}

void placeObject() {
  moveServo(arm,     armAngle,  ARM_DOWN);     delay(300);
  moveServo(gripper, gripAngle, GRIP_OPEN);    delay(500);  // release
  moveServo(arm,     armAngle,  ARM_UP);       delay(300);
  moveServo(gripper, gripAngle, GRIP_CLOSED);  delay(200);
}

// Smoothly sweep a servo so the object is not jerked
void moveServo(Servo &sv, int &current, int target) {
  int step = (target > current) ? 1 : -1;
  while (current != target) {
    current += step;
    sv.write(current);
    delay(SERVO_STEP_DELAY);
  }
}

// ---------------- Line following ----------------
void readSensors() {
  for (int i = 0; i < 8; i++) s[i] = digitalRead(SENSOR_PINS[i]);  // black=1, white=0
}

bool match(int a, int b, int c, int d, int e, int f, int g, int h) {
  return s[0]==a && s[1]==b && s[2]==c && s[3]==d &&
         s[4]==e && s[5]==f && s[6]==g && s[7]==h;
}

bool allBlack() {
  for (int i = 0; i < 8; i++) if (!s[i]) return false;
  return true;
}

void followLine() {
  if (match(0,0,0,1,1,0,0,0) || match(0,0,1,1,1,1,0,0) || match(0,1,1,1,1,1,1,0)) {
    forward();
  }
  else if (
    match(0,1,1,1,1,1,1,1) || match(0,0,1,1,1,1,1,1) || match(0,0,0,1,1,1,1,1) ||
    match(0,0,0,0,1,1,1,1) || match(0,0,0,0,0,1,1,1) || match(0,0,0,0,0,0,1,1) ||
    match(0,0,0,0,0,0,0,1) || match(0,0,0,0,0,1,1,0) || match(0,0,0,0,1,1,0,0) ||
    match(0,0,0,0,1,1,1,0) || match(0,0,0,1,1,1,0,0) || match(0,0,0,1,1,1,1,0) ||
    match(0,0,1,1,1,1,1,0)) {
    turnRight();
  }
  else if (
    match(1,1,1,1,1,1,1,0) || match(1,1,1,1,1,1,0,0) || match(1,1,1,1,1,0,0,0) ||
    match(1,1,1,1,0,0,0,0) || match(1,1,1,0,0,0,0,0) || match(1,1,0,0,0,0,0,0) ||
    match(1,0,0,0,0,0,0,0) || match(0,1,1,0,0,0,0,0) || match(0,0,1,1,0,0,0,0) ||
    match(0,1,1,1,0,0,0,0) || match(0,0,1,1,1,0,0,0) || match(0,1,1,1,1,0,0,0) ||
    match(0,1,1,1,1,1,0,0)) {
    turnLeft();
  }
  else {
    motorStop();
  }
}

// ---------------- Motors ----------------
void forward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); analogWrite(ENA, SPEED_FORWARD);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); analogWrite(ENB, SPEED_FORWARD);
}
void turnRight() {
  digitalWrite(IN1, LOW);  digitalWrite(IN2, LOW); analogWrite(ENA, 0);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); analogWrite(ENB, SPEED_TURN);
}
void turnLeft() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); analogWrite(ENA, SPEED_TURN);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, LOW); analogWrite(ENB, 0);
}
void motorStop() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW); analogWrite(ENA, 0);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW); analogWrite(ENB, 0);
}
void driveForward(unsigned long ms) {
  forward(); delay(ms); motorStop();
}
