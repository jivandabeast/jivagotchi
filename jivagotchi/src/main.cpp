/*
 * Jiva-gotchi: An Arduino Tamagotchi Clone 
 * Written By: Jivan RamjiSingh
 * 2022-01-23
 * Licensed under GPL v3.0
*/

#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include <U8g2lib.h>
#include <EEPROM.h>
#include <RTClib.h>

/**
 * Pin Definitions
 */

#define buttonA 2 
#define buttonB 3
#define buttonC 4

/**
 * Tamagotchi Class
 * Holds all the values related to the user's tamagotchi
 */
class tamagotchi {
  
  public:
    int hunger;
    int happy;
    int discipline;
    int level;
    bool health;
    bool soiled;
    bool misbehave;

    tamagotchi() {
      hunger = 50;
      happy = 50;
      discipline = 100;
      level = 1;
      health = true;
      soiled = true;
      misbehave = true;
    }

    void print() {
      Serial.println();
      Serial.print(F("Happy: "));
      Serial.println(happy);
      Serial.print(F("Hunger: "));
      Serial.println(hunger);
      Serial.print(F("Health: "));
      Serial.println(health);
      Serial.print(F("Discipline: "));
      Serial.println(discipline);
      Serial.print(F("Level: "));
      Serial.println(level);
      Serial.print(F("Soiled: "));
      Serial.println(soiled);
    }
};

/**
 * Global Variables
 */
volatile char sleepCnt = 0;
int snacks_fed = 0;
tamagotchi jiv;
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
RTC_DS1307 rtc;
bool pass_time, over_under, right_left, heal_tama, scold_tama, clean_tama;
bool changed = true;
const char* activities[5] = {
  "Over Under",
  "Right Left",
  "Heal",
  "Scold",
  "Clean"
};

/**
 * Bitmaps
 */
static const int idle_width = 16;
static const int idle_height = 24;

static const unsigned char level_1_idle_0_bits[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xf8, 0x1f, 0x08, 0x10, 0x08, 0x10, 0x08, 0x10, 0x08, 0x10,
  0xe8, 0x38, 0xc8, 0x18, 0xc8, 0x18, 0x08, 0x10, 0x08, 0x10, 0x08, 0x10,
  0xf8, 0x1f, 0x08, 0x10, 0x08, 0x10, 0xf8, 0x1f, 0x38, 0x0e, 0x18, 0x06
};

static const unsigned char level_1_idle_1_bits[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xf8, 0x1f, 0x08, 0x10, 0x08, 0x10, 0x08, 0x10,
  0x08, 0x10, 0xe8, 0x38, 0xc8, 0x18, 0xc8, 0x18, 0x08, 0x10, 0x08, 0x10,
  0x08, 0x10, 0xf8, 0x1f, 0x08, 0x10, 0x08, 0x10, 0xf8, 0x1f, 0x38, 0x0e
};

static const unsigned char level_1_idle_2_bits[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x1f, 0x08, 0x10, 0x08, 0x10,
  0x08, 0x10, 0x08, 0x10, 0xe8, 0x38, 0xc8, 0x18, 0xc8, 0x18, 0x08, 0x10,
  0x08, 0x10, 0x08, 0x10, 0xf8, 0x1f, 0x08, 0x10, 0x08, 0x10, 0xf8, 0x1f
};

static const unsigned char level_1_idle_3_bits[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xf8, 0x1f, 0x08, 0x10, 0x08, 0x10, 0x08, 0x10,
  0x08, 0x10, 0xe8, 0x38, 0xc8, 0x18, 0xc8, 0x18, 0x08, 0x10, 0x08, 0x10,
  0x08, 0x10, 0xf8, 0x1f, 0x08, 0x10, 0x08, 0x10, 0xf8, 0x1f, 0x38, 0x0e
};

static const unsigned char level_2_idle_0_bits[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xf8, 0x1f, 0xf8, 0x1f, 0xf8, 0x1f, 0xf8, 0x1f, 0x38, 0x10, 0x18, 0x10,
  0xf8, 0x38, 0xc8, 0x18, 0xc8, 0x18, 0x08, 0x10, 0x08, 0x10, 0x08, 0x10,
  0xf8, 0x1f, 0x08, 0x10, 0x08, 0x10, 0xf8, 0x1f, 0x38, 0x0e, 0x18, 0x06
};

