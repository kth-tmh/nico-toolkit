/* ------------------------------------------------------------------------- */
/*           Cepstrum-transform activities of the units of a group           */
/* ------------------------------------------------------------------------- */
/*                                                                           */
/*   Copyright (C) 1996, 1997, 1998 Nikko Ström                              */
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
#include <string.h>
#include <unistd.h>
#include "RTDNN.h"
#include "System.h"
#include "Command.h"
#include "Math.h"
#include "math.h"


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
