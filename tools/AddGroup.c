/* ------------------------------------------------------------------------- */
/*             Adds groups to a Recurrent Time Delay Neural Net              */
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
#include "System.h"
#include "Command.h"
#include "Math.h"

void 
usage(void) {
  printf("USAGE: AddGroup [options] Group Net\n");
  printf("       Option                                                         Default\n");
  printf("       -u number Add multiple unnamed groups. 'Group' is the parent   (off)\n");

  exit(0);
}  


int
main(int argc, char *argv[]) {   
  Net      *net;
  Group    *G;
  char   sw, *NetName, *Name;
  int    multiple = 0;
  int    Protection = PUBLIC; 
  int    n = -1, p;

  if (argc == 1) usage();

  InitCommand(argv,argc);
  while (is_option()) {
    switch (sw = GetOption()) {
       case 'u':  
        multiple = 1;
        n = GetInt("Number of groups", 0, 100);
        break;
      default: ErrorExit(SYSTEM_ERR_EXIT, "Unknown switch %c.", sw);
    }
  }
  if (multiple) Name = GetName("Name of the group");
  else  Name = GetName("Name of the parent-group");
  NetName = GetName("Name of the Net"); 
  EndCommand();
  net = LoadNet(NetName);
 
  if (multiple) {
    p = GetId(net, Name, -1);
    if (GetIdType(net, p) != GROUP) {
      ErrorExit(SYSTEM_ERR_EXIT, "Parent to multiple groups must be a group.");
    }
    AddGroups(net, p, Protection, n);
  }
  else {
    G = AddNamedGroup(net, Name, PUBLIC);
    Join(net, net->rootgroup->id, G->id);
  }

  AddLogEntry(net->logbook, argv, argc); 
  SaveNet(NetName, net);
  FreeNet(net); 

  return SYSTEM_OK_EXIT;
}

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  AddGroup.c                                       */
/* --------------------------------------------------------------------------*/
