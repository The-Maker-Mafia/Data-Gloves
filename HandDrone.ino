// depending on the imu orientation, you might need to change the max_accel_dir section

#include <map>
#include <Arduino_LSM9DS1.h>

int num_datapts = 100;
bool controlling = false;
int timeCounter = 0;

bool spinning = false;
int spin_duration = 30;
float angvel_threshold = 100;
float closed_threshold; // set based on calibration
float open_threshold; // set based on calibration

float Ax, Ay, Az;
float Gx, Gy, Gz;

int spincount;

const int FLEXPINS[] = {A1, A2, A3, A4, A5};
const int num_flexpins = 5;

const int BUTTON_PIN = 8;
const int LED_PIN = LED_BUILTIN;

const float VCC = 5; // Change this to 3V for BLE? (Measured voltage of Ardunio 5V line)
const float R_DIV = 47500.0; // (Measured resistance of 3.3k resistor)

// CALIBRATION DATA
const int numCalibrationPoints = 10;
float straightResistanceCalibrations[num_flexpins][numCalibrationPoints];
float bendResistanceCalibrations[num_flexpins][numCalibrationPoints];

//float straightResistances[num_flexpins];
//float bendResistances[num_flexpins];
float straightResistances[] = {400.40, 390.60, 400.10, 357.30, 391.50};
float bendResistances[] = {388.50, 331.10, 359.30, 328.10, 325.00};

bool calibrated = false;
bool calibrating = false;
bool shouldCalibrate = true; // turn off for rapid testing

void setup() {
  Serial.begin(9600);
//  while (!Serial);
  Serial.println("Started");

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }
  Serial.println("Accelerometer sample rate = " + String(IMU.accelerationSampleRate()) + "Hz");
  Serial.println("Gyroscope sample rate = " + String(IMU.gyroscopeSampleRate()) + "Hz");
  
  flash_led(2, 500);
  for (int pin = 0; pin < num_flexpins; pin++) {
    pinMode(FLEXPINS[pin], INPUT);
  }
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(LED_PIN, LOW);
  Serial.println("Setup complete");
  //  pinMode(OUTPUT_PIN, OUTPUT); // use later
  
}

  
void loop() {
  
  // TRIGGER CALIBRATION
  int button = digitalRead(BUTTON_PIN);
  if (button == 0 && calibrating == false && shouldCalibrate) {
    Serial.println("Button 0, calibrating false");
    calibrating = true;
    calibrate();
  }


  // POST CALIBRATION
  if (calibrated == true || !shouldCalibrate) {

    // WATCH FOR INITIALIZATION COMMAND
    if (!controlling) {
//      Serial.println("average_flex" + String(average_flex()));
//      Serial.println("average_resistance" + String(average_resistance()));
//      if(average_flex() < open_threshold) {
      if(average_resistance() > open_threshold) {
        Serial.println("INITIALIZATION command triggered.");
        controlling = true;
      }
    }

    if (controlling) {
    
      IMU.readAcceleration(Ax, Ay, Az);
      String max_accel_dir;
//      if (Gx > 0.5) {max_accel_dir = "right";}
//      else if (Gx < 0.5) {max_accel_dir = "left";}
//      else if (Gy > 0.5) {max_accel_dir = "backward";}
//      else if (Gy < 0.5) {max_accel_dir = "forward";}

      // change this section depending on imu orientation
      if (max(abs(Gz), abs(Gy)) == abs(Gz)) {
        if (Gz > 0) {max_accel_dir = "backward";}
        else {max_accel_dir = "forward";}
      }
      else if (max(abs(Gz), abs(Gy)) == abs(Gy)) {
        if (Gy > 0) {max_accel_dir = "left";}
          else {max_accel_dir = "right";}
      }
      
//      float max_acceleration = max(abs(Ax), abs(Ay));
//      
//      float max_acceleration = max(Ax, Ay, max(Ay, Az));
//      if (max_acceleration == Ax) {string max_accel_dir = "x"}
//      else if (max_acceleration == Ay) {string max_accel_dir = "y"}
//      else {String max_accel_dir = "z"}

//      Serial.println("Acceleration X: " + String(Ax) + " ||| Acceleration Y: " + String(Ay) + " ||| Acceleration Z: " + String(Az));
//      Serial.println("Max Accleration Dir: " + max_accel_dir)

      IMU.readGyroscope(Gz, Gy, Gz);
      float max_angvel = max(Gx, max(Gy, Gz));
//      String max_angvel_dir;
//      if (max_angvel == Ax) {max_angvel_dir = "x";}
//      else if (max_angvel == Ay) {max_angvel_dir = "y";}
//      else {max_angvel_dir = "z";}
  
      Serial.println("Angular Velocity X: " + String(Gx) + " ||| Angular Velocity Y: " + String(Gy) + " ||| Angular Velocity Z: " + String(Gz));
//      Serial.println("Max Angular Velocity Direction: " + max_angvel_dir);
        
      // WATCH FOR SHUTDOWN COMMAND
//      Serial.println("average_flex" + String(average_flex()));
//      Serial.println("average_resistance" + String(average_resistance()));
//      if(average_flex() > closed_threshold) {
      if(average_resistance() < closed_threshold) {
        Serial.println("SHUTDOWN command triggered.");
        controlling = false;
      }

      if(!spinning) {

        // WATCH FOR SPIN COMMAND
        if(abs(max_angvel) > angvel_threshold) {
          spinning = true;
          spincount = 0;
          Serial.println("SPIN command triggered.");
          // drone.spin(max_accel_dir);
          Serial.println("SPIN PARAMS || direction: " + max_accel_dir + ", speed: default"); // should we make speed adjustable?
        }

        // MOVE DRONE
        else {
          Serial.println("MOVE because no spin.");
//           calculate angle
//           drone.move(map(angle));
          Serial.println("MOVE PARAMS || Acceleration X: " + String(Ax) + ", Acceleration Y: " + String(Ay) + " ||| Acceleration Z: " + String(Az));
        }
      }

      if(spinning) {
        // WATCH FOR END OF SPIN
        Serial.println("max_angvel: " + String(max_angvel));
        if(abs(max_angvel) < angvel_threshold) {
          spincount++;
          Serial.println("Not spinning for " + String(spincount));
          if(spincount > spin_duration) {
            spinning = false;
          }
        }
      }
    }
  }
  
  delay(100); // smallest time collection interval
  
}



