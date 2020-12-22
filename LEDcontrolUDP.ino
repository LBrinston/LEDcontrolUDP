#include <Arduino.h> 
#include <WiFi.h>
#include <FastLED.h>
#include <AsyncUDP.h>
#include "creds.h"

#define NUM_LEDS 2
#define DATA_PIN 25

/*
* Packet design:
* "###LED#0xRRGGBBLED#0xRRGGBB"
* 3 byte, 1 byte, 3 bytes, 1 byte, 3 bytes = 14 bytes <- for two leds
*
* If LED# is a 1 byte hex value we could address up to 64 LEDs with one variable
*/

// Object instantiations
CRGB leds[NUM_LEDS];
AsyncUDP udp;

// Prototypes
String convertInto2ByteString(int number);
String convertIntTo3ByteString(int number); 
String convertIntTo4ByteString(int number);
String checkColourCode (CRGB *led);
void receivePacket(AsyncUDPPacket packet);
bool isHexChar(char ch);
uint32_t hex2int(const char *hex);

// Variables
int packetNumber;

// LED vars
int ledIndex;   // Index of LED to address

unsigned int packetLen;

int ledInReq = NUM_LEDS; // Set to the max # of LEDS to control

// Packet vars
int checkSum = 0;
int PacketIndex = 0;
int TXstate = 0;
String outPacket;

uint8_t inByte = 0;

int RXstate=0; // Tracks progression through packet reconstruction
int RXindex=0;
String inPacket; // Holds the data parsed out of our UDP packet
int inPacketLength; // Holds the length of that packet
int calChkSum = 0; // Holds the value of the ChkSum calculated during parsing of the recieved packet
int reChkSum = 0; // Holds the value of the ChkSum sent with the UDP packet
 
// Timing vars
unsigned long previousMillis=0;
const long interval=100; // change this to 100 if needed
 
// WiFi and UDP vars
int udpPort = 1234;
boolean connected = false;//Are we currently connected?


void setup() {

  // Configure our LED object
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  Serial.begin(115200);
  connectToWiFi(mySSID, myPASSWORD);
  
  
  inPacket=""; 

  // put your setup code here, to run once:
/*  
  for (int pin = 0; pin < digitalInReq; pin++)
  {
	  pinMode(pin, INPUT); // set the number of input pins required
  }
  for(int pin=8;pin<14;pin++)
  {
    pinMode(pin,OUTPUT); // set the number of ouput pins required
  }
  digitalWrite(12, LOW); //flash 2LEDS acktive low for 1 Sec
  digitalWrite(13, LOW);
  delay(1000);
  digitalWrite(12, HIGH);
  digitalWrite(13, HIGH);
*/
}

void loop() {
	// put your main code here, to run repeatedly:
  unsigned long currentMillis = millis();
	
/*
* Packet design:
* "###LED#0xRRGGBBLED#0xRRGGBB"
* 3 byte, 1 byte, 3 bytes, 1 byte, 3 bytes = 14 bytes <- for two leds
*
* If LED# is a 1 byte hex value we could address up to 64 LEDs with one variable
*/
  
//  switch(TXstate)
//	{
//	case 0: // begin making output packet
//		outPacket = "###"; //header
//		checkSum = 0;
//		PacketIndex = 0;
//		analogPin=0;
//    //digitalPin=0;
//    
//    ledIndex = 0; // FastLED uses an ledArray object that starts at index 0
//    digitalPin=digitalInReq -1;//start with Most significant pin 
//
//		outPacket += convertIntTo3ByteString(packetNumber++); //inc packet number add to outPacket string
//		packetNumber %= 1000;   //packetnumber rollover code 
//		TXstate = 1;  //move to next state
//		break;
//
//  // ledIndex reporting
//	case 1:	// continue making output packet and start with index of first LED to control
//    //outPacket += convertIntTo4ByteString(analogRead(analogPin++));
//    outPacket += convertIntTo2ByteString(ledIndex);
//    
//    // In OG protocol Wayne reported all analog pin states sequentially. We want to alternate ledInex, ledColour, etc
//    // Until ledIndex = NUM_LEDS?
//    // Then we proceed to chkSum
//    // Can probably unconditionally proceed to colour reporting?
//
//    if(ledIndex == (NUM_LEDS-1)){
//      TXstate = 3; // Proceed to chkSum
//    }
//    else
//    {
//      TXstate = 2; // Proceed to ledColour reporting
//    }
//    
//    /* Original
//    if (analogPin == analogReq) // In OG protocol Wayne reported all analog pin states sequentially. We want to alternate ledInex, ledColour, 
//		{
//			TXstate = 2;// move to next state when all analog complete
//		}*/
//		break;
//
//  // ledColour reporting
//	case 2:
//		outPacket += checkColourCode(&leds[ledIndex]);
//    ledIndex++; // Increase our ledIndex
//
//		if (ledIndex == (NUM_LEDS-1))
//		{
//			TXstate = 3;// Proceed to chkSum
//		}
//    else 
//    {
//      TXstate = 2; // Go back to ledIndex reporting
//    }
//		break;
//	case 3:
//    for(int i = 3; i < 38; i++)
//    {
//      checkSum +=(byte)outPacket[i];//calculate check sum
//    }
//    checkSum %= 1000; //trucate check sum to 3 digits
//		outPacket += convertIntTo3ByteString(checkSum);
//		outPacket += "\r\n";// add carriage return, line feed
//		packetLen = outPacket.length();//set packet length to send
//    TXstate = 4; //move to next state
//		break;
//	case 4: // stay in case 4 until entire packet is sent and interval time expires
//		if (PacketIndex == packetLen)// when entire packet is sent check interval
//		{
//        //https://www.arduino.cc/en/tutorial/BlinkWithoutDelay
//        if (currentMillis - previousMillis >= interval) 
//        {
//          // save the last time the packet was sent
//          previousMillis = currentMillis;
//			    TXstate = 0; //reset the state when the whole packet is sent and after the interval
//        }  
//		}
//		break;
//	}
//  if (outPacket[PacketIndex]!=0)// if a packet is available send it
//  {
//    // replace with UDP
//	  Serial.write(outPacket[PacketIndex++]);//blocking function send one byte at a time
//  }
  // replace with UDP
  
  if (udp.listen(udpPort)){
    udp.onPacket(receivePacket); // callback - call receivePacket when there is a UDP Packet      
  }
  /*
  if (Serial.available() > 0) // check if any bytes available to receive
  {
    receivePacket(); // call receive function
  }*/
 
}

