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

#define CONTROL_RATE 64



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

Oscil <SIN256_NUM_CELLS, AUDIO_RATE> lSin(SIN256_DATA);
Oscil <SAW256_NUM_CELLS, AUDIO_RATE> lSaw(SAW256_DATA);
Oscil <TRIANGLE_ANALOGUE512_NUM_CELLS, AUDIO_RATE> lTri(TRIANGLE_ANALOGUE512_DATA);
Oscil <SQUARE_ANALOGUE512_NUM_CELLS, AUDIO_RATE> lSqu(SQUARE_ANALOGUE512_DATA);

byte volume; //0 - 255
byte gain = 50; //0 - 100, so volume isn't ever too loud, volume = gain;
byte waveform = 0; // 0 - 3 

//Low Pass Filter Vars
LowPassFilter lpf;
byte cutoffFreq = 255;
byte resonance = 0;

//LFO Vars
byte lfoWaveform = 0;
float lfoScale = 0;
float lfoFrequency = 6.5f;
byte lfoDestinationIndex = 0;
int8_t lfoValue;
int8_t volumeLFOValue;

int pitchBend = 0;

//Int Mappings
IntMap mapTo255 = IntMap(0, 1023, 0, 255);
int beatLength = 1000;

IntMap mapToNoteLength = IntMap(0, 1023, 0, 925);
IntMap mapTo100 = IntMap(0, 1023, 0, 100);

//Arp Vars
byte middleCMidi = 60;
int rootMidiNote = middleCMidi;
byte rootMidiIndex = 0;

byte sequenceLength = 1;
byte sequenceIndex = 0;
int sequenceMidiOffset = 0;
byte upDownMult;

float noteFrequency;

byte gatePercent = 50;
byte modeIndex = 0;
byte octaveShiftIndex = 4;
int octaveShiftNoteCount = 0;
byte patternIndex = 0;
byte insertIndex = 0;

//Scale Intervals
//T-T-s-T-T-T-s
byte MajorIntervals[] = { 2, 2, 1, 2, 2, 2, 1 };
//T-s-T-T-T-s-T
byte DorianIntervals[] = { 2, 1, 2, 2, 2, 1, 2 };
//s-T-T-T-s-T-T 	
byte PhyrgianIntervals[] = { 1, 2, 2, 2, 1, 2, 2 };
//T-T-T-s-T-T-s 	
byte LydianIntervals[] = { 2, 2, 2, 1, 2, 2, 1 };
//T-T-s-T-T-s-T
byte MixolydianIntervals[] = { 2, 2, 1, 2, 2, 1, 2 };
//T-s-T-T-s-T-T 	
byte AeolianIntervals[] = { 2, 1, 2, 2, 1, 2, 2 };
//s-T-T-s-T-T-T
byte LocrianIntervals[] = { 1, 2, 2, 1, 2, 2, 2 };

EventDelay noteDelay;
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
	
	noteDelay.set(1000);
	
	updateFrequency();
	updateLFOFrequency();

	Serial.begin(9600);


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
				lcd.print("Gain:");
				lcd.setCursor(0, 1);
				lcd.print(gain);
				break;
			case 2:
				lcd.print("Filter Freq.:");
				lcd.setCursor(0, 1);
				lcd.print(cutoffFreq);
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
				lcd.print("LFO Scale:");
				lcd.setCursor(0, 1);
				lcd.print(lfoScale);
				break;

			case 6:
				lcd.print("LFO Rate:");
				lcd.setCursor(0, 1);
				lcd.print(lfoFrequency);
				break;

			case 7:
				lcd.print("LFO Destination:");
				lcd.setCursor(0, 1);
				strcpy_P(lcdBuffer, (char*)pgm_read_word(&(lfoDestStrings[lfoDestinationIndex])));
				lcd.print(lcdBuffer);
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
				lcd.print(60000.0 / beatLength);
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
				lcd.print("Sequence Length:");
				lcd.setCursor(0, 1);
				lcd.print(sequenceLength);
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
				if (waveform == 4)
					waveform = 3;
				break;

			case 1:
				gain = mapTo100(potVals[1]);

				if (noteOn)
					volume = gain;
				break;

			case 2:
				cutoffFreq = mapTo255(potVals[2]);
				lpf.setCutoffFreq(cutoffFreq);
				break;

			case 3:
				pitchBend = map(potVals[3], 0, 1023, -500, 500);
				break;

			case 4:
				lfoWaveform = map(potVals[4], 0, 1023, 0, 4);
				if (lfoWaveform == 4)
					lfoWaveform = 3;
				break;

			case 5:
				lfoScale = mapTo100(potVals[5]);
				break;

			case 6:
				lfoFrequency = map(potVals[6], 0, 1023, 30, 4000);
				updateLFOFrequency();
				break;

			case 7:
				lfoDestinationIndex = map(potVals[7], 0, 1023, 0, 3);
				if (lfoDestinationIndex == 3)
					lfoDestinationIndex = 2;
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
				beatLength = 1080 - potVals[0];
				break;

			case 1:
				gatePercent = mapTo100(potVals[1]);
				break;

			case 2:
				rootMidiIndex = map(potVals[2], 0, 1023, 0, 12);
				restartSequence();
				if (rootMidiIndex == 12)
					rootMidiIndex = 11;
				rootMidiNote = middleCMidi + rootMidiIndex;
				break;

			case 3:
				modeIndex = map(potVals[3], 0, 1023, 0, 7);
				restartSequence();
				if (modeIndex == 7)
					modeIndex = 6;
				break;

			case 4:
				octaveShiftIndex = map(potVals[4], 0, 1023, 0, 9);
				restartSequence();
				if (octaveShiftIndex == 9)
					octaveShiftIndex = 8;
				break;

			case 5:
				sequenceLength = map(potVals[5], 0, 1023, 1, 29);
				restartSequence();
				if (sequenceLength == 29)
					sequenceLength = 28;
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

