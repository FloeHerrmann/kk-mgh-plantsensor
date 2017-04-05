/*********************************************************************
 This is an example for our nRF51822 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

#include <Arduino.h>
#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
	#include <SoftwareSerial.h>
#endif

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

#define FACTORYRESET_ENABLE         1
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"

Adafruit_BluefruitLE_SPI ble( BLUEFRUIT_SPI_CS , BLUEFRUIT_SPI_IRQ , BLUEFRUIT_SPI_RST );

void error( const __FlashStringHelper*err ) {
	Serial.println( err );
	while( 1 );
}

void setup(void) {

	while (!Serial);
	delay(500);
	Serial.begin(115200);

	Serial.print(F("Initialising the Bluefruit LE module -> "));

	if( !ble.begin( VERBOSE_MODE ) ) {
		error( F( "Couldn't find Bluefruit, make sure it's in Command mode & check wiring" ) );
	} else {
		Serial.println( F("OK") );
	}

	ble.verbose( false );
	ble.echo( false );

	if ( FACTORYRESET_ENABLE ) {
		Serial.print( F( "Performing a factory reset -> " ) );
		if ( !ble.factoryReset() ){
			error( F( "Couldn't factory reset" ) );
		} else {
			Serial.println( F("OK") );
		}
	}

	Serial.println( F( "----------------" ) );
	Serial.println( F( "Bluefruit info" ) );
	ble.info();



	while( !ble.isConnected() ) {
			delay(500);
	}

	if ( ble.isVersionAtLeast( MINIMUM_FIRMWARE_VERSION ) ){
		ble.sendCommandCheckOK( "AT+HWModeLED=" MODE_LED_BEHAVIOUR );
	}

	Serial.println( F("DATA Mode") );
	ble.setMode(BLUEFRUIT_MODE_DATA);

	Serial.println( F( "----------------" ) );
}

void loop(void) {
	char outputs[ BUFFERSIZE + 1 ];

	if( ble.available() ) {
		String receivedCmd = "";
		while( ble.available() ) {
			int c = ble.read();
			receivedCmd += (char)c;
		}
		receivedCmd.replace( "\r" , "" );
		receivedCmd.replace( "\n" , "" );

		Serial.print( "ReceivedCmd -> " );
		Serial.println( receivedCmd );

		if( receivedCmd.equals( "C:GetData" ) ) {
			String answer = "A:123/12.3/12/12345";

			//Serial.print( "Answer Length -> " );
			//Serial.println( answer.length() + 1 );

			//answer.toCharArray( outputs , answer.length() );
			//outputs[ answer.length() + 1 ] =  0;

			Serial.print( "Send Data -> " );
			Serial.println( answer );

			ble.println( answer );
		}
	}
}
