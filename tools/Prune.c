/* ------------------------------------------------------------------------- */
/*             Pruning of connections based on gradient information.         */
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
#include "ParamData.h"
#include "RTDNN.h"
#include "System.h" 
#include "Command.h" 
#include "Simulation.h" 


void 
usage(void) {
  printf("USAGE: Prune [options] Net | Prune -g crit [options] Net Input\n");
  printf("       Option                                                        Default\n");
  printf("       -S                Treat 'Input' as an input file script       (off)\n");
  printf("       -M group1 group2  Mark connections from group1 to group2      (all)\n");
  printf("       -C group N M      Keep at least N and at most M inflows\n");
  printf("                         to each unit in 'group'                     (off)\n");
  printf("       -w crit           Weight criterion                            (0.10)\n");
  printf("       -g crit           Max error increment/pruned connection,\n");
  printf("                         in promille of tot error\n");

  exit(0);
}


int
main(int argc, char *argv[]) {  
  Net *ann; 
  NetWork *net; 
  Connection *c;  
  Unit *u;   
  char  OutExt[64]={"out"};
  char  OutDir[256]={"."};  
  char   *Input = NULL;
  char   *group1, *group2;
  char  **InList;    
  int     NumFiles;
  char   *Net;
  float WCrit = 0.10;
  float GCrit = 1.00; 
  float E = 0.0;
  int   scriptflag = 0, markflag = 0, order2_flag = 0;
  int   g1, g2;
  int   restricted = 0;
  int   num_marks = 0;
  char  sw;   
  int     i, j = 0, jj, k;
  float **ScoreTable;
  float   score;
  int   **IndexTable;
  int    *NumInflows;
  int    *MaxInflows = 0;
  int    *MinInflows = 0;
  
  if (argc == 1) usage();

  InitCommand(argv, argc);
  while (is_option()) {
    switch (sw = GetOption()) {
      case 'S':
        scriptflag=1;
        break; 
      case 'w':
        WCrit = GetFloat("-w option", 0.0, 100.0);
        break; 
      case 'g':
        GCrit = GetFloat("-g option", 0.0, 1000.0);
        GCrit /= 1000; /* Use promille for convienience. */
        order2_flag = 1;
        break; 
      case 'C':
        GetName("-C option, first argument");
        GetInt("-C 2nd argument", 0, 10000);
        GetInt("-C 3rd argument", 0, 10000);
        num_marks++;
        restricted = 1;
        break;  /* Save this for the next pass */
     case 'x':
        strcpy(OutExt,GetName("Output file extension"));
        break;
      case 'd':
        strcpy(OutDir,GetName("Output path name"));
        break;
      case 'M':
        GetName("First arg of -M option");  
        GetName("Second arg of -M option"); 
        num_marks++;
        markflag = 1;
        break; /* Save this for the next pass */
      default: ErrorExit(SYSTEM_ERR_EXIT, "Unknown switch %c", sw);
    }
  } 
  Net = GetName("Name of Net"); 
  if (order2_flag)
    Input = GetName("Name of Input file or (with -S option) Script file");
  EndCommand();

  if (restricted && markflag) ErrorExit(SYSTEM_ERR_EXIT, 
    "The -M option and the -C option are mutually incompatible.");

  ann = LoadNet(Net);
  net = Compile(ann);

  UnMarkAll(ann, -1);
  if (restricted || markflag) {

    if (restricted) {
      CALLOC(MinInflows, num_marks, int)
      CALLOC(MaxInflows, num_marks, int)
    }

    num_marks = 0;

    InitCommand(argv, argc);
    while (!IsEndCommand()) {
      if (is_option()) {
        sw = GetOption();
        if (sw == 'C') {
          num_marks++;

          group1 = GetName("-C option, first argument");
          if ((g1 = GetId(ann, group1, -1)) == -1) 
            ErrorExit(SYSTEM_ERR_EXIT, "Object %s does not exist.", group1);

          MinInflows[num_marks] = GetInt("-C 2nd argument", 0, 10000);
          MaxInflows[num_marks] = 
            GetInt("-C 3rd argument", MinInflows[num_marks], 10000);

          MarkConnections(ann, ann->rootgroup->id, g1, num_marks);
        }
        else if (sw == 'M') {

          group1 = GetName("Second arg of -M option");
          i++;
          if ((g1 = GetId(ann, group1, -1)) == -1) 
            ErrorExit(SYSTEM_ERR_EXIT, "Object %s does not exist.", group1);

          group2 = GetName("First arg of -M option");
          i++;
          if ((g2 = GetId(ann, group2, -1)) == -1) 
            ErrorExit(SYSTEM_ERR_EXIT, "Object %s does not exist.", group2);

          MarkConnections(ann, g1, g2, 1);
        }
      }
      else SkipArg();
    }
  }


  /* Compute the gradient if necessary */
  if (order2_flag) { 
    if (scriptflag) {
      InList = LoadFileList(Input, "", "", &NumFiles);
    }
    else {
      NumFiles = 1;
      CALLOC(InList, 1, char *)
	STRDUP(InList[0], Input)
	}
    for(k = 0; k < NumFiles; k++) {
      LoadExtData(net, InList[k], 1, 1);
      Forward(net); 
      E += Backward(net);
    }
    GCrit = GCrit * E / 1000.0;
  }

  if (restricted) { /* Restriction on number of inflows */

    CALLOC(ScoreTable, ann->NumUnits, float *)
    CALLOC(IndexTable, ann->NumUnits, int *)
    for (i = 0; i < net->NumUnits; i++) {
      CALLOC(ScoreTable[i], net->NumInflows[i], float)
      CALLOC(IndexTable[i], net->NumInflows[i], int)
    }
  
    CALLOC(NumInflows, ann->NumUnits, int);

    /* Test on all marked connections */
    for (i = 0; i < net->NumUnits; i++) {
      for (j = 0; j < net->NumInflows[i]; j++) {
        c = net->ConLink[i][j];
        u = ann->UTab[i];
        u->mark = c->mark;

        if (c->mark) {
          if (ann->CTab[c->index] == NULL || !c->mark) continue;

          score = fabs(net->InW[i][j]);
          if (order2_flag) score *= fabs(net->InGW[i][j]);

          /* Sort the ScoreTable with the new entry */
          ScoreTable[i][NumInflows[i]] = score;
          IndexTable[i][NumInflows[i]] = c->index;
          for (jj = NumInflows[i]; jj > 0; jj--) {
            if (ScoreTable[i][jj - 1] < ScoreTable[i][jj]) {
              k = IndexTable[i][jj];
              IndexTable[i][jj] = IndexTable[i][jj - 1];
              IndexTable[i][jj - 1] = k;
              score = ScoreTable[i][jj];
              ScoreTable[i][jj] = ScoreTable[i][jj - 1];
              ScoreTable[i][jj - 1] = score;
            }
          }

          NumInflows[i]++;
        }
      }
    }

    for (i = 0; i < ann->NumUnits; i++) {
      u = ann->UTab[i];

      /* Up to MaxInflows connections are allowed  */
      for (jj = MaxInflows[u->mark]; jj < NumInflows[i]; jj++) {
        DeleteConnection(ann, ann->CTab[IndexTable[i][jj]]);
      }

      /* Keep at least MinInflows connections to each unit */
      for (jj = MinInflows[u->mark]; 
           jj < MaxInflows[u->mark] - 1 && jj < NumInflows[i]; j++) {
        if (ScoreTable[i][j] < GCrit &&
            GETPTR(ann, IndexTable[i][jj], Connection)->w < WCrit)
	  DeleteConnection(ann, ann->CTab[IndexTable[i][jj]]);
      }
    }

    free(MinInflows);
    free(MaxInflows);
    FREE2D(ScoreTable, ann->NumUnits)
    FREE2D(IndexTable, ann->NumUnits)
    free(NumInflows);
  }

  else { /* No restriction on number of inflows */
    /* If no connections are marked - process all connections. */
    if (!markflag) SetAllConnectionMarks(ann, 1);

    for (i = 0; i < net->NumUnits; i++) {
      for (j = 0; j < net->NumInflows[i]; j++) {

        c = net->ConLink[i][j];

        if (c->mark) {
          if (ann->CTab[c->index] == NULL) continue; /* already deleted */

          if (c->mark && fabs(net->InW[i][j]) < WCrit && 
              (!order2_flag || fabs(net->InW[i][j] * net->InGW[i][j]) < GCrit)) {
            DeleteConnection(ann, c);
          }
        }
      } 
    }           
  }

  ReHash(ann);

  AddLogEntry(ann->logbook, argv, argc); 
  SaveNet(Net, ann); 
  FreeExtData(net);
  FreeNetWork(net);
  FreeNet(ann);

  return SYSTEM_OK_EXIT;
}

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  Prune.c                                          */
/* --------------------------------------------------------------------------*/