static const unsigned char level_2_idle_1_bits[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xf8, 0x1f, 0xf8, 0x1f, 0xf8, 0x1f, 0xf8, 0x1f, 0x38, 0x10,
  0x18, 0x10, 0xf8, 0x38, 0xc8, 0x18, 0xc8, 0x18, 0x08, 0x10, 0x08, 0x10,
  0x08, 0x10, 0xf8, 0x1f, 0x08, 0x10, 0x08, 0x10, 0xf8, 0x1f, 0x38, 0x0e
};

static const unsigned char level_2_idle_2_bits[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xf8, 0x1f, 0xf8, 0x1f, 0xf8, 0x1f, 0xf8, 0x1f,
  0x38, 0x10, 0x18, 0x10, 0xf8, 0x38, 0xc8, 0x18, 0xc8, 0x18, 0x08, 0x10,
  0x08, 0x10, 0x08, 0x10, 0xf8, 0x1f, 0x08, 0x10, 0x08, 0x10, 0xf8, 0x1f
};

static const unsigned char level_2_idle_3_bits[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xf8, 0x1f, 0xf8, 0x1f, 0xf8, 0x1f, 0xf8, 0x1f, 0x38, 0x10,
  0x18, 0x10, 0xf8, 0x38, 0xc8, 0x18, 0xc8, 0x18, 0x08, 0x10, 0x08, 0x10,
  0x08, 0x10, 0xf8, 0x1f, 0x08, 0x10, 0x08, 0x10, 0xf8, 0x1f, 0x38, 0x0e
};

static const unsigned char level_3_idle_0_bits[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xe0, 0x07, 0xf0, 0x0f, 0xf8, 0x1f, 0xf8, 0x1f, 0x3c, 0x10,
  0x1c, 0x10, 0xdc, 0x18, 0xdc, 0x78, 0x3c, 0x70, 0xfc, 0x7f, 0xf8, 0x3f,
  0xc8, 0x3f, 0x88, 0x1f, 0x08, 0x17, 0xc8, 0x13, 0x28, 0x0a, 0x18, 0x06 
};

static const unsigned char level_3_idle_1_bits[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xe0, 0x07, 0xf0, 0x0f, 0xf8, 0x1f, 0xf8, 0x1f,
  0x3c, 0x10, 0x1c, 0x10, 0xdc, 0x18, 0xdc, 0x78, 0x3c, 0x70, 0xfc, 0x7f,
  0xf8, 0x3f, 0xc8, 0x3f, 0x88, 0x1f, 0x08, 0x17, 0xc8, 0x13, 0x38, 0x0e 
};

static const unsigned char level_3_idle_2_bits[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x07, 0xf0, 0x0f, 0xf8, 0x1f,
  0xf8, 0x1f, 0x3c, 0x10, 0x1c, 0x10, 0xdc, 0x18, 0xdc, 0x78, 0x3c, 0x70,
  0xfc, 0x7f, 0xf8, 0x3f, 0xc8, 0x3f, 0x88, 0x1f, 0x08, 0x17, 0xf8, 0x1f 
};

static const unsigned char level_3_idle_3_bits[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xe0, 0x07, 0xf0, 0x0f, 0xf8, 0x1f, 0xf8, 0x1f,
  0x3c, 0x10, 0x1c, 0x10, 0xdc, 0x18, 0xdc, 0x78, 0x3c, 0x70, 0xfc, 0x7f,
  0xf8, 0x3f, 0xc8, 0x3f, 0x88, 0x1f, 0x08, 0x17, 0xc8, 0x13, 0x38, 0x0e 
};

static const unsigned char level_4_idle_0_bits[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x1f, 0x78, 0x10, 0x3c, 0x10,
  0xfc, 0x38, 0xdc, 0x18, 0xdc, 0x18, 0x3c, 0x30, 0xfc, 0x7f, 0xf8, 0x3f,
  0xf8, 0x3f, 0x88, 0x1f, 0x88, 0x1f, 0xf8, 0x1f, 0xb8, 0x0f, 0x98, 0x07
};

