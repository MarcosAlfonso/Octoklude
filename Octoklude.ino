#include <PotMUX.h>
#include <LiquidCrystal.h>

#include <MozziGuts.h>
#include <Oscil.h>
#include <tables/saw256_int8.h> // saw table for oscillator
#include <tables/sin256_int8.h> // sin table for oscillator
#include <tables/triangle_analogue512_int8.h> // tri table for oscillator
#include <tables/square_analogue512_int8.h> // square table for oscillator


// initialize the library with the numbers of the interface pins 
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);


//Synth Vars
#define CONTROL_RATE 256
Oscil <SIN256_NUM_CELLS, AUDIO_RATE> aSin(SIN256_DATA);
Oscil <SAW256_NUM_CELLS, AUDIO_RATE> aSaw(SAW256_DATA);
Oscil <TRIANGLE_ANALOGUE512_NUM_CELLS, AUDIO_RATE> aTri(TRIANGLE_ANALOGUE512_DATA);
Oscil <SQUARE_ANALOGUE512_NUM_CELLS, AUDIO_RATE> aSqu(SQUARE_ANALOGUE512_DATA);

byte volume = 128; //0 - 255
byte waveform = 0; // 0 - 3 
int freq = 440; //~20 - ~4000


//Sequencer Vars
String Keys[] = { "A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#" };
int KeysLength = 12;

String Modes[] = { "Major", "Dorian", "Phyrgian", "Lydian", "Mixolydian", "Minor", "Locrian" };
int ModesLength = 7;

int seqLengthMin = 1;
int seqLengthMax = 64;

int BpmMin = 40;
int BpmMax = 220;

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


	aSin.setFreq(freq);
	startMozzi(CONTROL_RATE);

}

void updateControl()
{
	//updatePots ultimately uses analogRead, which Mozzi overrides. I had to comment a line in MozziGuts.cpp:startMozzi(control_rate) to make this work. Could be too slow later?
	updatePots();

	volume = map(potVals[0], 0, 1023, 0, 255);
	waveform = map(potVals[1], 0, 1023, 0, 3);
	freq = map(potVals[2], 0, 1023, 27, 4000);

	aSin.setFreq(freq);
	aSaw.setFreq(freq);
	aTri.setFreq(freq);
	aSqu.setFreq(freq);
	
}

int updateAudio()
{
	switch (waveform)
	{
	case 0:
		return ((int)aSin.next() * volume) >> 8;
		break;
	case 1:
		return ((int)aSqu.next() * volume) >> 8;
		break;
	case 2:
		return ((int)aTri.next() * volume) >> 8;
		break;
	case 3:
		return ((int)aSaw.next() * volume) >> 8;
		break;
	}
}

void loop()
{
	audioHook();
}

//Checks the state of all Pots, compares them to previous, if any are changed (>epsilon) new value is stored, returns true
bool updatePots()
{

	bool potChanged = false;
	int epsilon = 3;

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


/*
delay(25);

bool updateLCD = updatePots();
int mappedValue = 0;

if (updateLCD)
{
//Sequencer Settings Roughed out
switch (mostRecentPot) {
case 0:
lcd.clear();
lcd.setCursor(0, 0);
mappedValue = map(potVals[mostRecentPot], 0, 1023, 0, KeysLength);
lcd.print("Key: " + Keys[mappedValue]);
break;

case 1:
lcd.clear();
lcd.setCursor(0, 0);
mappedValue = map(potVals[mostRecentPot], 0, 1023, 0, ModesLength);
lcd.print("Mode: " + Modes[mappedValue]);
break;

case 2:
lcd.clear();
lcd.setCursor(0, 0);
mappedValue = map(potVals[mostRecentPot], 0, 1023, seqLengthMin, seqLengthMax);
lcd.print("Seq Length: " + (String)mappedValue);
break;



case 3:
lcd.clear();
lcd.setCursor(0, 0);
mappedValue = map(potVals[mostRecentPot], 0, 1023, BpmMin, BpmMax);
lcd.print("BPM: " + (String)mappedValue);
break;
/*
case 4:
lcd.clear();
lcd.setCursor(0, 0);
mappedValue = map(potVals[mostRecentPot], 0, 1023, 0, seqModesLength);
lcd.print("Mode: " + seqModes[mappedValue]);
break;

case 5:
lcd.clear();
lcd.setCursor(0, 0);
mappedValue = map(potVals[mostRecentPot], 0, 1023, 0, seqModesLength);
lcd.print("Mode: " + seqModes[mappedValue]);
break;

case 6:
lcd.clear();
lcd.setCursor(0, 0);
mappedValue = map(potVals[mostRecentPot], 0, 1023, 0, seqModesLength);
lcd.print("Mode: " + seqModes[mappedValue]);
break;

case 7:
lcd.clear();
lcd.setCursor(0, 0);
mappedValue = map(potVals[mostRecentPot], 0, 1023, 0, seqModesLength);
lcd.print("Mode: " + seqModes[mappedValue]);
break;
*/





