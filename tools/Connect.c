/* ------------------------------------------------------------------------- */
/*                       Adds connections to an RTDNN                        */
/* ------------------------------------------------------------------------- */
/*                                                                           */
/*   Copyright (C) 1994, 1995, 1996, 1997, 1998 Nikko Strom                  */
/*                                                                           */
/*   All rights reserved.                                                    */
/*                                                                           */
/*   Developed by Nikko Strom at the Dept. of Speech, Music and Hearing,     */
/*   KTH (Royal Institute of Technology), Sweden.                            */
/*                                                                           */
/*   http://www.speech.kth.se                                                */
/*                                                                           */
/*   KTH                                                                     */
/*   Institutionen for Tal, musik och horsel                                 */
/*   S-100 44 STOCKHOLM                                                      */
/*   SWEDEN                                                                  */
/*                                                                           */
/*   Project web site: http://nico.sourceforge.net/                          */
/*                                                                           */
/* ------------------------------------------------------------------------- */
/*                                                                           */
/*         This software is part of the NICO toolkit for developing          */
/*                  Recurrent Time Delay Neural Networks                     */
/*                                                                           */
/* Redistribution and use in source and binary forms, with or without        */
/* modification, are permitted provided that the following conditions        */
/* are met:                                                                  */
/*     * Redistributions of source code must retain the above copyright      */
/*       notice, this list of conditions and the following disclaimer.       */
/*     * Redistributions in binary form must reproduce the above copyright   */
/*       notice, this list of conditions and the following disclaimer in     */
/*       the documentation and/or other materials provided with the          */
/*       distribution.                                                       */
/*     * Neither the name of KTH or Institutionen for Tal, musik och         */
/*       horsel nor the names of its contributors may be used to endorse     */
/*       or promote products derived from this software without specific     */
/*       prior written permission.                                           */
/*                                                                           */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       */
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT         */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR     */
/* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT      */
/* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,     */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED  */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR    */
/* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF    */
/* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING      */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS        */
/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.              */
/*                                                                           */
/* ------------------------------------------------------------------------- */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "RTDNN.h"
#include "System.h"
#include "Command.h"
#include "nMath.h"

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
