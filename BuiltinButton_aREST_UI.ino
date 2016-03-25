/* 
  This a simple example of the aREST UI Library for the ESP8266.
  See the README file for more details.
 
  Written in 2014 by Marco Schwartz under a GPL license. 
*/

// Import required libraries
#include <ESP8266WiFi.h>
#include <aREST.h>
#include <aREST_UI.h>
#include <EEPROM.h>

// WiFi parameters
const char* ssid = "SSIS";
const char* password = "PASSWORD";

char state = 0;
int buttonState = 0;         // variable for reading the pushbutton status

#define RELAYPIN 12
#define LEDPIN 13
#define BUTTONPIN 0     // the number of the pushbutton pin

#define EEPROM_STATE_ADDRESS 128

// Create aREST instance
aREST_UI rest = aREST_UI();

// The port to listen for incoming TCP connections 
#define LISTEN_PORT           80

// Create an instance of the server
WiFiServer server(LISTEN_PORT);

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

  // Create button to control pin 5
  rest.button(RELAYPIN, "RELAY");
  rest.button(LEDPIN, "LED");
  
  // Give name and ID to device
  rest.set_id("1");
  rest.set_name("esp8266");
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
 
  // Start the server
  server.begin();
  Serial.println("Server started");
  
  // Print the IP address
  Serial.println(WiFi.localIP());
}

void loop() {
  // read the state of the pushbutton value:
  buttonState = digitalRead(BUTTONPIN);

  // check if the pushbutton is pressed.
  // if it is, the buttonState is HIGH:
  if (buttonState == LOW) {
    state = (state == 0) ? 1 : 0;
    updateIO();
    delay(1000);
  }
  
  // Handle REST calls
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  while(!client.available()){
    delay(1);
  }
  rest.handle(client);
 
}
