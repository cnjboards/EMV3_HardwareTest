#include <Arduino.h>

// this module test all the digital IO on the EM V3
// pin1 and pin2 - outputs 1 and 2
// pin3 and pin4 - inputs 1 and 2
void blink_start(int pin1, int pin2, int pin3, int pin4) {
  pinMode(pin1, OUTPUT_OPEN_DRAIN); // output 1
  pinMode(pin2, OUTPUT_OPEN_DRAIN); // output 2
  pinMode(pin3, INPUT_PULLDOWN); // input 1
  pinMode(pin4, INPUT_PULLDOWN); // input 2
  pinMode(15, INPUT_PULLUP); // soft reset input
  pinMode ( 27 , INPUT ); // rpm input

  int current1 = LOW;
  int current2 = HIGH;

  while(1) {
      digitalWrite(pin1, current1);
      if(current1 == LOW)
	      current1 = HIGH;
      else
	      current1 = LOW;
      
      digitalWrite(pin2, current2);
      if(current2 == LOW)
	      current2 = HIGH;
      else
	      current2 = LOW;
      
      // Display digital IO
      printf ( "-----------------------------------------------------------\n" );
      printf ( "Digital IO test: Outputs toggle every second\n" );
      printf ( "-----------------------------------------------------------\n" );
      printf ( "Digital Inputs In1-%d In2-%d Alt-%d (Note: 1 means input is low (0V), 0 means input is high (12V))\n", digitalRead(pin3), digitalRead(pin4), digitalRead(27));
      printf ( "Factory Reset Input-%d\n" , digitalRead(15));
      printf ( "Digital Outputs Out1-%d Out2-%d (Note: 1 means relay is off, 0 means relay is on)\n" , digitalRead(pin1), digitalRead(pin2));
      printf ( "-----------------------------------------------------------\n" );
      
      // check for key pess
      if(Serial.read() != -1)
	return;

      delay(2000);
    } // end while
} // end blink start

