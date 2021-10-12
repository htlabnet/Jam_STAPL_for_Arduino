
#include "jbimcu.h"



#if defined(USE_STATIC_MEMORY)
  unsigned char static_memory_heap[N_STATIC_MEMORY_BYTES] = { 0 };
#endif /* USE_STATIC_MEMORY */

#if defined(USE_STATIC_MEMORY) || defined(MEM_TRACKER)
  unsigned int n_bytes_allocated = 0;
#endif /* USE_STATIC_MEMORY || MEM_TRACKER */

#if defined(MEM_TRACKER)
  unsigned int peak_memory_usage = 0;
  unsigned int peak_allocations = 0;
  unsigned int n_allocations = 0;
#if defined(USE_STATIC_MEMORY)
  unsigned int n_bytes_not_recovered = 0;
#endif /* USE_STATIC_MEMORY */
  const DWORD BEGIN_GUARD = 0x01234567;
  const DWORD END_GUARD = 0x76543210;
#endif /* MEM_TRACKER */




void gpio_init_jtag() {
  // JTAG Pin Setting
  pinMode(PIN_JTAG_TCK, OUTPUT);
  pinMode(PIN_JTAG_TDI, OUTPUT);
  pinMode(PIN_JTAG_TMS, OUTPUT);
  pinMode(PIN_JTAG_TDO, INPUT);

  // Pin Default
  digitalWrite(PIN_JTAG_TCK, LOW);
  digitalWrite(PIN_JTAG_TDI, HIGH);
  digitalWrite(PIN_JTAG_TMS, HIGH);

  if(IOStream)IOStream->println("GPIO Initialize Done");
}


void gpio_close_jtag() {
  // Set Low
  digitalWrite(PIN_JTAG_TCK, LOW);
  digitalWrite(PIN_JTAG_TDI, LOW);
  digitalWrite(PIN_JTAG_TMS, LOW);

  // Set Input
  pinMode(PIN_JTAG_TCK, INPUT);
  pinMode(PIN_JTAG_TDI, INPUT);
  pinMode(PIN_JTAG_TMS, INPUT);
  pinMode(PIN_JTAG_TDO, INPUT);
}


void gpio_set_tdi() {
  digitalWrite(PIN_JTAG_TDI, HIGH);
}


void gpio_clear_tdi() {
  digitalWrite(PIN_JTAG_TDI, LOW);
}


void gpio_set_tms() {
  digitalWrite(PIN_JTAG_TMS, HIGH);
}


void gpio_clear_tms() {
  digitalWrite(PIN_JTAG_TMS, LOW);
}


void gpio_set_tck() {
  digitalWrite(PIN_JTAG_TCK, HIGH);
}


void gpio_clear_tck() {
  digitalWrite(PIN_JTAG_TCK, LOW);
}


bool gpio_get_tdo() {
  return digitalRead(PIN_JTAG_TDO);
}


void *jbi_malloc(unsigned int size) {
  unsigned int n_bytes_to_allocate = 
#if defined(USE_STATIC_MEMORY) || defined(MEM_TRACKER)
    sizeof(unsigned int) +
#endif /* USE_STATIC_MEMORY || MEM_TRACKER */
#if defined(MEM_TRACKER)
    (2 * sizeof(DWORD)) +
#endif /* MEM_TRACKER */
    (POINTER_ALIGNMENT * ((size + POINTER_ALIGNMENT - 1) / POINTER_ALIGNMENT));

  unsigned char *ptr = 0;


#if defined(MEM_TRACKER)
  if ((n_bytes_allocated + n_bytes_to_allocate) > peak_memory_usage)
  {
    peak_memory_usage = n_bytes_allocated + n_bytes_to_allocate;
  }
  if ((n_allocations + 1) > peak_allocations)
  {
    peak_allocations = n_allocations + 1;
  }
#endif /* MEM_TRACKER */

#if defined(USE_STATIC_MEMORY)
  if ((n_bytes_allocated + n_bytes_to_allocate) <= N_STATIC_MEMORY_BYTES)
  {
    ptr = (&(static_memory_heap[n_bytes_allocated]));
  }
#else /* USE_STATIC_MEMORY */ 
  ptr = (unsigned char *) malloc(n_bytes_to_allocate);
#endif /* USE_STATIC_MEMORY */

#if defined(USE_STATIC_MEMORY) || defined(MEM_TRACKER)
  if (ptr != 0)
  {
    unsigned int i = 0;

#if defined(MEM_TRACKER)
    for (i = 0; i < sizeof(DWORD); ++i)
    {
      *ptr = (unsigned char) (BEGIN_GUARD >> (8 * i));
      ++ptr;
    }
#endif /* MEM_TRACKER */

    for (i = 0; i < sizeof(unsigned int); ++i)
    {
      *ptr = (unsigned char) (size >> (8 * i));
      ++ptr;
    }

#if defined(MEM_TRACKER)
    for (i = 0; i < sizeof(DWORD); ++i)
    {
      *(ptr + size + i) = (unsigned char) (END_GUARD >> (8 * i));
      /* don't increment ptr */
    }

    ++n_allocations;
#endif /* MEM_TRACKER */

    n_bytes_allocated += n_bytes_to_allocate;
  }
#endif /* USE_STATIC_MEMORY || MEM_TRACKER */

  return ptr;
}


