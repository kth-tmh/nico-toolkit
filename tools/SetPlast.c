/* ------------------------------------------------------------------------- */
/*                 Set plasticity of connections in an RTDNN                 */
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
  printf("USAGE: SetPlast [option] From To Net\n");
  printf("       Option                                                        Default\n");
  printf("       -w float     Plasticity value                                 (0.0)\n");

  exit(0);
}  


int
main(int argc, char *argv[]) {   
  Net  *net;
  Connection *c;
  char *NetName, *Source, *Target;
  int i, s, t;
  int sw;
  float plasticty = 0.0;

  if (argc == 1) usage();

  InitCommand(argv, argc);
  while (is_option()) {
    switch (sw = GetOption()) {
      case 'w':  
        plasticty = GetFloat("Value of plasticity", 0.0, 10000.0); 
        break;
      default: ErrorExit(SYSTEM_ERR_EXIT, "Unknown switch %c", sw);
    }
  }
  Source = GetName("Name of the source-group"); 
  Target = GetName("Name of the target-group"); 
  NetName = GetName("Name of the Net"); 
  EndCommand();

  net = LoadNet(NetName); 
  s = GetId(net, Source, UNSPEC); 
  if (s == -1) ErrorExit(SYSTEM_ERR_EXIT, "Group name %s not found.", Source);

  t = GetId(net, Target, -1); 
  if (t == -1) ErrorExit(SYSTEM_ERR_EXIT, "Group name %s not found.", Target);

  SetAllConnectionMarks(net, 0);

  MarkConnections(net, s, t, 1);
  for (i = 0; i < net->NumConnections; i++) {
    c = net->CTab[i];
    if (c->mark) c->plast = plasticty;
  }

  AddLogEntry(net->logbook, argv, argc);
  SaveNet(NetName, net);
  FreeNet(net); 

  return SYSTEM_OK_EXIT;
}

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  SetPlast.c                                       */
/* --------------------------------------------------------------------------*/
