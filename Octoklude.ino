#include <PotMUX.h>

#include <LiquidCrystal.h>
// initialize the library with the numbers of the interface pins 
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

#define NUM_POTS 8

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
int potVals[] = { -1, -1, -1, -1, -1, -1, -1, -1 };
int mostRecentPot = 0;

void setup(){
	Pot.Initialize();

	// set up the LCD's number of rows and columns: 
	lcd.begin(16, 2);

	//set midi baud rate
	Serial.begin(9600);

	//Home message
	lcd.setCursor(0, 0);
	lcd.print("   Welcome to   ");
	lcd.setCursor(0, 1);
	lcd.print(" the  Octoklude ");
	delay(2500);

	lcd.clear();

}

void loop()
{

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

		}
	}
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






