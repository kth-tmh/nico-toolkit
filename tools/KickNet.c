/* ------------------------------------------------------------------------- */
/*                "Kick" a RTDNN by changing the weights                     */
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
#include <math.h>
#include <string.h>
#include <unistd.h>
#include "Math.h"
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
