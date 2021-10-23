#ifndef INC_JBIMCU_H
#define INC_JBIMCU_H

#include <stdbool.h>
#include <Arduino.h>
#include "jbiarduino.h"

#define USE_STATIC_MEMORY   (8 * 1024)    // Memory size in KB. SDRAM max size is 8MB.
#define STATIC_MEMORY_ADDR  (0xc0000000)  // SDRAM Address

#if defined(USE_STATIC_MEMORY)
  #define N_STATIC_MEMORY_KBYTES ((unsigned int) USE_STATIC_MEMORY)
  #define N_STATIC_MEMORY_BYTES (N_STATIC_MEMORY_KBYTES * 1024)
  #define POINTER_ALIGNMENT sizeof(DWORD)
#else /* USE_STATIC_MEMORY */
  #include <malloc.h>
  #define POINTER_ALIGNMENT sizeof(byte)
#endif /* USE_STATIC_MEMORY */


void gpio_init_jtag();
void gpio_close_jtag();
void gpio_set_tdi();
void gpio_clear_tdi();
void gpio_set_tms();
void gpio_clear_tms();
void gpio_set_tck();
void gpio_clear_tck();
bool gpio_get_tdo();


extern "C" {
  void *jbi_malloc(unsigned int size);
  void jbi_free(void *ptr);
  int jbi_jtag_io(int tms, int tdi, int read_tdo);
  void jbi_message(char *message_text);
  void jbi_export_integer(char *key, long value);
  char conv_to_hex(unsigned long value);
  void jbi_export_boolean_array(char *key, unsigned char *data, long count);
  void jbi_delay(long microseconds);
}


#endif /* INC_JBIARDUINO_H */
