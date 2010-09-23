/* ------------------------------------------------------------------------- */
/*                "Kick" a RTDNN by changing the weights                     */
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
#include <math.h>
#include <string.h>
#include <unistd.h>
#include "nMath.h"
#include "RTDNN.h"
#include "Command.h" 
#include "System.h" 
#include "Simulation.h" 


void 
usage(void) {
  printf("USAGE: KickNet [options] Net\n");
  printf("       Option                                                     Default\n");
  printf("       -m o1 o2  Select connections from o1 to o2                 (all)\n");
  printf("       -d div    Divide selected weights by 'div'                 (2.0)\n");
  printf("       -R p      Relatively randomize selected weights (max p%%)  (div by 2)\n");
  printf("       -r m      Initialize randomize selected weights (abs max m)(div by 2)\n");

  exit(0);
}


int
main(int argc, char *argv[]) {       
  Net  *ann;
  char *NetName;
  Connection *c;
  int   random_flag = 0, rel_random_flag = 0, mark_flag = 0;
  float divisor = 2.0, random_bound = 0.0;
  char  sw, *group1, *group2;
  int   g1, g2;
  int   i;

  if (argc == 1) usage();

  InitCommand(argv, argc);
  while (is_option()) {
    switch (sw = GetOption()) {
    case 'd':
      divisor = GetFloat("Divisor for weigths", 0.1, 100.0);
      break; 
    case 'r':
      random_bound = GetFloat("Bound for random weights", 0.0, 10.0);
      random_flag = 1;
      break; 
    case 'R':
      random_bound = GetFloat("Bound for relative random weights (%%)", 
        0.0, 200.0);
      random_bound /= 100.0;
      rel_random_flag = 1;
      break; 
    case 'm':  
      mark_flag = 1;
      GetName("from-object for s-option");
      GetName("to-object for s-option");
      break; /* Save this for the next pass */
    default: ErrorExit(SYSTEM_ERR_EXIT, "Unknown switch %c.", sw);
    }
  } 
  NetName = GetName("Name of neural net definition file"); 
  EndCommand();

  ann = LoadNet(NetName);

  if (mark_flag) {
    UnMarkAll(ann, -1);
    InitCommand(argv, argc);
    for (i = 1; i < argc - 1; i++) {
      if (is_option()) {
        if (GetOption() == 'm') {

          group1 = GetName("First arg of -s option");
          i++;
          if ((g1 = GetId(ann, group1, -1)) == -1) 
            ErrorExit(SYSTEM_ERR_EXIT, "Object %s does not exist.", group1);

          group2 = GetName("Second arg of -s option");
          i++;
          if ((g2 = GetId(ann, group2, -1)) == -1) 
            ErrorExit(SYSTEM_ERR_EXIT, "Object %s does not exist.", group2);
 
          MarkConnections(ann, g1, g2, 1);
        }
      }
      else SkipArg();
    }
  }

  AddLogEntry(ann->logbook, argv, argc); 

  if (random_flag) Randomize();

  for (i = 0; i < ann->NumConnections; i++) {
    c = ann->CTab[i];
    if (!c->mark || c->plast == 0.0) continue;

    if (random_flag) {
      c->w = RectRand() * 2.0 * random_bound - random_bound; 
    }
    else if (rel_random_flag) {
      c->w += c->w * (RectRand() * 2.0 * random_bound - random_bound); 
    }
    else {
      c->w /= divisor;
    }
  }

  SaveNet(NetName, ann); 

  FreeNet(ann);

  return SYSTEM_OK_EXIT;
}

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  KickNet.c                                        */
/* --------------------------------------------------------------------------*/