static const unsigned char level_4_idle_1_bits[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x1f, 0x78, 0x10,
  0x3c, 0x10, 0xfc, 0x38, 0xdc, 0x18, 0xdc, 0x18, 0x3c, 0x30, 0xfc, 0x7f,
  0xf8, 0x3f, 0xf8, 0x3f, 0x88, 0x1f, 0x88, 0x1f, 0xf8, 0x1f, 0xb8, 0x0f
};

static const unsigned char level_4_idle_2_bits[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x1f,
  0x78, 0x10, 0x3c, 0x10, 0xfc, 0x38, 0xdc, 0x18, 0xdc, 0x18, 0x3c, 0x30,
  0xfc, 0x7f, 0xf8, 0x3f, 0xf8, 0x3f, 0x88, 0x1f, 0x88, 0x1f, 0xf8, 0x1f
};

static const unsigned char level_4_idle_3_bits[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x1f, 0x78, 0x10,
  0x3c, 0x10, 0xfc, 0x38, 0xdc, 0x18, 0xdc, 0x18, 0x3c, 0x30, 0xfc, 0x7f,
  0xf8, 0x3f, 0xf8, 0x3f, 0x88, 0x1f, 0x88, 0x1f, 0xf8, 0x1f, 0xb8, 0x0f
};

/**
 * Function definitions
 */

/**
 * Clear Screen
 * Clears the OLED screen so that a new frame can be shown
 */
void clearScreen() {
  u8g2.clear();
  u8g2.clearBuffer();
}

/**
 * Print Image
 * Prints a bitmap image from flash memory
 * 
 * @param   width   The height of the image
 * @param   height  The width of the image
 * @param   pic     A PROGMEM variable containing the bitmap of the image
 * @param   clear   OPTIONAL - A boolean, true to clear the screen false to keep it (Default: True)
 * @param   posx    OPTIONAL - X Position of the image to be drawn (Default: 0)
 * @param   posy    OPTIONAL - Y Position of the image to be drawn (Default: 0)
 */
void printImage(int width, int height, const unsigned char *pic, bool clear = true, int posx = 0, int posy = 0) {
  if (clear) { 
    clearScreen();
  }
  u8g2.drawXBMP(posx, posy, width, height, pic);
  u8g2.sendBuffer();
}

/**
 * Print Text
 * Prints text onto the display
 * 
 * @param   text    String to be printed
 * @param   clear   OPTIONAL - A boolean, true to clear the screen false to keep it (Default: True)
 * @param   posx    OPTIONAL - X Position of the image to be drawn (Default: 0)
 * @param   posy    OPTIONAL - Y Position of the image to be drawn (Default: 0)
 */
void printText(const char *text, bool clear = true, int posx = 0, int posy = 0) {
  if (clear) {
    clearScreen();
  }
  // u8g2_font_ncenB08_tr
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(posx, posy, text);
  u8g2.sendBuffer();
}

/**
 * Print F() Style Text
 * Because SRAM is a valuable resource
 * 
 * @param   text    The F() string
 * @param   clear   Whether or not to clear the screen before printing
 * @param   posx    X Position for printing
 * @param   posy    Y Position for printing
 */
void print_f_text(const __FlashStringHelper* text, bool clear = true, int posx = 0, int posy = 0) {
  if (clear) {
    clearScreen();
  }
  u8g2.setCursor(posx, posy);
  u8g2.print(text);
  u8g2.sendBuffer();
}

/**
 * Print Tama Stats
 * Displays tama data on the screen
 * 
 * @param   tama    Tamagotchi object containing requested data
 * @param   clear   Whether or not to clear the screen before displaying
 */
