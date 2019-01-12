#include <RHMesh.h>
#include <RadioHead.h>
#include <RHDatagram.h>
#include <RHReliableDatagram.h>
#include <RHRouter.h>
#include <RH_RF95.h>

#include <MIDIUSB.h>

#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7

#define RF95_FREQ 917.0 // Sets transmit frequency
#define RF95_POWER 13 // Sets transmit power from 5 - 23 dBm

#define greenLed 9 // defines pin for red led in the NKK switch
#define redLed 6 // defines pin for green led in the NKK switch

#define nkkSwitch 3 // defines input pin for NKK switch
#define modeSwitch 11 // defines input pin for mode switch

RH_RF95 rf95(RFM95_CS, RFM95_INT); // Initializes radio driver

int HUIZone = 0; // Instantiates a variable to keep track of the current HUI zone selection
bool playLit = false; // Instantiates a variable to keep track of the current status of the virtual play light
bool recLit = false; // Instantiates a variable to keep track of the current status of the virtual record light
char lightStatus = '0'; // Instantiates a variable to keep track of the current status of the physical light
unsigned long displayBuff = 0; // Instantiates a variable to help avoid sending a play command when starting recording
static int playWait = 500; // How long to wait before it is safe to assume that the record light is not flashing in ms
unsigned long playTime = 0; // How long has it been since play light was last turned on
char mode = 'a'; // Keeps track of what mode the controller is in (auto or manual)
char manualStatus = '0'; // Keeps track of the manual light status
unsigned long lastRead = 0; // Helps to avoid spamming radio when switch is held
int timeHeld = 0; // Keeps track of how long the button has been held down
unsigned long timer = 0; // Keeps track of how long the button has been held down
bool oldSwitchState = true; // Helps keep track of if the switch has been released
bool ledRainbow = false;  // Helps fade the switch when in rainbow mode
unsigned long rainbowTime = 0;  // Helps fade the switch when in rainbow mode
bool oldModeState = false; // Helps figure out if the mode switch just changed



void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

// //Unneccisary for this revision of the firmware
//void controlChange(byte channel, byte control, byte value) {
//  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
//  MidiUSB.sendMIDI(event);
//}

void radioSetup(){
   pinMode(RFM95_RST, OUTPUT);
   digitalWrite(RFM95_RST, HIGH);
   
     Serial.println("Feather LoRa TX Test!"); //For debug
   
    // manual reset
    digitalWrite(RFM95_RST, LOW);
    delay(10);
    digitalWrite(RFM95_RST, HIGH);
    delay(10);
   
    while (!rf95.init()) {
      Serial.println("LoRa radio init failed"); //For debug
      while (1);
    }
    Serial.println("LoRa radio init OK!"); //For debug
   
    // Defaults after init are 915.00MHz, modulation GFSK_Rb250Fd250, +13dbM
    if (!rf95.setFrequency(RF95_FREQ)) {
      Serial.println("setFrequency failed"); //For debug
      while (1);
    }
    Serial.print("Set Freq to: "); Serial.println(RF95_FREQ); //For debug

    // you can set transmitter powers from 5 to 23 dBm:
    rf95.setTxPower(RF95_POWER, false);
    Serial.print("Set Power to: "); Serial.println(RF95_POWER); //For debug
}


void ledColor(char color){
  if (color == 'r'){
      digitalWrite(redLed, LOW);
      digitalWrite(greenLed, HIGH);
      ledRainbow = false;  
  }
  
  if (color == 'g'){
      digitalWrite(redLed, HIGH);
      digitalWrite(greenLed, LOW);
      ledRainbow = false;    
  } 
  
  if (color == 'y'){
      digitalWrite(redLed, LOW);
      digitalWrite(greenLed, LOW);
      ledRainbow = false;  
  }
  
  if (color == '0'){
      digitalWrite(redLed, HIGH);
      digitalWrite(greenLed, HIGH);
      ledRainbow = false;   
  }
  if (color == '!'){
      digitalWrite(greenLed,HIGH);
      digitalWrite(redLed, HIGH);
      ledRainbow = true;
  }
}

