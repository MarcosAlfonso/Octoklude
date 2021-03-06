// LCD_Strings.h

#ifndef _LCD_STRINGS_h
#define _LCD_STRINGS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif


#endif

#include <avr/pgmspace.h>

//Waveform Strings
const char wave0[] PROGMEM = "Sine";
const char wave1[] PROGMEM = "Square";
const char wave2[] PROGMEM = "Triangle";
const char wave3[] PROGMEM = "Sawtooth";
const char* const waveformStrings[] PROGMEM = { wave0, wave1, wave2, wave3 };

//LFO Destination Strings
const char lfoDest0[] PROGMEM = "Gain";
const char lfoDest1[] PROGMEM = "Pitch";
const char lfoDest2[] PROGMEM = "Gain + Pitch";
const char* const lfoDestStrings[] PROGMEM = { lfoDest0, lfoDest1, lfoDest2 };


//Key Strings
const char key0[] PROGMEM = "A";
const char key1[] PROGMEM = "A#";
const char key2[] PROGMEM = "B";
const char key3[] PROGMEM = "C";
const char key4[] PROGMEM = "C#";
const char key5[] PROGMEM = "D";
const char key6[] PROGMEM = "D#";
const char key7[] PROGMEM = "E";
const char key8[] PROGMEM = "F";
const char key9[] PROGMEM = "F#";
const char key10[] PROGMEM = "G";
const char key11[] PROGMEM = "G#";
const char* const keyStrings[] PROGMEM = { key0, key1, key2, key3, key4, key5, key6, key7, key8, key9, key10, key11 };

//Octave Offset Strings
const char octOff0[] PROGMEM = "-4";
const char octOff1[] PROGMEM = "-3";
const char octOff2[] PROGMEM = "-2";
const char octOff3[] PROGMEM = "-1";
const char octOff4[] PROGMEM = "0";
const char octOff5[] PROGMEM = "+1";
const char octOff6[] PROGMEM = "+2";
const char octOff7[] PROGMEM = "+3";
const char octOff8[] PROGMEM = "+4";
const char* const octaveOffsetStrings[] PROGMEM = { octOff0, octOff1, octOff2, octOff3, octOff4, octOff5, octOff6, octOff7, octOff8 };

//Pattern Strings
const char pattern0[] PROGMEM = "Up";
const char pattern1[] PROGMEM = "Down";
const char pattern2[] PROGMEM = "Up + Down";
const char pattern3[] PROGMEM = "Random";
const char* const patternStrings[] PROGMEM = { pattern0, pattern1, pattern2, pattern3 };

//Insert Strings
const char insert0[] PROGMEM = "Off";
const char insert1[] PROGMEM = "Low";
const char insert2[] PROGMEM = "Middle";
const char insert3[] PROGMEM = "High";
const char insert4[] PROGMEM = "Random";
const char* const insertStrings[] PROGMEM = { insert0, insert1, insert2, insert3, insert4 };



