// Designed by Ryan Page in April 2017 for the 
// Music Thing Modular Radio Music eurorack module by Tom Whitwell.
// Much of this code is a fork of the module-test firmware
// Many thanks to Tom for his amazing designs

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveformSine   sine1;          //xy=98,118
AudioSynthWaveformSine   sine3;          //xy=100,210
AudioSynthWaveformSine   sine4;          //xy=101,256
AudioSynthWaveformSine   sine2;          //xy=102,158
AudioSynthWaveformSine   sine5;          //xy=104,341
AudioSynthWaveformSine   sine8;          //xy=106,489
AudioSynthWaveformSine   sine6;          //xy=111,386
AudioSynthWaveformSine   sine7;          //xy=114,431
AudioSynthWaveformSine   sine9;          //xy=121,596
AudioSynthWaveformSine   sine10;         //xy=122,641
AudioSynthWaveformSine   sine12;         //xy=122,744
AudioSynthWaveformSine   sine14;         //xy=126,861
AudioSynthWaveformSine   sine13;         //xy=127,815
AudioSynthWaveformSine   sine11;         //xy=128,690
AudioSynthWaveformSine   sine15;         //xy=145,904
AudioSynthWaveformSine   sine16;         //xy=169,972
AudioMixer4              mixer2;         //xy=355,277
AudioMixer4              mixer1;         //xy=358,157
AudioMixer4              mixer4;         //xy=358,379
AudioMixer4              mixer5;         //xy=388,853
AudioMixer4              mixer3;         //xy=511,269
AudioOutputAnalog        dac1;           //xy=623,136
AudioConnection          patchCord1(sine1, 0, mixer1, 0);
AudioConnection          patchCord2(sine3, 0, mixer1, 2);
AudioConnection          patchCord3(sine4, 0, mixer1, 3);
AudioConnection          patchCord4(sine2, 0, mixer1, 1);
AudioConnection          patchCord5(sine5, 0, mixer2, 0);
AudioConnection          patchCord6(sine8, 0, mixer2, 3);
AudioConnection          patchCord7(sine6, 0, mixer2, 1);
AudioConnection          patchCord8(sine7, 0, mixer2, 2);
AudioConnection          patchCord9(sine9, 0, mixer4, 0);
AudioConnection          patchCord10(sine10, 0, mixer4, 1);
AudioConnection          patchCord11(sine12, 0, mixer4, 3);
AudioConnection          patchCord12(sine14, 0, mixer5, 1);
AudioConnection          patchCord13(sine13, 0, mixer5, 0);
AudioConnection          patchCord14(sine11, 0, mixer4, 2);
AudioConnection          patchCord15(sine15, 0, mixer5, 2);
AudioConnection          patchCord16(sine16, 0, mixer5, 3);
AudioConnection          patchCord17(mixer2, 0, mixer3, 1);
AudioConnection          patchCord18(mixer1, 0, mixer3, 0);
AudioConnection          patchCord19(mixer4, 0, mixer3, 2);
AudioConnection          patchCord20(mixer5, 0, mixer3, 3);
AudioConnection          patchCord21(mixer3, dac1);
AudioControlSGTL5000     sgtl5000_1;     //xy=111,1063
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
    AudioMemory(15);
  pinMode(RESET_BUTTON, INPUT);
  pinMode(RESET_CV, INPUT);
  pinMode(RESET_LED, OUTPUT);
  pinMode(LED0,OUTPUT);
  pinMode(LED1,OUTPUT);
  pinMode(LED2,OUTPUT);
  pinMode(LED3,OUTPUT);

  mixer1.gain(0, 0.24);
  mixer1.gain(1, 0.24);
  mixer1.gain(2, 0.24);
  mixer1.gain(3, 0.24);

  mixer2.gain(0, 0.24);
  mixer2.gain(1, 0.24);
  mixer2.gain(2, 0.24);
  mixer2.gain(3, 0.24);

  mixer4.gain(0, 0.24);
  mixer4.gain(1, 0.24);
  mixer4.gain(2, 0.24);
  mixer4.gain(3, 0.24);

  mixer5.gain(0, 0.24);
  mixer5.gain(1, 0.24);
  mixer5.gain(2, 0.24);
  mixer5.gain(3, 0.24);

  mixer3.gain(0, 0.2);
  mixer3.gain(1, 0.2);
  mixer3.gain(2, 0.2);
  mixer3.gain(3, 0.2);
  
  Serial.begin(9600);
  

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
analogWrite(LED0,cv2 /4);

