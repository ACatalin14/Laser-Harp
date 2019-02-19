#include "Notes.h"

const int LDR_READER_PIN = A0;						// one pin which reads values from each LDR one at a time
const int LDR_PIN[] = { 2, 3, 4, 5, 6, 7, 8, 9 };	// each pin powers a specific LDR
const int LASERS_PIN = 10;							// pin which powers all the lasers at once
const int SPEAKER_PIN = 11;
const int BUTTON_PIN[] = { A1, A2, A3, A4 };		// A1 - reset button, A2 - pause/unpause button, A3 - increase 1 octave button, A4 - decrease 1 octave button

const double LDR_SENSITIVITY = 0.7;					// values between 0 and 1, 1 is for the most sensitive
const int READING_BASE_VALUES_DELAY = 1000;			// # of ms allocated for reading ambiental/laser values for LDRs
const int DEBOUNCE_BUTTON_DELAY = 50;
const int PLAYING_NOTE_DELAY = 100;					// # of ms allocated for playing a note
const int LDR_COUNT = 8;							// # of LDRs
const int BUTTON_COUNT = 4;							// # of buttons
const int OCTAVE_COUNT = 7;							// # of used octaves


const int notes[OCTAVE_COUNT][LDR_COUNT]  = {
	{ NOTE_C1, NOTE_D1, NOTE_E1, NOTE_F1, NOTE_G1, NOTE_A1, NOTE_B1, NOTE_C2 },	
	{ NOTE_C2, NOTE_D2, NOTE_E2, NOTE_F2, NOTE_G2, NOTE_A2, NOTE_B2, NOTE_C3 },	
	{ NOTE_C3, NOTE_D3, NOTE_E3, NOTE_F3, NOTE_G3, NOTE_A3, NOTE_B3, NOTE_C4 },	
	{ NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5 },	
	{ NOTE_C5, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_G5, NOTE_A5, NOTE_B5, NOTE_C6 },	
	{ NOTE_C6, NOTE_D6, NOTE_E6, NOTE_F6, NOTE_G6, NOTE_A6, NOTE_B6, NOTE_C7 },	
	{ NOTE_C7, NOTE_D7, NOTE_E7, NOTE_F7, NOTE_G7, NOTE_A7, NOTE_B7, NOTE_C8 }
};


int LDRValue[LDR_COUNT] = { 0 };					// current LDR values for the harp
unsigned long LDRAmbientValue[LDR_COUNT] = { 0 };	// LDR values when natural light falls over the LDRs
unsigned long LDRLaserValue[LDR_COUNT] = { 0 };		// LDR values when laser beams are over the LDRs
int currentOctave = 3;								// a value of 3 means the forth octave is "displayed" on the harp
int queueLDR[LDR_COUNT];							// not a proper queue, it's used for the LDRs which are requested to play at once in the order they were requested
int queueLength;									// the queue starts from index 0 and finishes to index queueLength - 1
int pausedHarp;										// values: false = harp is unpaused, true = harp is paused

int buttonState[BUTTON_COUNT];						// values: 0 = botton is up, 1 = botton is down
int lastButtonState[BUTTON_COUNT];
unsigned long lastDebounceButtonTime[BUTTON_COUNT];

void setup() {
	Serial.begin(9600);
	
	pinMode(LDR_READER_PIN, INPUT);
	for (int i = 0; i < LDR_COUNT; i++) {
		pinMode(LDR_PIN[i], OUTPUT);
	}

	pinMode(LASERS_PIN, OUTPUT);
	pinMode(SPEAKER_PIN, OUTPUT);

	for (int i = 0; i < LDR_COUNT; i++) {
		pinMode(BUTTON_PIN[i], INPUT_PULLUP);
	}
	
	digitalWrite(LASERS_PIN, HIGH);

	for (int i = 0; i < LDR_COUNT; i++) {
		queueLDR[i] = -1;
	}
	pausedHarp = 0;
	
	readBaseValues();
}

void loop() {
	checkButton(0, resetHarp);
	checkButton(1, pauseUnpauseHarp);
	if (!pausedHarp) {
		checkButton(2, increaseOctave);
		checkButton(3, decreaseOctave);
		readLightValues();
		playChords();
	}
}

// this function reads the light intensity gotten from the lasers/natural light so that we can use those values 
// for comparison with the current light intensity while playing the harp
void readBaseValues() {
	for (int i = 0; i < LDR_COUNT; i++) {
		LDRAmbientValue[i] = 0;
		LDRLaserValue[i] = 0;
	}

	// we take each base case: with and without lasers pointing at LDRs
	for (int ambientalCase = 0; ambientalCase < 2; ambientalCase++) {
		ambientalCase? 
			Serial.println("Reading base values for Ambiental Case") :
			Serial.println("Reading base values for Laser Case");
			
		int timesToRead = 0;				// number of times we've read the LDR values, so that we can make a mean afterwards
		unsigned long startReadingTime;		// moment of time when we started reading values

		ambientalCase?
			digitalWrite(LASERS_PIN, LOW) :
			digitalWrite(LASERS_PIN, HIGH);
		
		startReadingTime = millis();
	
		while (millis() - startReadingTime < READING_BASE_VALUES_DELAY) {
			for (int i = 0; i < LDR_COUNT; i++) {								// take each LDR
				digitalWrite(LDR_PIN[(i + LDR_COUNT - 1) % LDR_COUNT], LOW);		// power off the previous LDR
				digitalWrite(LDR_PIN[i], HIGH);										// power on the current LDR	

				ambientalCase?	// read the value from the current LDR and add it to calculate the mean
					LDRAmbientValue[i] += analogRead(LDR_READER_PIN) :
					LDRLaserValue[i] += analogRead(LDR_READER_PIN);
			}
			timesToRead++;
			delay(10);	// add a small pause so that we can't mess up with the LDRs
		}
	
		for (int i = 0; i < LDR_COUNT; i++) {
			ambientalCase?	// now calculate the mean
				LDRAmbientValue[i] /= timesToRead :
				LDRLaserValue[i] /= timesToRead;
		}
	}
	
	digitalWrite(LASERS_PIN, HIGH);
}

