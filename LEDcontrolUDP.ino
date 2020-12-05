#include <Arduino.h> 
#include <WiFi.h>
#include <FastLED.h>
#include "AsyncUDP.h"
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

// Instantiate our LED object
CRGB leds[NUM_LEDS];
AsyncUDP udp;

// Prototypes
String convertInto2ByteString(int number);
String convertIntTo3ByteString(int number); 
String convertIntTo4ByteString(int number);
String checkColourCode (CRGB *led);
void receivePacket(AsyncUDPPacket packet);
bool isHexChar(char ch);

int packetNumber;

int analogPin;
int digitalPin;
int ledIndex;   // Index of LED to address

unsigned int packetLen;

int analogReq = 6;// set to the Max number of analog pins required
int digitalInReq = 8;// set to the Max number of digital input pins required

int ledInReq = NUM_LEDS; // Set to the max # of LEDS to control

int checkSum = 0;
int PacketIndex = 0;
int TXstate = 0;
int RXstate=0;
int RXindex=0;
String outPacket; 
String inPacket;
unsigned long previousMillis=0;
const long interval=100; // change this to 100 if needed
 

int udpPort = 1234;
//Are we currently connected?
boolean connected = false;


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
  int calChkSum=0;
  int recChkSum=0;
  String ChkSum;
  char inByte;

  //inByte=Serial.read();
  
  //Serial.print("Byte ");
  //Serial.print(inByte);
  //Serial.print(" RX state ");
  //Serial.println(RXstate);
  //inPacket+="\0";
  switch(RXstate)
  {
  case 0:
  case 1:
  case 2: 
    if(inByte=='#')
    {
      inPacket += inByte;
      RXstate++;
    }
    else
    {
      RXstate=0;
      inPacket="";
    }
    break;
// LED Indices and ColourCodes
  case 3: case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 11: case 12: case 13: case 14: case 15: case 16: case 17: case 18:
   // if(inByte=='0' || inByte=='1')
   // {
   //   inPacket += inByte;
   //   RXstate++;
   // }
   // else
   // {
   //   RXstate=0;
   //   inPacket="";
   // }
   if (isHexChar(char ch)){
     inPacket += inByte;
     RXstate++;
   }
   else{
     RXstate=0;
     inPacket="";
   }
    break;

//  case 9: case 10: case 11:
//      if(inByte >='0' && inByte<='9')
//      {
//        inPacket += inByte;
//        RXstate++;
//      }
//      else
//      {
//        RXstate=0;
//        inPacket="";
//      }
//      break;
//
   // chkSum   
   case 20:
      RXstate=0;
      //Serial.println(inPacket);
      if(inPacket.substring(0,3)=="###")
      {
        //Serial.println(inPacket);
        //### 00 FFFFFF 01 FFFFFF Checksum
        
        for(int i=3;i<20;i++)
        {
          calChkSum +=(byte) inPacket[i];
        }
        ChkSum=inPacket.substring(9,12);
        recChkSum=ChkSum.toInt();
        if (calChkSum==recChkSum) //Check the checksum!
        {
          // Checksum checks out! Do what the packet says
          
          //### 00 FFFFFF 01 FFFFFF Checksum

          // leds[].setColorCode(0x4355FF)
          
          for(int i=0;i<6;i++)
          {
            if (inPacket[i+3]=='0')
            {
              digitalWrite(13-i, LOW);
            }
            else
            {
              digitalWrite(13-i, HIGH);
            }
          }
        }  
      }
      RXstate=0;
      RXindex=0;
      inPacket="";

      break; 
  } 
  //Serial.println(inPacket);
}

// ConvToString Functions
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

bool isHexChar(char ch) {
  return ('0' <= ch && ch <= '9') || ('A' <= ch && ch <= 'F');
}