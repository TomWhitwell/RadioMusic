#include <Bounce.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

#define CHAN_POT_PIN 9 // pin for Channel pot
#define CHAN_CV_PIN 8 // pin for Channel CV 
#define TIME_POT_PIN 7 // pin for Time pot
#define TIME_CV_PIN 6 // pin for Time CV
#define RESET_BUTTON 8 // Reset button 
#define RESET_LED 11 // Reset LED indicator 
#define RESET_CV 9 // Reset pulse input 
#define BANK_BUTTON 2 // Bank Button 
#define LED0 6
#define LED1 5
#define LED2 4
#define LED3 3
Bounce bankSwitch = Bounce( BANK_BUTTON, 20 ); 

// La Monte Young's scale for Well Tuned Piano, derived from http://www.kylegann.com/wtp.html
// scale[notes from Eb][octaves from Eb0 to Eb8] 
// Fifths: 8-3-10-5 / 9-4-11-6-1 / 0-7-2 
// Minor 7ths: 8-9-0 / 3-4-7 / 10-11-2 / 5-6
float scale[12][9]={{19.44,38.89,77.78,155.55,311.1,622.2,1244.4,2488.8,4977.6},
{21.53,43.06,86.13,172.26,344.52,689.04,1378.08,2756.15,5512.3},
{21.87,43.75,87.5,174.99,349.99,699.97,1399.95,2799.9,5599.8},
{22.33,44.66,89.32,178.64,357.28,714.56,1429.12,2858.23,5716.46},
{25.52,51.04,102.08,204.16,408.32,816.64,1633.27,3266.55,6533.1},
{25.12,50.24,100.48,200.97,401.94,803.88,1607.76,3215.51,6431.02},
{28.71,57.42,114.84,229.68,459.36,918.72,1837.43,3674.87,7349.74},
{29.17,58.33,116.66,233.33,466.65,933.3,1866.6,3733.2,7466.4},
{29.77,59.55,119.09,238.19,476.37,952.74,1905.49,3810.97,7621.95},
{34.03,68.05,136.11,272.21,544.43,1088.85,2177.7,4355.4,8710.8},
{33.49,66.99,133.98,267.96,535.92,1071.84,2143.67,4287.35,8574.69},
{38.28,76.56,153.12,306.24,612.48,1224.96,2449.91,4899.82,9799.65}};

int chords[6][4] = {
{8,3,10,5},  
{9,4,11,6},
{0,7,2,0},
{8,9,0,0},
{3,4,7,3},
{10,11,12,10},
};


#define SINECOUNT 32
float FREQ[SINECOUNT];
float AMP[SINECOUNT] = { 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9,0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9 };
float FREQ_START[SINECOUNT];
//float FREQ_END[SINECOUNT] = {8.175, 10.3, 12.25, 16.35, 20.60, 24.5, 32.7, 41.2, 49,65.41, 82.41, 98, 130.81, 164.81, 196,261.63, 329.63,392,523.25, 659.25, 783.99,1046.50,1318.51,1567.98,2093,2637.02,3135.96,4186.01,5274.04,6271.93,8372.02,16744.04};
float FREQ_END[SINECOUNT] = {
  261.63, 329.63,392,523.25, 659.25, 783.99,1046.50,1318.51,1567.98,2093,2637.02,3135.96,4186.01,5274.04,6271.93,8372.02,16744.04,8.175, 10.3, 12.25, 16.35, 20.60, 24.5, 32.7, 41.2, 49, 65.41, 82.41, 98, 130.81, 164.81, 196 };
//float FREQ_END[SINECOUNT] = {8.175, 10.3, 12.25, 16.35, 20.60, 24.5, 32.7, 41.2, 49, 65.41, 82.41, 98, 130.81, 164.81, 196,261.63};

float increment = 0.01;
#define SMOOTHSTEP(x) ((x) * (x) * (3 - 2 * (x))) //SMOOTHSTEP expression.
Bounce resetSwitch = Bounce( RESET_BUTTON, 20 ); // Bounce setup for Reset


#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

