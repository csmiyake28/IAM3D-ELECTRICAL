#include <Arduino.h>

struct WheelSpeeds {
  float frontLeft;
  float frontRight;
  float rearLeft;
  float rearRight;
};

struct MotorPins {
  uint8_t pwm;
  uint8_t in1;
  uint8_t in2;
  bool reversed;
};

// Receiver signal pins
const uint8_t SIDE_PIN   = A0; // CH1
const uint8_t FORWARD_PIN = A1; // CH2
const uint8_t TURN_PIN   = A2; // CH4
const uint8_t ENABLE_PIN = A3; // CH5

// Motor driver pins: PWM, IN1, IN2, reverse direction
const MotorPins FL = {3, 2, 4, false};
const MotorPins FR = {5, 7, 8, false};
const MotorPins RL = {6, 10, 11, false};
const MotorPins RR = {9, 12, 13, false};

const unsigned long PULSE_TIMEOUT_US = 30000;

// Initial calibration; adjust after viewing receiver readings.
const int PULSE_MIN = 1000;
const int PULSE_CENTER = 1500;
const int PULSE_MAX = 2000;
const int DEAD_ZONE_US = 40;

// Limit initial testing to 30% PWM.
const float MAX_DRIVE = 0.30f;

bool validPulse(unsigned long pulse) {
  return pulse >= 900 && pulse <= 2100;
}

float pulseToCommand(unsigned long pulse) {
  int offset = (int)pulse - PULSE_CENTER;

  if (abs(offset) <= DEAD_ZONE_US) {
    return 0.0f;
  }

  float command;

  if (offset > 0) {
    command = (offset - DEAD_ZONE_US) /
              float(PULSE_MAX - PULSE_CENTER - DEAD_ZONE_US);
  } else {
    command = (offset + DEAD_ZONE_US) /
              float(PULSE_CENTER - PULSE_MIN - DEAD_ZONE_US);
  }

  return constrain(command, -1.0f, 1.0f);
}

WheelSpeeds calculateWheelSpeeds(float forward,
                                float sideways,
                                float rotation) {
  WheelSpeeds wheels;

  wheels.frontLeft  = forward + sideways + rotation;
  wheels.frontRight = forward - sideways - rotation;
  wheels.rearLeft   = forward - sideways + rotation;
  wheels.rearRight  = forward + sideways - rotation;

  float largest = 1.0f;
  largest = max(largest, abs(wheels.frontLeft));
  largest = max(largest, abs(wheels.frontRight));
  largest = max(largest, abs(wheels.rearLeft));
  largest = max(largest, abs(wheels.rearRight));

  wheels.frontLeft  /= largest;
  wheels.frontRight /= largest;
  wheels.rearLeft   /= largest;
  wheels.rearRight  /= largest;

  return wheels;
}

void setupMotor(const MotorPins &motor) {
  pinMode(motor.pwm, OUTPUT);
  analogWrite(motor.pwm, 0);

  pinMode(motor.in1, OUTPUT);
  pinMode(motor.in2, OUTPUT);

  digitalWrite(motor.in1, LOW);
  digitalWrite(motor.in2, LOW);
}

void setMotor(const MotorPins &motor, float command) {
  command = constrain(command, -1.0f, 1.0f);

  if (motor.reversed) {
    command = -command;
  }

  // Remove drive before changing direction inputs.
  analogWrite(motor.pwm, 0);

  if (command == 0.0f) {
    digitalWrite(motor.in1, LOW);
    digitalWrite(motor.in2, LOW);
    return;
  }

  digitalWrite(motor.in1, command > 0 ? HIGH : LOW);
  digitalWrite(motor.in2, command > 0 ? LOW : HIGH);

  int duty = (int)(abs(command) * MAX_DRIVE * 255.0f);
  analogWrite(motor.pwm, duty);
}

void drive(const WheelSpeeds &wheels) {
  setMotor(FL, wheels.frontLeft);
  setMotor(FR, wheels.frontRight);
  setMotor(RL, wheels.rearLeft);
  setMotor(RR, wheels.rearRight);
}

void stopMotors() {
  WheelSpeeds stopped = {0, 0, 0, 0};
  drive(stopped);
}

void setup() {
  setupMotor(FL);
  setupMotor(FR);
  setupMotor(RL);
  setupMotor(RR);

  pinMode(SIDE_PIN, INPUT);
  pinMode(FORWARD_PIN, INPUT);
  pinMode(TURN_PIN, INPUT);
  pinMode(ENABLE_PIN, INPUT);

  Serial.begin(115200);
}

void loop() {
  unsigned long sidePulse =
      pulseIn(SIDE_PIN, HIGH, PULSE_TIMEOUT_US);

  unsigned long forwardPulse =
      pulseIn(FORWARD_PIN, HIGH, PULSE_TIMEOUT_US);

  unsigned long turnPulse =
      pulseIn(TURN_PIN, HIGH, PULSE_TIMEOUT_US);

  unsigned long enablePulse =
      pulseIn(ENABLE_PIN, HIGH, PULSE_TIMEOUT_US);

  bool signalsValid =
      validPulse(sidePulse) &&
      validPulse(forwardPulse) &&
      validPulse(turnPulse) &&
      validPulse(enablePulse);

  bool enabled = signalsValid && enablePulse > 1700;

  if (enabled) {
    float sideways = pulseToCommand(sidePulse);
    float forward = pulseToCommand(forwardPulse);
    float rotation = pulseToCommand(turnPulse);

    // Negate an axis here if the stick direction is backward.
    drive(calculateWheelSpeeds(forward, sideways, rotation));
  } else {
    stopMotors();
  }

  static unsigned long lastPrint = 0;

  if (millis() - lastPrint >= 250) {
    lastPrint = millis();

    Serial.print("Side: "); Serial.print(sidePulse);
    Serial.print(" Forward: "); Serial.print(forwardPulse);
    Serial.print(" Turn: "); Serial.print(turnPulse);
    Serial.print(" Enable: "); Serial.print(enablePulse);
    Serial.print(" Driving: "); Serial.println(enabled);
  }
}