void jbi_free(void *ptr) {
  if
  (
#if defined(MEM_TRACKER)
    (n_allocations > 0) &&
#endif /* MEM_TRACKER */
    (ptr != 0)
  )
  {
    unsigned char *tmp_ptr = (unsigned char *) ptr;

#if defined(USE_STATIC_MEMORY) || defined(MEM_TRACKER)
    unsigned int n_bytes_to_free = 0;
    unsigned int i = 0;
    unsigned int size = 0;
#endif /* USE_STATIC_MEMORY || MEM_TRACKER */
#if defined(MEM_TRACKER)
    DWORD begin_guard = 0;
    DWORD end_guard = 0;


    tmp_ptr -= sizeof(DWORD);
#endif /* MEM_TRACKER */
#if defined(USE_STATIC_MEMORY) || defined(MEM_TRACKER)
    tmp_ptr -= sizeof(unsigned int);
#endif /* USE_STATIC_MEMORY || MEM_TRACKER */
    ptr = tmp_ptr;

#if defined(MEM_TRACKER)
    for (i = 0; i < sizeof(DWORD); ++i)
    {
      begin_guard |= (((DWORD)(*tmp_ptr)) << (8 * i));
      ++tmp_ptr;
    }
#endif /* MEM_TRACKER */

#if defined(USE_STATIC_MEMORY) || defined(MEM_TRACKER)
    for (i = 0; i < sizeof(unsigned int); ++i)
    {
      size |= (((unsigned int)(*tmp_ptr)) << (8 * i));
      ++tmp_ptr;
    }
#endif /* USE_STATIC_MEMORY || MEM_TRACKER */

#if defined(MEM_TRACKER)
    tmp_ptr += size;

    for (i = 0; i < sizeof(DWORD); ++i)
    {
      end_guard |= (((DWORD)(*tmp_ptr)) << (8 * i));
      ++tmp_ptr;
    }

    if ((begin_guard != BEGIN_GUARD) || (end_guard != END_GUARD))
    {
      fprintf(stderr, "Error: memory corruption detected for allocation #%d... bad %s guard\n",
        n_allocations, (begin_guard != BEGIN_GUARD) ? "begin" : "end");
    }

    --n_allocations;
#endif /* MEM_TRACKER */

#if defined(USE_STATIC_MEMORY) || defined(MEM_TRACKER)
    n_bytes_to_free = 
#if defined(MEM_TRACKER)
    (2 * sizeof(DWORD)) +
#endif /* MEM_TRACKER */
    sizeof(unsigned int) +
    (POINTER_ALIGNMENT * ((size + POINTER_ALIGNMENT - 1) / POINTER_ALIGNMENT));
#endif /* USE_STATIC_MEMORY || MEM_TRACKER */

#if defined(USE_STATIC_MEMORY)
    if ((((unsigned long) ptr - (unsigned long) static_memory_heap) + n_bytes_to_free) == (unsigned long) n_bytes_allocated)
    {
      n_bytes_allocated -= n_bytes_to_free;
    }
#if defined(MEM_TRACKER)
    else
    {
      n_bytes_not_recovered += n_bytes_to_free;
    }
#endif /* MEM_TRACKER */
#else /* USE_STATIC_MEMORY */
#if defined(MEM_TRACKER)
    n_bytes_allocated -= n_bytes_to_free;
#endif /* MEM_TRACKER */
    free(ptr);
#endif /* USE_STATIC_MEMORY */
  }
#if defined(MEM_TRACKER)
  else
  {
    if (ptr != 0)
    {
      fprintf(stderr, "Error: attempt to free unallocated memory\n");
    }
  }
#endif /* MEM_TRACKER */
}


