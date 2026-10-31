/*
 * 8-Sensor Line Follower - Electronova 2026 (Team Error 404)
 * Board  : Arduino Uno (or compatible)
 * Driver : L298N dual H-bridge
 * Sensor : 8-channel IR array (digital outputs)
 * Logic  : black line = 1, white surface = 0
 */

// ---- Sensor pins (left -> right) ----
const int SENSOR_PINS[8] = {A0, A1, A2, 5, 6, 7, 8, 9};
int s[8];

// ---- L298N motor driver pins ----
const int ENA = 3;   // Left motor speed (PWM)
const int IN1 = 2;
const int IN2 = 4;
const int ENB = 11;  // Right motor speed (PWM)
const int IN3 = 10;
const int IN4 = 12;

// ---- Speeds (0-255) ----
const int SPEED_FORWARD = 80;
const int SPEED_TURN    = 60;

// Function prototypes (so the sketch also builds in PlatformIO / VS Code)
bool match(int a, int b, int c, int d, int e, int f, int g, int h);
void forward();
void right();
void left();
void motor_stop();

void setup() {
  for (int i = 0; i < 8; i++) pinMode(SENSOR_PINS[i], INPUT);
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  Serial.begin(9600);
  delay(2000);  // time to place the robot on the track
}

void loop() {
  for (int i = 0; i < 8; i++) s[i] = digitalRead(SENSOR_PINS[i]);

  // Uncomment to calibrate / debug sensors from Serial Monitor
  // for (int i = 0; i < 8; i++) { Serial.print(s[i]); Serial.print(' '); }
  // Serial.println(); delay(200);

  // Pattern -> action (same patterns as the original hackathon code)
  if (match(0,0,0,1,1,0,0,0) || match(0,0,1,1,1,1,0,0) || match(0,1,1,1,1,1,1,0)) {
    forward();
  }
  else if (
    match(0,1,1,1,1,1,1,1) || match(0,0,1,1,1,1,1,1) || match(0,0,0,1,1,1,1,1) ||
    match(0,0,0,0,1,1,1,1) || match(0,0,0,0,0,1,1,1) || match(0,0,0,0,0,0,1,1) ||
    match(0,0,0,0,0,0,0,1) || match(0,0,0,0,0,1,1,0) || match(0,0,0,0,1,1,0,0) ||
    match(0,0,0,0,1,1,1,0) || match(0,0,0,1,1,1,0,0) || match(0,0,0,1,1,1,1,0) ||
    match(0,0,1,1,1,1,1,0)) {
    right();
  }
  else if (
    match(1,1,1,1,1,1,1,0) || match(1,1,1,1,1,1,0,0) || match(1,1,1,1,1,0,0,0) ||
    match(1,1,1,1,0,0,0,0) || match(1,1,1,0,0,0,0,0) || match(1,1,0,0,0,0,0,0) ||
    match(1,0,0,0,0,0,0,0) || match(0,1,1,0,0,0,0,0) || match(0,0,1,1,0,0,0,0) ||
    match(0,1,1,1,0,0,0,0) || match(0,0,1,1,1,0,0,0) || match(0,1,1,1,1,0,0,0) ||
    match(0,1,1,1,1,1,0,0)) {
    left();
  }
  else {
    motor_stop();
  }
}

// True if the 8 sensor readings equal the given pattern
bool match(int a, int b, int c, int d, int e, int f, int g, int h) {
  return s[0]==a && s[1]==b && s[2]==c && s[3]==d &&
         s[4]==e && s[5]==f && s[6]==g && s[7]==h;
}

void forward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);  analogWrite(ENA, SPEED_FORWARD);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);  analogWrite(ENB, SPEED_FORWARD);
}

void right() {  // left motor off, right motor on
  digitalWrite(IN1, LOW);  digitalWrite(IN2, LOW);  analogWrite(ENA, 0);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);  analogWrite(ENB, SPEED_TURN);
}

void left() {   // right motor off, left motor on
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);  analogWrite(ENA, SPEED_TURN);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, LOW);  analogWrite(ENB, 0);
}

void motor_stop() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW); analogWrite(ENA, 0);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW); analogWrite(ENB, 0);
}
