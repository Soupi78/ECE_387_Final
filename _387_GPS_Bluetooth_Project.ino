//**This code is a mild adaptation of the Adafruit "shield_sdlog" example code found in the GPS library 
//**made by Ladyaya & Bill Greiman with additions made to imtegrate bluetooth functionality.   
//**Comments made my me, Ethan Sobol, are prefaced with "**" and will highlight the few key diffrences 
//**which make my code run with my build, comments without the "**" are from the libraries. 
//**Many Adafruit comments were removed for readability, but important notes were left. 
//**This build assumes you are using the Arduino UNO and 
//**the Adafruit Ultimate GPS Logger Shield W/ Soft Serial connection and the Adafruit Bluefruit LE. 
//**If using an Arduino uno, there isn't enough SRAM to use both Bluetooth and SD logging
//**at the same time, however the code in functional for both so I've simply 
//**commented out the SD logging in this build to highlight the bluetooth functionality.  
//------------------------------BLUETOOTH---------------------------------▼
#include "Adafruit_BLE_UART.h" //**Importing the new bluetooth library
#define ADAFRUITBLE_REQ 10     //**Setting up the bluetooth pins
#define ADAFRUITBLE_RDY 2      //This should be an interrupt pin, on Uno thats #2 or #3
#define ADAFRUITBLE_RST 9      //**Setting the reset pin
#include <avr/pgmspace.h>      //**Library for PROGMEM (to try and save SRAM)
Adafruit_BLE_UART Bluetooth = Adafruit_BLE_UART(ADAFRUITBLE_REQ, ADAFRUITBLE_RDY, ADAFRUITBLE_RST);//*8setting the Bluetooth module
//------------------------------------------------------------------------▲

#include <SPI.h>
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
//#include <SD.h>
#include <avr/sleep.h>

SoftwareSerial mySerial(8, 7);
Adafruit_GPS GPS(&mySerial);

// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences
#define GPSECHO  true
/* set to true to only log to SD when GPS has a fix, for debugging, keep it false */
#define LOG_FIXONLY true //**for my python code, this needs to be true otherwise the first value saved will be garbage data

// this keeps track of whether we're using the interrupt
// off by default!
boolean usingInterrupt = false;
void useInterrupt(boolean); // Func prototype keeps Arduino 0023 happy

// Set the pins used
#define chipSelect 10
#define ledPin 13

//File logfile;

// read a Hex value and return the decimal equivalent
uint8_t parseHex(char c) {
  if (c < '0')
    return 0;
  if (c <= '9')
    return c - '0';
  if (c < 'A')
    return 0;
  if (c <= 'F')
    return (c - 'A')+10;
}
//----------------------------------------------------------------------------▼
// blink out an error code  //**commented out this error handeling case to save SRAM
//void error(uint8_t errno) {
//  /*
//  if (SD.errorCode()) {
//   putstring("SD error: ");
//   Serial.print(card.errorCode(), HEX);
//   Serial.print(',');
//   Serial.println(card.errorData(), HEX);
//   }
//   */
//  while(1) {
//    uint8_t i;
//    for (i=0; i<errno; i++) {
//      digitalWrite(ledPin, HIGH);
//      delay(100);
//      digitalWrite(ledPin, LOW);
//      delay(100);
//    }
//    for (i=errno; i<10; i++) {
//      delay(200);
//    }
//  }
//}
//---------------------------------------------------------------------------▲
void setup() {
  Serial.begin(9600);//**must change this to 9600 to get clear data
//------------------------------BLUETOOTH------------------------------------▼
  Bluetooth.setDeviceName("GPSDATA"); /* 7 characters max!**sets the name of what you'd see on your phone*/
  Bluetooth.begin(); 
//---------------------------------------------------------------------------▲
//  Serial.println(F( );
  pinMode(ledPin, OUTPUT);
  pinMode(10, OUTPUT);
//---------------------------------------------------------------------------▼
// if (!SD.begin(chipSelect)) {//**Commented out this error case to save SRAM       
//    Serial.println(F("Card init. failed!"));
//    error(2);
//  }
//---------------------------------------------------------------------------▲
  char filename[15];
  strcpy(filename, "GPSLOG00.TXT");//**This is the name given to the text file in which the data is written and eventualy read.
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = '0' + i/10;
    filename[7] = '0' + i%10;}