// HELPER FUNCTIONS

//float average_flex()
//{
//  float angles[5];
//  for (int finger = 0; finger < num_flexpins; finger++) {
//    float flexR = read_resistance(FLEXPINS[finger]);
//    float angle = map(flexR, straightResistances[finger], bendResistances[finger],
//                      0, 90.0);
//    angles[finger] = angle;
//  }
////  Serial.println("ANGLES || 1: " + String(angles[0]) + " 2: " + String(angles[1]) + " 3: " + String(angles[2]) + " 4: " + String(angles[3]) + " 5: " + String(angles[4]));
//  float avg_flex = average(angles, num_flexpins);
//  return avg_flex;
//}

float average_resistance()
{
  float resistances[5];
  for (int finger = 0; finger < num_flexpins; finger++) {
    float flexR = read_resistance(FLEXPINS[finger]);
    resistances[finger] = flexR;
  }
  float avg_resistance = average(resistances, num_flexpins);
  return avg_resistance;
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

// read resistance
float read_resistance(int pin_to_read) {
  // Read the ADC, and calculate voltage and resistance from it
  int flexADC = analogRead(pin_to_read);
  float flexV = flexADC * VCC / 1023.0;
  float flexR = R_DIV * (VCC / flexV - 1.0);
  //return flexR;
  return flexADC;
}

// average list
float average (float * array, int len) {
  long sum = 0L;
  for (int i = 0 ; i < len ; i++)
    sum += array [i];
  return  ((float) sum) / len;
}


// calibrate flex
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

  open_threshold = average(straightResistances, num_flexpins) + 20; // some buffer for intention
  closed_threshold = average(bendResistances, num_flexpins); // some buffer for intention
  Serial.println("open_threshold" + String(open_threshold));
  Serial.println("closed_threshold" + String(closed_threshold));

  Serial.println("Calibration complete.");
  flash_led(5, 500);
  
  calibrated = true;
  calibrating = false;
}