// Very Messy Additive Synthesis Engine
// each waveform is the fundamental frequency
// multiplied/divided by a value derived from the time pot
// These are recursively multiplied for each successive waveform

      sine1.frequency((analogRead(CHAN_POT_PIN)*2 + analogRead(CHAN_CV_PIN)*2));
      sine1.amplitude(0.9);
      sine2.frequency((analogRead(CHAN_POT_PIN)*2 + analogRead(CHAN_CV_PIN)*2) * ((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001));
      sine2.amplitude(0.9);
      sine3.frequency((analogRead(CHAN_POT_PIN)*2 + analogRead(CHAN_CV_PIN)*2) * (((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001)*((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001)));
      sine3.amplitude(0.9);
      sine4.frequency((analogRead(CHAN_POT_PIN)*2 + analogRead(CHAN_CV_PIN)*2) * (((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001)*((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001)) * ((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001));
      sine4.amplitude(0.9);
      sine5.frequency((analogRead(CHAN_POT_PIN)*2 + analogRead(CHAN_CV_PIN)*2) * (((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001)*((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001)) * ((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001) * ((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001));
      sine5.amplitude(0.9);
      sine6.frequency((analogRead(CHAN_POT_PIN)*2 + analogRead(CHAN_CV_PIN)*2) * (((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001)*((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001)) * ((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001) * ((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001) * ((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001));
      sine6.amplitude(0.9);
      sine7.frequency((analogRead(CHAN_POT_PIN)*2 + analogRead(CHAN_CV_PIN)*2) * (((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001)*((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001)) * ((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001) * ((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001) * ((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001) * ((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001));
      sine7.amplitude(0.9);
      sine8.frequency((analogRead(CHAN_POT_PIN)*2 + analogRead(CHAN_CV_PIN)*2) * (((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001)*((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001)) * ((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001) * ((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001) * ((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001) * ((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001) * ((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001));
      sine8.amplitude(0.9);

      sine9.frequency((analogRead(CHAN_POT_PIN)*2 + analogRead(CHAN_CV_PIN)*2) / ((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001));
      sine9.amplitude(0.9);
      sine10.frequency((analogRead(CHAN_POT_PIN)*2 + analogRead(CHAN_CV_PIN)*2) / (((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001)*((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001)));
      sine10.amplitude(0.9);
      sine11.frequency((analogRead(CHAN_POT_PIN)*2 + analogRead(CHAN_CV_PIN)*2) / (((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001)*((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001)) * ((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001));
      sine11.amplitude(0.9);
      sine12.frequency((analogRead(CHAN_POT_PIN)*2 + analogRead(CHAN_CV_PIN)*2) / (((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001)*((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001)) * ((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001) * ((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001));
      sine12.amplitude(0.9);
      sine13.frequency((analogRead(CHAN_POT_PIN)*2 + analogRead(CHAN_CV_PIN)*2) / (((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001)*((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001)) * ((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001) * ((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001) * ((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001));
      sine13.amplitude(0.9);
      sine14.frequency((analogRead(CHAN_POT_PIN)*2 + analogRead(CHAN_CV_PIN)*2) / (((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001)*((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001)) * ((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001) * ((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001) * ((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001) * ((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001));
      sine14.amplitude(0.9);
      sine15.frequency((analogRead(CHAN_POT_PIN)*2 + analogRead(CHAN_CV_PIN)*2) / (((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001)*((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001)) * ((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001) * ((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001) * ((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001) * ((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001) * ((analogRead(TIME_POT_PIN) + analogRead(TIME_CV_PIN)) * 0.001));
      sine15.amplitude(0.9);
     

       sine16.frequency((analogRead(CHAN_POT_PIN)*2 + analogRead(CHAN_CV_PIN)*2));
      sine16.amplitude(0.0);

}

// WRITE A 4 DIGIT BINARY NUMBER TO LED0-LED3 
void ledWrite(int n){
  digitalWrite(RESET_LED, HIGH && (n & B00010000));
  digitalWrite(LED0, HIGH && (n & B00001000));
  digitalWrite(LED1, HIGH && (n & B00000100));
  digitalWrite(LED2, HIGH && (n & B00000010));
  digitalWrite(LED3, HIGH && (n & B00000001)); 
}


