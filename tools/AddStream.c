/* ------------------------------------------------------------------------- */
/*            Adds a Stream to a Recurrent Time Delay Neural Net             */
/* ------------------------------------------------------------------------- */
/*                                                                           */
/*   Copyright (C) 1994, 1995, 1996, 1997, 1998 Nikko Ström                  */
/*                                                                           */
/*   All rights reserved.                                                    */
/*                                                                           */
/*   Developed by Nikko Ström at the Dept. of Speech, Music and Hearing,     */
/*   KTH (Royal Institute of Technology), Sweden.                            */
/*                                                                           */
/*   Nikko Ström, nikko@speech.kth.se                                        */
/*                                                                           */
/*   KTH                                                                     */
/*   Institutionen för Tal, musik och hörsel                                 */
/*   S-100 44 STOCKHOLM                                                      */
/*   SWEDEN                                                                  */
/*                                                                           */
/*   http://www.speech.kth.se/                                               */
/*                                                                           */
/* ------------------------------------------------------------------------- */
/*                                                                           */
/*         This software is part of the NICO toolkit for developing          */
/*                  Recurrent Time Delay Neural Networks                     */
/*                                                                           */
/* Permission to use, copy, or modify these programs and their documentation */
/* for educational and research purposes only and without fee is hereby      */
/* granted, provided that this copyright and permission notice appear on all */
/* copies and supporting documentation.  For any other uses of this software,*/
/* in original or modified form, including but not limited to distribution   */
/* in whole or in part, specific prior permission from the copyright holder  */
/* must be obtained. The copyright holder makes no representations about the */
/* suitability of this software for any purpose. It is provided "as is"      */
/* without express or implied warranty.                                      */
/*                                                                           */
/* ------------------------------------------------------------------------- */

#include <stdlib.h>
#include <stdio.h>
#include "RTDNN.h"
#include "ParamData.h"
#include "System.h"
#include "Command.h"
#include "Math.h"


void 
usage(void) {
  printf("USAGE: AddStream [options] Size Mode Name Net\n");
  printf("         Mode ::=  r | w | t | i | 0\n");
  printf("           (read, write, target, interactive or no-action)\n");
  printf("       Option                                                        Default\n");
  printf("       -x         Specify file extension                             (data)\n");
  printf("       -d         Specify default directory                          (.)\n");
  printf("       -f filter  Data will be piped through 'filter'                (off)\n");
  printf("       -F format  Specify file format (ascii, binary, htk etc.)      (binary)\n");
  printf("       -S file    Load component names from rows of 'file'           (unnamed)\n");

  exit(0);
}  


int
main(int argc, char *argv[]) {  
  Net  *net;
  char *ext = {"data"}; 
  char *dir = {"."}; 
  char *format; 
  ParamFileType f = BINARY;
  int size, n;
  int named_comp = 0;
  StreamType mode = NOACTION;
  Stream *str;
  char sw, *NetName, *StreamName, *Mode, *Filter = NULL;
  char *ScriptFile = NULL;
  char **NameList = NULL;
  int  i;

  if (argc == 1) usage();

  InitCommand(argv, argc);
  while (is_option()) {
    switch (sw = GetOption()) {
      case 'S':  
        named_comp = 1;
        ScriptFile = GetName("Name of script file holding unit names");
        break;
      case 'x':
        ext = GetName("File Extension");  
        break;
      case 'd':  
        dir = GetName("Default directory");  
        break;
      case 'f':  
        Filter = GetName("Pipe command");  
        break;
      case 'F': 
        f = Str2ParamType(format = GetName("Parmater format"));
        if (f == -1)
          ErrorExit(SYSTEM_ERR_EXIT, "Unknown format %s.", format);
        break;
      default: ErrorExit(SYSTEM_ERR_EXIT, "Unknown switch %c.", sw);
    }
  }
  size = GetInt("Floats/Frame", 1, 10000); 
  Mode = GetName("Mode of the stream (r, w, t or 0)"); 
  StreamName = GetName("Name of the group"); 
  NetName = GetName("Name of the Net"); 
  EndCommand();

  if (strlen(Mode) != 1) 
    ErrorExit(SYSTEM_ERR_EXIT, "Mode of the stream must be r, w, t, i or 0");
  switch (Mode[0]) {
    case 'r':
      mode = READ;
      break;

    case 'w':
      mode = WRITE;
      break;

    case 't':
      mode = TARGET;
      break;

    case 'i':
      mode = INTERACTIVE;
      break;

    case '0':
      mode = NOACTION;
      break;

    default: ErrorExit(SYSTEM_ERR_EXIT, "Mode of the stream must be r, w, t, i or 0");
  }

  if (named_comp) {
    NameList = LoadObjectList(ScriptFile, &n);
    if (n != size) ErrorExit(SYSTEM_ERR_EXIT, 
      "The script (%s) contains %i names, but the stream has %i components",
      ScriptFile, n, size);
  }
 
  net = LoadNet(NetName); 
  str = AddStream(net, StreamName, mode, size);
  Join(net, net->rootgroup->id, str->id);

  str->ext     = ext;
  str->size    = size;
  str->path    = dir;
  str->format  = f;

  if (Filter) {
    FREE(str->Filter)
    STRDUP(str->Filter, Filter)
  }

  if (named_comp) {
    for (i = 0; i < size; i++) {
      FREE(str->CompName[i])
      STRDUP(str->CompName[i], NameList[i])
    }
  }

  AddLogEntry(net->logbook, argv, argc); 
  SaveNet(NetName, net);

#ifdef TRAILING_MEMFREE
  if (named_comp) {
    FREE2D(NameList, n);
  }
  FreeNet(net); 
#endif

  return SYSTEM_OK_EXIT;
}

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  AddStream.c                                      */
/* --------------------------------------------------------------------------*/
