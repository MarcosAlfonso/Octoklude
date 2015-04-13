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

#include <IntMap.h>

#define CONTROL_RATE 64


// initialize the library with the numbers of the interface pins 
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
EventDelay infrequentDelay;

//Arduino Vars
int switchPin = 10;

LowPassFilter lpf;

//Synth Vars
Oscil <SIN256_NUM_CELLS, AUDIO_RATE> aSin(SIN256_DATA);
Oscil <SAW256_NUM_CELLS, AUDIO_RATE> aSaw(SAW256_DATA);
Oscil <TRIANGLE_ANALOGUE512_NUM_CELLS, AUDIO_RATE> aTri(TRIANGLE_ANALOGUE512_DATA);
Oscil <SQUARE_ANALOGUE512_NUM_CELLS, AUDIO_RATE> aSqu(SQUARE_ANALOGUE512_DATA);

byte volume = 50; //0 - 255
byte waveform = 0; // 0 - 3 
byte cutoffFreq = 255;
int noteLength = 1000; // ~10 - ~2000
int freq = 440; //~20 - ~4000
bool isSynthMode;

//Synth Strings
String waveformStrings[] = { "Sine", "Square", "Triangle", "Sawtooth" };

byte CScaleIndex = 0;
int CScaleFreq[] { 261, 297, 330, 349, 392, 440, 494, 523, 494, 440, 392, 349, 330, 297, 261 };

//Int Mappings
IntMap mapTo255 = IntMap(0, 1023, 0, 255);
int noteLengthMax = 1000;
IntMap mapToNoteLength = IntMap(0, 1023, 0, 925);
IntMap mapFreq = IntMap(0, 1023, 27, 4000);

//Sequencer Vars
EventDelay noteLengthDelay;
bool noteOn;

/*
String Keys[] = { "A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#" };
int KeysLength = 12;

String Modes[] = { "Major", "Dorian", "Phyrgian", "Lydian", "Mixolydian", "Minor", "Locrian" };
int ModesLength = 7;

int seqLengthMin = 1;
int seqLengthMax = 64;

int BpmMin = 40;
int BpmMax = 220;
*/

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
	delay(1500);

	lcd.clear();

	infrequentDelay.set(100);

	noteLengthDelay.set(noteLengthMax - noteLength);

	aSin.setFreq(CScaleFreq[CScaleIndex]);
	aSaw.setFreq(CScaleFreq[CScaleIndex]);
	aTri.setFreq(CScaleFreq[CScaleIndex]);
	aSqu.setFreq(CScaleFreq[CScaleIndex]);

	lpf.setCutoffFreq(cutoffFreq);
	startMozzi(CONTROL_RATE);

}

void infrequentControl()
{
	isSynthMode = digitalRead(switchPin);

	lcd.clear();
	lcd.setCursor(0, 0);

	if (isSynthMode)
	{
		switch (mostRecentPot)
		{
		case 0:
			lcd.print("Waveform:");
			lcd.setCursor(0, 1);
			lcd.print(waveformStrings[waveform]);
			break;

		case 1:
			lcd.print("Filter Freq:");
			lcd.setCursor(0, 1);
			lcd.print(cutoffFreq);
			break;

		default:
			break;
		}
	}
	else
	{
		lcd.clear();
		lcd.setCursor(0, 0);
		switch (mostRecentPot)
		{
		case 0:
			lcd.print("BPM:");
			lcd.setCursor(0, 1);
			lcd.print(60000.0/noteLength);
			break;

		case 1:
			lcd.print("Frequency:");
			lcd.setCursor(0, 1);
			lcd.print(freq);
			break;

		default:
			break;
		}

	}


}

//!!!
//This whole function I think could be a lot more efficient if we used switches for most recent pots, not sure though
void frequentControl()
{
	//updatePots ultimately uses analogRead, which Mozzi overrides. I had to comment a line in MozziGuts.cpp:startMozzi(control_rate) to make this work. Could be too slow later?
	//if a pot has changed, go in and change shiz
	if (updatePots())
	{
		//Synth Control Logic Here
		if (isSynthMode)
		{
			waveform = map(potVals[0], 0, 1023, 0, 4);
			cutoffFreq = mapTo255(potVals[1]);

			lpf.setCutoffFreq(cutoffFreq);

		}
		//Sequencer Control Logic Here
		else
		{
			noteLength = noteLengthMax - mapToNoteLength(potVals[0]);

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
			CScaleIndex++;
			volume = 50;
			aSin.setFreq(CScaleFreq[CScaleIndex % 15]);
			aSaw.setFreq(CScaleFreq[CScaleIndex % 15]);
			aTri.setFreq(CScaleFreq[CScaleIndex % 15]);
			aSqu.setFreq(CScaleFreq[CScaleIndex % 15]);
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
bool updatePots()
{

	bool potChanged = false;
	int epsilon = 10;

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

	return potChanged;
}




