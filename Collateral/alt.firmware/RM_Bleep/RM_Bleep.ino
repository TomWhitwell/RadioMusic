/*
  Bleep
 Turns top left front panel LED on for one second, then off for one second, repeatedly.
 Does the same with a 440hz sine wave. 
 
 This example code is in the public domain.
 */


#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

// GUItool: begin automatically generated code
AudioSynthWaveformSine   sine1;          //xy=563,136
AudioOutputAnalog        dac1;           //xy=733,133
AudioConnection          patchCord1(sine1, dac1);
// GUItool: end automatically generated code


// In the Radio Music Hardware,  Bank LEDs are 3,4,5,6, Reset LED is 11. 
// Choose an LED and give it a name:
int led = 3;

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);     

  // Create some audio memory   
  AudioMemory(5);
  // Create a sine wave oscillator 
  sine1.frequency(440);

}

// the loop routine runs over and over again forever:
void loop() {
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  sine1.amplitude(0.9);      // Turn up the sine wave output 
  delay(1000);               // wait for a second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  sine1.amplitude(0);      // Turn off the sine wave output 

  delay(1000);               // wait for a second
}


