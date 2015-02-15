##Module Hardware Test file  

[![](https://i.vimeocdn.com/video/503708029.webp?mw=600&q=70)](https://vimeo.com/117123053)  
[CLICK TO WATCH THE VIDEO](https://vimeo.com/117123053)  

ModuleTest.ino is a quick way to check that the hardware in your module is working correctly.  

If you cut the power trace, you'll need both +/-12v ribbon cable power and USB power to upload firmware to the module.  

###Method 1: Basic  
Install the file moduletest.cpp.hex using Teensy Loader in the same way you installed the original firmware ([as explained here](https://github.com/TomWhitwell/RadioMusic/wiki/Preparing-the-Teensy-3.1)) 
You can see lights flashing on the module, it will produce sounds, those sounds will moduluate as you turn the knobs and apply CV. Knobs and CV are also mapped to the 4 LEDs at the top.  

###Method 2: Advanced 
To get feedback from the module and see what's going on, you'll need to install the full [Teensyduino environment](http://www.pjrc.com/teensy/td_download.html). Once that is installed and working (i.e. you can install Blink and see expected results) download and run the [Module Test Sketch](https://github.com/TomWhitwell/RadioMusic/tree/master/Collateral/ModuleTest).  
- You'll see all five LEDs flash in sequence   
- While they're flashing, the module will generate audio - various sine wave tones    
- Once the startup routine is finished, the 4 LEDs are linked to 2xknobs and 2xCV inputs  
- If you run the serial monitor in Arduino, you'll see the knob / CV input values as 0-1024. 
    - Expect to see some jitter from noise - maybe jumping up and down 10 points (these numbers are smoothed by the firmware later on), but you should be able to move each pot from 0 - 1024 smoothly.  

##Bonus  
- sinesfun.ino is a ludicrous 32-channel sinewave drone machine with tunings based on La Monte Young's well tuned piano. Just press the reset button to move to another chord. 

##Bonus 2 
- stereodelay.hex and stereodelay.ino turn the Radio Music module into a STUPID stereo delay module. With many caveats. 
    - [Audio demo](https://soundcloud.com/musicthing/radiomusicstereodelay)  
    - Input: The module expects a small (1.25v p-to-p) input signal that is floating at around 2.5v. That is very different from a modular signal, which is bipolar, 10v p-to-p. So, to get a clean (ish) signal, I used the two centre channels on a Maths to add a little offset and attenuate the signal a lot. 
    - Output: To get stereo output, I'm using the Reset Input as an output. I'm also using the Teensy Audio library's PWM output module, which sends signals to pins 3 and 4. So I've run a jumper on the back of the teensy from Pin 5 (ie digital pin 3) to pin 11 (ie Digital pin 9). This may be a bad idea and might blow up your module. Mine is OK so far. LEDs 1 and 2 are on pins 3 & 4, so they flash all the time. 
    - Output levels: The main output via the DAC is much higher than the PWM output, so you'll need to play with the mixer a bit. 
    - Control. This is where it gets really stupid. The Teensy Audio input stream library uses the on-board ADC full time, so you can't use that ADC to read pots. So there are no controls on this delay module. Instead, delay time and feedback change randomly and independently across the two channels. The Teensy 3.1 does have a second ADC, so willbe a way around this. 

