//
// #################################################
//
//    Jam STAPL Byte-Code Player for Arduino
//
//    Copyright (C) 2021
//      Hideto Kikuchi / PJ (@pcjpnet) - http://pc-jp.net/
//
//
//    !!!!! NOT ALLOWED COMMERCIAL USE !!!!!
//
// #################################################
//


#if defined(STM32_CORE_VERSION)
  // STM32duino with STM32SD / FatFs Library
  #include <STM32SD.h>
#else
  // Raspberry Pi Pico
  // ESP32
  // Arduino Board [8.3 File Name]
  #include <SPI.h>
  #include <SD.h> // ONLY [8.3 File Name]
#endif


#include "jbiarduino.h"


void setup() {

  Serial.begin(115200);

  while (!Serial) {
    ; // Wait For Connect
  }
  delay(1000);

  Serial.println("############ Update Start ############");

  // Set SD CS Pin Number
  if (!SD.begin()) {
    Serial.println("SD Initialization Failed!");
    while (1);
  } else {
    Serial.println("SD Initialization Done.");
  }



  // Set Output Stream
  jbi_set_stream(Serial);

  // Set JTAG Pin(TCK, TDI, TMS, TDO)
  jbi_begin(5, 6, 7, 8);
  //jbi_begin(16, 17, 18, 19);


  //File jbc_file = SD.open("/sfp.jbc");
  File jbc_file = SD.open("/led.jbc");


  // Execute Program (Normal Operation)
  jbi_exec_program(jbc_file);


  // Execute Configure (MAX10 SRAM Write)
  //jbi_exec_configure(jbc_file);


}



void loop() {
  // main loop
  
}
