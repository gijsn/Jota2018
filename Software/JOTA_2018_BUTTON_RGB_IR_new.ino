/* Jota 2018 Code STAR WARS (c)GN

    Pin table:

*/

//included libraries
#include <FastLED.h>
#include <IRremote.h>
#include <EEPROM.h>


//PIN defines
#define A  7
#define B  6
#define C  4
#define D  3
#define E  2
#define F  9
#define G  10
#define DOT 5
#define BUTTON 0

#define RGB_PIN 8

#define IR_PORT PINA
#define IR_PIN 1


//RGB LED SECTION
#define NUM_LEDS 1
#define BRIGHTNESS 64
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

//IR REC SECTION
IRrecv irrecv(IR_PIN);
decode_results results;
int ir_lives = 9;
/*
  #define MAXPULSE 65000
  #define RESOLUTION 20
  const uint8_t max_pulses = 16;
  uint16_t pulses[max_pulses][2];
  uint8_t currentpulse = 0;
*/

CRGB leds[NUM_LEDS];

//  MENU SECTION \\
//mode defines
#define DICE 0
#define COMM 1 //communication mode
#define LASER 2 //laser mode
#define COUNTING 3 //counting mode
#define RGBLED 4 //
#define MENU_SIZE 5

//button press defines
#define NOPRESS 0
#define SHORTPRESS 1
#define LONGPRESS 2
#define LONGPRESSDELAY 2000

//LASER defines
#define DEAD 0
#define LIFE 1
#define POWER 2

//BUTTON PRESS GLOBALS

long button_press_start = 0;
bool button_counting = false;
bool button_lastpress_long = false;

//7SEG GLOBALS
byte toDisplay = B11111111;
long flash_interval = 500;
long previous_flash_millis = 0;
bool on = true;
byte numbers[] =     {B00000011, B10011111, B00100101, B00001101, B10011001, B01001001, B01000001, B00011111, B00000001, B00001001, B11111111};
//Sign explanation        0         1          2          3          4          5          6          7           8         9      (10)Clear


//MENU GLOBALS
byte menu[] = {DICE, COMM, LASER, COUNTING, RGBLED};
byte menu_disp[] = {B00010000, B11000000, B01100010, B10000100, B01100000};
//Sign explanation:    A           b           C         d         E
uint8_t current_menu_index = 0;
bool menu_mode = false;
int address = 0;



//MENU ITEM GLOBALS
//Communication mode
int comm_number = 0;

//Laser mode
byte life = 0;

//communication mode
byte inverse[] =     {B01001000, B11101101, B10011000, B10001001, B00101101, B00001011, B00001010, B11101001, B00001000, B00001001, B11111111};
//Numbers but with inverse signs

//Counting mode
bool counting_paused = false;
long previous_counting_millis = 0;
long counting_interval = 1000;
int counting_index = 0;

//Rgb led mode
bool rgb_led_paused = false;
int rgb_led_counter = 0;



