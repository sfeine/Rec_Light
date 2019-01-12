#include <RHMesh.h>
#include <RadioHead.h>
#include <RHDatagram.h>
#include <RHReliableDatagram.h>
#include <RHRouter.h>
#include <RH_RF95.h>

#include <Adafruit_NeoPixel.h>
#include <SPI.h>

#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7

#define RF95_FREQ 917.0 //Sets transmit frequency

#define lightsPin 6

#define LED 13

#define numPix 80

RH_RF95 rf95(RFM95_CS, RFM95_INT); // Initializes radio driver

Adafruit_NeoPixel strip = Adafruit_NeoPixel(numPix, lightsPin, NEO_GRB + NEO_KHZ800); //WS2812b strip setup


void setup() {

  //---------------------------------------------------------------------------------------------------------------
  
  //LED strip setup
  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  //---------------------------------------------------------------------------------------------------------------
  
  // Radio setup
  
   pinMode(RFM95_RST, OUTPUT);
   digitalWrite(RFM95_RST, HIGH);
   
    Serial.begin(115200); //For debug
//    while (!Serial) {
//    delay(1);
//    }
   
    delay(100);
   
     Serial.println("Feather LoRa RX Test!"); //For debug
   
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
    rf95.setTxPower(13, false);
  
  //---------------------------------------------------------------------------------------------------------------

}

void loop() {

  //Radio stuff
   if (rf95.available())
    {
      // Should be a message for us now
      uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
      uint8_t len = sizeof(buf);
      char msg[len]; //Creates a character array which will store the message from the buffer in a useable format

      if (rf95.recv(buf, &len))
      {
        digitalWrite(LED, HIGH);
        RH_RF95::printBuffer("Received: ", buf, len);
//        Serial.print("Got: "); //For debug
//        Serial.println((char*)buf); //For debug
        Serial.print("RSSI: "); //For debug
        Serial.println(rf95.lastRssi(), DEC); //For debug


        
        for (int i = 0; i < len; i++)
           {
             msg[i]= (char*)buf[i];
           }

        Serial.println(msg); //For debug

        // If recording display all red leds
        if (strcmp(msg,"record")==0)
          {
            colorWipe(strip.Color(255, 0, 0), 10); // Red 
            Serial.println("red"); //For debug
          }

        // If playing back display all yellow leds
        if (strcmp(msg,"play")==0)
          {
            colorWipe(strip.Color(255, 150, 0), 10); // Yellow
          }

        // If off, turn off all leds
        if (strcmp(msg,"off")==0)
          {
            colorWipe(strip.Color(0, 0, 0), 10); // off
          }

        // If safe display all green leds
        if (strcmp(msg,"safe")==0)
          {
            colorWipe(strip.Color(0, 250, 0), 10); // green
          }

        // If rainbow display fading rainbow colors
        if (strcmp(msg,"rainbow")==0)
          {
            rainbowCycle(50);
          }
     
//        // Send a reply
//        uint8_t data[] = "And hello back to you";
//        rf95.send(data, sizeof(data));
//        rf95.waitPacketSent();
//        Serial.println("Sent a reply"); //For debug
//        digitalWrite(LED, LOW);
      }
      else
      {
        Serial.println("Receive failed"); //For debug
      }
    }

  //---------------------------------------------

  
//  // LED display testing:
//    colorWipe(strip.Color(255, 0, 0), 50); // Red
//    colorWipe(strip.Color(255, 150, 0), 50); // Yellow
//    colorWipe(strip.Color(0, 255, 0), 50); // Green
//  
//  
//    
//    rainbowCycle(50);

}


//--------------------------------------------------------------------------------
//LED display functions

//// Fade in a solid color
//void colorFade(uint32_t c, uint8_t fade) {
//  for(uint16_t j=0; j<fade)
//    for(uint16_t i=0; i<strip.numPixels(); i++) {
//      strip.setPixelColor(i, c);
//      strip.show();
//      delay(wait);
//    }
//}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}


// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    if (rf95.available())
    {
      return;
    }
    delay(wait);
  }
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
