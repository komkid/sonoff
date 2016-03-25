// Import required libraries
#include <EEPROM.h>

char state = 0;
int buttonState = 0;         // variable for reading the pushbutton status

#define RELAYPIN 12
#define LEDPIN 13
#define BUTTONPIN 0     // the number of the pushbutton pin

#define EEPROM_STATE_ADDRESS 128

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
  Serial.println("Update IO");
  EEPROM.write(EEPROM_STATE_ADDRESS, state);
  EEPROM.commit();
}

void setup(void)
{  
  // Start Serial
  Serial.begin(115200);
  
  pinMode(BUTTONPIN, INPUT);
  pinMode(RELAYPIN, OUTPUT);
  #ifdef LEDPIN
    pinMode(LEDPIN, OUTPUT);
  #endif

  Serial.println("Reading last state");
  EEPROM.begin(512);
  state = EEPROM.read(EEPROM_STATE_ADDRESS) == 1 ? 1 : 0;
  updateIO();
  Serial.println("Setting last state");
}

void loop() {
  // read the state of the pushbutton value:
  buttonState = digitalRead(BUTTONPIN);

  // check if the pushbutton is pressed.
  if (buttonState == LOW) {
    Serial.println("Toggle state");
    state = (state == 0) ? 1 : 0;
    updateIO();
    delay(1000);
  }
}
