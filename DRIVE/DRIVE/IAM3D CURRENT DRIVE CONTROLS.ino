#include <Arduino.h>
#include <math.h>

struct Motor {
  uint8_t in1;     // Hardware PWM
  uint8_t in2;     // Digital direction
  bool reversed;   // Flip if this wheel is mounted in reverse
};

// Order: front left, front right, rear left, rear right.
const Motor motors[] = {
  {3, 2, false},
  {5, 4, false},
  {6, 7, false},
  {9, 8, false}
};

const uint8_t SIDE_PIN    = A0; // CH1
const uint8_t TURN_PIN    = A1; // CH2
const uint8_t FORWARD_PIN = A2; // CH3
// A3 / CH4 reserved
const uint8_t ENABLE_PIN  = A4; // CH5

const unsigned long RX_TIMEOUT_US = 30000UL;
const int CENTER_US = 1500;
const int DEAD_ZONE_US = 40;
const int AXIS_SPAN_US = 500;
const int MAX_PWM = 102; // 40% of 255

bool armed = false;
bool sawOff = false;

bool validPulse(unsigned long pulse) {
  return pulse >= 900 && pulse <= 2100;
}

float axisCommand(unsigned long pulse) {
  int offset = (int)pulse - CENTER_US;

  if (abs(offset) <= DEAD_ZONE_US) return 0.0f;

  offset += (offset > 0) ? -DEAD_ZONE_US : DEAD_ZONE_US;
  float value = float(offset) / (AXIS_SPAN_US - DEAD_ZONE_US);
  return constrain(value, -1.0f, 1.0f);
}

void setMotor(const Motor &motor, float command) {
  command = constrain(command, -1.0f, 1.0f);
  if (motor.reversed) command = -command;

  int duty = (int)(fabsf(command) * MAX_PWM);

  if (duty == 0) {
    digitalWrite(motor.in2, LOW);
    analogWrite(motor.in1, 0);           // Both low: coast
  } else if (command > 0.0f) {
    digitalWrite(motor.in2, LOW);
    analogWrite(motor.in1, duty);        // Forward during PWM high
  } else {
    digitalWrite(motor.in2, HIGH);
    analogWrite(motor.in1, 255 - duty);  // Reverse during PWM low
  }
}

void setup() {
  for (const Motor &motor : motors) {
    digitalWrite(motor.in1, LOW);
    digitalWrite(motor.in2, LOW);
    pinMode(motor.in1, OUTPUT);
    pinMode(motor.in2, OUTPUT);
  }

  pinMode(SIDE_PIN, INPUT);
  pinMode(TURN_PIN, INPUT);
  pinMode(FORWARD_PIN, INPUT);
  pinMode(ENABLE_PIN, INPUT);

  Serial.begin(115200);
  Serial.println("CH5 off, center CH1/CH2/CH3, then CH5 on.");
}

void loop() {
  unsigned long enable = pulseIn(ENABLE_PIN, HIGH, RX_TIMEOUT_US);

  // Stop before waiting for the other channels if CH5 is off or missing.
  if (!validPulse(enable) || enable <= 1700) {
    armed = false;
    for (const Motor &motor : motors) setMotor(motor, 0.0f);
  }

  unsigned long side = pulseIn(SIDE_PIN, HIGH, RX_TIMEOUT_US);
  unsigned long turn = pulseIn(TURN_PIN, HIGH, RX_TIMEOUT_US);
  unsigned long forward = pulseIn(FORWARD_PIN, HIGH, RX_TIMEOUT_US);

  bool valid = validPulse(enable) && validPulse(side) &&
               validPulse(turn) && validPulse(forward);

  float s = valid ? axisCommand(side) : 0.0f;
  float t = valid ? axisCommand(turn) : 0.0f;
  float f = valid ? axisCommand(forward) : 0.0f;

  if (!valid) {
    armed = false;
    sawOff = false;
  } else if (enable < 1300) {
    sawOff = true;
  } else if (enable > 1700 && !armed && sawOff &&
             s == 0.0f && t == 0.0f && f == 0.0f) {
    armed = true;
    sawOff = false;
  }

  // X-layout mecanum mixing: FL, FR, RL, RR.
  float wheel[] = {f + s + t, f - s - t, f - s + t, f + s - t};

  float largest = 1.0f;
  for (float speed : wheel) {
    if (fabsf(speed) > largest) largest = fabsf(speed);
  }

  for (uint8_t i = 0; i < 4; i++) {
    wheel[i] = armed ? wheel[i] / largest : 0.0f;
    setMotor(motors[i], wheel[i]);
  }

  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= 250) {
    lastPrint = millis();
    Serial.print("Side: ");    Serial.print(side);
    Serial.print(" Forward: "); Serial.print(forward);
    Serial.print(" Turn: ");    Serial.print(turn);
    Serial.print(" Enable: ");  Serial.print(enable);
    Serial.print(" Armed: ");   Serial.print(armed);
    Serial.print(" FL: ");      Serial.print(wheel[0], 2);
    Serial.print(" FR: ");      Serial.print(wheel[1], 2);
    Serial.print(" RL: ");      Serial.print(wheel[2], 2);
    Serial.print(" RR: ");      Serial.println(wheel[3], 2);
  }
}
}
