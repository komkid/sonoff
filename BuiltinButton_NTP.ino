// Import required libraries
#include <ESP8266WiFi.h>
#include <EEPROM.h>

// WiFi parameters
const char* ssid = "SSID";
const char* password = "PASSWORD";

char state = 0;
int buttonState = 0;         // variable for reading the pushbutton status

#define RELAYPIN 12
#define LEDPIN 13
#define BUTTONPIN 0     // the number of the pushbutton pin

#define EEPROM_STATE_ADDRESS 128

/*################################ NTP ################################*/
#include <WiFiUdp.h>
unsigned int localPort = 2390;      // local port to listen for UDP packets

/* Don't hardwire the IP address or we won't get the benefits of the pool.
 *  Lookup the IP address for the host name instead */
//IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "time.nist.gov";

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;
/*################################ NTP ################################*/

void updateIO() {
  if (state == 1) {
    digitalWrite(RELAYPIN, HIGH);
    #ifdef LEDPIN
      digitalWrite(LEDPIN, LOW);
    #endif
  }
  else {
    state = 0;
    digitalWrite(RELAYPIN, LOW);
    #ifdef LEDPIN
      digitalWrite(LEDPIN, HIGH);
    #endif
  }
}

void saveState() {
  EEPROM.write(EEPROM_STATE_ADDRESS, state);
  EEPROM.commit();
}

void setup(void)
{  
  // Start Serial
  Serial.begin(115200);
  
  EEPROM.begin(512);
  state = EEPROM.read(EEPROM_STATE_ADDRESS) == 1 ? 1 : 0;
  updateIO();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
 
  // Print the IP address
  Serial.println(WiFi.localIP());
  
/*################################ NTP ################################*/
  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());
/*################################ NTP ################################*/
}

void loop() {
  // read the state of the pushbutton value:
  buttonState = digitalRead(BUTTONPIN);

  // check if the pushbutton is pressed.
  // if it is, the buttonState is HIGH:
  if (buttonState == LOW) {
    state = (state == 0) ? 1 : 0;
    updateIO();
    saveState();
    delay(1000);
  }
  
/*################################ NTP ################################*/
  unsigned int h;
  unsigned int m;
  unsigned long delayInterval = 10 * 1000;
  //get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServerIP); 

  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);
  
  int cb = udp.parsePacket();
  if (!cb) {
    Serial.println("no packet yet");
    delayInterval = 10000;
  }
  else {
    Serial.print("packet received, length=");
    Serial.println(cb);
    // We've received a packet, read the data from it
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Seconds since Jan 1 1900 = " );
    Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears + (7 * 60 * 60);
    // print Unix time:
    Serial.println(epoch);


    // print the hour, minute and second:
    Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    h = (epoch  % 86400L) / 3600;
    Serial.print(h); // print the hour (86400 equals secs per day)
    Serial.print(':');
    if ( ((epoch % 3600) / 60) < 10 ) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    m = (epoch  % 3600) / 60;
    Serial.print(m); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    if ( (epoch % 60) < 10 ) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println(epoch % 60); // print the second
  }
  // wait ten seconds before asking for the time again
  //unsigned long delayInterval = 10 * 60 * 1000; // 5 min
  if(h == 18 && m == 30){
    state = 1;
    updateIO();
  }

  if(h == 6 && m == 30){
    state = 0;
    updateIO();
  }

  delay(delayInterval);
/*################################ NTP ################################*/
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}