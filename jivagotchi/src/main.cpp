#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <U8g2lib.h>

// Define used pins

// U8g2 Constructor
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// Main Tamagotchi Class
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
      happy = 100;
      discipline = 100;
      level = 0;
      health = true;
      soiled = true;
    }

    void print() {
      Serial.println(happy);
      Serial.println(hunger);
      Serial.println(health);
      Serial.println(discipline);
      Serial.println(level);
      Serial.println(soiled);
    }
};

// Define variables
volatile char sleepCnt = 0;
int snacks_fed = 0;
tamagotchi jiv;

// Handle Bitmaps
static const int u8g_logo_width = 16;
static const int u8g_logo_height = 16;
static unsigned char u8g_logo_bits[] = {
  0x00, 0x00, 0x7c, 0x00, 0x82, 0x00, 0x82, 0x00,
  0x82, 0x00, 0x82, 0x00, 0x82, 0x00, 0x7c, 0x3e,
  0x00, 0x41, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
  0x80, 0x80, 0x80, 0x80, 0x00, 0x41, 0x00, 0x3e };

// Define functions
void passTime(tamagotchi& tama) {
  Serial.println("Passing time");

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
    Serial.println("Make happy");
  } else if (tama.hunger <= 0) {
    // call for food
    Serial.println("Feed pls");
  } else if (random(tama.discipline) == tama.discipline) {
    // call for food/attention regardless of the status
    Serial.println("Needs discipline");
  } else {
    // If tama is full & happy, decrement those statuses
    tama.happy -= 5;
    tama.hunger -= 5;
  }
}

void overUnder(tamagotchi& tama) {
  Serial.println("Playing Over/Under");

  // Set up the game
  int first = random(1, 11);
  int second = random(1, 11);
  bool user_guess;

  // Display first number
  Serial.println(first);
  
  // Prompt for user input
  // True: Higher
  // False: Lower
  Serial.println("Will the next number be higher or lower?");
  user_guess = true;

  // Show second number
  Serial.println(second);

  if ((first < second) && user_guess ) {
    Serial.println("You were right!");
  } else if (first == second) {
    Serial.println("...that's gotta be cheating, right?");
  } else {
    Serial.println("Try again next time!");
  }

  // Set tama happiness level
  tama.happy += 10;
  Serial.print("Happiness set to ");
  Serial.print(tama.happy);
  Serial.println();
}

void rightLeft(tamagotchi& tama) {
  Serial.println("Playing Right/Left");

  // Set up the game
  bool direction = random(0, 2);
  bool choice;

  // Prompt user for input
  Serial.println("Which direction will they go?");
  choice = false;

  // Evaluate results
  if (direction == choice) {
    Serial.println("Congratulations!");
  } else {
    Serial.println("Too bad, so sad.");
  }

}

void heal(tamagotchi& tama) {
  if (!tama.health) {
    Serial.println("Healing your tama!");
    tama.health = true;
  } else {
    Serial.println("Tama is not sick!");
  }
}

void scold(tamagotchi& tama) {
  if (tama.misbehave) {
    tama.misbehave = false;
    tama.discipline += 25;
  } else {
    tama.happy -= 20;
  }
}

void clean(tamagotchi& tama) {
  if (tama.soiled) {
    tama.soiled = false;
    Serial.println("Tama is now clean!");
  }
}

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

void clearScreen() {
  u8g2.clear();
  u8g2.clearBuffer();
}

void printImage(int width, int height, unsigned char *pic, bool clear = true, int posx = 0, int posy = 0) {
  if (clear) { 
    clearScreen();
  }
  u8g2.drawXBM(posx, posy, width, height, pic);
  u8g2.sendBuffer();
}

void printText(const char *text, bool clear = true, int posx = 0, int posy = 0) {
  if (clear) {
    clearScreen();
  }
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(posx, posy, text);
  u8g2.sendBuffer();
}

void setup() {
  Serial.begin(9600);

  u8g2.begin();

  Serial.println("Setup Complete");
}

void loop() {
  // passTime(jiv);
  // delay(500);
  // Serial.println("---");
  // overUnder(jiv);
  // delay(500);
  // Serial.println("---");
  // rightLeft(jiv);
  // jiv.print();

  // u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
  // u8g2.drawStr(0,10,"Hello World!");	// write something to the internal memory
  // u8g2.sendBuffer();					// transfer internal memory to the display

  printImage(u8g_logo_width, u8g_logo_height, u8g_logo_bits);
  delay(1000);
  printText("Hello World", false, 0, 29);
  delay(1000);

  // doSleep();
  Serial.println();
}

// When WatchDog timer causes µC to wake it comes here
ISR (WDT_vect) {

	// Turn off watchdog, we don't want it to do anything (like resetting this sketch)
	wdt_disable();

	// Increment the WDT interrupt count
	sleepCnt++;

	// Now we continue running the main Loop() just after we went to sleep
}

