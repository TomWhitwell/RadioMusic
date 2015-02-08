##Using the edited Teensy files to compile your own firmware  

These files are based on the standard Arduino and Teensyduino libraries, but contain modified code required to make Radio Music work.   
I suspect there's a better way to do this, and would welcome any suggestions!  

play_sd_raw.cpp and play_sd_raw.h files go in the Teensyduino Audio library.  
SD.cpp goes inside the Arduino package itself (at least on a Mac)  

The changes to these files are not great:  

###play_sd_raw.cpp 

1. Adds a new function: playFrom(filename, startPoint) which starts the file at a particular point.  It actually starts at the modulo of the start point vs the file size, so if you start at a point past the end of the file, it will start within the file, offset by the remainder.  ([More info](*https://github.com/TomWhitwell/RadioMusic/wiki/Troubleshooting-the-Radio-Music-module#what-does-the-reset-button-do))
1. Adds a section to AudioPlaySdRaw::update that checks whether the file is playing normally (i.e. that it has read 256 bytes to play). If not, it sets a new variable 'failed' to true. This enables hot swapping. Normally, pulling out the SD card causes the playback to get stuck in a loop. 
1. Adds a new variable: fileOffset which returns the current position of the playing file in bytes. This enables the radio-style switching between playing files.  

###play_sd_raw.h  

1. Just adds new functions and variables: playFrom, fileOffset, failed 

###SD.cpp 

1. Adds if (root.isOpen()) root.close(); which enables repeated calls to start the SD library, makes hot-swapping possible.  


  

 