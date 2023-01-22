#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

// Define used pins
#define ledPin 2

// Define variables
volatile char sleepCnt = 0;
int snacks_fed = 0;

// Main Tamagotchi Class
class tamagotchi {
  public:
    int hunger;
    int happy;
    int discipline;
    int level;
    bool health;
    bool soiled;

    tamagotchi(String name) {
      hunger = 50;
      happy = 100;
      discipline = 0;
      level = 0;
      health = true;
      soiled = true;
    }
};

// Define functions
void doBlink();
void passTime(tamagotchi& tama);
void overUnder(tamagotchi& tama);
void rightLeft(tamagotchi& tama);

void setup() {
  Serial.begin(9600);

  // Flash LED once
  digitalWrite(ledPin, LOW);
  pinMode(ledPin, OUTPUT);

  Serial.println('Setup Complete');
}

void loop() {

  doBlink();

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

	while (sleepCnt < 4) {

		// Turn of Brown Out Detection (low voltage). This is automatically re-enabled upon timer interrupt
		sleep_bod_disable();

		// Ensure we can wake up again by first disabling interrupts (temporarily) so
		// the wakeISR does not run before we are asleep and then prevent interrupts,
		// and then defining the ISR (Interrupt Service Routine) to run when poked awake by the timer
		noInterrupts();

		// clear various "reset" flags
		MCUSR = 0; 	// allow changes, disable reset
		WDTCSR = bit (WDCE) | bit(WDE); // set interrupt mode and an interval
		WDTCSR = bit (WDIE) | bit(WDP2) | bit(WDP1); //| bit(WDP0);    // set WDIE, and 1 second delay
		wdt_reset();

		// Send a message just to show we are about to sleep
		Serial.println("Good night!");
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
	Serial.println("I'm awake!");

	// Reset sleep counter
	sleepCnt = 0;

	// Re-enable ADC if it was previously running
	ADCSRA = prevADCSRA;
}

// When WatchDog timer causes µC to wake it comes here
ISR (WDT_vect) {

	// Turn off watchdog, we don't want it to do anything (like resetting this sketch)
	wdt_disable();

	// Increment the WDT interrupt count
	sleepCnt++;

	// Now we continue running the main Loop() just after we went to sleep
}

void doBlink() {
  digitalWrite(ledPin, HIGH);
  delay(10);
  digitalWrite(ledPin, LOW);
  delay(200);
  digitalWrite(ledPin, HIGH);
  delay(10);
  digitalWrite(ledPin, LOW);
}

void passTime(tamagotchi& tama) {
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
  } else if (tama.hunger <= 0) {
    // call for food
  } else if (random(tama.discipline) == tama.discipline) {
    // call for food/attention regardless of the status
  } else {
    // If tama is full & happy, decrement those statuses
    tama.happy -= 5;
    tama.hunger -= 5;
  }
}

void overUnder(tamagotchi& tama) {
  Serial.println('Playing Over/Under');

  // Set up the game
  int first = random(1, 11);
  int second = random(1, 11);
  bool user_guess;

  // Display first number
  Serial.println(first);
  
  // Prompt for user input
  // True: Higher
  // False: Lower
  Serial.println('Will the next number be higher or lower?');
  user_guess = true;

  // Show second number
  Serial.println(second);

  if ((first < second) && user_guess ) {
    Serial.println('You were right!');
  } else if (first == second) {
    Serial.println("...that's gotta be cheating, right?");
  } else {
    Serial.println('Try again next time!');
  }

  // Set tama happiness level
  tama.happy += 10;
  Serial.print('Happiness set to ');
  Serial.print(tama.happy);
  Serial.println();
}

void rightLeft(tamagotchi& tama) {
  Serial.println('Playing Right/Left');

  // Set up the game
  bool direction = random(0, 2);
  bool choice;

  // Prompt user for input
  Serial.println('Which direction will they go?');
  choice = false;

  // Evaluate results
  if (direction == choice) {
    Serial.println("Congratulations!");
  } else {
    Serial.println('Too bad, so sad.');
  }

}