/*
Tests the RadioMusic Module is working correctly 
 */
 
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

// GUItool: begin automatically generated code
AudioSynthWaveformSine   sine1;          //xy=156,46
AudioSynthWaveformSineModulated sine_fm1;       //xy=261,128
AudioOutputAnalog        dac1;           //xy=485,101
AudioConnection          patchCord1(sine1, sine_fm1);
AudioConnection          patchCord2(sine_fm1, dac1);
// GUItool: end automatically generated code


 
 
 
 

#define LED0 6
#define LED1 5
#define LED2 4
#define LED3 3
#define CHAN_POT_PIN 9 // pin for Channel pot
#define CHAN_CV_PIN 6 // pin for Channel CV 
#define TIME_POT_PIN 7 // pin for Time pot
#define TIME_CV_PIN 8 // pin for Time CV
#define RESET_BUTTON 8 // Reset button 
#define RESET_LED 11 // Reset LED indicator 
#define RESET_CV 9 // Reset pulse input 

void setup() {
    AudioMemory(5);
  pinMode(RESET_BUTTON, INPUT);
  pinMode(RESET_CV, INPUT);
  pinMode(RESET_LED, OUTPUT);
  pinMode(LED0,OUTPUT);
  pinMode(LED1,OUTPUT);
  pinMode(LED2,OUTPUT);
  pinMode(LED3,OUTPUT);
  
  Serial.begin(9600);
  
elapsedMillis countdown; 
while (countdown*2 < 2000){
        sine1.frequency(countdown);
      sine1.amplitude(0.9);

      sine_fm1.frequency(4000-(countdown*2));
      sine_fm1.amplitude(0.9);
      delay(10);
}
  
  
    Serial.println ("Hi, welcome to the Radio Music setup test");
  Serial.println ("");
  Serial.println ("Cycling through the lights:");
 for (int x = 0; x < 5; x++){
      Serial.println("Lighting LED:");
 
  for(int i = 0; i<5; i++){
      ledWrite(pow(2,i));
      Serial.print(i);
      Serial.print(" ");
              sine1.frequency(i*250);
      sine_fm1.frequency(2500 - (x*125));

      delay(250);
  }
  Serial.println("");
 }
  
  
  Serial.println ("Inputs mapped to LEDs:");
  Serial.println ("[1] Station Pot");
  Serial.println ("[2] Start Pot");
  Serial.println ("[3] Tune CV");  
  Serial.println ("[4] Start CV");  
  

}

elapsedMillis update = 0;

// the loop routine runs over and over again forever:
void loop() {

int averagecount = 50;  
int pot1; 
int pot2; 
int cv1; 
int cv2; 
for(int i = 0; i< averagecount ; i++){  
  
 pot1 += analogRead(CHAN_POT_PIN);
 pot2 +=  analogRead(TIME_POT_PIN);  
 cv1 += analogRead(CHAN_CV_PIN); 
 cv2 += analogRead(TIME_CV_PIN); 
}

pot1 = pot1 / averagecount; 
pot2 = pot2 / averagecount; 
cv1 = cv1 / averagecount; 
cv2 = cv2 / averagecount; 


if (update > 50){

Serial.print("Channel pot=");
Serial.print(pot1);
Serial.print(" Time pot=");
Serial.print(pot2);
Serial.print(" Channel CV=");
Serial.print(cv1);
Serial.print(" Time CV=");
Serial.println(cv2);
update = 0;
}
  
analogWrite(LED3,pot1 / 4);
analogWrite(LED2,pot2 / 4);
analogWrite(LED1,cv1 /4);
analogWrite(LED0,cv1 /4);

      sine1.frequency(analogRead(CHAN_POT_PIN)*2 + analogRead(CHAN_CV_PIN)*2);
      sine1.amplitude(0.9);

      sine_fm1.frequency(analogRead(TIME_POT_PIN)*2 + analogRead(TIME_CV_PIN)*2);
      sine_fm1.amplitude(0.9);



}

// WRITE A 4 DIGIT BINARY NUMBER TO LED0-LED3 
void ledWrite(int n){
  digitalWrite(RESET_LED, HIGH && (n & B00010000));
  digitalWrite(LED0, HIGH && (n & B00001000));
  digitalWrite(LED1, HIGH && (n & B00000100));
  digitalWrite(LED2, HIGH && (n & B00000010));
  digitalWrite(LED3, HIGH && (n & B00000001)); 
}