void print_stats(tamagotchi& tama, bool clear = true) {
  if (clear) {
    clearScreen();
  }

  if (true) {
      char happy[12];
      snprintf(happy, sizeof(happy), "Happy: %d%%", tama.happy);
      printText(happy, false, 0, 35);

    }
    if (true) {
      char hunger[12];
      snprintf(hunger, sizeof(hunger), "Hunger: %d%%", tama.hunger);
      printText(hunger, false, 0, 45);

    } 
    if (true) {
      char discipline[16];
      snprintf(discipline, sizeof(discipline), "Discipline: %d%%", tama.discipline);
      printText(discipline, false, 0, 55);

    } 
    if (true) {
      char level[12];
      snprintf(level, sizeof(level), "%d", tama.level);
      printText(level, false, 20, 20);

    } 
    
    // else if (jiv.health != ohealth) {
    //   // Serial.println("Health");
    //   ohealth = jiv.health;
    //   if (jiv.health) {
    //     // Healthy
    //   } else {
    //     // Not healthy
    //   }
    // } else if (jiv.soiled != osoiled) {
    //   // Serial.println("Soiled");
    //   osoiled = jiv.soiled;

    //   if (jiv.soiled) {
    //     // Soiled
    //   } else {
    //     // Not Soiled
    //   }
    // } else if (jiv.misbehave != omisbehave) {
    //   // Serial.println("Misbehave");
    //   omisbehave = jiv.misbehave;

    //   if (jiv.misbehave){
    //     // Misbehaving
    //   } else {
    //     // Not misbehaving
    //   }
    // }
}

/**
 * Processes the "life functions" of the tamagotchi (getting hungry, bored, bathroom, etc.)
 * This function lowers those values to facilitate gameplay
 * 
 * @param   tama    The tamagotchi object to be processed
 */
void passTime(tamagotchi& tama) {
  // Serial.println("Passing time");

  if (tama.soiled) {
    if (random(100) > 75) {
      tama.health = false;
    }
  } else {
    if (random(100) > 90) {
      tama.soiled = false;
    }
  }

  if (tama.happy <= 0) {
    // call for attention
    // Serial.println("Make happy");
  } else if (tama.hunger <= 0) {
    // call for food
    // Serial.println("Feed pls");
  } else if (random(tama.discipline) == tama.discipline) {
    // call for food/attention regardless of the status
    // Serial.println("Needs discipline");
  } else {
    // If tama is full & happy, decrement those statuses
    tama.happy -= 5;
    tama.hunger -= 5;
  }
}

/**
 * Tama Balance Check
 * Make sure that certain tama values do not exceed/drop below their range
 * 
 * @param   tama    The tamagotchi object to be checked
 */
void check_bal(tamagotchi& tama) {
  if (tama.hunger > 100) {
    tama.hunger = 100;
  }
  if (tama.happy > 100) {
    tama.happy = 100;
  }
  if (tama.discipline > 100) {
    tama.discipline = 100;
  }
  if (tama.level > 4) {
    tama.level = 4;
  }

  if (tama.hunger < 0) {
    tama.hunger = 0;
  }
  if (tama.happy < 0) {
    tama.happy = 0;
  }
  if (tama.discipline < 0) {
    tama.discipline = 0;
  }
  if (tama.level < 1) {
    tama.level = 1;
  }
}

/**
 * Tamagotchi Game: Over Under
 * Guessing whether the second of two random numbers between 1 - 10 will be higher or lower than the first
 * Playing the game will increase the happiness level of the tamagotchi
 * 
 * TODO: Game splash screen?
 * 
 * @param   tama    The tamagotchi object to be processed
 */
void overUnder(tamagotchi& tama) {
  // Set up the game
  int first = random(1, 11);
  int second = random(1, 11);
  bool user_guess;

  // Display first number
  char first_num[2];
  sprintf(first_num, "%d", first);

  // Prompt for input
  printText(first_num, true, 0, 10);
  print_f_text(F("Up A"), false, 0, 20);
  print_f_text(F("Low B"), false, 0, 30);
  print_f_text(F("Confirm C"), false, 0, 40);
  while (digitalRead(buttonC) == HIGH) {
    if (digitalRead(buttonA) == LOW) {
      user_guess = true;
      print_f_text(F("GUESS: OVER"), false, 0, 50);
    } else if (digitalRead(buttonB) == LOW) {
      user_guess = false;
      print_f_text(F("GUESS: UNDER"), false, 0, 50);
    }
  }

  // Evaluate and display results
  char second_num[2];
  sprintf(second_num, "%d", second);
  if (((first < second) && user_guess) || ((first > second && !user_guess))) {
    printText(first_num, true, 0, 10);
    printText(second_num, false, 0, 20);
    print_f_text(F("POGCHAMP"), false, 0, 30);
  } else if (first == second) {
    printText(first_num, true, 0, 10);
    printText(second_num, false, 0, 20);
    print_f_text(F("...no comment..."), false, 0, 30);
  } else {
    printText(first_num, true, 0, 10);
    printText(second_num, false, 0, 20);
    print_f_text(F("Sadge"), false, 0, 30);
  }

  delay(500);
  // Set tama happiness level
  tama.happy += 10;
  changed = true;
  check_bal(tama);
  print_f_text(F("C to close."), false, 0, 50);
  while (digitalRead(buttonC) == HIGH) {

  }
}

