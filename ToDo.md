To Do: 

HARDWARE 

- Add 2nd pot & CV
- Add bank switching DONE
- Add reset input 

- Optimise pot range 
-- Try schottky diodes 
-- Consider trimmer 
-- Consider attenuator
-- Consider pot & CV on separate ADC channels DONE

- Experiment with +5v or +3.3v voltage regulators 
-- Measure current draw 

- Research SD cards 
-- through hole 
-- vertical 

- Bank switching 
-- Decide on # of banks available DONE 4 LEDs = up to 16 banks 

- Issues: 
-- Negative input current creates small positive reading on CV inputs 
-- Check range of useful travel on pots 

SOFTWARE 

- Bank Switching DONE
- Add time knob/CV 
- Add Reset input 

- Add eternal play with eeprom writes UNCLEAR HOW 

- Mute clicks on file changes 
- Support Wav Files 

- ISSUES: 
-- Many of white noise bursts in time pot - need to limit everything to 512 byte blocks? 
-- Continue work on between-station jittering 
-- Files don't loop RESOLVED 
-- as station knob is moved, repeatedly calls the same file, causing clicking RESOLVED 

