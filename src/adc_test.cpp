#include <Arduino.h>
#include <Adafruit_ADS1X15.h>

// ADS1115 Module Instantiation
// ADS1115 @ 0x48 I2C
Adafruit_ADS1115 adsB1;
unsigned bank1Present = 0; // flag to detemine if populated
int16_t adc_B1_bits [ 4 ];
float adc_B1_volts [ 4 ];

// ADS1115 @ 0x49 I2C
Adafruit_ADS1115 adsB2;
unsigned bank2Present = 0; // flag to detemine if populated
int16_t adc_B2_bits [ 4 ];
float adc_B2_volts [ 4 ];

// Initialize the ADS1115 modules, could be up to 2 x ADS1115 at 0x48 and 0x49 
void setupADS1115 ( void )
{
  // Set the ADC gain to 1. By design we expect input range @ the ADS1115 from 0-3.3V 
  // based on the onboard resistor values.
  //
  // Values at terminals: Bank 1 will divide 0-18V into 0-3.3V
  // Values at terminals: Bank 2 will divide 0-5V into 0-3.3V at the ADC
  //
  //                                                                   ADS1115
  //                                                                   -------
  adsB1 .setGain ( GAIN_ONE );        // 1x gain   +/- 4.096V  1 bit = 0.125mV
  adsB2 .setGain ( GAIN_ONE );        // 1x gain   +/- 4.096V  1 bit = 0.125mV
  if (!  adsB1  .  begin  ( 0x48 )) {
    Serial . println ( "Failed to initialize ADS @0x48." );
  } else {
    Serial . println ( "Initialized ADS @0x48 - Bank 1" );
    bank1Present = 1; // flag to indicate if bank 1 is available
  } // end if

  if ( ! adsB2 . begin ( 0x49 )) {
    Serial . println ( "Failed to initialize ADS @0x49." );
  } else {
    Serial . println ( "Initialized ADS @0x49 - bANK 2" );
    bank2Present = 1; // flag to indicate if bank 2 is available
  } // end if
} // end setupADS1115

// run continuous loop polling the available ADS1115 and the oil pressure analog I/O
void adc_test(void) {

  // setup the I2C adc's
  setupADS1115 ();

  while(1) {
    // Now read the adcs values:
    // First read the oil pressure sensor value. 
    // 0 V at terminal = 0 raw bits
    // 5 V at terminals ~= 2516 count (4096 is full scale)

      // Display digital IO
      printf ( "-----------------------------------------------------------\n" );
      printf ( "Analog Input test\n" );
      printf ( "-----------------------------------------------------------\n" );
      printf ( "Pressure Sensor Raw ADC - %d  (1V ~= 570, 5V ~= 3735 )\n", analogRead(34));
      if ( bank1Present == 1 ) {
        printf ( "ADS1115 0x48 (Bank1 0-12V): A0-%d  A1-%d  A2-%d A3-%d  (1V ~= 1470, 5V ~= 7352, 12V ~=17560 )\n", adsB1.readADC_SingleEnded (0), adsB1.readADC_SingleEnded (1), adsB1.readADC_SingleEnded (2), adsB1.readADC_SingleEnded (3));
      } // end if
      if ( bank2Present == 1 ) {
        printf ( "ADS1115 0x49 (Bank2 0-5V): A0-%d  A1-%d  A2-%d A3-%d  (1V ~= 5400, 5V ~= 26600 )\n", adsB2.readADC_SingleEnded (0), adsB2.readADC_SingleEnded (1), adsB2.readADC_SingleEnded (2), adsB2.readADC_SingleEnded (3));
      } // end if

      printf ( "-----------------------------------------------------------\n" );
      
      // check for key press and exit from the loop
      if(Serial.read() != -1)
	    return;

      delay(1000);
    } // end while
} // end adc_test