/**
 * Tamagotchi Game: Left Right
 * Guessing whether the tamagotchi will turn to the left or the right
 * Playing the game will increase the happiness level of the tamagotchi
 * 
 * TODO: Game splash screen?
 * 
 * @param   tama    The tamagotchi object to be processed
 */
void rightLeft(tamagotchi& tama) {
  // Set up the game
  bool direction = random(0, 2);
  bool user_guess;

  // Prompt user for input
  print_f_text(F("Left A"), false, 0, 10);
  print_f_text(F("Right B"), false, 0, 20);
  print_f_text(F("Confirm C"), false, 0, 30);
  while (digitalRead(buttonC) == HIGH) {
    if (digitalRead(buttonA) == LOW) {
      user_guess = true;
      print_f_text(F("GUESS: LEFT"), false, 0, 50);
    } else if (digitalRead(buttonB) == LOW) {
      user_guess = false;
      print_f_text(F("GUESS: RIGHT"), false, 0, 50);
    }
  }

  if (direction) {
    // left
    if (tama.level == 1) {
      printImage(idle_width, idle_height, level_1_idle_0_bits, true, 90, 0);
    } else if (tama.level == 2) {
      printImage(idle_width, idle_height, level_2_idle_0_bits, true, 90, 0);
    } else if (tama.level == 3) {
      printImage(idle_width, idle_height, level_3_idle_0_bits, true, 90, 0);
    } else if (tama.level == 4) {
      printImage(idle_width, idle_height, level_4_idle_0_bits, true, 90, 0);
    }
  } else {
    // right
    if (tama.level == 1) {
      printImage(idle_width, idle_height, level_1_idle_0_bits, true, 0, 0);
    } else if (tama.level == 2) {
      printImage(idle_width, idle_height, level_2_idle_0_bits, true, 0, 0);
    } else if (tama.level == 3) {
      printImage(idle_width, idle_height, level_3_idle_0_bits, true, 0, 0);
    } else if (tama.level == 4) {
      printImage(idle_width, idle_height, level_4_idle_0_bits, true, 0, 0);
    }
  }

  // Evaluate results
  if (direction == user_guess ) {
    print_f_text(F("POGCHAMP"), false, 0, 35);
  } else {
    print_f_text(F("Sadge"), false, 0, 35);
  }

  tama.happy += 5;
  changed = true;
  check_bal(tama);
  print_f_text(F("C to close."), false, 0, 50);
  while (digitalRead(buttonC) == HIGH) {

  }
}

/**
 * Healing the sickness of a tamagotchi
 * When a tamagotchi is sick, they will need to be given "medicine" to become healthy again
 * 
 * @param   tama    The tamagotchi object to be processed
 */
void heal(tamagotchi& tama) {
  if (!tama.health) {
    print_f_text(F("Healing..."), true, 10, 40);
    tama.health = true;
    delay(4000);
    print_f_text(F("Healed!"), true, 10, 40);
    delay(1000);
  } else {
    print_f_text(F("Jiv is not sick!"), true, 10, 40);
    tama.happy -= 10;
    delay(2000);
  }
  check_bal(tama);
  changed = true;
  delay(300);
  print_f_text(F("C to continue"), false, 0, 60);
  while (digitalRead(buttonC) == HIGH) {

  }
}

/**
 * Discipling the tamagotchi
 * When a tamagotchi is misbehaving, they will need to be disciplined 
 * Doing so increases the discipline meter
 * 
 * @param   tama    The tamagotchi object to be processed
 */
