/* ------------------------------------------------------------------------- */
/*                     Removes an object from a network.                     */
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
#include <unistd.h>
#include "RTDNN.h"
#include "System.h"
#include "Command.h"

void 
usage(void) {
  printf("USAGE: Remove Object Net\n");
  printf("       Option                                                        Default\n");
  printf("       -r       Delete recursively on complex objects                (off)\n");
  printf("       -c Dest  Delete connections from \"Object\" to \"Dest\".          (off)\n");

  exit(0);
}  


int
main(int argc, char *argv[]) {   
  Connection *c;
  char *NetName, *Object, *Dest = NULL;
  Net *net;
  int i, sw, obj, d, recursive=0, connections=0;

  if (argc == 1) usage();
  InitCommand(argv, argc);
  while (is_option()) {
    switch (sw = GetOption()) {
      case 'r':
        recursive = 1;
        break; 
      case 'c':
        connections = 1;
        Dest = GetName("Name of the destination object"); 
        break; 
      default: ErrorExit(SYSTEM_ERR_EXIT, "Unknown switch %c.", sw);
    }
  } 
  Object = GetName("Name of the object"); 
  NetName = GetName("Name of the Net"); 
  EndCommand();
  net = LoadNet(NetName); 
  if ((obj = GetId(net, Object, -1)) == -1) 
    ErrorExit(SYSTEM_ERR_EXIT, "No object %s in %s", Object, NetName);
 
  if (connections) {
    if ((d = GetId(net, Dest, -1)) == -1) 
      ErrorExit(SYSTEM_ERR_EXIT, "Destination object %s not found in %s", Dest, NetName);

    SetAllConnectionMarks(net, 0);

    MarkConnections(net, obj, d, 1);
    for (i = 0; i < net->NumConnections; i++) {
      c = net->CTab[i];
      if (c->mark) DeleteConnection(net, c);
    }
  }
  else DeleteObject(net, obj, recursive);

  ReHash(net);
  AddLogEntry(net->logbook, argv, argc); 
  SaveNet(NetName, net);
  FreeNet(net); 

  return SYSTEM_OK_EXIT;
}

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  Remove.c                                         */
/* --------------------------------------------------------------------------*/
