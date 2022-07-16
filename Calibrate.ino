const int FLEXPINS[] = {A1, A2, A3, A4, A5};
const int num_flexpins = 5;

const int BUTTON_PIN = 8;
const int LED_PIN = 6; // replace
//const int OUTPUT_PIN = A3; // use later

const float VCC = 5; // Measured voltage of Ardunio 5V line
const float R_DIV = 47500.0; // Measured resistance of 3.3k resistor

// CALIBRATION DATA
const int numCalibrationPoints = 20;
float straightResistanceCalibrations[num_flexpins][numCalibrationPoints];
float bendResistanceCalibrations[num_flexpins][numCalibrationPoints];
//float straightResistances[] = {37300, 37300, 37300, 37300, 37300};
//float bendResistances[] = {90000, 90000, 90000, 90000, 90000};
float straightResistances[num_flexpins];
float bendResistances[num_flexpins];

bool calibrated = false;
bool calibrating = false;

void setup() {
  delay(500);

  Serial.begin(115200);
  while (!Serial) {
    Serial.println("Waiting");
  }
  delay (1000);

  Serial.println("Setup starting");
  for (int pin = 0; pin < num_flexpins; pin++) {
    pinMode(FLEXPINS[pin], INPUT);
  }
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(LED_PIN, LOW);
  Serial.println("Setup complete");
  //  pinMode(OUTPUT_PIN, OUTPUT); // use later
}

void loop() {
  int button = digitalRead(BUTTON_PIN);
  Serial.println("button: " + String(button));
  Serial.println("calibrating: " + String(calibrating));
  
  if (button == 0 && calibrating == false) {
    Serial.println("Button 0, calibrating false");
    calibrating == true;
    calibrate();
  }

  if (calibrated == true) {

    for (int finger = 0; finger < num_flexpins; finger++) {
      float flexR = read_resistance(FLEXPINS[finger]);
      float angle = map(flexR, straightResistances[finger], bendResistances[finger],
                        0, 90.0);
      Serial.println("Finger: " + String(finger + 1) + "| Flex Resistance: " + String(flexR) +  "| Bend: " + String(angle) + " degrees");
    }

    Serial.println();
  

  }

  delay(3000);
}

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
  for (int i = 0 ; i < len ; i++)
    sum += array [i];
  return  ((float) sum) / len;
}

void calibrate() {
  Serial.println("Calibration started...");
  flash_led(5, 500); // flash for 5 seconds
  

  Serial.println("Calibrating open hand...");
  flash_led(2, 500); // flash for 1/2 seconds

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
      if(finger == 0){
        Serial.println(resistance);
      }
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
  flash_led(10, 500);
  
  calibrated = true;
  calibrating = false;
}