void scold(tamagotchi& tama) {
  if (tama.misbehave) {
    print_f_text(F("Scolding..."), true, 10, 40);
    tama.discipline += 25;
    tama.happy -= 5;
    tama.misbehave = false;
    delay(4000);
    print_f_text(F("Scolded!"), true, 10, 40);
    delay(1000);
  } else {
    print_f_text(F("Jiv isn't misbehaving"), true, 0, 30);
    delay(1000);
    print_f_text(F("... :( ..."), false, 20, 40);
    tama.happy -= 20;
  }
  check_bal(tama);
  changed = true;
  delay(300);
  print_f_text(F("C to continue"), false, 0, 60);
  while (digitalRead(buttonC) == HIGH) {

  }
}

/**
 * Cleaning excrement
 * When a tamagotchi poops, someone has to be the shit scraper
 * 
 * @param   tama    The tamagotchi object to be processed
 */
void clean(tamagotchi& tama) {
    if (tama.soiled) {
    print_f_text(F("Cleaning..."), true, 10, 40);
    tama.happy += 20;
    tama.soiled = false;
    delay(2000);
    print_f_text(F("Cleaned!"), true, 10, 40);
  } else {
    print_f_text(F("Jiv didn't poo!"), true, 0, 30);
  }
  check_bal(tama);
  changed = true;
  delay(300);
  print_f_text(F("C to continue"), false, 0, 60);
  while (digitalRead(buttonC) == HIGH) {

  }
}

/**
 * Idle Animations
 * Make the tama do a lil dance in the corner lol
 * 
 * @param   tama    The tamagotchi to make dance
 */
void idle_ani(tamagotchi& tama) {
  if (tama.level == 1) {
    printImage(idle_width, idle_height, level_1_idle_0_bits, false);
    delay(34);
    printImage(idle_width, idle_height, level_1_idle_1_bits, false);
    delay(34);
    printImage(idle_width, idle_height, level_1_idle_2_bits, false);
    delay(34);
    printImage(idle_width, idle_height, level_1_idle_3_bits, false);
    delay(40);
  } else if (tama.level == 2) {
    printImage(idle_width, idle_height, level_2_idle_0_bits, false);
    delay(34);
    printImage(idle_width, idle_height, level_2_idle_1_bits, false);
    delay(34);
    printImage(idle_width, idle_height, level_2_idle_2_bits, false);
    delay(34);
    printImage(idle_width, idle_height, level_2_idle_3_bits, false);
    delay(40);
  } else if (tama.level == 3) {
    printImage(idle_width, idle_height, level_3_idle_0_bits, false);
    delay(34);
    printImage(idle_width, idle_height, level_3_idle_1_bits, false);
    delay(34);
    printImage(idle_width, idle_height, level_3_idle_2_bits, false);
    delay(34);
    printImage(idle_width, idle_height, level_3_idle_3_bits, false);
    delay(40);
  } else if (tama.level == 4) {
    printImage(idle_width, idle_height, level_4_idle_0_bits, false);
    delay(34);
    printImage(idle_width, idle_height, level_4_idle_1_bits, false);
    delay(34);
    printImage(idle_width, idle_height, level_4_idle_2_bits, false);
    delay(34);
    printImage(idle_width, idle_height, level_4_idle_3_bits, false);
    delay(40);
  }
}

/**
 * Sleep Function
 * Puts the arduino into low power mode, so that a potential connected battery doesn't get drained
 */
