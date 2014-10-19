To Do: 

HARDWARE 

- Separate reset button and CV input 
- Remove Bank switch  

- Optimise pot range 
-- Try schottky diodes 
-- Consider trimmer 
-- Consider attenuator


- Experiment with +5v or +3.3v voltage regulators 
-- Measure current draw 

- Issues: 
-- Negative input current creates small positive reading on CV inputs 
-- Check range of useful travel on pots 


SOFTWARE 


Save settings: 
- Retain Bank after powerup 
- Retain playhead position after powerup 

- Work on time pot behaviour - very random at the moment 

- ISSUES: 
-- Check limits - get a burst of white noise on high voltage into channel pot 
-- Continue work on between-station jittering 

BACKLOG: 
- Support Wav Files 
- Pitch changing / reverse play 

--------------

HARDWARE - DONE 

- Add 2nd pot & CV DONE 
- Add bank switching DONE
- Add reset input DONE 
- Research SD cards 
-- through hole 
-- vertical 
- Bank switching 
-- Decide on # of banks available DONE 4 LEDs = up to 16 banks
-- Consider pot & CV on separate ADC channels DONE
- Add LED for resets 



SOFTWARE - DONE
-- Files don't loop RESOLVED 
-- as station knob is moved, repeatedly calls the same file, causing clicking RESOLVED 
-- Many of white noise bursts in time pot - need to limit everything to 512 byte blocks? / RESOLVED - caused by odd numbers 
- Mute clicks on file changes DONE 
- Add time knob/CV DONE 
- Add Reset input DONE 
- Single button Bank Switching 
