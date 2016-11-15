/*

Name:		RCSocketSwitch.ino
Created:	5/15/2016 3:31:44 PM
Author:	Andreas Doetsch


Switch a RC Socket on or off

This a simple example of the aREST Library in combination with the RC-Switch Library for the ESP8266.
Adadruit Huzzah Board

Webbrowser Command

ON:   http://192.168.1.240:80/switchOn?params=1111100000
OFF:  http://192.168.1.240:80/switchOff?params=1111100000

*/

// Debug mode
#ifndef DEBUG_MODE
#define DEBUG_MODE 0
#endif

// Use light answer mode
#ifndef LIGHTWEIGHT
#define LIGHTWEIGHT 1
#endif

// Import required libraries
#include <ESP8266WiFi.h>
#include <aREST.h>
#include <RCSwitch.h>
#include <string>

// Create RC switch instance
RCSwitch rcSwitch = RCSwitch();

// Create aREST instance
aREST rest = aREST();

// WiFi parameters
const char* ssid = "GVT-80D1";
const char* password = "01D07044665";

// The port to listen for incoming TCP connections 
#define LISTEN_PORT           80

// Create an instance of the server
WiFiServer server(LISTEN_PORT);

// Declare functions to be exposed to the RC-API
int rcSwitchOn(String command);
int rcSwitchOff(String command);

// save last command
String rcLastCommand = "";
int rcLastMode = -1;

int ledPinGreen = 13;
int ledPinYellow = 12;
int ledPinRed = 14;

// setup
void setup(void)
{

    pinMode(ledPinGreen, OUTPUT);
    digitalWrite(ledPinGreen, LOW);
    pinMode(ledPinYellow, OUTPUT);
    digitalWrite(ledPinYellow, LOW);
    pinMode(ledPinRed, OUTPUT);
    digitalWrite(ledPinRed, HIGH);


    // Start Serial
    Serial.begin(115200);
    delay(5);

    rcSwitch.enableTransmit(5);

    // Give name and ID to device
    rest.set_id("RCSS");
    rest.set_name("esp8266");
    rest.function("switchOn", rcSwitchOn);
    rest.function("switchOff", rcSwitchOff);

    delay(5);


    // Connect to WiFi
    WiFi.begin(ssid, password);
    // static ip: IP , GATEWAY , SUBNET
    WiFi.config(IPAddress(192, 168, 1, 240), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(100);
        digitalWrite(ledPinRed, HIGH);
        delay(500);
        Serial.print(".");
        digitalWrite(ledPinRed, LOW);
    }

    digitalWrite(ledPinRed, LOW);
    Serial.println("");
    Serial.println("WiFi connected");

    // Start the server
    server.begin();
    Serial.println("Server started");

    // Print the IP address
    Serial.println(WiFi.localIP());
    digitalWrite(ledPinGreen, HIGH);
}

void loop()
{
    // Handle REST calls
    WiFiClient client = server.available();
    if (!client)
    {
        return;
    }
    while (!client.available())
    {
        delay(1);
    }


    rest.handle(client);

}

/*
* Switch a device on or off
* #param command_  dipswitches of device 10 chars 1st 5 = groupcode, last 5 devicecode
* #param mode_  0=switch off 1=switch on
* #return  > 0 all ok <0 error 0 nothing done
*
*/
int rcSwitchOnOff(String command_, int mode_)
{
    int returnValue = 0;
    char groupCode[6] = "\0\0\0\0\0";
    char deviceCode[6] = "\0\0\0\0\0";

    digitalWrite(ledPinYellow, HIGH);

    if (command_ != NULL)
    {
        if (command_.length() == 10)
        {
            if (!rcLastCommand.equals(command_) || (rcLastMode != mode_))
            {
                command_.substring(0, 5).toCharArray(groupCode, 6);
                command_.substring(5, 10).toCharArray(deviceCode, 6);
                if ((mode_ != 0) && (mode_ != 1))
                {
                    returnValue = -3; // invalid switchmode 0=off; 1=on
                }
                else
                {
                    if (mode_ == 0)
                    {
                        rcSwitch.switchOff(groupCode, deviceCode);
                    }
                    else
                    {
                        rcSwitch.switchOn(groupCode, deviceCode);
                    }
                    rcLastCommand = command_;
                    rcLastMode = mode_;
                    returnValue = 1; // should be fine
                }
            }
        }
        else
        {
            returnValue = -2; // length error  
        }
    }
    else
    {
        returnValue = -1; // NULL value error
    }

    if (DEBUG_MODE)
    {
        Serial.println("rcSwitchOnOff(String <<" + command_ + ">>, int <<" + mode_ + ">>) return=" + String(returnValue));
        if (returnValue == 1)
        {
            Serial.println("groupCode=" + String(groupCode));
            Serial.println("deviceCode=" + String(deviceCode));
        }
    }

    digitalWrite(ledPinYellow, LOW);
    return returnValue;
}

// switch on a RC socket
int rcSwitchOn(String command)
{
    int returnValue = rcSwitchOnOff(command, 1);
    delay(150);
    return returnValue;
}


// switch off a RC socket
int rcSwitchOff(String command)
{
    int returnValue = rcSwitchOnOff(command, 0);
    delay(150);
    return returnValue;
}