// GUItool: begin automatically generated code
AudioSynthWaveformSine   sine3;          //xy=87,151
AudioSynthWaveformSine   sine4;          //xy=92,186
AudioSynthWaveformSine   sine1;          //xy=93,78
AudioSynthWaveformSine   sine2;          //xy=93,113
AudioSynthWaveformSine   sine5;          //xy=108,220
AudioSynthWaveformSine   sine13;         //xy=112,524
AudioSynthWaveformSine   sine9;          //xy=113,372
AudioSynthWaveformSine   sine6;          //xy=118,257
AudioSynthWaveformSine   sine7;          //xy=118,293
AudioSynthWaveformSine   sine10;         //xy=120,404
AudioSynthWaveformSine   sine20;         //xy=119,758
AudioSynthWaveformSine   sine17;         //xy=123,655
AudioSynthWaveformSine   sine21;         //xy=123,795
AudioSynthWaveformSine   sine12;         //xy=130,477
AudioSynthWaveformSine   sine18;         //xy=131,685
AudioSynthWaveformSine   sine11;         //xy=134,443
AudioSynthWaveformSine   sine19;         //xy=134,720
AudioSynthWaveformSine   sine22;         //xy=138,839
AudioSynthWaveformSine   sine14;         //xy=145,561
AudioSynthWaveformSine   sine8;          //xy=152,329
AudioSynthWaveformSine   sine23;         //xy=150,869
AudioSynthWaveformSine   sine16;         //xy=152,632
AudioSynthWaveformSine   sine24;         //xy=159,903
AudioSynthWaveformSine   sine15;         //xy=164,600
AudioSynthWaveformSine   sine25;         //xy=178,934
AudioSynthWaveformSine   sine26;         //xy=193,965
AudioSynthWaveformSine   sine28;         //xy=214,1033
AudioSynthWaveformSine   sine27;         //xy=216,998
AudioSynthWaveformSine   sine29;         //xy=240,1062
AudioSynthWaveformSine   sine30;         //xy=249,1095
AudioMixer4              mixer1;         //xy=259,125
AudioMixer4              mixer2;         //xy=259,189
AudioSynthWaveformSine   sine31;         //xy=266,1132
AudioSynthWaveformSine   sine32;         //xy=288,1171
AudioMixer4              mixer8;         //xy=306,712
AudioMixer4              mixer9;         //xy=314,810
AudioMixer4              mixer4;         //xy=320,418
AudioMixer4              mixer5;         //xy=325,508
AudioMixer4              mixer3;         //xy=406,92
AudioMixer4              mixer10;        //xy=408,979
AudioMixer4              mixer11;        //xy=449,1090
AudioMixer4              mixer6;         //xy=505,195
AudioOutputAnalog        dac1;           //xy=619,137
AudioMixer4              mixer7;         //xy=631,913
AudioConnection          patchCord1(sine3, 0, mixer1, 2);
AudioConnection          patchCord2(sine4, 0, mixer1, 3);
AudioConnection          patchCord3(sine1, 0, mixer1, 0);
AudioConnection          patchCord4(sine2, 0, mixer1, 1);
AudioConnection          patchCord5(sine5, 0, mixer2, 0);
AudioConnection          patchCord6(sine13, 0, mixer5, 0);
AudioConnection          patchCord7(sine9, 0, mixer4, 0);
AudioConnection          patchCord8(sine6, 0, mixer2, 1);
AudioConnection          patchCord9(sine7, 0, mixer2, 2);
AudioConnection          patchCord10(sine10, 0, mixer4, 1);
AudioConnection          patchCord11(sine20, 0, mixer8, 3);
AudioConnection          patchCord12(sine17, 0, mixer8, 0);
AudioConnection          patchCord13(sine21, 0, mixer9, 0);
AudioConnection          patchCord14(sine12, 0, mixer4, 3);
AudioConnection          patchCord15(sine18, 0, mixer8, 1);
AudioConnection          patchCord16(sine11, 0, mixer4, 2);
AudioConnection          patchCord17(sine19, 0, mixer8, 2);
AudioConnection          patchCord18(sine22, 0, mixer9, 1);
AudioConnection          patchCord19(sine14, 0, mixer5, 1);
AudioConnection          patchCord20(sine8, 0, mixer2, 3);
AudioConnection          patchCord21(sine23, 0, mixer9, 2);
AudioConnection          patchCord22(sine16, 0, mixer5, 3);
AudioConnection          patchCord23(sine24, 0, mixer9, 3);
AudioConnection          patchCord24(sine15, 0, mixer5, 2);
AudioConnection          patchCord25(sine25, 0, mixer10, 0);
AudioConnection          patchCord26(sine26, 0, mixer10, 1);
AudioConnection          patchCord27(sine28, 0, mixer10, 3);
AudioConnection          patchCord28(sine27, 0, mixer10, 2);
AudioConnection          patchCord29(sine29, 0, mixer11, 0);
AudioConnection          patchCord30(sine30, 0, mixer11, 1);
AudioConnection          patchCord31(mixer1, 0, mixer3, 0);
AudioConnection          patchCord32(mixer2, 0, mixer3, 1);
AudioConnection          patchCord33(sine31, 0, mixer11, 2);
AudioConnection          patchCord34(sine32, 0, mixer11, 3);
AudioConnection          patchCord35(mixer8, 0, mixer7, 0);
AudioConnection          patchCord36(mixer9, 0, mixer7, 1);
AudioConnection          patchCord37(mixer4, 0, mixer3, 2);
AudioConnection          patchCord38(mixer5, 0, mixer3, 3);
AudioConnection          patchCord39(mixer3, 0, mixer6, 0);
AudioConnection          patchCord40(mixer10, 0, mixer7, 2);
AudioConnection          patchCord41(mixer11, 0, mixer7, 3);
AudioConnection          patchCord42(mixer6, dac1);
AudioConnection          patchCord43(mixer7, 0, mixer6, 1);
// GUItool: end automatically generated code


