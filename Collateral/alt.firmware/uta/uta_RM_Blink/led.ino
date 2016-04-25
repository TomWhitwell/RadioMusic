/* helper functions for led */
/* blink functions */

#define NR_LEDS 5
#define LED_ON	true
#define LED_OFF	false

byte led_bitmap;
elapsedMillis ledx_cnt[NR_LEDS];
unsigned int ledx_millis[NR_LEDS];
byte ledx_brightness[NR_LEDS];
elapsedMillis leds_cnt;
unsigned int leds_millis;
int leds_brightness;
int nr_leds;

byte led_arr[NR_LEDS];

void init_led(){
	led_bitmap = 0x0;
	led_arr[0] = LED0;
	led_arr[1] = LED1;
	led_arr[2] = LED2;
	led_arr[3] = LED3;
	led_arr[4] = RESET_LED;
}

void set_leds_bin_num(int num){
	if(num & 0x8) analogWrite(LED0, 255);
	if(num & 0x4) analogWrite(LED1, 255);
	if(num & 0x2) analogWrite(LED2, 255);
	if(num & 0x1) analogWrite(LED3, 255);
}

void toogle_led_x(int led_x){
	led_bitmap ^= 1 << led_x;
//	Serial.println(led_bitmap);
//	if((led_bitmap >> led_x) & 0x1) digitalWrite(led_arr[led_x], HIGH);
//	else digitalWrite(led_arr[led_x], LOW); 
	if((led_bitmap >> led_x) & 0x1) analogWrite(led_arr[led_x], ledx_brightness[led_x]);
	else analogWrite(led_arr[led_x], 0); 
}

void toogle_leds(){
	for(int i=0; i<nr_leds; i++){
		toogle_led_x(i);
	}	
}

void set_led_x(int led_x, boolean state){
	if(state == LED_ON){
		led_bitmap |= (1 << led_x);
		analogWrite(led_arr[led_x], ledx_brightness[led_x]);
	} else {
		led_bitmap &= ~(1 << led_x);
		analogWrite(led_arr[led_x], 0);
	}
//	led_bitmap ^= (-state ^ led_bitmap) & (1 << led_x);
//	if((led_bitmap >> led_x) & 0x1) digitalWrite(led_arr[led_x], HIGH);
//	else digitalWrite(led_arr[led_x], LOW);
}

void set_leds(boolean state){
	for(int i=0; i<nr_leds; i++){
		set_led_x(i, state);
	}
}

void start_blink_led_x(int led_x, unsigned int blink_millis, boolean state, byte brightness){
	ledx_cnt[led_x] = 0;
	ledx_millis[led_x] = blink_millis;
	ledx_brightness[led_x] = brightness;
	set_led_x(led_x, state);
}

void start_blink_leds(unsigned int blink_millis, byte bitmap, int nr, byte brightness){
	leds_cnt = 0;
	leds_millis = blink_millis;
	nr_leds = nr;
	leds_brightness = brightness;
	for(int i=0; i<nr_leds; i++){
		ledx_brightness[i] = brightness;
		set_led_x(i, (boolean) ((bitmap>>i) & 0x1));
	}
}

boolean upd_led_x(int led_x){
	if(ledx_cnt[led_x] >= ledx_millis[led_x]){
		toogle_led_x(led_x);
		return true;
	}
	return false;
}

boolean upd_leds(){
	if(leds_cnt >= leds_millis){
		toogle_leds();
		return true;
	}
	return false;
}

void reset_blinking_led_x(int led_x, unsigned int blink_millis, byte brightness){
	ledx_cnt[led_x] = 0;
	ledx_millis[led_x] = blink_millis;
	ledx_brightness[led_x] = brightness;	
}

void reset_blinking_leds(unsigned int blink_millis, byte brightness){
	leds_cnt = 0;
	leds_millis = blink_millis;
	leds_brightness = brightness;	
}


