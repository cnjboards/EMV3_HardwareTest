#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// For Onewire bus and DS1820B temperature sensors
OneWire oneWire ( 17 );
DallasTemperature DS18B20 ( & oneWire );

// forward declarations
void SetupDS18B20 (void);
String GetAddressToString ( DeviceAddress );

// Onewire global defines
#define ONE_WIRE_MAX_DEV 15                 // The maximum number of devices
#define MISREADCNTLIMIT 5                   // number of misreads allowed in 30 attempts
#define MISREADCNTRESET 30                  // used to reset the misread count
int numberOfDevices;                        // Number of temperature devices found
int misReadCnt = 0 , misReadCntReset = 0;
float owTempValues[ONE_WIRE_MAX_DEV] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // storage of current temperature values
DeviceAddress owDevAddr[ONE_WIRE_MAX_DEV]; // array to hold the device addresses

void one_wire_scan(uint8_t pin) {

Serial.println("Scanning for one wire devices.");
Serial.println("One-Wire pin number is 17");
Serial.println("Remember to connect a 4.7K resistor between VCC and OneWire data!");

// start Onewire
DS18B20 . begin ();

  Serial.println ( "Parasite power is: " );
  if ( DS18B20 . isParasitePowerMode ()) {
    Serial . println ( "ON" );
  } else {
    Serial . println ( "OFF" );
  }

  // capture how many devices are on the bus

  numberOfDevices = DS18B20 . getDeviceCount ();

  // if no devices found then return
  if ( numberOfDevices == 0 ) {
    Serial . println ( "No onewire devices found!" );
    return;
  } // end if

  // print number of devices found
  Serial . print ( "Device count: " );
  Serial . println ( numberOfDevices );

  // assume temperature so read all temperatures
  DS18B20 . requestTemperatures ();

  // Loop through each device, print out address etc
  for ( int i = 0 ; i < numberOfDevices ; i++ ) {
    // poll each device
    if ( DS18B20 . getAddress ( owDevAddr[i] , i )) {
      Serial . print ( "Found device " );
      Serial . print ( i+1 , DEC );
      Serial . print ( " with address: " + GetAddressToString ( owDevAddr[i] ));
      Serial.print ( "  Resolution: " );
      Serial.print ( DS18B20 . getResolution ( owDevAddr[i] ));
      float tempC = DS18B20.getTempC ( owDevAddr[i] );
      Serial . print ( "  Temp C: " );
      Serial . println ( tempC );
    } else {
      Serial . print ( "Found ghost device at " );
      Serial . print ( i+1 , DEC );
      Serial . print ( " but could not detect address. Check power and cabling" );
    } // end if
  }  // End for
}  // End SetupDS18B20

// Convert onewire device id to String

String GetAddressToString ( DeviceAddress deviceAddress )
{
  String str = "";
  for ( uint8_t i = 0 ; i < 8 ; i++ ) {
    if ( deviceAddress [ i ] < 16 )
      str += String ( 0 , HEX );
    str += String (deviceAddress [ i ] , HEX );
  }
  return str;
} // End GetAddressToString