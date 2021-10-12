
#include "jbiarduino.h"

Stream *IOStream;
uint8_t PIN_JTAG_TCK;
uint8_t PIN_JTAG_TDI;
uint8_t PIN_JTAG_TMS;
uint8_t PIN_JTAG_TDO;

char *error_text[] =
{
/* JBIC_SUCCESS            0 */ "success",
/* JBIC_OUT_OF_MEMORY      1 */ "out of memory",
/* JBIC_IO_ERROR           2 */ "file access error",
/* JAMC_SYNTAX_ERROR       3 */ "syntax error",
/* JBIC_UNEXPECTED_END     4 */ "unexpected end of file",
/* JBIC_UNDEFINED_SYMBOL   5 */ "undefined symbol",
/* JAMC_REDEFINED_SYMBOL   6 */ "redefined symbol",
/* JBIC_INTEGER_OVERFLOW   7 */ "integer overflow",
/* JBIC_DIVIDE_BY_ZERO     8 */ "divide by zero",
/* JBIC_CRC_ERROR          9 */ "CRC mismatch",
/* JBIC_INTERNAL_ERROR    10 */ "internal error",
/* JBIC_BOUNDS_ERROR      11 */ "bounds error",
/* JAMC_TYPE_MISMATCH     12 */ "type mismatch",
/* JAMC_ASSIGN_TO_CONST   13 */ "assignment to constant",
/* JAMC_NEXT_UNEXPECTED   14 */ "NEXT unexpected",
/* JAMC_POP_UNEXPECTED    15 */ "POP unexpected",
/* JAMC_RETURN_UNEXPECTED 16 */ "RETURN unexpected",
/* JAMC_ILLEGAL_SYMBOL    17 */ "illegal symbol name",
/* JBIC_VECTOR_MAP_FAILED 18 */ "vector signal name not found",
/* JBIC_USER_ABORT        19 */ "execution cancelled",
/* JBIC_STACK_OVERFLOW    20 */ "stack overflow",
/* JBIC_ILLEGAL_OPCODE    21 */ "illegal instruction code",
/* JAMC_PHASE_ERROR       22 */ "phase error",
/* JAMC_SCOPE_ERROR       23 */ "scope error",
/* JBIC_ACTION_NOT_FOUND  24 */ "action not found",
};

#define MAX_ERROR_CODE (int)((sizeof(error_text)/sizeof(error_text[0]))+1)


unsigned char *file_buffer = NULL;
long file_length = 0L;



void jbi_set_stream(Stream &stream_port){
  IOStream = &stream_port;
  if(IOStream)IOStream->println("Stream Setting Done");
}


// TCK, TDI, TMS, TDO Pin Number
void jbi_begin(uint8_t tck, uint8_t tdi, uint8_t tms, uint8_t tdo) {
  PIN_JTAG_TCK = tck;
  PIN_JTAG_TDI = tdi;
  PIN_JTAG_TMS = tms;
  PIN_JTAG_TDO = tdo;
  if(IOStream)IOStream->println("Set Pin TCK : " + String(tck));
  if(IOStream)IOStream->println("Set Pin TDI : " + String(tdi));
  if(IOStream)IOStream->println("Set Pin TMS : " + String(tms));
  if(IOStream)IOStream->println("Set Pin TDO : " + String(tdo));

  gpio_init_jtag();
}