void setup() {
  // put your setup code here, to run once:
  delay(1000);
  FastLED.addLeds<LED_TYPE, RGB_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  irrecv.enableIRIn();
  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(C, OUTPUT);
  pinMode(D, OUTPUT);
  pinMode(E, OUTPUT);
  pinMode(F, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(DOT, OUTPUT);
  toDisplay = numbers[0];
  disp7Seg(toDisplay);
  delay(1000);
  leds[0] = CRGB::Black;
  FastLED.show();
  uint8_t tmp_menu_item = EEPROM.read(address);
  if (tmp_menu_item < MENU_SIZE) {
    current_menu_index = tmp_menu_item;
    disp7Seg(menu_disp[current_menu_index]);
    delay(1000);
    shortpress_functions();
  }

}

int checkButtonPress() {
  if (digitalRead(BUTTON) == LOW) {
    //button is pressed, now start checking for how long, if not already counting ;)
    if (!button_counting && !button_lastpress_long) {
      button_press_start = millis();
      button_counting = true;
    }
    long button_press_time = millis() - button_press_start;
    if (button_press_time > LONGPRESSDELAY && !button_lastpress_long) {
      button_lastpress_long = true;
      button_counting = false;
      return LONGPRESS;
    }
  } else {
    //button is not pressed, check if just released using the counting flag
    if (button_counting) {
      button_counting = false;
      long button_press_time = millis() - button_press_start;
      if (button_press_time < LONGPRESSDELAY) {
        return SHORTPRESS;
      }
    } else {
      button_lastpress_long = false;
    }

  }
  return NOPRESS;
}


void loop() {
  int button_result = checkButtonPress();
  //  if (result == SHORTPRESS) {
  //    toDisplay = numbers[3];
  //    flashing = false;
  //    //disp7Seg(numbers[3], true);
  //  } else if (result == LONGPRESS) {
  //    toDisplay = numbers[5];
  //    flashing = true;
  //    //disp7Seg(numbers[5], true);
  //  }
  //  disp7Seg(toDisplay);

  if (menu_mode && button_result == LONGPRESS) {
    //stop menu mode
    menu_mode = false;
    toDisplay = B00000000;
    //save the current menu index to EEPROM
    EEPROM.write(address, current_menu_index);
    //set correct pin functions

    //do the function that shortpress would do.
    shortpress_functions();
  }
  else if (!menu_mode && button_result == LONGPRESS) {
    //start menu mode
    pinMode(B, OUTPUT);
    pinMode(F, OUTPUT);
    menu_mode = true;
    toDisplay = menu_disp[current_menu_index];
  }
  else if (menu_mode && button_result == SHORTPRESS) {
    //next menu item

    current_menu_index++;
    if (current_menu_index >= MENU_SIZE) {
      current_menu_index = 0;
    }

    toDisplay = menu_disp[current_menu_index];
  }
  else if (!menu_mode && button_result == SHORTPRESS) {
    //normal shortpress function
    shortpress_functions();
  }

  //things to do continuously if not in menu mode
  if (!menu_mode) {
    do_continuously();
  }


  disp7Seg(toDisplay);
}

void shortpress_functions() {
  byte roll;

  switch (current_menu_index) {
    case DICE:
      //roll dice again
      leds[0] = CRGB::Black;
      FastLED.show();
      roll = (int)random(0, 10);

      disp7Seg(B01111111);
      delay(200);
      disp7Seg(B10111111);
      delay(200);
      disp7Seg(B11111101);
      delay(200);
      disp7Seg(B11111011);
      delay(200);
      toDisplay = numbers[roll];
      break;
    case COMM:
      leds[0] = CRGB::Black;
      FastLED.show();
      byte roll;
      roll = (int)random(0, 10);

      //increase comm digit
      toDisplay = inverse[roll];
      break;
    case LASER:
      //do nothing, a different button activates this
      //toDisplay = numbers[10];

      pinMode(B, INPUT_PULLUP);
      pinMode(F, INPUT_PULLUP);

      toDisplay = B11111110;
      life = LIFE;
      //life = LIFE;
      break;
    case COUNTING:
      leds[0] = CRGB::Black;
      FastLED.show();
      counting_paused = !counting_paused;
      break;
    case RGBLED:
      //pause/unpause rgbled
      rgb_led_paused = !rgb_led_paused;
      break;

  }
}

void do_continuously() {
  bool b = false;
  bool f = false;
  switch (current_menu_index) {
    case DICE:
      //nothing here
      break;
    case COMM:
      //do nothing here
      break;
    case LASER:
      //check for IR pulse
      if (IR_receive()) {
        life = DEAD;
      }
      switch (life) {
        case DEAD:
          leds[0] = CRGB::Green; //somehow this is red
          break;
        case LIFE:
          leds[0] = CRGB::Red; //somehow this is green..
          break;
        case POWER:
          leds[0] = CRGB::Blue;
          break;
      }
      b = digitalRead(B);
      f = digitalRead(F);
      if (!b) {
        life = LIFE;
      }
      if (!f) {
        life = POWER;
      }
      FastLED.show();
      //checkExtraButtons();
      break;
    case COUNTING:
      if (millis() - previous_counting_millis >= counting_interval) {
        previous_counting_millis = millis();
        if (!counting_paused) {
          counting_index++;
        }
      }
      if (counting_index > 9) {
        counting_index = 0;
      }

      toDisplay = numbers[counting_index];
      break;
    case RGBLED:
      //pause/unpause rgbled
      if (!rgb_led_paused) {
        rgb_led_counter++;
      }
      leds[0] = ColorFromPalette(RainbowColors_p, rgb_led_counter, BRIGHTNESS, LINEARBLEND);
      FastLED.show();
      delay(20);
      break;

  }
}




bool IR_receive() {
  if (irrecv.decode(&results)) {
    irrecv.resume(); // Receive the next value
    return true;
  }
  delay(100);
  return false;
}

void disp7Seg(byte val) {
  if (millis() - previous_flash_millis >= flash_interval) {
    if (menu_mode) {
      previous_flash_millis = millis();
      on = !on;
    } else {
      on = true;
    }
  }
  if (!on) {
    val = B11111111;
  }

  digitalWrite(A, val & B10000000);
  digitalWrite(B, val & B01000000);
  digitalWrite(C, val & B00100000);
  digitalWrite(D, val & B00010000);
  digitalWrite(E, val & B00001000);
  digitalWrite(F, val & B00000100);
  digitalWrite(G, val & B00000010);
  digitalWrite(DOT, val & B00000001);

}