void setup(){
    pinMode(BANK_BUTTON,INPUT);
  pinMode(RESET_BUTTON, INPUT);
  pinMode(RESET_CV, INPUT);
  pinMode(RESET_LED, OUTPUT);
  pinMode(LED0,OUTPUT);
  pinMode(LED1,OUTPUT);
  pinMode(LED2,OUTPUT);
  pinMode(LED3,OUTPUT);
  AudioMemory(50);
  Serial.begin(9600);

  mixer1.gain(0,0.25);
  mixer1.gain(1,0.25);
  mixer1.gain(2,0.25);
  mixer1.gain(3,0.25);
  mixer2.gain(0,0.25);
  mixer2.gain(1,0.25);
  mixer2.gain(2,0.25);
  mixer2.gain(3,0.25);

  mixer3.gain(0,0.25);
  mixer3.gain(1,0.25);
  mixer3.gain(2,0.25);
  mixer3.gain(3,0.25);
  mixer4.gain(0,0.25);
  mixer4.gain(1,0.25);
  mixer4.gain(2,0.25);
  mixer4.gain(3,0.25);

  mixer5.gain(0,0.25);
  mixer5.gain(1,0.25);
  mixer5.gain(2,0.25);
  mixer5.gain(3,0.25);
  mixer6.gain(0,.9);
  mixer6.gain(1,0.9);
  mixer6.gain(2,0);
  mixer6.gain(3,0);

  mixer7.gain(0,0.25);
  mixer7.gain(1,0.25);
  mixer7.gain(2,0.25);
  mixer7.gain(3,0.25);
  mixer8.gain(0,0.25);
  mixer8.gain(1,0.25);
  mixer8.gain(2,0.25);
  mixer8.gain(3,0.25);

  mixer9.gain(0,0.25);
  mixer9.gain(1,0.25);
  mixer9.gain(2,0.25);
  mixer9.gain(3,0.25);
  mixer10.gain(0,0.25);
  mixer10.gain(1,0.25);
  mixer10.gain(2,0.25);
  mixer10.gain(3,0.25);

  mixer11.gain(0,0.25);
  mixer11.gain(1,0.25);
  mixer11.gain(2,0.25);
  mixer11.gain(3,0.25);


  randomise();
}


float N = 100000.0; // NUMBER OF STEPS 
int count  = 0; 
float v; 
float x; 


int chordPick;
boolean  RESET_CHANGED;
void loop(){
  Serial.println(AudioMemoryUsageMax());
  while (count < N) {  // loop until we've reached number of steps  

    for (int i = 0; i < SINECOUNT; i++){  // go through all  voices 
      v = count / N; 
      v = SMOOTHSTEP(v); 
      x = (FREQ_END[i] * v) + (FREQ_START[i] * (1 - v)); 
      FREQ[i] = x; 
    };
    updateSines();
    count++;
  }  

   if ( resetSwitch.update() ) {
    RESET_CHANGED = resetSwitch.read();
  }// do nothing  

while (!RESET_CHANGED){
   if ( resetSwitch.update() ) {
    RESET_CHANGED = resetSwitch.read();
  }// do nothing  
}//  randomise();


endToStart();
  chordEND2(chordPick);
  count = 0;
  chordPick++;
  if (chordPick > 5) chordPick = 0;

}



