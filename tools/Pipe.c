/* ------------------------------------------------------------------------- */
/*                        Pipe one group to another.                         */
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
#include "math.h"

void 
usage(void) {
  printf("USAGE: Pipe [options] Source Target Net\n");
  printf("       Option                                                        Default\n");
  printf("       -d delay         Connection delay                             (0)\n"); 
  printf("       -D from to       All delays between 'From' and 'To'           (off)\n");
  printf("       -r bound         Sets bound for random inititialization       (0.1)\n"); 
  printf("       -w weight        Constant weight (also sets plasticity=0)     (random)\n"); 
  printf("       -p plasticity    Set the plastictity of the connections       (1.0)\n");

  exit(0);
}  


int
main(int argc, char *argv[]) {   
  Net  *net;
  Connection *c;
  Group *tg, *sg;
  int tn, sn, tu, su, s, t;
  char *NetName, *Source, *Target;
  int i;
  int sw, w_flag = 0;
  float w, const_w = 1.0;
  float RandBound = 0.1;
  float plast = 1.0; int delay, d_from = 0, d_to = 0;


  if (argc == 1) usage();

  InitCommand(argv, argc);
  while (is_option()) {
    switch (sw = GetOption()) {
      case 'd':
        d_from = d_to = GetInt("Delay of pipe connections", -50, 50); 
        break;
      case 'D':  
        d_from = GetInt("Time-delay (first arg of -D option)", -50, 50); 
        d_to = GetInt("Time-delay (second arg of -D option)", -50, 50); 
        if (d_from > d_to) {
          ErrorExit(SYSTEM_ERR_EXIT, 
            "Second argumet of -D option is greater than the first.");
        }
        break;
      case 'p':
        plast = GetFloat("Plasticty of pipe connections", 0.0, 1.0);
        break;
      case 'r':
        RandBound = GetFloat("bound for random inititialization", 0.0, 10.0);
        plast = 0.0;
        break;
      case 'w':
        w_flag = 1;
        const_w = GetFloat("Weights of pipe connections", 0.0, 10.0);
        plast = 0.0;
        break;

      default: ErrorExit(SYSTEM_ERR_EXIT, "Unknown switch %c.", sw);
    }
  }
  Source = GetName("Name of the source-group"); 
  Target  = GetName("Name of the target-group"); 
  NetName = GetName("Name of the Net"); 
  EndCommand();
  net = LoadNet(NetName); 
  s = GetId(net, Source, GROUP);
  if (s == -1) ErrorExit(SYSTEM_ERR_EXIT, "Group name %s not found.", Source);

  sg = GETPTR(net, s, Group);
  sn = sg->NumMem; 
  t = GetId(net, Target, GROUP); 
  if (t == -1)  {
    tg = AddNamedGroup(net, Target, PUBLIC);
    Join(net, net->rootgroup->id, (t = tg->id));
    AddUnits(net, t, LINEAR, NONOUTPUT, sn, 0.0);
  }
  else tg = GETPTR(net, t, Group);
  tn = tg->NumMem; 
  if (tn != sn) {
    ErrorExit(SYSTEM_ERR_EXIT, "'%s' and '%s' have different number of members.", 
              Target, Source);
  }

  for (i = 0; i < sn; i++) {
    tu = tg->MemTab[i];
    su = sg->MemTab[i];
    if (GetIdType(net, tu) != UNIT) {
      ErrorExit(SYSTEM_ERR_EXIT, "Object other than a unit found in %s.", Target);
      exit(0);
    }
    if (GetIdType(net, su) != UNIT) {
      ErrorExit(SYSTEM_ERR_EXIT, "Object other than a unit found in %s.", Source);
    }
    
    for (delay = d_from; delay <= d_to; delay++) {
      if (w_flag) {
        c = AddConnection(net, tu, su, delay, const_w);
        c->w = const_w; /* force the new value */
      }
      else {
        w = (RectRand() * 2.0 - 1.0) / 2.0 * RandBound;
        c = AddConnection(net, tu, su, delay, w);
      }
      c->plast = plast;
    }
  }

  OrderUnits(net);
  SortConnections(net);

  AddLogEntry(net->logbook, argv, argc);
  SaveNet(NetName, net);
  FreeNet(net); 

  return SYSTEM_OK_EXIT;
}

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  Pipe.c                                           */
/* --------------------------------------------------------------------------*/
