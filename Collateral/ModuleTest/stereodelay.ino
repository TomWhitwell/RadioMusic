/*
 * A simple hardware test which receives audio on the A2 analog pin
 * and sends it to the PWM (pin 3) output and DAC (A14 pin) output.
 *
 * This example code is in the public domain.
 */
/*
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>



AudioInputAnalog         adc1(A6);           //xy=231,115
AudioMixer4              mixer1;         //xy=455,113
AudioMixer4 mixer2;
AudioMixer4 mixer3;

AudioEffectDelay         delay1;         //xy=467,257
AudioEffectDelay         delay2;         //xy=467,257

AudioOutputAnalog        dac1;           //xy=733,133
AudioOutputPWM           pwm1; 
AudioConnection          patchCord1(adc1, 0, mixer1, 0);
AudioConnection          patchCord2(mixer1, delay1);
AudioConnection          patchCord3(delay1, 1, pwm1, 0);
AudioConnection          patchCord4(mixer1, 0, mixer2, 0 );
AudioConnection          patchCord5(delay1, 0, mixer1, 1);
AudioConnection          patchCord6(mixer2, 0, dac1,0 );

*/



#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

// GUItool: begin automatically generated code
AudioInputAnalog         adc1(A6);           //xy=231,115
AudioMixer4              mixer1;         //xy=455,113
AudioEffectDelay         delay1;         //xy=467,257
AudioEffectDelay         delay2;         //xy=475,515
AudioMixer4              mixer3;         //xy=478,390
AudioMixer4              mixer2;         //xy=617,135
AudioOutputAnalog        dac1;           //xy=733,133
AudioOutputPWM           pwm1;           //xy=852,427
AudioConnection          patchCord1(adc1, 0, mixer1, 0);
AudioConnection          patchCord2(adc1, 0, mixer3, 0);
AudioConnection          patchCord3(mixer1, delay1);
AudioConnection          patchCord4(mixer1, 0, mixer2, 0);
AudioConnection          patchCord5(delay1, 0, mixer1, 1);
AudioConnection          patchCord6(delay2, 0, mixer3, 1);
AudioConnection          patchCord7(mixer3, pwm1);
AudioConnection          patchCord8(mixer3, delay2);
AudioConnection          patchCord9(mixer2, dac1);
// GUItool: end automatically generated code










void setup() {
//  Serial.begin(38400);
  
  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(200);

  delay1.delay(0, 330);
  delay1.disable(1);
  delay1.disable(2);
  delay1.disable(3); 
  delay1.disable(4);
  delay1.disable(5);
  delay1.disable(6);
  delay1.disable(7);

  delay2.delay(0, 330);
  delay2.disable(1);
  delay2.disable(2);
  delay2.disable(3);
  delay2.disable(4);
  delay2.disable(5);
  delay2.disable(6);
  delay2.disable(7);


  mixer1.gain(0,0.75);
  mixer1.gain(1,0.60);
  mixer2.gain(0,0.6);
  mixer3.gain(0,0.75);
  mixer3.gain(1,0.50);


}

elapsedMillis lchange; 
elapsedMillis rchange; 
elapsedMillis lfchange;
elapsedMillis rfchange;

int ltime = 1000; 
int rtime = 500; 
int lftime = 500;
int rftime = 500;
int ldelay = 000; 
int rdelay=000;
float lflevel = 0.5;
float rflevel = 0.5;

void loop() {
  
  if (lchange>ltime){
  ldelay = ldelay+random(10);
ltime = random(1000);
lchange=0;
if (ldelay>330){ldelay = 1;};
  delay1.delay(0,ldelay);    
  }
  
  if (rchange>rtime){
   rdelay = rdelay - random(10);
  rtime = random(1000); 
    rchange = 0;
    if (rdelay<1){rdelay = 330;};
  delay2.delay(1,rdelay);
    
  }
  
if (lfchange>lftime){
lflevel = (float)random(14)/10;
 mixer1.gain(1,lflevel);
 lftime = random(500);
  lfchange = 0;
}

if (rfchange>rftime){
rflevel = (float)random(14)/10;
 mixer3.gain(1,rflevel);
 rftime = random(500);
  rfchange = 0;
}

//Serial.println(AudioMemoryUsageMax());

//
  delay(random(100));
}


