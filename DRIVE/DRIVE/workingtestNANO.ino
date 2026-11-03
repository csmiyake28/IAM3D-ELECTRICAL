int ch1, ch2, ch3, ch4, ch5, ch6;

void setup() {
  Serial.begin(19200);
  pinMode(2, INPUT); //Here we make a channel for each of the inputs having all different pin modes 
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  pinMode(6, INPUT);
  pinMode(7, INPUT);
}

void loop() {
  ch1 = pulseIn(2, HIGH, 25000);  // microseconds
  ch2 = pulseIn(3, HIGH, 25000); // Setting them to each of the right channels 
  ch3 = pulseIn(4, HIGH, 25000);
  ch4 = pulseIn(5, HIGH, 25000);
  ch5 = pulseIn(6, HIGH, 25000);
  ch6 = pulseIn(7, HIGH, 25000);


  Serial.print("CH1: "); Serial.print(ch1);
  Serial.print("  CH2: "); Serial.print(ch2); // print out each of the readings on each thing
  Serial.print("  CH3: "); Serial.print(ch3);
  Serial.print("  CH4: "); Serial.print(ch4);
  Serial.print("  CH5: "); Serial.print(ch5);
  Serial.print("  CH6: "); Serial.println(ch6);
}