int jbi_jtag_io(int tms, int tdi, int read_tdo) {
  int tdo = 0;
  //int i = 0;
  //int result = 0;
  //char ch_data = 0;

  //if (!jtag_hardware_initialized)
  //{
  //  initialize_jtag_hardware();
  //  jtag_hardware_initialized = TRUE;
  //}


  // set TDI and TMS to correct value
  if(tdi)
    gpio_set_tdi();
  else
    gpio_clear_tdi();

  if(tms)
    gpio_set_tms();
  else
    gpio_clear_tms();

  if (read_tdo) {
    // Read TDO
    tdo = (int)gpio_get_tdo();
  }

  // Pulse TCK
  gpio_set_tck();
  gpio_clear_tck();

  //if (tck_delay != 0) delay_loop(tck_delay);

  return (tdo);
}


void jbi_message(char *message_text) {
  if(IOStream)IOStream->print("[MSG] ");
  if(IOStream)IOStream->println(message_text);
}


void jbi_export_integer(char *key, long value) {
  if(IOStream)IOStream->print("Export: key = ");
  if(IOStream)IOStream->print(key);
  if(IOStream)IOStream->print(", value = ");
  if(IOStream)IOStream->println(value);
}


#define HEX_LINE_CHARS 72
#define HEX_LINE_BITS (HEX_LINE_CHARS * 4)

char conv_to_hex(unsigned long value) {
  char c;
  if (value > 9) {
    c = (char) (value + ('A' - 10));
  } else {
    c = (char) (value + '0');
  }
  return (c);
}


void jbi_export_boolean_array(char *key, unsigned char *data, long count) {
  char string[HEX_LINE_CHARS + 1];
  long i, offset;
  unsigned long size, line, lines, linebits, value, j, k;

  if (count > HEX_LINE_BITS) {
    if(IOStream)IOStream->print("Export: key = ");
    if(IOStream)IOStream->print(key);
    if(IOStream)IOStream->print(", ");
    if(IOStream)IOStream->print(count);
    if(IOStream)IOStream->println(" bits, value = HEX");
    lines = (count + (HEX_LINE_BITS - 1)) / HEX_LINE_BITS;

    for (line = 0; line < lines; ++line) {
      if (line < (lines - 1)) {
        linebits = HEX_LINE_BITS;
        size = HEX_LINE_CHARS;
        offset = count - ((line + 1) * HEX_LINE_BITS);
      } else {
        linebits = count - ((lines - 1) * HEX_LINE_BITS);
        size = (linebits + 3) / 4;
        offset = 0L;
      }

      string[size] = '\0';
      j = size - 1;
      value = 0;

      for (k = 0; k < linebits; ++k) {
        i = k + offset;
        if (data[i >> 3] & (1 << (i & 7))) value |= (1 << (i & 3));
        if ((i & 3) == 3) {
          string[j] = conv_to_hex(value);
          value = 0;
          --j;
        }
      }
      if ((k & 3) > 0) string[j] = conv_to_hex(value);

      if(IOStream)IOStream->println(string);
    }

    //fflush(stdout);
  } else {
    size = (count + 3) / 4;
    string[size] = '\0';
    j = size - 1;
    value = 0;

    for (i = 0; i < count; ++i) {
      if (data[i >> 3] & (1 << (i & 7))) value |= (1 << (i & 3));
      if ((i & 3) == 3) {
        string[j] = conv_to_hex(value);
        value = 0;
        --j;
      }
    }
    if ((i & 3) > 0) string[j] = conv_to_hex(value);

    if(IOStream)IOStream->print("Export: key = ");
    if(IOStream)IOStream->print(key);
    if(IOStream)IOStream->print(", ");
    if(IOStream)IOStream->print(count);
    if(IOStream)IOStream->print(" bits, value = HEX ");
    if(IOStream)IOStream->println(string);
    //fflush(stdout);
  }
}


void jbi_delay(long microseconds) {
  if (microseconds < 16383) {
    delayMicroseconds(microseconds);
  } else {
    delay(microseconds / 1000);
  } 
}