void doSleep() {
  // Disable the ADC (Analog to digital converter, pins A0 [14] to A5 [19])
	static byte prevADCSRA = ADCSRA;
	ADCSRA = 0;

	/* Set the type of sleep mode we want. Can be one of (in order of power saving):
	 SLEEP_MODE_IDLE (Timer 0 will wake up every millisecond to keep millis running)
	 SLEEP_MODE_ADC
	 SLEEP_MODE_PWR_SAVE (TIMER 2 keeps running)
	 SLEEP_MODE_EXT_STANDBY
	 SLEEP_MODE_STANDBY (Oscillator keeps running, makes for faster wake-up)
	 SLEEP_MODE_PWR_DOWN (Deep sleep)
	 */
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable()
	;

  // 8 seconds, (30 min * 60s)/8s = 225 loops
	while (sleepCnt < 38) {

		// Turn of Brown Out Detection (low voltage). This is automatically re-enabled upon timer interrupt
		sleep_bod_disable();

		// Ensure we can wake up again by first disabling interrupts (temporarily) so
		// the wakeISR does not run before we are asleep and then prevent interrupts,
		// and then defining the ISR (Interrupt Service Routine) to run when poked awake by the timer
		noInterrupts();

		// clear various "reset" flags
		MCUSR = 0; 	// allow changes, disable reset
		WDTCSR = bit (WDCE) | bit(WDE); // set interrupt mode and an interval
		WDTCSR = bit (WDIE) | bit(WDP3) | bit(WDP0); // set WDIE, and 8 second delay https://microcontrollerslab.com/arduino-watchdog-timer-tutorial/
		wdt_reset();

		// Send a message just to show we are about to sleep
    Serial.print(".");
		Serial.flush();

		// Allow interrupts now
		interrupts();

		// And enter sleep mode as set above
		sleep_cpu();
	}

	// --------------------------------------------------------
	// µController is now asleep until woken up by an interrupt
	// --------------------------------------------------------

	// Prevent sleep mode, so we don't enter it again, except deliberately, by code
	sleep_disable();

	// Wakes up at this point when timer wakes up µC
	Serial.print("-");

	// Reset sleep counter
	sleepCnt = 0;

	// Re-enable ADC if it was previously running
	ADCSRA = prevADCSRA;
}

/**
 * Setup Function
 * Arduino managed, called only when the device powers on
 */
void setup() {
  Serial.begin(9600);

  randomSeed(analogRead(A0));

  u8g2.begin();
  u8g2.clear();
  u8g2.clearBuffer();

  if (! rtc.begin()) {
  Serial.println("Couldn't find RTC");
  Serial.flush();
  while (1) delay(10);
  }

  pinMode(buttonA, INPUT_PULLUP);
  pinMode(buttonB, INPUT_PULLUP);
  pinMode(buttonC, INPUT_PULLUP);
}

/**
 * Main Loop
 * Arduino Managed, loops indefinitely after the setup function has completed
 * 
 * TODO: Write tama values to EEPROM
 * TODO: Incorporate passage of time
 * TODO: Automatic decrementing of stats
 * TODO: Automatic low-power mode/sleep
 */
void loop() {
  DateTime now = rtc.now();
  // Serial.println(now.unixtime());

  if (digitalRead(buttonA) == LOW) {
    delay(500);
    int i = 0;
    printText(activities[i], true, 25, 25);
    while (digitalRead(buttonC) == HIGH) {
      if (digitalRead(buttonB) == LOW) {
        if (i == 4) {
          i = 0;
        } else {
          i++;
        }
        printText(activities[i], true, 25, 25);
        delay(500);
      } else if (digitalRead(buttonA) == LOW) {
        if (i == 0) {
          i = 4;
        } else {
          i--;
        }
        printText(activities[i], true, 25, 25);
        delay(500);
      }
    }
    switch(i) {
      case 0:
        over_under = true;
        break;
      case 1:
        right_left = true;
        break;
      case 2:
        heal_tama = true;
        break;
      case 3:
        scold_tama = true;
        break;
      case 4:
        clean_tama = true;
        break;
    }
    clearScreen();
  }

  if (pass_time) {
    passTime(jiv);
  } else if (over_under) {
    overUnder(jiv);
    over_under = false;
  } else if (right_left) {
    rightLeft(jiv);
    right_left = false;
  } else if (heal_tama) {
    heal(jiv);
    heal_tama = false;
  } else if (scold_tama) {
    scold(jiv);
    scold_tama = false;
  } else if (clean_tama) {
    clean(jiv);
    clean_tama = false;
  } else {
    idle_ani(jiv);
    jiv.print();

    if (changed) {
      print_stats(jiv);
      changed = false;
    }
  }
}

/**
 * Interrupts
 */

// When WatchDog timer causes µC to wake it comes here
ISR (WDT_vect) {

	// Turn off watchdog, we don't want it to do anything (like resetting this sketch)
	wdt_disable();

	// Increment the WDT interrupt count
	sleepCnt++;

	// Now we continue running the main Loop() just after we went to sleep
}