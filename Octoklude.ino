#include "LCD_Strings.h"
#include <PotMUX.h>
#include <LiquidCrystal.h>

#include <MozziGuts.h>
#include <Oscil.h>
#include <tables/saw256_int8.h> // saw table for oscillator
#include <tables/sin256_int8.h> // sin table for oscillator
#include <tables/triangle_analogue512_int8.h> // tri table for oscillator
#include <tables/square_analogue512_int8.h> // square table for oscillator
#include <EventDelay.h>
#include <LowPassFilter.h>
#include <mozzi_rand.h>
#include <mozzi_midi.h>
#include <mozzi_fixmath.h>
#include <IntMap.h>

#define CONTROL_RATE 128



// initialize the library with the numbers of the interface pins 
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
EventDelay infrequentDelay;

//Arduino Vars
int switchPin = 10;




bool potChanged = false;
bool lcdNeedsRefresh = false;
bool isSynthMode;

char lcdBuffer[20];

//Synth Vars
Oscil <SIN256_NUM_CELLS, AUDIO_RATE> aSin(SIN256_DATA);
Oscil <SAW256_NUM_CELLS, AUDIO_RATE> aSaw(SAW256_DATA);
Oscil <TRIANGLE_ANALOGUE512_NUM_CELLS, AUDIO_RATE> aTri(TRIANGLE_ANALOGUE512_DATA);
Oscil <SQUARE_ANALOGUE512_NUM_CELLS, AUDIO_RATE> aSqu(SQUARE_ANALOGUE512_DATA);

byte volume = 50; //0 - 255
byte waveform = 0; // 0 - 3 

//Low Pass Filter Vars
LowPassFilter lpf;
byte cutoffFreq = 255;
byte resonance = 0;

//LFO Vars
byte lfoWaveform = 0;

int pitchBend = 0;

int noteLength = 1000; // ~10 - ~2000



//Int Mappings
IntMap mapTo255 = IntMap(0, 1023, 0, 255);
int noteLengthMax = 1000;
IntMap mapToNoteLength = IntMap(0, 1023, 0, 925);
IntMap mapTo100 = IntMap(0, 1023, 0, 100);

//Arp Vars
byte middleCMidi = 60;
byte rootMidiNote = middleCMidi;
byte midiOffset = 0;
byte midiRange = 7;

byte rootMidiIndex = 0;
float noteFrequency;

byte gatePercent = 50;
byte modeIndex = 0;
byte octaveShiftIndex = 3;
byte octaveRange = 1;
byte patternIndex = 0;
byte insertIndex = 0;

EventDelay noteLengthDelay;
bool noteOn;

//Potentiometer Vars
#define NUM_POTS 8
int potVals[] = { -1, -1, -1, -1, -1, -1, -1, -1 };
int mostRecentPot = 0;

void setup(){

	Pot.Initialize();

	// set up the LCD's number of rows and columns: 
	lcd.begin(16, 2);

	//Home message
	lcd.setCursor(0, 0);
	lcd.print("   Welcome to   ");
	lcd.setCursor(0, 1);
	lcd.print(" the  Octoklude ");
	delay(1000);

	lcd.clear();

	infrequentDelay.set(50);

	noteLengthDelay.set(noteLengthMax - noteLength);

	updateFrequency();

	lpf.setCutoffFreq(cutoffFreq);
	startMozzi(CONTROL_RATE);

}

//Basically updates the LCD readout and checks what mode we're in
void infrequentControl()
{
	isSynthMode = digitalRead(switchPin);

	if (lcdNeedsRefresh)
	{
		lcd.clear();
		lcd.setCursor(0, 0);
		lcdNeedsRefresh = false;

		//If in synth mode
		if (isSynthMode)
		{
			//Big switch for Synth LCD
			switch (mostRecentPot)
			{
			case 0:
				lcd.print("Waveform:");
				lcd.setCursor(0, 1);
				strcpy_P(lcdBuffer, (char*)pgm_read_word(&(waveformStrings[waveform])));
				lcd.print(lcdBuffer);
				break;

			case 1:
				lcd.print("Filter Freq.:");
				lcd.setCursor(0, 1);
				lcd.print(cutoffFreq);
				break;
			case 2:
				lcd.print("Filter Res.:");
				lcd.setCursor(0, 1);
				lcd.print(resonance);
				break;

			case 3:
				lcd.print("Pitch Bend:");
				lcd.setCursor(0, 1);
				lcd.print(pitchBend);
				break;

			case 4:
				lcd.print("LFO Waveform:");
				lcd.setCursor(0, 1);
				strcpy_P(lcdBuffer, (char*)pgm_read_word(&(waveformStrings[lfoWaveform])));
				lcd.print(lcdBuffer);
				break;

			case 5:
				lcd.print("LFO Depth:");
				lcd.setCursor(0, 1);
				lcd.print("undefined");
				break;

			case 6:
				lcd.print("LFO Rate:");
				lcd.setCursor(0, 1);
				lcd.print("undefined");
				break;

			case 7:
				lcd.print("LFO Destination:");
				lcd.setCursor(0, 1);
				lcd.print("undefined");
				break;

			default:
				break;
			}
		}
		//if in arp mode
		else
		{
			//Big switch for Arp LCD
			switch (mostRecentPot)
			{
			case 0:
				lcd.print("BPM:");
				lcd.setCursor(0, 1);
				lcd.print(60000.0 / noteLength);
				break;

			case 1:
				lcd.print("Gate Percent:");
				lcd.setCursor(0, 1);
				lcd.print(gatePercent);
				break;

			case 2:
				lcd.print("Root Note:");
				lcd.setCursor(0, 1);
				strcpy_P(lcdBuffer, (char*)pgm_read_word(&(keyStrings[rootMidiIndex])));
				lcd.print(lcdBuffer);
				break;

			case 3:
				lcd.print("Mode:");
				lcd.setCursor(0, 1);
				strcpy_P(lcdBuffer, (char*)pgm_read_word(&(modeStrings[modeIndex])));
				lcd.print(lcdBuffer);
				break;

			case 4:
				lcd.print("Octave Shift:");
				lcd.setCursor(0, 1);
				strcpy_P(lcdBuffer, (char*)pgm_read_word(&(octaveOffsetStrings[octaveShiftIndex])));
				lcd.print(lcdBuffer);
				break;

			case 5:
				lcd.print("Octave Range:");
				lcd.setCursor(0, 1);
				lcd.print(octaveRange);
				break;

			case 6:
				lcd.print("Pattern:");
				lcd.setCursor(0, 1);
				strcpy_P(lcdBuffer, (char*)pgm_read_word(&(patternStrings[patternIndex])));
				lcd.print(lcdBuffer);
				break;

			case 7:
				lcd.print("Insert:");
				lcd.setCursor(0, 1);
				strcpy_P(lcdBuffer, (char*)pgm_read_word(&(insertStrings[insertIndex])));
				lcd.print(lcdBuffer);
				break;

			default:
				break;
			}

		}
	}


}

