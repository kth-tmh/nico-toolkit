/* ------------------------------------------------------------------------- */
/*           Cepstrum-transform activities of the units of a group           */
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
#include "nMath.h"
#include "nMath.h"


void 
usage(void) {
  printf("USAGE: MakeCep [options] Group CepGroup Net\n");
  printf("       Option                                                Default\n");
  printf("       -n num      Create 'num' cepstrum coefficients        (size of 'Group')\n");
  printf("       -e factor   Multiply the zeroth coeff with 'factor'   (1.0)\n");
  printf("       -s factor   Multiply non-zeroth coeff's with 'factor' (1.0)\n");
  printf("       -0          Remove the zeroth cepstrum coefficient    (keep)\n");
  printf("       -p plast    Set plastictity of created connections    (0.0)\n");

  exit(0);
}  


int
main(int argc, char *argv[]) {   
  Net  *net;
  Connection *c;
  Group *g, *cep;
  char *NetName, *GroupName;
  char *CepstrumName;
  int i, j, N, M = 0;
  int cepunit, ounit;
  int sw;
  float plast = 0.0;
  float coeff, pi_over_size, energy_factor = 1.0, scaling_factor = 1.0;
  int remove_zeroth = 1;        

  if (argc == 1) usage();

  InitCommand(argv, argc);
  while (is_option()) {
    switch (sw = GetOption()) {
      case 'p':
        plast = GetFloat("Plasticty of pipe connections", 0.0, 1.0);
        break;
      case 'n':
        M = GetInt("Number of cepstrum coefficients", 1, 1000);
        break;
      case 'c':
        CepstrumName = GetName("Cepstrum group name");
        break;
      case 'e':
        energy_factor = GetFloat("Zeroth coeff scaling", -10.0, 10.0);
        break;
      case 's':
        scaling_factor = GetFloat("Scaling for other than zeroth coeff", 
                                   -10.0, 10.0);
        break;
      case '0':
        remove_zeroth = 1;        
        break;
      default: ErrorExit(SYSTEM_ERR_EXIT, "Unknown switch %c.\n", sw);
    }
  }
  GroupName = GetName("Name of the group");
  CepstrumName = GetName("Name of the cepstrum group");
  NetName = GetName("Name of the Net"); 
  EndCommand();

  net = LoadNet(NetName); 

  /* Create the group and its units */
  i = GetId(net, GroupName, GROUP);
  if (i == -1) ErrorExit(SYSTEM_ERR_EXIT, "Group name %s not found.", GroupName);

  g = (Group *)GetPtr(net, i);

  /* Size of cepstrum in M and input in N */
  N = g->NumMem; 
  if (M && M > N) {
    ErrorExit(SYSTEM_ERR_EXIT, "Cannot create %i cep coefficients from %i input units.", 
              M, N);
  }
  if (!M) M = N;

  cep = AddNamedGroup(net, CepstrumName, SIGNAL);
  Join(net, GetId(net, "network", GROUP), cep->id);
  AddUnits(net, cep->id, LINEAR, NONOUTPUT, M, 0.0);

  pi_over_size = PI / ((float)N - 1.0);
  for (i = 0; i < M; i++) {
    cepunit = cep->MemTab[i];
    for (j = 0; j < N; j++) {
      ounit = g->MemTab[j];

      if (GetIdType(net, ounit) != UNIT) {
        ErrorExit(SYSTEM_ERR_EXIT, "Object other than a unit found in %s.", GroupName);
      }
      coeff = cos(i * j * pi_over_size);
      if (i == 0) coeff *= energy_factor;
      else coeff *= scaling_factor;
      if (i != 0 || !remove_zeroth) {
        c = AddConnection(net, cepunit, ounit, 0, coeff);
        c->plast = plast;
      }
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
/*           END OF FILE :  MakeCep.c                                        */
/* --------------------------------------------------------------------------*/
