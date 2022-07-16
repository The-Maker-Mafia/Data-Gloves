//Constants:
const int ledPin = 3;   //pin 3 has PWM funtion
const int flexPin = A0; //pin A0 to read analog input

//Variables:
int flexAmount; //save analog value


void setup(){
  
  Serial.begin(9600);       //Begin serial communication

}

void loop(){
  flexAmount = analogRead(flexPin);         //Read and save analog value from potentiometer
  Serial.println(flexAmount);               //Print value
  flexAmount = map(flexAmount, 700, 900, 0, 255);//Map value 0-1023 to 0-255 (PWM)
  
  voltageAmount = map(flexAmount, 0, 255, 0, 5);
  
  delay(100);                          //Small delay
}