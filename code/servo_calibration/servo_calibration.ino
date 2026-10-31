/*
 * Servo calibration helper (gripper + arm)
 * Open Serial Monitor (9600 baud, "Newline" line ending) and type:
 *   g90   -> move gripper servo to 90 degrees
 *   a120  -> move arm servo to 120 degrees
 * Note down the angles for GRIP_OPEN, GRIP_CLOSED, ARM_UP, ARM_DOWN
 * and put them in phase2_pick_and_place.ino.
 * Keep the gripper free of objects while finding the limits so the
 * servo does not stall against the gears.
 */
#include <Servo.h>

const int GRIPPER_PIN = 13;
const int ARM_PIN     = A3;
Servo gripper, arm;

void setup() {
  Serial.begin(9600);
  gripper.attach(GRIPPER_PIN);
  arm.attach(ARM_PIN);
  gripper.write(90);
  arm.write(90);
  Serial.println(F("Type g<angle> or a<angle>, e.g. g60 or a110"));
}

void loop() {
  if (!Serial.available()) return;
  char which = Serial.read();
  int angle = Serial.parseInt();
  while (Serial.available()) Serial.read();   // flush rest of line
  angle = constrain(angle, 0, 180);
  if (which == 'g' || which == 'G') { gripper.write(angle); Serial.print(F("Gripper -> ")); Serial.println(angle); }
  else if (which == 'a' || which == 'A') { arm.write(angle); Serial.print(F("Arm -> ")); Serial.println(angle); }
}
