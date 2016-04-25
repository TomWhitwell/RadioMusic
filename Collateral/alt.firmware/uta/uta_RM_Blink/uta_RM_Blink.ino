#include <EEPROM.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Bounce.h>
#include "config.h"

#define RESTART_ADDR       0xE000ED0C
#define WRITE_RESTART(val) ((*(volatile uint32_t *)RESTART_ADDR) = (val))

#define DEBUG	1
#if DEBUG == 1
		#define dbg_println(str)	Serial.println(str)
		#define dbg_print(str)		Serial.print(str)
#else
		#define dbg_println(str)
		#define dbg_print(str)	
#endif

// In the Radio Music Hardware,  Bank LEDs are 3,4,5,6, Reset LED is 11. 
//#define LED1						3
//#define LED2						4
//#define LED3						5
//#define LED4						6
#define RESET_LED					11
#define RESET_BUTTON 				8 // Bank Button 
#define SELECT_POT					A7
#define UTA_RESET_MODE_MILLIS		5000  // 5sec
#define UTA_RESET_MODE				1
#define UTA_NORMAL_MODE				0
#define RESET_BLINK_MILLIS			100
#define SD_BUF_SIZE					512

// global variable
boolean btn_state;
String firmware;
String loaded_firmware = "software.bin";

int get_firmware_number(){
	return map(analogRead(SELECT_POT),0,1023,0,15);
}
void upd_reset_mode_interface(){
	int val = get_firmware_number();
	set_leds_bin_num(val);
}
void uta_reboot(){
	delay(500);
	WRITE_RESTART(0x5FA0004);
	delay(500);
}

//void upd_current_firmware_file(String str){
//	// delete old file
//	SD.remove("firmware/current.txt");
//	File file = SD.open("firmware/current.txt", FILE_WRITE);
//	file.print(str);
//	file.close();
//}

void select_new_firmware(){
	int num = get_firmware_number();
	File to;
	File from;
	int cnt = 0;
	byte sd_buf[SD_BUF_SIZE];

	app_stop();	
	
	/* verify existence of firmware */
	String selected = String("firmware/");
	selected += num;
	selected += ".bin";
	
	char tmp[50];
	selected.toCharArray(tmp,selected.length()+1);
	
	if(SD.exists(tmp)){
		/* delete current loaded firware */
		loaded_firmware.toCharArray(tmp,loaded_firmware.length()+1);
		SD.remove(tmp);
		/* create new file */
		to = SD.open(tmp, FILE_WRITE);
		/* copy the selected firmware to the next firmwre to load */
		selected.toCharArray(tmp,selected.length()+1);
		from = SD.open(tmp);
		if(from){
			/* init led as copy feedback: blink everything half sec */
			start_blink_leds(500,0b11111,5,200);
			while(from.available()){
//					b = from.read();
//					to.write(b);

				sd_buf[cnt] = from.read();
				cnt++;
				
//					/* make the leds blinks */
				if(upd_leds) reset_blinking_leds(500,200); 
				
				if((cnt % 40) == 0) dbg_println(".");
				else dbg_print(".");
				
				if(cnt == SD_BUF_SIZE){
					to.write(sd_buf,cnt);
					cnt = 0;
				}					
			}
			if(cnt > 0) to.write(sd_buf,cnt);
			from.close();
		}
		to.close();
	}
}

Bounce uta_resetSwitch = Bounce( RESET_BUTTON, 20 ); // Bounce setup for Reset
elapsedMillis uta_state_cnt;
elapsedMillis uta_led_cnt;
byte uta_mode;
byte reset_btn_state;

void setup(){
// initialize the digital pin as an output.
	pinMode(LED0, OUTPUT);
	pinMode(LED1, OUTPUT);
	pinMode(LED2, OUTPUT);
	pinMode(LED3, OUTPUT);
    pinMode(RESET_LED, OUTPUT);

	pinMode(RESET_BUTTON, INPUT);
	Serial.begin(9600);
	Serial.println("start setup");
	

	
	SPI.setMOSI(7);
	SPI.setSCK(14);

// 	OPEN SD CARD 
	int crashCountdown;
	if (!(SD.begin(10))){
		while (!(SD.begin(10))){
			delay(100);
			crashCountdown++;
			if (crashCountdown > 6) uta_reboot();
		}
	}
	init_led();

//  call specific setup
	app_setup();
}

void loop(){
	// transition from released to pushed or pushed to release
	if (uta_resetSwitch.update()){
		btn_state = uta_resetSwitch.read();
		// 0 => released
		// 1 => pushed
		if(btn_state){
			if(uta_mode == UTA_RESET_MODE){
				/* take new firmware and reboot */
				select_new_firmware();
				uta_reboot();
			}
			uta_state_cnt = 0;
			uta_mode = UTA_NORMAL_MODE;
		} 
		else {
			/* release the button */
			/* nothing to do? */
		}
	} else {
		if(btn_state){
			if( (uta_mode == UTA_NORMAL_MODE) && (uta_state_cnt >= UTA_RESET_MODE_MILLIS) ){
				uta_mode = UTA_RESET_MODE;
				// entering reset mode, start leds blinking
				start_blink_leds(RESET_BLINK_MILLIS, 0b1111, 4, 60);
				
			} else if(uta_mode == UTA_RESET_MODE) {
				// we are in reset mode, just update leds
				if(upd_leds()) reset_blinking_leds(RESET_BLINK_MILLIS, 60);
				
			} else {
				uta_mode = UTA_NORMAL_MODE;
				/* nothing to do? */
			}
		} 
		else {
			if(uta_mode == UTA_RESET_MODE){
				// we are in reset mode, just update leds
				if(upd_leds()) reset_blinking_leds(RESET_BLINK_MILLIS, 60);
			} else {
				// we are in normal mode
				set_leds(0);
			}
		}
	}
	
	// update interface: in RESET mode, SELECT_POT indicates next firmware to load
	if(uta_mode == UTA_RESET_MODE){
		upd_reset_mode_interface();
	} else {
		/* do something */
		app_loop();
	}
}