void lightColor(char color){

  if(color == 'r'){
          //Radio transmit
       Serial.print("Transmitting...   "); // Send a message to rf95_server
  
      char radiopacket[20] = "record";
      Serial.println(radiopacket);
      radiopacket[19] = 0;
      delay(10);
      rf95.send((uint8_t *)radiopacket, 20);

      lightStatus= 'r';
      Serial.println(lightStatus);

      ledColor('r');
  }

  if(color == 'y'){
        //Radio transmit
       Serial.print("Transmitting...   "); // Send a message to rf95_server
  
      char radiopacket[20] = "play";
      Serial.println(radiopacket);
      radiopacket[19] = 0;
      delay(10);
      rf95.send((uint8_t *)radiopacket, 20);

      lightStatus= 'p';
      Serial.println(lightStatus);

      ledColor('y');
  }

  if(color == '0'){
            //Radio transmit
       Serial.print("Transmitting...   "); // Send a message to rf95_server
  
      char radiopacket[20] = "off";
      Serial.println(radiopacket);
      radiopacket[19] = 0;
      delay(10);
      rf95.send((uint8_t *)radiopacket, 20);

      lightStatus= '0';
      Serial.println(lightStatus);

      ledColor('0');  
  }

  if(color == 'g'){
           //Radio transmit
       Serial.print("Transmitting...   "); // Send a message to rf95_server
  
      char radiopacket[20] = "safe";
      Serial.println(radiopacket);
      radiopacket[19] = 0;
      delay(10);
      rf95.send((uint8_t *)radiopacket, 20);

      lightStatus= '0';
      Serial.println(lightStatus);
      ledColor('g'); 
  }

  if(color == '!'){
           //Radio transmit
       Serial.print("Transmitting...   "); // Send a message to rf95_server
  
      char radiopacket[20] = "rainbow";
      Serial.println(radiopacket);
      radiopacket[19] = 0;
      delay(10);
      rf95.send((uint8_t *)radiopacket, 20);

      lightStatus= '0';
      Serial.println(lightStatus);
      ledColor('!');
  }
}

void setup() {
      
  Serial.begin(115200); //For debug
//    while (!Serial) {
//    delay(1);
//    }
   
   delay(500);
   
   radioSetup(); 
   
   pinMode(greenLed, OUTPUT); //green led
   pinMode(redLed, OUTPUT); //red led

   digitalWrite(greenLed,HIGH); // ensure that green led is off
   digitalWrite(redLed,HIGH); // ensure that red led is off

   pinMode(nkkSwitch, INPUT_PULLUP); // momentary button
   pinMode(modeSwitch, INPUT_PULLUP); // mode switch

   lightColor('0'); 
      
}


