/* ------------------------------------------------------------------------- */
/*       Speed and acceleration of activities of the units of a group        */
/* ------------------------------------------------------------------------- */
/*                                                                           */
/*   Copyright (C) 1996, 1997, 1998 Nikko Strom                              */
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
#include <string.h>
#include <unistd.h>
#include "RTDNN.h"
#include "System.h"
#include "Command.h"
#include "Math.h"
#include "math.h"


void 
usage(void) {
  printf("USAGE: MakeDiff [options] Group DiffGroup Net\n");
  printf("       Option                                                Default\n");
  printf("       -r lag      Max delay and look-ahead                  (2)\n");
  printf("       -p plast    Set plastictity of created connections    (0.0)\n");

  exit(0);
}  


/*  Computes the weights of the look-ahead connections for first      */
/*  derivative computation (delays are anti-symmetrical).             */
/*  'reach' is the number of look-aheads (and delays).                */
/*  The return value is a float-vector of size 'reach'.               */
/*  This regression is identical to the one used in the HTK toolkit.  */
float *
DeltaCoeff(int reach) {
  float *coeff;
  float  s;
  int    i;

  CALLOC(coeff, reach, float)

  for (i = 1, s = 0.0; i <= reach; i++) {
    coeff[i - 1] = i;
    s += (float) i * i;
  }
  s *= 2.0;
  for (i = 0; i < reach; i++) coeff[i] /= s;

  return coeff;
}


int
main(int argc, char *argv[]) {   
  Net   *net;
  char  *NetName, *GroupName;
  char  *SpeedName;
  float  plast = 0.0;
  Group *g, *speed;
  int    speedunit, ounit;
  int    i, d, N, reach = 2;
  char   sw;
  float *coeff;

  if (argc == 1) usage();

  InitCommand(argv, argc);
  while (is_option()) {
    switch (sw = GetOption()) {
      case 'p':
        plast = GetFloat("Plasticty of pipe connections", 0.0, 1.0);
        break;
      case 'r':
        reach = GetInt("Reach of regression", 1, 20);
        break;
      default: ErrorExit(SYSTEM_ERR_EXIT, "Unknown switch %c.\n", sw);
    }
  }
  GroupName = GetName("Name of the group"); 
  SpeedName = GetName("Name of the speed-group"); 
  NetName = GetName("Name of the Net"); 
  EndCommand();

  net = LoadNet(NetName); 


  /* Create the group and it's units */

  i = GetId(net, GroupName, GROUP);
  if (i == -1) ErrorExit(SYSTEM_ERR_EXIT, "Group name %s not found.", GroupName);
  g = (Group *)GetPtr(net, i);
  N = g->NumMem; 

  speed = AddNamedGroup(net, SpeedName, SIGNAL);
  Join(net, GetId(net, "network", GROUP), speed->id);
  AddUnits(net, speed->id, LINEAR, NONOUTPUT, N, 0.0);


  /* Add connctions estimating the speed */

  coeff = DeltaCoeff(reach);

  for (i = 0; i < N; i++) {
    speedunit = speed->MemTab[i];
    ounit = g->MemTab[i];

    if (GetIdType(net, ounit) != UNIT) {
      ErrorExit(SYSTEM_ERR_EXIT, "Object other than a unit found in %s.", GroupName);
    }

    for (d = 1; d <= reach; d++) {
      AddConnection(net, speedunit, ounit,  d, -coeff[d - 1])->plast = plast;
      AddConnection(net, speedunit, ounit, -d,  coeff[d - 1])->plast = plast;
    }
  }

  FREE(coeff)

  OrderUnits(net);
  SortConnections(net);

  AddLogEntry(net->logbook, argv, argc);
  SaveNet(NetName, net);
  FreeNet(net); 

  return SYSTEM_OK_EXIT;
}

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  MakeDiff.c                                       */
/* --------------------------------------------------------------------------*/
