/* ------------------------------------------------------------------------- */
/*                  Joins an object to a group or template                   */
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

void 
usage(void) {
  printf("USAGE: Join Child Parent Net\n");
  exit(0);
}  


int
main(int argc, char *argv[]) {   
  char *NetName, *Parent, *Child;
  Net  *net;
  char sw;
  int p, c;

  if (argc == 1) usage();

  InitCommand(argv, argc);
  while (is_option()) {
    switch (sw = GetOption()) {
      default: ErrorExit(SYSTEM_ERR_EXIT, "Unknown switch %c", sw);
    }
  }
  Child  = GetName("Name of the child"); 
  Parent = GetName("Name of the parent"); 
  NetName = GetName("Name of the Net"); 
  EndCommand();

  net = LoadNet(NetName); 
  if ((p = GetId(net, Parent, UNSPEC)) == -1) {
    ErrorExit(SYSTEM_ERR_EXIT, "Parent (%s) nonexistent.", Parent);
  }
  if ((c = GetId(net, Child, UNSPEC)) == -1) {
    ErrorExit(SYSTEM_ERR_EXIT, "Child (%s) nonexistent.", Child);
  }
  if (GetIdType(net, p) == GROUP) {
    if (GetIdType(net, c) != GROUP && GetIdType(net, c) != UNIT) {
      ErrorExit(SYSTEM_ERR_EXIT, 
        "Only units and sub-groups can be members of a group.");
    }
  }
  else ErrorExit(SYSTEM_ERR_EXIT, "Only templates and groups have structure.");

  Join(net, p, c);

  AddLogEntry(net->logbook, argv, argc); 
  SaveNet(NetName, net);
  FreeNet(net); 

  return SYSTEM_OK_EXIT;
}

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  Join.c                                           */
/* --------------------------------------------------------------------------*/
