/* ------------------------------------------------------------------------- */
/*               Import an object from one network to another.               */
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
  printf("USAGE: Import Object SourceNet Net\n");
  printf("       Option                                                      Default\n");
  printf("       -m Group   Move 'Object' to 'Group' in 'Net'                (network)\n");

  exit(0);
}  


int
main(int argc, char *argv[]) {   
  char *SourceNet, *TargetNet, *Object, *Parent = NULL;
  Net  *net1, *net2;
  int obj, new, np, move_flag = 0;
  char sw;

  if (argc == 1) usage();

  InitCommand(argv, argc);
  while (is_option()) {
    switch (sw = GetOption()) {
      case 'm':  
        Parent = GetName("Parent for copied object"); 
        move_flag = 1;
        break;
      default: ErrorExit(SYSTEM_ERR_EXIT, "Unknown switch %c", sw);
    }
  }
  Object    = GetName("Name of the object"); 
  SourceNet = GetName("Name of the source net"); 
  TargetNet = GetName("Name of the net"); 
  EndCommand();

  net1 = LoadNet(SourceNet); 
  net2 = LoadNet(TargetNet);
 
  if ((obj = GetId(net1, Object, UNSPEC)) == -1)
    ErrorExit(SYSTEM_ERR_EXIT, "No Object %s in %s", Object, SourceNet);
 
  new = Import(net1, obj, net2);

  if (move_flag) {
    if ((np = GetId(net2, Parent, UNSPEC)) == -1) {
      ErrorExit(SYSTEM_ERR_EXIT, "Parent (%s) non-existent.", Parent);
    }
  }
  else np = net2->rootgroup->id;

  Join(net2, np, new);

  OrderUnits(net2);
  SortConnections(net2);

  AddLogEntry(net2->logbook, argv, argc); 

  SaveNet(TargetNet, net2);
  FreeNet(net1); 
  FreeNet(net2); 

  return SYSTEM_OK_EXIT;
}

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  Import.c                                         */
/* --------------------------------------------------------------------------*/