void updateSines(){
  AudioNoInterrupts();  

  sine1.frequency(FREQ[0]);
  sine2.frequency(FREQ[1]);
  sine3.frequency(FREQ[2]);
  sine4.frequency(FREQ[3]);
  sine5.frequency(FREQ[4]);
  sine6.frequency(FREQ[5]);
  sine7.frequency(FREQ[6]);
  sine8.frequency(FREQ[7]);
  sine9.frequency(FREQ[8]);
  sine10.frequency(FREQ[9]);
  sine11.frequency(FREQ[10]);
  sine12.frequency(FREQ[11]);
  sine13.frequency(FREQ[12]);
  sine14.frequency(FREQ[13]);
  sine15.frequency(FREQ[14]);
  sine16.frequency(FREQ[15]);

  sine17.frequency(FREQ[16]);
  sine18.frequency(FREQ[17]);
  sine19.frequency(FREQ[18]);
  sine20.frequency(FREQ[19]);
  sine21.frequency(FREQ[20]);
  sine22.frequency(FREQ[21]);
  sine23.frequency(FREQ[22]);
  sine24.frequency(FREQ[23]);
  sine25.frequency(FREQ[24]);
  sine26.frequency(FREQ[25]);
  sine27.frequency(FREQ[26]);
  sine28.frequency(FREQ[27]);
  sine29.frequency(FREQ[28]);
  sine30.frequency(FREQ[29]);
  sine31.frequency(FREQ[30]);
  sine32.frequency(FREQ[31]);



  sine1.amplitude(AMP[0]);
  sine2.amplitude(AMP[1]);
  sine3.amplitude(AMP[2]);
  sine4.amplitude(AMP[3]);
  sine5.amplitude(AMP[4]);
  sine6.amplitude(AMP[5]);
  sine7.amplitude(AMP[6]);
  sine8.amplitude(AMP[7]);
  sine9.amplitude(AMP[8]);
  sine10.amplitude(AMP[9]);
  sine11.amplitude(AMP[10]);
  sine12.amplitude(AMP[11]);
  sine13.amplitude(AMP[12]);
  sine14.amplitude(AMP[13]);
  sine15.amplitude(AMP[14]);
  sine16.amplitude(AMP[15]);

  sine17.amplitude(AMP[16]);
  sine18.amplitude(AMP[17]);
  sine19.amplitude(AMP[18]);
  sine20.amplitude(AMP[19]);
  sine21.amplitude(AMP[20]);
  sine22.amplitude(AMP[21]);
  sine23.amplitude(AMP[22]);
  sine24.amplitude(AMP[23]);
  sine25.amplitude(AMP[24]);
  sine26.amplitude(AMP[25]);
  sine27.amplitude(AMP[26]);
  sine28.amplitude(AMP[27]);
  sine29.amplitude(AMP[28]);
  sine30.amplitude(AMP[29]);
  sine31.amplitude(AMP[30]);
  sine32.amplitude(AMP[31]);



  AudioInterrupts();

}


void silent(){
  for (int i = 0; i < SINECOUNT; i++){
AMP[i] = 0;  
}
}

void randomise(){
  for (int i = 0; i < SINECOUNT; i++){
    FREQ_START[i] = random(10000);
    FREQ [i] = FREQ_START[i];
    Serial.print("Start=");
    Serial.print(FREQ_START[i]);
    Serial.print(" | End = ");
    Serial.println(FREQ_END[i]);
  }
  N=random(100000);
}

void randomiseEND(){
  for (int i = 0; i < SINECOUNT; i++){
    FREQ_END[i] = random(10000);
    FREQ [i] = FREQ_START[i];
  }
}






void chordEND(){

  int scale[4];
 scale[0] = 0;
 scale[1] = random(11);
 scale[2] = random(11);
 scale[3] = random(11);
  int scalecount = 3;
  int j = 0; 
  int octave = 0;
  int startnote = 0;
  for (int i = 0; i < SINECOUNT; i++){
    
    FREQ_END[i] = numToFreq(21+ startnote +scale[j]+(octave * 12));
    j++;
    if ( j >= scalecount){
     j = 0;
    octave++; 
    }
  }
}

void chordEND2(int chooseChord){
int scalecount = 4;
  int j = 0; 
  int octave = 0;
  int startnote = 0;
  for (int i = 0; i < SINECOUNT; i++){
    
    FREQ_END[i] = scale[chords[chooseChord][j]][octave] ;
    j++;
    if ( j >= scalecount){
     j = 0;
    octave++; 
    }
  }
}

// scale[notes from Eb][octaves from Eb0 to Eb8] 
// Fifths: 8-3-10-5 / 9-4-11-6-1 / 0-7-2 
// Minor 7ths: 8-9-0 / 3-4-7 / 10-11-2 / 5-6


float numToFreq(int input) {
  int number = input - 21; // set to midi note numbers = start with 21 at A0 
  number = number - 48; // A0 is 48 steps below A4 = 440hz
  float result; 
  result = 440*(pow (1.059463094359,number));
  return result;   
}

void endToStart(){
    for (int i = 0; i < SINECOUNT; i++){
    FREQ_START[i] = FREQ[i];
    }
}

// WRITE A 4 DIGIT BINARY NUMBER TO LED0-LED3 
void ledWrite(int n){
  digitalWrite(LED0, HIGH && (n & B00001000));
  digitalWrite(LED1, HIGH && (n & B00000100));
  digitalWrite(LED2, HIGH && (n & B00000010));
  digitalWrite(LED3, HIGH && (n & B00000001)); 
}

