##Radio Music

[![Join the chat at https://gitter.im/TomWhitwell/RadioMusic](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/TomWhitwell/RadioMusic?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

### A virtual radio module for Eurorack 
![Image from matcsat, Muffwiggler forum](https://raw.githubusercontent.com/TomWhitwell/RadioMusic/master/Collateral/BuildImages/rmpic.jpg)

- [Full Documentation](https://github.com/TomWhitwell/RadioMusic/wiki)
    - [Module manual](https://github.com/TomWhitwell/RadioMusic/wiki/How-to-use-the-Radio-Music-module)  
    - [Build documents](https://github.com/TomWhitwell/RadioMusic/wiki/Building-the-Radio-Music-module)  
    - [Setting up the SD Card](https://github.com/TomWhitwell/RadioMusic/wiki/SD-Card%3A-Format-%26-File-Structure)  
    - [Suggested sound library](https://github.com/TomWhitwell/RadioMusic/wiki/Audio-for-the-SD-Card)  
- [Hardware: Eagle CAD files](https://github.com/TomWhitwell/RadioMusic/tree/master/RadioMusicHardware)
- [Hardware: Parts List](https://github.com/TomWhitwell/RadioMusic/wiki/BOM---Parts-List)
- [Firmware: Teensyduino files](https://github.com/TomWhitwell/RadioMusic/tree/master/RadioMusic)
- [Report Issues with hardware or firmware](https://github.com/TomWhitwell/RadioMusic/issues)

This is not a radio. It is a sample player that behaves like a radio.  

A digital way to simulate radio-powered compositions by John Cage, Karlheinz Stockhausen and Don Buchla's voltage-controlled radio experiments. It uses a teensy3.1 to play files from a SD card to simulate a voltage controlled AM/FM/Shortwave/Time Travel radio. 

####Progress  

March 2015  
- Updated firmware Beta produced by Jouni Stenroos, fixing several long-term issues including file count limit, hot swapping and resets.  

February 2015  
- [Thonk kits shipping](http://www.thonk.co.uk/product-category/music-thing-modular/)  
- Module being used by:  
    - [Robin Rimbaud (Scanner)](https://twitter.com/robinrimbaud/status/562663428830289921)   
    - [Chris Carter](https://twitter.com/chris_carter_/status/562889299621076993)  
    - [Russell Haswell](https://twitter.com/russellhaswell/status/552213743363690496)  
    - [Richard Devine](https://instagram.com/p/900BnMjQjw/)  
- [Big collection of video and audio demos](http://musicthing.co.uk/modular/?p=1087)  

January 2015  
- Rev2 PCB Files confirmed good  

December 2014  
- [Rev2 PCB files](https://github.com/TomWhitwell/RadioMusic/tree/master/RadioMusicHardware/Gerbers/Rev%202%20Gerbers%20Dec%202014) published, prototyping in process  

November 2014  
- First PCB build   
- [Video 1](http://instagram.com/p/vCNc37DmSj/)  
- [Video 2](http://instagram.com/p/vnv1T-DmUm/)  
- [Longer demo video](http://vimeo.com/113050279)  
- [Audio: Long demo](https://soundcloud.com/musicthing/radio-music-20-minutes-of-madness)  
- [Audio: Random Drums](https://soundcloud.com/musicthing/radio-music-random-drums)  
- [Audio: Random Voices](https://soundcloud.com/musicthing/radio-music-random-voices)   

October 2014   
- Very early prototyping     


####Features
- 4hp x 40mm deep  
- On-board 5v regulator, does not need a 5v rail. 
- 75ma Current Draw   
    - 60ma from 5v regulator  
    - 8ma from -12v  
- Up to 32gb storage, 16 banks of 75 channels each  
- Files stored as headerless .raw wav files 
- Customise the module by editing settings.txt on the SD Card 

![EarlyPanelDesign](https://raw.githubusercontent.com/TomWhitwell/RadioMusic/master/Collateral/img.png)

####License  
Radio Music is Open Hardware  
All hardware and software design in this project is Creative Commons licensed by Tom Whitwell: [CC-BY-SA: Attribution / ShareAlike](https://creativecommons.org/licenses/by-sa/3.0/)  
If you use any work in this project, you should credit me, and you must republish any changes you make under the same license.   
This license does permit commercial use of these designs, but consider getting in touch before selling anything.  
This project includes work from the [Teensy](https://www.pjrc.com/teensy/) project, which is not covered by this license. The suggested audio content is not covered by this license.  
