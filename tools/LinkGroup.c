/* ------------------------------------------------------------------------- */
/*            Links units in a group to a stream of extenal data             */
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
  printf("USAGE: LinkGroup Stream Group Net\n");
  exit(0);
}  


int
main(int argc, char *argv[]) {   
  Stream *str;
  Group  *gr;
  char *NetName, *Stream, *Group;
  Net  *net;
  int i, g, s, sw;

  if (argc == 1) usage();

  InitCommand(argv, argc);
  while (is_option()) {
    switch (sw = GetOption()) {
      default: ErrorExit(SYSTEM_ERR_EXIT, "Unknown switch %c", sw);
    }
  }
  Stream = GetName("Name of the stream"); 
  Group = GetName("Name of the group"); 
  NetName = GetName("Name of the Net"); 
  EndCommand();

  net = LoadNet(NetName); 

  s = GetId(net, Stream, STREAM); 
  str = GetPtr(net, s);
  if (s == -1) ErrorExit(SYSTEM_ERR_EXIT, "Stream name %s not found.", Stream);

  g = GetId(net, Group, GROUP); 
  gr = GetPtr(net, g);
  if (g == -1) ErrorExit(SYSTEM_ERR_EXIT, "Name of group (%s) not found.", Group);

  if (gr->NumMem != str->size)  {
    ErrorExit(SYSTEM_ERR_EXIT, 
      "The group must contain exactly %i units and nothing else.", str->size);
  }
  for (i = 0; i < gr->NumMem; i++) {
    if (GetIdType(net, gr->MemTab[i]) != UNIT)  {
      ErrorExit(SYSTEM_ERR_EXIT, "The group must contain exactly %i units and nothing else.",
                str->size);
    }
  }

  for (i = 0; i < gr->NumMem; i++) LinkUnit(net, s, i, gr->MemTab[i]);

  AddLogEntry(net->logbook, argv, argc); 
  SaveNet(NetName, net);
  FreeNet(net); 

  return SYSTEM_OK_EXIT;
}

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  LinkGroup.c                                      */
/* --------------------------------------------------------------------------*/


