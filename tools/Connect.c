/* ------------------------------------------------------------------------- */
/*                       Adds connections to an RTDNN                        */
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
#include "Math.h"

void 
usage(void) {
  printf("USAGE: Connect [option] From To Net\n");
  printf("       Option                                                        Default\n");
  printf("       -d delay     Delay connections (or lookahead if negative)     (0 delay)\n");
  printf("       -D from to   All delays between 'From' and 'To'               (off)\n");
  printf("       -r bound     Bound on random initialisation                   (0.10)\n");
  printf("       -2           2-way connection, i.e., both ways                (off)\n");
  printf("       -w w         Constant connection                              (rand)\n");
  printf("       -s frac      Sparse conection. 'frac' is %% connection prob.   (off)\n");

  exit(0);
}  


int
main(int argc, char *argv[]) {   
  Net  *net;
  char *NetName, *Source, *Target;
  int t, s;
  int sw, delay, d_from = 0, d_to = 0;
  int fullflag = 0, wflag = 0, sparse = 0;
  float Fraction = 1.0;
  float RandBound = 0.1;
  float w = 0.0;

  if (argc == 1) usage();

  InitCommand(argv, argc);
  while (is_option()) {
    switch (sw = GetOption()) {
      case '2':  
        fullflag = 1;
        break;
      case 'd':  
        d_from = d_to = GetInt("Time-delay", -50, 50); 
        break;
      case 'D':  
        d_from = GetInt("Time-delay (first arg of -D option)", -50, 50); 
        d_to = GetInt("Time-delay (second arg of -D option)", -50, 50); 
        if (d_from > d_to) {
          ErrorExit(SYSTEM_ERR_EXIT, 
            "Second argumet of -D option is greater than the first.");
        }
        break;
      case 'w':  
        wflag = 1;
        w = GetFloat("Value of constant connection weight", -1000, 1000.0); 
        break;
      case 'r':
        RandBound = GetFloat("Init random bound", 0.0, 100.0);
        break;
      case 's':
        Fraction = GetFloat("Fraction of connections (%%) added", 0.0, 100.0);
        sparse = 1;
        break;
      default:
        ErrorExit(SYSTEM_ERR_EXIT, "Unknown switch %c.\n", sw);
    }
  }

  Source = GetName("Name of the source-group"); 
  Target = GetName("Name of the target-group"); 
  NetName = GetName("Name of the Net"); 
  EndCommand();

  net = LoadNet(NetName); 

  s = GetId(net, Source, -1); 
  if (s == -1) ErrorExit(SYSTEM_ERR_EXIT, "Group name %s not found.", Source);

  t = GetId(net, Target, -1); 
  if (t == -1) ErrorExit(SYSTEM_ERR_EXIT, "Group name %s not found.", Target);

  Randomize();
  for (delay = d_from; delay <= d_to; delay++) {

    if (wflag) {
      ConstConnect(net, s, t, delay, w);
    } 
    else if (sparse) {
      SparseConnect(net, s, t, delay, RandBound, Fraction);
      if (fullflag) Connect(net, t, s, delay, RandBound);
    }
    else {
      Connect(net, s, t, delay, RandBound);
      if (fullflag) Connect(net, t, s, delay, RandBound);
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
/*           END OF FILE :  Connect.c                                        */
/* --------------------------------------------------------------------------*/