//---------------------------------------------------------------------------▼
// create if does not exist, do not open existing, write, sync after write
//    if (! SD.exists(filename)) {//**commented out error case for SRAM 
//      break;
//    }
//  logfile = SD.open(filename, FILE_WRITE);
//  if( ! logfile ) {
//    Serial.print(F("Couldnt create ")); 
//    Serial.println(filename);
//    error(3);
//  }
//---------------------------------------------------------------------------▲

  Serial.print(F("Writing to ")); 
  Serial.println(filename);

  // connect to the GPS at the desired rate
  GPS.begin(9600);

  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);//**need this turned on to keep the input to the parsing code consistant
  // uncomment this line to turn on only the "minimum recommended" data
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // For logging data, we don't suggest using anything but either RMC only or RMC+GGA
  // to keep the log files at a reasonable size
  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 100 millihertz (once every 10 seconds), 1Hz or 5Hz update rate

  // Turn off updates on antenna status, if the firmware permits it
  GPS.sendCommand(PGCMD_NOANTENNA);

  // the nice thing about this code is you can have a timer0 interrupt go off
  // every 1 millisecond, and read data from the GPS for you. that makes the
  // loop code a heck of a lot easier!
  useInterrupt(true);

  Serial.println(F("Ready!"));
}


// Interrupt is called once a millisecond, looks for any new GPS data, and stores it
SIGNAL(TIMER0_COMPA_vect) {
  char c PROGMEM = GPS.read();
  #ifdef UDR0
      if (GPSECHO)
        if (c) UDR0 = c;   
  #endif
}

void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
    usingInterrupt = true;
  } 
  else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
    usingInterrupt = false;
  }
}

void loop() {
  //----------------------Bluetooth-----------------------▼
   Bluetooth.pollACI();//Tell the nRF8001 to do whatever it should be working on.
   //-----------------------------------------------------▲
  if (! usingInterrupt) {
    // read data from the GPS in the 'main loop'
    char c PROGMEM = GPS.read();
    // if you want to debug, this is a good time to do it!
    if (GPSECHO)
      if (c) Serial.print(c);
  }
  
  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences! 
    // so be very wary if using OUTPUT_ALLDATA and trying to print out data
    
    // Don't call lastNMEA more than once between parse calls!  Calling lastNMEA 
    // will clear the received flag and can cause very subtle race conditions if
    // new data comes in before parse is called again.
    char *stringptr PROGMEM = GPS.lastNMEA();
    
    if (!GPS.parse(stringptr))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another

    // Sentence parsed! 
    Serial.println(F(" OK\n"));
    if (LOG_FIXONLY && !GPS.fix) {
      Serial.print(F("No Fix:"));
//-------------------------Bluetooth----------------------------▼
    String s PROGMEM = ("No Fix \n");//**If in this loop there is no valid data, so we send "No Fix" to the bluetooth
      Serial.println(s);
      //We need to convert the line to bytes, no more than 20 at a time
      uint8_t sendbuffer[20];//**sets the buffer size for bluetooth transmission then prints it for debugging 
      s.getBytes(sendbuffer, 20);
      char sendbuffersize PROGMEM = min(20, s.length());
      Serial.print(F("\n* Sending -> \"")); Serial.print((char *)sendbuffer); Serial.println(F("\""));
      //write the data
      Bluetooth.write(sendbuffer, sendbuffersize);//send the data over 
//--------------------------------------------------------------▲
      return;
    }
    //log check
    Serial.println(F("Loged"));
    uint8_t stringsize PROGMEM = strlen(stringptr);
//--------------------------------Bluetooth----------------------▼
    //if (stringsize != logfile.write((uint8_t *)stringptr, stringsize))//**write the string to the SD file
    //    error(4);//**commented out to save SRAM
    if (strstr(stringptr, "RMC") || strstr(stringptr, "GGA")) //;  logfile.flush();
    Serial.println();
    String s PROGMEM = (GPS.lastNMEA());//**sets the string to be transmitted to be the last GPS position logged
      Serial.println(s);
      //We need to convert the line to bytes, no more than 20 at a time
      uint8_t sendbuffer[20];
      s.getBytes(sendbuffer, 20);
      char sendbuffersize PROGMEM = min(20, s.length());
      Serial.print(F("\n* Sending -> \"")); Serial.print((char *)sendbuffer); Serial.println(F("\""));
      //write the data
      Bluetooth.write(sendbuffer, sendbuffersize);//**sends the data to the bluetooth 
//---------------------------------------------------------------▲
   
  }
}
/* End code */