void readLightValues() {
	for (int i = 0; i < LDR_COUNT; i++) {
		digitalWrite(LDR_PIN[i], LOW);
	}
	
	for (int i = 0; i < LDR_COUNT; i++) {								// take each LDR
		digitalWrite(LDR_PIN[(i + LDR_COUNT - 1) % LDR_COUNT], LOW);		// power off the previous LDR
		digitalWrite(LDR_PIN[i], HIGH);										// power on the current LDR	
		LDRValue[i] = analogRead(LDR_READER_PIN);							// read the value from the current LDR
	}

	// for debugging purposes:
	for (int i = 0; i < LDR_COUNT; i++) {
		Serial.print("LDR");
		Serial.print(i);
		Serial.print(":");
		Serial.print(LDRValue[i]);
		Serial.print("|");
	}
	Serial.println("");
}

// this function will call onClickFunc whenever the button with the specified ID is pressed
void checkButton(int buttonId, void (*onClickFunc)() ) {
	int reading = digitalRead(BUTTON_PIN[buttonId]);

	if (reading != lastButtonState[buttonId]) {
		lastDebounceButtonTime[buttonId] = millis();
	}

	if (millis() - lastDebounceButtonTime[buttonId] > DEBOUNCE_BUTTON_DELAY) {
		if (reading != buttonState[buttonId]) {
			buttonState[buttonId] = reading;
			if (buttonState[buttonId] == 0) {	
				// button was pressed entirely (down+up)
				onClickFunc();
			}
		}
	}

	lastButtonState[buttonId] = reading;
}

void resetHarp() {
	if (pausedHarp) {
		digitalWrite(LASERS_PIN, HIGH);
		delay(500);			// add an "effect" to show the resetting is processing
		pausedHarp = false;
	}
	digitalWrite(LASERS_PIN, LOW);
	delay(500);				// add an "effect" to show the resetting is processing
	readBaseValues();
	currentOctave = 3;

	for (int i = 0; i < queueLength; i++) {
		queueLDR[i] = -1;
	}
	queueLength = 0;
}

void pauseUnpauseHarp() {
	if (!pausedHarp) {
		pausedHarp = true;
		digitalWrite(LASERS_PIN, LOW);
	} else {
		pausedHarp = false;
		digitalWrite(LASERS_PIN, HIGH);
	}
}

void increaseOctave() {
	if (currentOctave < OCTAVE_COUNT - 1) {
		currentOctave++;	
	}
}

void decreaseOctave() {
	if (currentOctave > 0) {
		currentOctave--;	
	}
}

void playChords() {
	// first determine which chords to be played, by checking the queueLDR
	for (int i = 0; i < LDR_COUNT; i++) {	// for each LDR
		if (LDRValue[i] < LDRLaserValue[i] * LDR_SENSITIVITY + LDRAmbientValue[i] * (1 - LDR_SENSITIVITY) ) {
			// luminosity is decreased for current LDR
			if (!isInQueue(i)) {
				// first time when LDR is requested to play, so add to queue
				if (queueLength < LDR_COUNT) {
					queueLDR[queueLength] = i;
					queueLength++;
				} else {
					// it shall be impossible to enter this block
					Serial.println("Why queue longer than LDR Set?!");
				}
			}
		} else {
			// luminosity is extremely high, so a laser beam is shining the LDR
			if (isInQueue(i)) {
				// this LDR was in the queue, it needs to be deleted
				int queuePosition;
				for (int j = 0; j < queueLength; j++) {
					if (i == queueLDR[j]) {
						queuePosition = j;
						break;
					}
				}
				for (int j = queuePosition; j < queueLength - 1; j++) {
					queueLDR[j] = queueLDR[j + 1];
				}
				queueLDR[queueLength - 1] = -1;
				queueLength--;
			}
		}
	}

	// now play the chords from the queue one after another extremly quickly 
	// to show that my harp can't handle more than one chord AT A TIME
	for (int i = 0; i < queueLength; i++) {
		tone(SPEAKER_PIN, notes[currentOctave][ queueLDR[i] ], PLAYING_NOTE_DELAY);
	}
}

// checks if a given LDR is in the queueLDR
bool isInQueue(int LDR) {
	for (int i = 0; i < queueLength; i++) {
		if (queueLDR[i] == LDR) {
			return true;
		}
	}
	return false;
}