void noteEvent()
{	
		noteOn = !noteOn;

		if (noteOn)
		{
			updateSequence();

			noteDelay.set(beatLength*(gatePercent/100.0));

		}
		else
		{
			volume = 0;
			noteDelay.set(beatLength*((100-gatePercent) / 100.0));

		}
		noteDelay.start();
}

void updateControl()
{
	if (infrequentDelay.ready())
	{
		infrequentControl();
		infrequentDelay.start();
	}
	
	frequentControl();

	updateLFOValue();

	updateFrequency();

	//noteLengthDelay is called depending on noteLength setting
	if (noteDelay.ready())
	{
		noteEvent();
	}


}

void updateSequence()
{	
	upDownMult = 1;

	//pattern down inverse
	if (patternIndex == 1)
	{
		//upDownMult = -1;
	}

	if (sequenceIndex > sequenceLength)
	{
		restartSequence();
	}

	if (patternIndex == 3)
	{
		int low = -sequenceLength*2;
		int high = sequenceLength*2;
		sequenceMidiOffset = rand(low,high);
	}
	else
	{
		switch (modeIndex)
		{
		case 0:
			sequenceMidiOffset += MajorIntervals[(sequenceIndex) % 7] * upDownMult;
			break;
		case 1:
			sequenceMidiOffset += DorianIntervals[(sequenceIndex) % 7] * upDownMult;
			break;

		case 2:
			sequenceMidiOffset += PhyrgianIntervals[(sequenceIndex) % 7] * upDownMult;
			break;

		case 3:
			sequenceMidiOffset += LydianIntervals[(sequenceIndex) % 7] * upDownMult;

			break;

		case 4:
			sequenceMidiOffset += MixolydianIntervals[(sequenceIndex) % 7] * upDownMult;
			break;

		case 5:
			sequenceMidiOffset += AeolianIntervals[(sequenceIndex) % 7] * upDownMult;
			break;

		case 6:
			sequenceMidiOffset += LocrianIntervals[(sequenceIndex) % 7] * upDownMult;
			break;

		default:
			break;
		}
	}
	
	sequenceIndex++;

}

void restartSequence()
{
	sequenceMidiOffset = 0;
	sequenceIndex = 0;
}

void updateFrequency()
{
	//Calculates octave shift (12 semitones) based off knob value
	octaveShiftNoteCount = (octaveShiftIndex - 4) * 12;

	noteFrequency = Q16n16_mtof(rootMidiNote + octaveShiftNoteCount + sequenceMidiOffset) + pitchBend;

	if (lfoDestinationIndex == 1 || lfoDestinationIndex == 2)
		noteFrequency += lfoValue;
	
	aSin.setFreq(noteFrequency);
	aSaw.setFreq(noteFrequency);
	aTri.setFreq(noteFrequency);
	aSqu.setFreq(noteFrequency);
}

void updateLFOValue()
{
	//Get raw lfo value
	switch (lfoWaveform)
	{
	case 0:
		lfoValue = lSin.next();
		break;
	case 1:
		lfoValue = lSqu.next();
		break;
	case 2:
		lfoValue = lTri.next();
		break;
	case 3:
		lfoValue = lSaw.next();
		break;
	}

	lfoValue *= (lfoScale / 100.0);

	volumeLFOValue = map(lfoValue, -128, 128, 0, gain);

}


void updateLFOFrequency()
{
	lSin.setFreq(lfoFrequency);
	lSaw.setFreq(lfoFrequency);
	lTri.setFreq(lfoFrequency);
	lSqu.setFreq(lfoFrequency);
}

int updateAudio()
{

	int signal;

	if (noteOn)
	{
		volume = gain;
	}


	if ((lfoDestinationIndex == 0 || lfoDestinationIndex == 2) && volume - volumeLFOValue > 0)
		volume -= volumeLFOValue;


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




