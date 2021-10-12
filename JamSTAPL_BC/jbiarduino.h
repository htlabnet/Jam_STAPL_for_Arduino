#ifndef INC_JBIARDUINO_H
#define INC_JBIARDUINO_H

#include <Arduino.h>

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

#include "jbimcu.h"

extern "C" {
  #include "jbimain.h"
}


extern Stream *IOStream;
extern uint8_t PIN_JTAG_TCK;
extern uint8_t PIN_JTAG_TDI;
extern uint8_t PIN_JTAG_TMS;
extern uint8_t PIN_JTAG_TDO;


void jbi_set_stream(Stream &stream_port);
void jbi_begin(uint8_t tck, uint8_t tdi, uint8_t tms, uint8_t tdo);
void jbi_exec(File &JBI_FILE, char *action);
void jbi_exec_program(File &JBC_FILE);
void jbi_exec_configure(File &JBC_FILE);


#endif /* INC_JBIARDUINO_H */
