/****************************************************************************/
/*																			*/
/*	Module:			jbimain.h												*/
/*																			*/
/*					Copyright (C) Altera Corporation 1998-2001				*/
/*																			*/
/*	Description:	Definitions of JTAG constants, types, and functions		*/
/*																			*/
/****************************************************************************/

#ifndef INC_JBIMAIN_H
#define INC_JBIMAIN_H

#include "jbiport.h"
#include "jbiexprt.h"
#include "jbijtag.h"
#include "jbicomp.h"

/****************************************************************************/
/*																			*/
/*	Function Prototypes														*/
/*																			*/
/****************************************************************************/
int jbi_strlen(char *string);
long jbi_atol(char *buffer);
void jbi_ltoa(char *buffer, long number);
char jbi_toupper(char ch);
int jbi_stricmp(char *left, char *right);
void jbi_strncpy(char *left, char *right, int count);
void jbi_make_dword(unsigned char *buf, unsigned long num);
unsigned long jbi_get_dword(unsigned char *buf);

JBI_RETURN_TYPE jbi_execute
(
  PROGRAM_PTR program,
  long program_size,
  char *workspace,
  long workspace_size,
  char *action,
  char **init_list,
  int reset_jtag,
  long *error_address,
  int *exit_code,
  int *format_version
);

JBI_RETURN_TYPE jbi_get_note
(
  PROGRAM_PTR program,
  long program_size,
  long *offset,
  char *key,
  char *value,
  int length
);

JBI_RETURN_TYPE jbi_check_crc
(
  PROGRAM_PTR program,
  long program_size,
  unsigned short *expected_crc,
  unsigned short *actual_crc
);

JBI_RETURN_TYPE jbi_get_file_info
(
  PROGRAM_PTR program,
  long program_size,
  int *format_version,
  int *action_count,
  int *procedure_count
);

JBI_RETURN_TYPE jbi_get_action_info
(
  PROGRAM_PTR program,
  long program_size,
  int index,
  char **name,
  char **description,
  JBI_PROCINFO **procedure_list
);


#endif /* INC_JBIMAIN_H */