void receivePacket(AsyncUDPPacket packet)
{   
  uint8_t *data = packet.data(); // copy pointer to the data
  int dataLength = packet.length(); // copy the length of that data
  String rxChkSum = "";

  // Before parsing retrieve the ChkSum sent with the packet
  // ###00FFFFFF01FFFFFFChkSum/c/r
  // CheckSum = 3 bytes
  for (int j = (dataLength-5); j < (dataLength-2);j++){
    rxChkSum += (char) data[j];
  }
  reChkSum = rxChkSum.toInt();
  Serial.println(reChkSum);

  // Works - prints out correct data that is sent via UDP
  // Parse out the recieved packet inside this loop minus the checksum and \c\r
  for (int i; i<(dataLength-2); i++){
    packetParse(data[i]); // Pass by value
  }
  Serial.println("Exit parse");
}

// Data Wrangling Functions
//////////////////////////////////////////////////////////////////
String convertIntTo4ByteString(int number)
{
	char byteString[5] = "";
  sprintf(byteString,"%04d",number);
	return byteString;
}

String convertIntTo3ByteString(int number)
{
	char byteString[4] = "";
  sprintf(byteString,"%03d",number);
	return byteString;
}

String convertIntTo2ByteString(int number)
{
	char byteString[2] = "";
  sprintf(byteString,"%04d",number);
	return byteString;
}

String convertIntTo2ByteStringHex(int number)
{
	char byteString[2] = "";
  sprintf(byteString,"%02X",number);
	return byteString;
}

/**
 * hex2int
 * take a hex string and convert it to a 32bit number (max 8 hex digits)
 * https://stackoverflow.com/questions/10156409/convert-hex-string-char-to-int
 */
uint32_t hex2int(const char *hex) {
    uint32_t val = 0;
    while (*hex) {
        // get current character then increment
        uint8_t byte = *hex++; 
        // transform hex character to the 4bit equivalent number, using the ascii table indexes
        if (byte >= '0' && byte <= '9') byte = byte - '0';
        else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
        else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;    
        // shift 4 to make space for new digit, and add the 4 bits of the new digit 
        val = (val << 4) | (byte & 0xF);
    }
    return val;
}

// Evaluates if a character is a Hex character
bool isHexChar(char ch) {
  return ('0' <= ch && ch <= '9') || ('A' <= ch && ch <= 'F');
}


// LED Functions
//////////////////////////////////////////////////////////////////
String checkColourCode (CRGB *led){
  String ColourCode; // will return ColourCode value for 

  //Concatenate the 2byte colour values for each colour channel into a string for cramming into our UDP packet
  ColourCode = convertIntTo2ByteStringHex(led->r)+convertIntTo2ByteStringHex(led->g)+convertIntTo2ByteStringHex(led->b);

  return ColourCode;
}


