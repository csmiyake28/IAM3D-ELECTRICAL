
// Positive = forward; negative = reverse.

struct WheelSpeeds {
  float frontLeft;
  float frontRight;
  float rearLeft;
  float rearRight;
};

WheelSpeeds calculateWheelSpeeds(float forward,
                                float sideways,
                                float rotation) {
  WheelSpeeds wheels;

  // Positive sideways = right.
  // Positive rotation = clockwise.
  wheels.frontLeft  = forward + sideways + rotation;
  wheels.frontRight = forward - sideways - rotation;
  wheels.rearLeft   = forward - sideways + rotation;
  wheels.rearRight  = forward + sideways - rotation;

  // Scale all outputs together if any exceeds magnitude 1.
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

void printWheelSpeeds(const WheelSpeeds &wheels) {
  Serial.print("FL: ");
  Serial.print(wheels.frontLeft, 2);

  Serial.print("  FR: ");
  Serial.print(wheels.frontRight, 2);

  Serial.print("  RL: ");
  Serial.print(wheels.rearLeft, 2);

  Serial.print("  RR: ");
  Serial.println(wheels.rearRight, 2);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Controls: w/s forward/back, a/d sideways");
  Serial.println("q/e rotate, x stop");
}

void loop() {
  if (Serial.available() == 0) {
    return;
  }

  char command = Serial.read();

  float forward = 0;
  float sideways = 0;
  float rotation = 0;
  const float speed = 0.5f;

  switch (command) {
    case 'w': forward = speed;   break;
    case 's': forward = -speed;  break;
    case 'a': sideways = -speed; break;
    case 'd': sideways = speed;  break;
    case 'q': rotation = -speed; break;
    case 'e': rotation = speed;  break;
    case 'x': break;  // All values remain zero.
    default: return; // Ignore other characters and line endings.
  }

  Serial.print("Command: ");
  Serial.println(command);

  printWheelSpeeds(
    calculateWheelSpeeds(forward, sideways, rotation)
  );
}
