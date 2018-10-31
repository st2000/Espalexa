/*
 * This is an example on how to use Espalexa and its device declaration methods
 * to control a dimmer of which we only have relative control over.  This
 * specific example sends IR commands to a Lutron wall switch.
 */ 
#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include <Espalexa.h>

#include <IRremoteESP8266.h>
#include <IRsend.h>

// Codes are in Global Cache format less the emitter ID and request ID.
// These codes can be found in GC's Control Tower database.


uint16_t Lutron_favorite[31] = {
	39000,1,15,810,270,90,270,90,360,90,180,90,90,180,90,270,360,810,270,90,270,90,360,90,180,90,90,180,90,270,360};

uint16_t Lutron_lower[27] = {
	39000,1,13,810,270,90,270,90,450,90,360,90,90,90,540,810,270,90,270,90,450,90,360,90,90,90,540};

uint16_t Lutron_off[31] = {
	39000,1,15,810,270,90,270,90,90,360,90,90,270,90,180,90,450,810,270,90,270,90,90,360,90,90,270,90,180,90,450};

uint16_t Lutron_on[15] = {
	39000,1,1,810,270,90,270,90,360,90,180,90,90,270,630};

uint16_t Lutron_raise[27] = {
	39000,1,13,810,270,90,270,90,450,90,270,90,270,90,450,810,270,90,270,90,450,90,270,90,270,90,450};


#define IR_LED 4  // ESP8266 GPIO pin to use. Recommended: 4 (D2).

IRsend irsend(IR_LED);  // Set the GPIO to be used to sending the message.

// prototypes
boolean connectWifi();

// callback functions
void lutronChanged(uint8_t brightness);

// Change this!!  rbf!!
const char* ssid = "ssid";
const char* password = "password";

boolean wifiConnected = false;

Espalexa espalexa;

EspalexaDevice* device1; //this is optional

uint8_t brightness_old = 255/2;

void setup()
{
	irsend.begin();
	Serial.begin(115200);
	// Initialise wifi connection
	wifiConnected = connectWifi();

	if(wifiConnected){

		// Define your devices here. 
		device1 = new EspalexaDevice("lutron light", lutronChanged);
		espalexa.addDevice(device1);
		device1->setValue(128);

		espalexa.begin();

	} else
	{
		while (1) {
			Serial.println("Cannot connect to WiFi. Please check data and reset the ESP.");
			delay(2500);
		}
	}
}

void loop()
{
	espalexa.loop();
	delay(1);
}

// Our callback functions.
void lutronChanged(uint8_t brightness) 
{
	// Turn on if user says 100% or if user says on.
	if((brightness == 255) || (brightness == 128))
	{
		// Increase brightness to full.
		irsend.sendGC(Lutron_on, 15);
		Serial.print("Turn on lutron ");
		Serial.println(brightness);
	}
	else
	{
		// Turn off if user says off.
		if(brightness == 0)
		{
			// Decrease brightness to off.
			irsend.sendGC(Lutron_off, 31);
			Serial.print("Turn off lutron ");
			Serial.println(brightness);
		}
		else
		{
			// We always exit set to 128.  So if brighter the value will likely
			// be more then 128 but less then 255.
			if(brightness > 128)
			{
				// Increase brightness.
				irsend.sendGC(Lutron_raise, 27);
				Serial.print("Turn up lutron ");
				Serial.println(brightness);
			}
			// We always exist set to 128.  So if dimmer the value will likely
			// be less then 128 but more then 0.
			else
			{
				// Decrease brightness.
				irsend.sendGC(Lutron_lower, 27);
				Serial.print("Turn down lutron ");
				Serial.println(brightness);
			}
		}
	}
	// Update the state of brightness to 128 every time.
	// This allows us to always test up/down relative to 128 for
	// brightness up/down control w/o running into the limit of 
	// Alexa's range.
	device1->setValue(128);
}

// connect to wifi – returns true if successful or false if not
boolean connectWifi(){
	boolean state = true;
	int i = 0;

	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	Serial.println("");
	Serial.println("Connecting to WiFi");

	// Wait for connection
	Serial.print("Connecting...");
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
		if (i > 20){
			state = false; break;
		}
		i++;
	}
	Serial.println("");
	if (state){
		Serial.print("Connected to ");
		Serial.println(ssid);
		Serial.print("IP address: ");
		Serial.println(WiFi.localIP());
	}
	else {
		Serial.println("Connection failed.");
	}
	return state;
}