void jbi_exec(File &JBC_FILE, char *action) {


  JBI_RETURN_TYPE exec_result = JBIC_SUCCESS;
  JBI_RETURN_TYPE crc_result = JBIC_SUCCESS;
  unsigned short expected_crc = 0;
  unsigned short actual_crc = 0;

  int format_version = 0;

  int action_count = 0;
  int procedure_count = 0;

  long offset = 0L;
  char key[33] = {0};
  char value[257] = {0};

  int index = 0;
  char *action_name = NULL;
  char *description = NULL;
  JBI_PROCINFO *procedure_list = NULL;
  JBI_PROCINFO *procptr = NULL;

  char *workspace = NULL;
  long workspace_size = 0;
  //char *action = NULL;
  char *init_list[10];
  init_list[0] = NULL;
  int reset_jtag = 1;
  long error_address = 0L;
  int exit_code = 0;
  char *exit_string = NULL;

  uint32_t start_time = 0;
  uint32_t end_time = 0;
  int time_delta = 0;


  // Get File Size
  file_length = JBC_FILE.size();
  if(IOStream)IOStream->println("File Size : " + String(file_length));


  // Malloc
  //file_buffer = (unsigned char *) malloc((size_t) file_length);
  file_buffer = (unsigned char *) jbi_malloc(file_length);
  if (file_buffer == NULL){
    if(IOStream)IOStream->println("!!!!! malloc FAILED : " + String(file_length));
    return;
  } else {
    if(IOStream)IOStream->println("malloc Done : " + String(file_length));
  }


  // File Read
  for (long i = 0; i < file_length; i++) {
    *(file_buffer + i) = JBC_FILE.read();
  }
  if(IOStream)IOStream->println("JBC File Open Done");


  // Check CRC
  if(IOStream)IOStream->println("Check CRC");
  crc_result = jbi_check_crc(file_buffer, file_length, &expected_crc, &actual_crc);
  if (crc_result == JBIC_CRC_ERROR) {
    switch (crc_result) {
      case JBIC_SUCCESS:
        if(IOStream)IOStream->println("CRC matched: CRC value = " + String(actual_crc, HEX));
        break;

      case JBIC_CRC_ERROR:
        if(IOStream)IOStream->println("CRC mismatch: expected " + String(expected_crc, HEX) + ", actual " + String(actual_crc, HEX));
        break;

      case JBIC_UNEXPECTED_END:
        if(IOStream)IOStream->println("Expected CRC not found, actual CRC value = " + String(actual_crc, HEX));
        break;

      case JBIC_IO_ERROR:
        if(IOStream)IOStream->println("Error: File format is not recognized.");
        while(1);
        break;

      default:
        if(IOStream)IOStream->println("CRC function returned error code " + String(crc_result));
        break;
    }
  }
  if(IOStream)IOStream->println("CRC Done");


  // Display file format version
  if(IOStream)IOStream->println("Get File Info");
  jbi_get_file_info(file_buffer, file_length, &format_version, &action_count, &procedure_count);
  if (format_version == 2) {
    if(IOStream)IOStream->println("\tJam STAPL (2.0)");
  } else {
    if(IOStream)IOStream->println("\tpre-standardized Jam 1.1");
  }


  // Dump out NOTE fields
  if(IOStream)IOStream->println("Dump NOTE Fields");
  while (jbi_get_note(file_buffer, file_length, &offset, key, value, 256) == 0) {
    if(IOStream)IOStream->println("\tNOTE " + String(key) + " = " + String(value));
  }


  // Dump the action table
  if ((format_version == 2) && (action_count > 0)) {
    if(IOStream)IOStream->println("Actions available in this file:");
    for (index = 0; index < action_count; ++index) {
      jbi_get_action_info(file_buffer, file_length, index, &action_name, &description, &procedure_list);
      if (description == NULL) {
        if(IOStream)IOStream->print("\t");
        if(IOStream)IOStream->println(action_name);
      } else {
        if(IOStream)IOStream->print("\t");
        if(IOStream)IOStream->print(action_name);
        if(IOStream)IOStream->print(" / ");
        if(IOStream)IOStream->println(description);
      }
      procptr = procedure_list;
      while (procptr != NULL) {
        if (procptr->attributes != 0) {
          if(IOStream)IOStream->print("\t");
          if(IOStream)IOStream->print(procptr->name);
          if(IOStream)IOStream->print(" ");
          if(IOStream)IOStream->println((procptr->attributes == 1) ? "optional" : "recommended");
        }
        procedure_list = procptr->next;
        jbi_free(procptr);
        procptr = procedure_list;
      }
    }
  }


  // Execute the Jam STAPL ByteCode program
  start_time = millis();
  exec_result = jbi_execute(file_buffer, file_length, workspace, workspace_size, action, init_list, reset_jtag, &error_address, &exit_code, &format_version);
  end_time = millis();

  if (exec_result == JBIC_SUCCESS) {
    if (format_version == 2) {
      switch (exit_code) {
        case  0: exit_string = "Success"; break;
        case  1: exit_string = "Checking chain failure"; break;
        case  2: exit_string = "Reading IDCODE failure"; break;
        case  3: exit_string = "Reading USERCODE failure"; break;
        case  4: exit_string = "Reading UESCODE failure"; break;
        case  5: exit_string = "Entering ISP failure"; break;
        case  6: exit_string = "Unrecognized device"; break;
        case  7: exit_string = "Device revision is not supported"; break;
        case  8: exit_string = "Erase failure"; break;
        case  9: exit_string = "Device is not blank"; break;
        case 10: exit_string = "Device programming failure"; break;
        case 11: exit_string = "Device verify failure"; break;
        case 12: exit_string = "Read failure"; break;
        case 13: exit_string = "Calculating checksum failure"; break;
        case 14: exit_string = "Setting security bit failure"; break;
        case 15: exit_string = "Querying security bit failure"; break;
        case 16: exit_string = "Exiting ISP failure"; break;
        case 17: exit_string = "Performing system test failure"; break;
        default: exit_string = "Unknown exit code"; break;
      }
    } else {
      switch (exit_code) {
        case 0: exit_string = "Success"; break;
        case 1: exit_string = "Illegal initialization values"; break;
        case 2: exit_string = "Unrecognized device"; break;
        case 3: exit_string = "Device revision is not supported"; break;
        case 4: exit_string = "Device programming failure"; break;
        case 5: exit_string = "Device is not blank"; break;
        case 6: exit_string = "Device verify failure"; break;
        case 7: exit_string = "SRAM configuration failure"; break;
        default: exit_string = "Unknown exit code"; break;
      }
    }
  if(IOStream)IOStream->print("Exit code = " + String(exit_code) + "... ");
  if(IOStream)IOStream->println(exit_string);
  } else if ((format_version == 2) && (exec_result == JBIC_ACTION_NOT_FOUND)) {
          if ((action == NULL) || (*action == '\0'))
          {
            if(IOStream)IOStream->println("Error: no action specified for Jam STAPL file.\nProgram terminated.");
          }
          else
          {
            if(IOStream)IOStream->print("Error: action ");
            if(IOStream)IOStream->print(action);
            if(IOStream)IOStream->println(" is not supported for this Jam STAPL file.\nProgram terminated.");
          }
  } else if (exec_result < MAX_ERROR_CODE) {
          if(IOStream)IOStream->print("Error at address " + String(error_address) + ": ");
          if(IOStream)IOStream->print(error_text[exec_result]);
          if(IOStream)IOStream->println(".\nProgram terminated.");
  } else {
          if(IOStream)IOStream->print("Unknown error code ");
          if(IOStream)IOStream->println(exec_result);
  }


  // Print out elapsed time
  time_delta = (int) (end_time - start_time) / 1000;
  if(IOStream)IOStream->print("Elapsed time = ");
  if(IOStream)IOStream->println(String(time_delta / 3600) + ":" + String((time_delta % 3600) / 60) + ":" + String(time_delta % 60));


}


// PROGRAM (Max2, Max10 CFM[_pof])
void jbi_exec_program(File &JBC_FILE) {
  jbi_exec(JBC_FILE, "PROGRAM");
}


// CONFIGURE (Max10 SRAM)
void jbi_exec_configure(File &JBC_FILE) {
  jbi_exec(JBC_FILE, "CONFIGURE");
}