//!!!
//This whole function I think could be a lot more efficient if we used switches for most recent pots, not sure though
void frequentControl()
{
	//updatePots ultimately uses analogRead, which Mozzi overrides. I had to comment a line in MozziGuts.cpp:startMozzi(control_rate) to make this work. Could be too slow later?
	//if a pot has changed, go in and change shiz
	updatePots();
	
	if (potChanged)
	{
		lcdNeedsRefresh = true;

		if (isSynthMode)
		{
			switch (mostRecentPot)
			{
			case 0:
				waveform = map(potVals[0], 0, 1023, 0, 4);
				break;

			case 1:
				cutoffFreq = mapTo255(potVals[1]);
				lpf.setCutoffFreq(cutoffFreq);
				break;

			case 2:
				resonance = mapTo255(potVals[2]);
				lpf.setResonance(resonance);
				break;

			case 3:
				pitchBend = map(potVals[3], 0, 1023, -200, 200);
				updateFrequency();
				break;

			default:
				break;
			}		
		}
		else
		{
			//Arp Control Logic Here
			switch (mostRecentPot)
			{
			case 0:
				noteLength = noteLengthMax - mapToNoteLength(potVals[0]);
				break;

			case 1:
				gatePercent = mapTo100(potVals[1]);
				break;

			case 2:
				rootMidiIndex = map(potVals[2], 0, 1023, 0, 12);
				if (rootMidiIndex == 12)
					rootMidiIndex = 11;
				rootMidiNote = middleCMidi + rootMidiIndex;
				break;

			case 3:
				modeIndex = map(potVals[3], 0, 1023, 0, 7);
				if (modeIndex == 7)
					modeIndex = 6;
				break;

			case 4:
				octaveShiftIndex = map(potVals[4], 0, 1023, 0, 7);
				if (octaveShiftIndex == 7)
					octaveShiftIndex = 6;
				break;

			case 5:
				octaveRange = map(potVals[5], 0, 1023, 1, 4);
				if (octaveRange == 4)
					octaveRange = 3;
				break;

			case 6:
				patternIndex = map(potVals[6], 0, 1023, 0, 4);
				if (patternIndex == 4)
					patternIndex = 3;
				break;

			case 7:
				insertIndex = map(potVals[7], 0, 1023, 0, 5);
				if (insertIndex == 5)
					insertIndex = 4;
				break;

			default:
				break;
			}

		}

	}
	
}

void noteLengthHandler()
{	
		noteOn = !noteOn;

		if (!noteOn)
		{
			volume = 0;
		}
		else
		{
			volume = 50;
			updateFrequency();
		}	

		noteLengthDelay.set(noteLength);

}

void updateControl()
{
	//infrequentDelay is called every 640 millis i believe
	if (infrequentDelay.ready())
	{
		infrequentControl();
		infrequentDelay.start();
	}
	
	frequentControl();

	//noteLengthDelay is called depending on noteLength setting
	if (noteLengthDelay.ready())
	{
		noteLengthHandler();
			

		noteLengthDelay.start();
	}


}

void updateFrequency()
{
	noteFrequency = Q16n16_mtof(rootMidiNote) + pitchBend;

	aSin.setFreq(noteFrequency);
	aSaw.setFreq(noteFrequency);
	aTri.setFreq(noteFrequency);
	aSqu.setFreq(noteFrequency);
}

int updateAudio()
{
	int signal;

		switch (waveform)
		{
		case 0:
			signal = ((int)aSin.next() * volume) >> 8;
			break;
		case 1:
			signal = ((int)aSqu.next() * volume) >> 8;
			break;
		case 2:
			signal = ((int)aTri.next() * volume) >> 8;
			break;
		case 3:
			signal = ((int)aSaw.next() * volume) >> 8;
			break;

		case 4:
			signal = ((int)aSaw.next() * volume) >> 8;
			break;
		}

		return lpf.next(signal);
}

void loop()
{
	audioHook();
}

//Checks the state of all Pots, compares them to previous, if any are changed (>epsilon) new value is stored, returns true
void updatePots()
{

	potChanged = false;
	int epsilon = 20;

	for (int i = 0; i<NUM_POTS; i++)
	{
		int oldPotVal = potVals[i];
		int newPotVal = Pot.Read(i);

		if (oldPotVal == -1 || abs(oldPotVal - newPotVal) > epsilon)
		{
			potVals[i] = newPotVal;
			mostRecentPot = i;
			potChanged = true;
		}
	}

}




