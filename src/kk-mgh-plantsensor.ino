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

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#include <Wire.h>

#include <Adafruit_TSL2561_U.h>

#define CHIRP_RESET 6

#define DHTPIN 5
#define DHTTYPE           DHT11

#define FACTORYRESET_ENABLE         1
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"

Adafruit_BluefruitLE_SPI ble( BLUEFRUIT_SPI_CS , BLUEFRUIT_SPI_IRQ , BLUEFRUIT_SPI_RST );
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

DHT_Unified dht(DHTPIN, DHTTYPE);

void error( const __FlashStringHelper*err ) {
	Serial.println( err );
	while( 1 );
}

unsigned int readI2CRegister16bit(int addr, int reg) {
	Wire.beginTransmission(addr);
	Wire.write(reg);
	Wire.endTransmission();
	delay(1100);
	Wire.requestFrom(addr, 2);
	unsigned int t = Wire.read() << 8;
	t = t | Wire.read();
	return t;
}

void setup(void) {

	while (!Serial);
	delay(500);
	Serial.begin(115200);

	Serial.print( F("Initialising I2C ->" ) );
	Wire.begin();
	Serial.println( F("OK") );

	Serial.print( F("Initialising CHIRP ->" ) );
	pinMode( CHIRP_RESET , OUTPUT );
	digitalWrite( CHIRP_RESET , LOW );
	delay( 100 );
	digitalWrite( CHIRP_RESET , HIGH );
	delay( 500 );
	readI2CRegister16bit( 0x20 , 0 );
	Serial.println( F("OK") );

	Serial.print( F( "Moisture Level -> " ) );
	uint32_t chirpValue = readI2CRegister16bit( 0x20 , 0 );
	Serial.println( chirpValue );

	Serial.print( F("Initialising DHT11 ->" ) );
	dht.begin();
	Serial.println( F("OK") );

	sensors_event_t event;
  	dht.temperature().getEvent(&event);

	if( isnan( event.temperature ) ) {
    	Serial.println("Error reading temperature!");
  	} else {
    	Serial.print("Temperature: ");
		Serial.print(event.temperature);
    	Serial.println(" *C");
  	}

	dht.humidity().getEvent(&event);
  	if( isnan( event.relative_humidity ) ) {
    	Serial.println( "Error reading humidity!" );
  	} else {
    	Serial.print("Humidity: ");
    	Serial.print(event.relative_humidity);
    	Serial.println("%");
  	}

	Serial.print( F( "Initialising TSL2561 -> " ) );
	tsl.begin();
	tsl.enableAutoRange(true);
    tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);
	Serial.println( F("OK") );

	Serial.print( F( "Light Level -> " ) );
	tsl.getEvent( &event );
	Serial.print( event.light );
	Serial.println(" Lux");

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

			sensors_event_t event;

			dht.temperature().getEvent( &event );
			int temperature = event.temperature;

			dht.humidity().getEvent( &event );
			int humidity = event.relative_humidity;

			uint32_t moisture = readI2CRegister16bit( 0x20 , 0 );

			tsl.getEvent( &event );
			int light = event.light;

			String answer = "A:";
			answer += moisture;
			answer += "/";
			answer += temperature;
			answer += "/";
			answer += humidity;
			answer += "/";
			answer += light;

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
