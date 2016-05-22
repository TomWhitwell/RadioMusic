##Using the edited Teensy files to compile your own firmware  

RMPlaySDRaw is based on Teensy play_sd_raw, but with additions specific for RadioMusic.
MovingAverage is a very simple implementation of a moving average.

These directories should be copied to the Arduino user library folder (on Mac: ~/Documents/Arduino/libraries)

1. Adds a new function: playFrom(filename, startPoint) which starts the file at a particular point.  It actually starts at the modulo of the start point vs the file size, so if you start at a point past the end of the file, it will start within the file, offset by the remainder. If the file is same as the previous one, a plain seek is performed instead of close/open  ([More info](*https://github.com/TomWhitwell/RadioMusic/wiki/Troubleshooting-the-Radio-Music-module#what-does-the-reset-button-do))
1. Adds a section to AudioPlaySdRaw::update that checks whether the file is playing normally (i.e. that it has read 256 bytes to play). If not, it calls hotswap callback. This enables hot swapping by allowing the callback to reboot the device.. Normally, pulling out the SD card causes the playback to get stuck in a loop. 
1. Adds a new variable: fileOffset which returns the current position of the playing file in bytes. This enables the radio-style switching between playing files.  
1. Adds a new function: preparePlayFrom(filename) which closes the previous running file and opens the new one but does not start playing it. 
1. Adds a new function: pause() which disables playing but leaves the file open.

###play_sd_raw.h  

1. Just adds new functions and variables: playFrom, fileOffset, hotswap_cb,  

