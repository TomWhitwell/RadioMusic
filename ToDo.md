To Do: 

HARDWARE 

- Separate reset button and CV input 
- Remove Bank switch DONE 
- Add LED for resets 

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

- Add 2nd pot & CV DONE 
- Add bank switching DONE
- Add reset input DONE 


SOFTWARE 

- Bank Switching DONE
- Add time knob/CV DONE 
- Add Reset input DONE 
- Add eternal play with eeprom writes TRY AGAIN 

- Mute clicks on file changes 
- Support Wav Files 

- ISSUES: 
-- Many of white noise bursts in time pot - need to limit everything to 512 byte blocks? / RESOLVED - caused by odd numbers 
-- Continue work on between-station jittering 
-- Files don't loop RESOLVED 
-- as station knob is moved, repeatedly calls the same file, causing clicking RESOLVED 