void loop() {
   midiEventPacket_t rx;

  do {
    
    if(displayBuff > millis()){
      displayBuff = millis();
    }
    
    if(playTime > millis()){
      playTime = millis();
    }

    if(lastRead > millis()){
      lastRead = millis();
    }
    if(rainbowTime > millis()){
      rainbowTime = millis();
    }
       // Is the controller in auto or manual mode?
   if(digitalRead(modeSwitch) == 1){
      mode = 'a';
      if(manualStatus != '0'){
        lightColor('0');
        delay(100);
      }
      manualStatus = '0';
      oldModeState = true;
   }

   else if(digitalRead(modeSwitch) != 1){
      mode = 'm';
      if(oldModeState == true){
          manualStatus = lightStatus;
          delay(100);
      }
      oldModeState = false;
   }

    rx = MidiUSB.read();
//    if (rx.header != 0) {
//      Serial.print("Received: ");
//      Serial.print(rx.header, HEX);
//      Serial.print(",");
//      Serial.print(rx.byte1, HEX);
//      Serial.print(",");
//      Serial.print(rx.byte2, HEX);
//      Serial.print(",");
//      Serial.println(rx.byte3, HEX);
//    }

//      // An attempt at replying to the HUI ping function, so far unsucessfull for some reason
//    if (rx.byte1 == 0x90 && rx.byte2 ==0x00 &&rx.byte3 == 0x00){
//      Serial.println("ping ");
//      delay(100);
//      noteOn(0,0x00,0x7f);
//    }
      // Figure out what zone of the HUI controller the DAW is trying to write to then store it for use later
    if (rx.byte1 == 0xb0 && rx.byte2 ==0x0c){
      HUIZone= rx.byte3;
     // Serial.print("zone= ");
     // Serial.println(HUIZone,HEX);
    }

      // Figure out if the DAW is commanding the play button led to be turned on
    if (HUIZone == 0x0e && rx.byte1 == 0xb0 && rx.byte2 ==0x2c &&rx.byte3 == 0x44){
      Serial.println("play on");
      playLit = true;

      playTime = millis();
    }
    

      // Figure out if the DAW is commanding the play button led to be turned off
    if (HUIZone == 0x0e && rx.byte1 == 0xb0 && rx.byte2 ==0x2c &&rx.byte3 == 0x04){
      Serial.println("play off");
      playLit = false;
    }
    
      // Figure out if the DAW is commanding the record button led to be turned on
    if (HUIZone == 0x0e && rx.byte1 == 0xb0 && rx.byte2 ==0x2c &&rx.byte3 == 0x45){
      Serial.println("rec on");
      recLit = true;
    }
    
       // Figure out if the DAW is commanding the record button led to be turned off
     if (HUIZone == 0x0e && rx.byte1 == 0xb0 && rx.byte2 ==0x2c &&rx.byte3 == 0x05){
      Serial.println("rec off");
      recLit = false;
      displayBuff = millis();
    } 
    
       // If the recording light is being commanded on and the play light is being commanded on then turn on physical rec light
    if (recLit == true && playLit == true && lightStatus != 'r' && mode == 'a'){
        lightColor('r');
    }

              // If the play light is being commanded on and the recording light is off then turn on physical play light
    if (recLit == false && playLit == true && lightStatus != 'p' && (millis() - displayBuff > playWait) && mode == 'a'){
        lightColor('y');
    }

     // If the recording light is being commanded off then turn off physical rec light
    if (recLit == false && playLit == false && lightStatus != '0' && (millis() - playTime > playWait) && mode == 'a'){
         lightColor('0');   
    }


   
// manual mode ------------------------------------------------------------------------------------------------

   if (mode == 'm' && digitalRead(nkkSwitch) == 0){
      oldSwitchState = false;
   }

        // Manual mode red
   if (mode == 'm' && manualStatus == '0' && digitalRead(nkkSwitch) == 1 && oldSwitchState == false && (millis() - lastRead) >= 1000 && timeHeld < 1000){
         lightColor('r');  
         manualStatus = 'r'; 
         lastRead = millis(); 
         Serial.print("status:  ");
         Serial.println(manualStatus);
         oldSwitchState = true;
         timeHeld = 0;
   }
   
        // Manual mode off
   if (mode == 'm' && manualStatus != '0'  && digitalRead(nkkSwitch) == 1 && oldSwitchState == false && (millis() - lastRead) >= 1000 && timeHeld < 1000){
         lightColor('0');
         manualStatus = '0';
         lastRead = millis();   
         Serial.print("status:  ");
         Serial.println(manualStatus);
         oldSwitchState = true;
         timeHeld = 0;
   }

        // Manual mode play
   if (mode == 'm' && manualStatus != 'p'  && digitalRead(nkkSwitch) == 1 && oldSwitchState == false && (millis() - lastRead) >= 1000 && timeHeld < 3000 && timeHeld >= 1000){
         lightColor('y');
         manualStatus = 'p';
         lastRead = millis();   
         Serial.print("status:  ");
         Serial.println(manualStatus);
         oldSwitchState = true;
         timeHeld = 0;
   }
          // Manual mode safe (green)
   if (mode == 'm' && manualStatus != 's'  && digitalRead(nkkSwitch) == 1 && oldSwitchState == false && (millis() - lastRead) >= 1000 && timeHeld < 5000 && timeHeld >= 3000){
         lightColor('g');
         manualStatus = 's';
         lastRead = millis();   
         Serial.print("status:  ");
         Serial.println(manualStatus);
         oldSwitchState = true;
         timeHeld = 0;
   }

      if (mode == 'm' && manualStatus != '!'  && digitalRead(nkkSwitch) == 1 && oldSwitchState == false && (millis() - lastRead) >= 1000 && timeHeld < 8000 && timeHeld >= 5000){
         lightColor('!');
         manualStatus = '!';
         lastRead = millis();   
         Serial.print("status:  ");
         Serial.println(manualStatus);
         oldSwitchState = true;
         timeHeld = 0;
   }

   if (mode == 'm' && digitalRead(nkkSwitch) == 0 && (millis() - timer) >= 250){
      timer = millis();
      timeHeld = timeHeld + 250;
      Serial.print("timeHeld   ");
      Serial.println(timeHeld);

      if (timeHeld >= 1000){
         ledColor('y'); 
      }
      
      if (timeHeld >= 3000){
         ledColor('g'); 
      }

      if (timeHeld >= 5000){
         ledColor('!'); 
      }
   }
   
  if(timeHeld >= 8000){
    timeHeld = 0;
    ledColor('0');
  }
  
  if(ledRainbow == true){
      rainbowTime = millis();
      analogWrite(greenLed, 128+127*cos(2*PI/5000*rainbowTime));
      analogWrite(redLed, 128+127*cos(2*PI/5000*(rainbowTime-500)));
   } 
    
  } while (rx.header != 0);

}