// WiFi and UDP Functions
//////////////////////////////////////////////////////////////////
void connectToWiFi(const char * ssid, const char * pwd){
  Serial.println("Connecting to WiFi network: " + String(ssid));

  // delete old config
  WiFi.disconnect(true);
  //register event handler
  WiFi.onEvent(WiFiEvent);
  
  //Initiate connection
  WiFi.begin(ssid, pwd);

  Serial.println("Waiting for WIFI connection...");
}

//wifi event handler
void WiFiEvent(WiFiEvent_t event){
    switch(event) {
      case SYSTEM_EVENT_STA_GOT_IP:
          //When connected set 
          Serial.print("WiFi connected! IP address: ");
          Serial.println(WiFi.localIP());  
          //initializes the UDP state
          //This initializes the transfer buffer
          connected = true;

          break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
          Serial.println("WiFi lost connection");
          connected = false;
          break;
      default: break;
    }
}

void listenForUDP (){
      if(udp.listen(1234)) {
        Serial.print("UDP Listening on IP: ");
        Serial.println(WiFi.localIP());
        udp.onPacket([](AsyncUDPPacket packet) {
            Serial.print("UDP Packet Type: ");
            Serial.print(packet.isBroadcast()?"Broadcast":packet.isMulticast()?"Multicast":"Unicast");
            Serial.print(", From: ");
            Serial.print(packet.remoteIP());
            Serial.print(":");
            Serial.print(packet.remotePort());
            Serial.print(", To: ");
            Serial.print(packet.localIP());
            Serial.print(":");
            Serial.print(packet.localPort());
            Serial.print(", Length: ");
            Serial.print(packet.length());
            Serial.print(", Data: ");
            Serial.write(packet.data(), packet.length());
            Serial.println();
            //reply to the client
            packet.printf("Got %u bytes of data", packet.length());
        });
    }
}


// Packet Parsing
// State machine for parsing packets
// Expects a passed value to evaluate 
///////////////////////////////////////////////
void packetParse (uint8_t value) {
// Example of expect packet format: ###00FFFFFF01FFFFFFChkSum/c/r
   switch(RXstate)
  {
    // Cases 0-2 all do the same thing 
    case 0:
    case 1:
    case 2: 
      if(value == '#') // For cases 0-2 we expect a '#'
      {
        RXstate++; // If the passed value is as expected (a #)
        inPacket += (char) value; // Build inPacket one byte at a time
      }
      else // If its not a '#' the packet is corrupt and we must start over
      {
        RXstate=0; // Reset our RXstate tracker
        inPacket=""; // Empty the packet we were building 
      }
    break;
    // LED Indices and ColourCodes processed here
    case 3: case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 11: case 12: case 13: case 14: case 15: case 16: case 17: case 18:
      if (isHexChar(value)){
        Serial.println("Its Hex!");
        inPacket += (char) value;
        RXstate++;
      }
      else{ // If our passed value is not a HexChar the packet must be corrupt and we must start over
        RXstate=0; 
        inPacket="";
      }
    break;
// Checksum
   case 19:   
      RXstate=0; // We've hit the checksum no need to keep parsing past this as I stashed the recieved checksum before this function was called
      
      /* A note on the Arduino .substring(from, to) implementation
      * from - inclusive
      * to - exclusive
      * Compared to the C++ & C# implemntation of .substring(pos, len)
      */
      if(inPacket.substring(0,3)=="###")
      {
        Serial.println("Header is good");
        //### 00 FFFFFF 01 FFFFFF Checksum
        for(int i=3;i<20;i++) // Skip the header for checksum calculation purposes
        {
          calChkSum +=(byte) inPacket[i]; // Calculate value of CheckSum one byte at a time
          //Serial.println(inPacket[i]);
        }
        Serial.println(inPacket);
        Serial.println(calChkSum);
        calChkSum %= 1000;
        //Serial.write("Calc'd ChkSum:");
        Serial.println("Calculation done!");
        
        if (calChkSum==reChkSum) //Check the checksums!
        {
          Serial.println("Checksum is good!");
          //### 00 FFFFFF   01 FFFFFF Checksum
          //012 34 5678910 11,12 
          //leds[0].setColorCode(0x000000);
          // Ugly to look at but work - not currently function beyond two LEDs
          Serial.println("Set the LEDs!");
          leds[hex2int(inPacket.substring(3,5).c_str())].setColorCode(hex2int(inPacket.substring(5,11).c_str()));
          leds[hex2int(inPacket.substring(11,13).c_str())].setColorCode(hex2int(inPacket.substring(13,19).c_str()));
          FastLED.show();
        }  

      }
      RXstate=0;
      RXindex=0;
      calChkSum=0; // Gotta reset this otherwise it accumulates 
      inPacket="";
    break; 
  } 
}

