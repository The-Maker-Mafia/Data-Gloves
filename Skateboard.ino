const int FLEXPINS[] = {A1, A2, A3, A4, A5};
const int num_flexpins = 5;

const int BUTTON_PIN = 8;
const int LED_PIN = 32;//LED_BUILTIN; // replace
//const int OUTPUT_PIN = A3; // use later
const int PWM = 5;
const float VCC = 5; // Measured voltage of Ardunio 5V line
const float R_DIV = 47500.0; // Measured resistance of 3.3k resistor

// CALIBRATION DATA
const int numCalibrationPoints = 10;
float straightResistanceCalibrations[num_flexpins][numCalibrationPoints];
float bendResistanceCalibrations[num_flexpins][numCalibrationPoints];

//float straightResistances[num_flexpins];
//float bendResistances[num_flexpins];
float straightResistances[] = {627, 559, 582, 523, 572};
float bendResistances[] = {585, 510, 555, 500, 500};

bool calibrated = false;
bool calibrating = false;
bool shouldCalibrate = true;

void flash_led(int num_times, float interval) {
  int i = 0;
  while (i < num_times) {
    digitalWrite(LED_PIN, HIGH);
    delay(interval);
    digitalWrite(LED_PIN, LOW);
    delay(interval);
    i++;
  }
}

bool fingerIsUp(float flex){
    if(flex < 45.0){
        return true;
    }
    return false;
}

int getCommand(float flex1, float flex2, float flex3, float flex4, float flex5) {
    //Serial.println("flex11: " + String(flex1) + " | flex2: " + String(flex2));

    
    // Here, if somebody had fingers 1, 2, and 4 up, then it would STOP because that's invalid, however, what if it was simply a case of their
    if(fingerIsUp(flex2) && !fingerIsUp(flex3) && !fingerIsUp(flex4) && !fingerIsUp(flex5)){
      return 1;
    }
    //Finger 1, 2 are up. Finger 3, 4 are down
    else if(fingerIsUp(flex2) && fingerIsUp(flex3) && !fingerIsUp(flex4) && !fingerIsUp(flex5)){
      return 2;
    }
    else if(fingerIsUp(flex2) && fingerIsUp(flex3) && fingerIsUp(flex4) && !fingerIsUp(flex5)) {
      return 3;
    }
    else if(fingerIsUp(flex2) && fingerIsUp(flex3) && fingerIsUp(flex4) && fingerIsUp(flex5)) {
      return 4;
    } 
    else {
      return 0;
    }
}

void runCommand(int command){
  if(command == 1){
    // speed = 25%
    Serial.println("Finger 2 lifted");
    analogWrite(PWM, 120);
  }
  else if (command == 2){
    // speed = 50%
    Serial.println("Finger 3 lifted");
    analogWrite(PWM, 125);
  }
  else if (command == 3){
    // speed = 75%
    Serial.println("Finger 4 lifted");
    analogWrite(PWM, 130);
  }
  else if (command == 4){
    // speed = 100%
    Serial.println("Finger 5 lifted");
    analogWrite(PWM, 135);
  }
  else if (command == 0){
    Serial.println("No fingers lifted");
    analogWrite(PWM, 0);
  }
}

void setup() {
  Serial.begin(115200);

  Serial.println("Setup starting");
  flash_led(2, 500);
  for (int pin = 0; pin < num_flexpins; pin++) {
    pinMode(FLEXPINS[pin], INPUT);
  }
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(LED_PIN, LOW);
  pinMode(PWM, OUTPUT);
  Serial.println("Setup complete");
  //  pinMode(OUTPUT_PIN, OUTPUT); // use later
}

void loop() {
  
  int button = digitalRead(BUTTON_PIN);
  if (button == 0 && calibrating == false && shouldCalibrate) {
    Serial.println("Button 0, calibrating false");
    calibrating == true;
    calibrate();
  }

  if (calibrated == true || !shouldCalibrate) {
    float angles[5];
    for (int finger = 0; finger < num_flexpins; finger++) {
      float flexR = read_resistance(FLEXPINS[finger]);
      float angle = map(flexR, straightResistances[finger], bendResistances[finger],
                        0, 90.0);
      angles[finger] = angle;
      //Serial.println("Finger: " + String(finger + 1) + "| Flex Resistance: " + String(flexR) +  "| Bend: " + String(angle) + " degrees");
    }
    const int command = getCommand(angles[0], angles[1], angles[2], angles[3], angles[4]);
    runCommand(command);
  }

}

float read_resistance(int pin_to_read) {
  // Read the ADC, and calculate voltage and resistance from it
  int flexADC = analogRead(pin_to_read);
  float flexV = flexADC * VCC / 1023.0;
  float flexR = R_DIV * (VCC / flexV - 1.0);
  //return flexR;
  return flexADC;
}

// assuming array is int.
float average (float * array, int len) {
  long sum = 0L;
  for (int i = 0 ; i < len ; i++){
   sum += array [i]; 
  }
  return  ((float) sum) / len;
}

void calibrate() {
  Serial.println("Calibration started...");

  Serial.println("Calibrating open hand...");
  flash_led(2, 500); // flash for 1 seconds

  for (int calibration_point = 0; calibration_point < numCalibrationPoints; calibration_point++) {
    for (int finger = 0; finger < num_flexpins; finger++) {
      float resistance = read_resistance(FLEXPINS[finger]);
      if(finger == 0){
        Serial.println(resistance);
      }
      straightResistanceCalibrations[finger][calibration_point] = resistance;
    }
    delay(500);
  }

  Serial.println("Calibrating closed hand..."); // flash for a ssecond
  flash_led(2, 500);
  for (int calibration_point = 0; calibration_point < numCalibrationPoints; calibration_point++) {
    for (int finger = 0; finger < num_flexpins; finger++) {
      float resistance = read_resistance(FLEXPINS[finger]);
      bendResistanceCalibrations[finger][calibration_point] = resistance;
    }
    delay(500);
  }
  Serial.println("Calibration data collected. Calculating values...");

  /*
  Serial.println("Printing Finger 5");
  for (int i = 0 ; i < 20 ; i++) {
    Serial.println(straightResistanceCalibrations[4] [i]);
    Serial.println(bendResistanceCalibrations[4] [i]);
  }
  */
  Serial.println("-------------------");
  for (int finger = 0; finger < num_flexpins; finger++) {
    straightResistances[finger] = average(straightResistanceCalibrations[finger], numCalibrationPoints);
    Serial.println("Finger " + String(finger + 1) + " | Straight Resistance: " + straightResistances[finger]);
    bendResistances[finger] = average(bendResistanceCalibrations[finger], numCalibrationPoints);
    Serial.println("Finger " + String(finger + 1) + " | Bend Resistance: " + bendResistances[finger]);
  }

  Serial.println("Calibration complete.");
  flash_led(5, 500);
  
  calibrated = true;
  calibrating = false;
}
