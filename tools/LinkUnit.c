/* ------------------------------------------------------------------------- */
/*          Links a unit to an element of a stream of external data          */
/* ------------------------------------------------------------------------- */
/*                                                                           */
/*   Copyright (C) 1995, 1996, 1997, 1998 Nikko Ström                        */
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
#include "System.h"
#include "Command.h"
#include "Math.h"

void 
usage(void) {
  printf("USAGE: LinkUnit Stream Position Unit Net\n");
  exit(0);
}  


int
main(int argc, char *argv[]) {   
  Stream *str;
  char *NetName, *Stream, *Unit;
  Net  *net;
  int u, s, sw, pos;

  if (argc == 1) usage();

  InitCommand(argv, argc);
  while (is_option()) {
    switch (sw = GetOption()) {
      default: ErrorExit(SYSTEM_ERR_EXIT, "Unknown switch %c", sw);
    }
  }
  Stream = GetName("Name of the stream"); 
  pos = GetInt("Position in stream", 0, 100);
  Unit = GetName("Name of the unit"); 
  NetName = GetName("Name of the Net"); 
  EndCommand();

  net = LoadNet(NetName); 
  s = GetId(net, Stream, STREAM); 
  if (s == -1) ErrorExit(SYSTEM_ERR_EXIT, "Stream name %s not found.", Stream);

  u = GetId(net, Unit, UNIT); 
  if (u == -1) ErrorExit(SYSTEM_ERR_EXIT, "Name of unit (%s) not found.", Unit);

  str = GetPtr(net, s);
  if (pos >= str->size) {
    ErrorExit(SYSTEM_ERR_EXIT, "Position (%i) greater than size of stream.", pos);
  }
  LinkUnit(net, s, pos, u);

  AddLogEntry(net->logbook, argv, argc); 
  SaveNet(NetName, net);
  FreeNet(net); 

  return SYSTEM_OK_EXIT;
}

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  LinkUnit.c                                       */
/* --------------------------------------------------------------------------*/
