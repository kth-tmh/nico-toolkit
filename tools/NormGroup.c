/* ------------------------------------------------------------------------- */
/*             Normalize the values of a group of linear units.              */
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
  printf("USAGE: NormGroup [options] Net Input\n");
  printf("       Option                                               Default\n");
  printf("       -S             Treat 'Input' as an inputfile  script (off)\n");
  printf("       -g group       Select the units in 'group'\n");
  printf("       -d mult        Center around mean\n");
  printf("                      mapping: [mean-s*mult, mean+s*mult] -> [-1,1]\n");
  printf("                                                            (min,max -> -1,1)\n");

  exit(0);
}


int
main(int argc, char *argv[]) {  
  Net      *ann;
  NetWork  *net;
  Unit         *u;
  Unit        **MarkedUnits;
  Connection   *c;
  Connection  **MarkedBias;
  Connection ***MarkedInflows;
  int          *NumInflows;
  char    sw;   
  char   *Net;
  char   *Input;    
  char  **InList;    
  int     NumFiles;
  int     NumUnits, NumMarked;
  int scriptflag=0, markflag=0, std_flag=0;
  int     i, j, k, TotT=0;
  float  *mean, *stddev, *min, *max, *sum, *sum2, var, this;
  float *A;
  float scale = 1.0;

  if (argc == 1) usage();
  InitCommand(argv, argc);
  while (is_option()) {
    switch (sw = GetOption()) {
      case 'S':
        scriptflag = 1;
        break; 
      case 'g':
        markflag = 1;
        GetName("Name of a group of linear units"); /* Leave until next pass */
        break; 
      case 'd':
        std_flag = 1;
        scale = GetFloat("Multiplier for -d option", 0.0, 1000.0);
        break; 
      default: ErrorExit(SYSTEM_ERR_EXIT, "Unknown switch %c.", sw);
    }
  } 

  if (!markflag) ErrorExit(SYSTEM_ERR_EXIT, "No units marked");

  Net = GetName("Name of Net");
  Input = GetName("Name of Input file or (with -S option) Script file");
  EndCommand();

  ann = LoadNet(Net);
  NumUnits = ann->NumUnits;

  /* Scan the arguments again to mark units in the ann */
  InitCommand(argv, argc);
  UnMarkAll(ann, UNIT);
  for (i = 1; i < argc - 2; i++) {
    char *group_name;
    int gr_id;

    if (is_option()) {
      if (GetOption() == 'g') {
        group_name = GetName("Name of a group of linear units");
        i++;
        if ((gr_id = GetId(ann, group_name, GROUP)) == -1) {
          ErrorExit(SYSTEM_ERR_EXIT, "Group %s not found.", group_name);
        }
        RecursiveMark(ann, gr_id, 1);
      }
    }
    else SkipArg();
  }

  /* Add a biases if not already there */
  for (i = 0; i < NumUnits; i++) {
    if ((u = ann->UTab[i])->mark)
       AddConnection(ann, u->id, ann->biasunit->id, 0, 0.0);
  }

  /* Count the number of units and check that they are linear */ 
  NumMarked = 0;
  for (i = 0; i < NumUnits; i++) {
    u = ann->UTab[i];

    if (u->mark && u->type != LINEAR) 
      ErrorExit(SYSTEM_ERR_EXIT, "'NormGroup' works only on linear units");
    else if (u->mark) NumMarked++;
  }

 if (NumMarked == 0) 
   ErrorExit(SYSTEM_ERR_EXIT, "No units marked for normalization");

  CALLOC(MarkedUnits, NumMarked, Unit *)
  CALLOC(MarkedBias, NumMarked, Connection *)
  CALLOC(MarkedInflows, NumMarked, Connection **)
  CALLOC(NumInflows, NumMarked, int)

  /* Set up the list of marked units */
  for (i = 0, j = 0; i < NumUnits; i++) {
    if (ann->UTab[i]->mark) MarkedUnits[j++] = ann->UTab[i];
  }

  net = Compile(ann);

  /* Set up the lists of biases and inflows to the marked units */
  for (i = 0; i < NumMarked; i++) {
    u = MarkedUnits[i];

    CALLOC(MarkedInflows[i], net->NumInflows[u->index], Connection *)
    NumInflows[i] = 0;

    for (j = 0; j < ann->NumConnections; j++) {
      c = ann->CTab[j];
      if (c->to == u->id) {
        if (c->from == ann->biasunit->id) {
          MarkedBias[i] = c;
        }
        else {
          MarkedInflows[i][NumInflows[i]++] = c;
        }
      }
    }
  }

  if (scriptflag) {
    InList = LoadFileList(Input, "", "", &NumFiles);
  }
  else {
    NumFiles = 1;
    CALLOC(InList, 1, char *)
    STRDUP(InList[0], Input)
  }

  CALLOC(max, NumMarked, float)
  CALLOC(min, NumMarked, float)
  CALLOC(sum, NumMarked, float)
  CALLOC(sum2, NumMarked, float)
  CALLOC(mean, NumMarked, float)
  CALLOC(stddev, NumMarked, float)

  /* Run the net and collect the stat's */
  for (k = 0; k < NumFiles; k++) {

    LoadExtData(net, InList[k], 1, 1);
    Forward(net);

    A = net->A + net->MaxDelay * NumUnits;

    if (k == 0) { /* Initialize stat-arrays only for the first file */
      for (j = 0; j < NumMarked; j++) {
        min[j] = max[j] = A[MarkedUnits[j]->index];
        sum[j] = sum2[j] = 0.0;
      }
    }  

    for (i = 0; i < net->T; i++) {
      for (j = 0; j < NumMarked; j++) {

       /* Sums are divided "on the fly" by the number of frames in the 
          current file - this is not to loose accuracy when processing 
          large databases  */
        this = A[i * NumUnits + MarkedUnits[j]->index];
        sum[j] += this / (float)net->T;
        sum2[j] += this * this / (float)net->T; 

        if      (this > max[j]) max[j] = this;
        else if (this < min[j]) min[j] = this;
      }
    }
    TotT += net->T;
  }

  /* Normalize using the stat's collected */

  if (std_flag) {

    /* First get the mean and stddev from the counts... */
    for (j = 0; j < NumMarked; j++) {
     /* Here sums are divided only by the number of files; Division by nymber 
        of frames was done in the previous loop */
      mean[j] = sum[j] / (float)NumFiles;
      var = sum2[j] / (float)NumFiles - mean[j] * mean[j];
      if (var <= 0.0) stddev[j] = 0.0; /* Shouldn't really be necessary */
      else stddev[j] = sqrt(var);
    }

    /* ...then alter the weights */
    for (i = 0; i < NumMarked; i++) {
      u = MarkedUnits[i];

      MarkedBias[i]->w -= mean[i];

      for (j = 0; j < NumInflows[i]; j++) {
        MarkedInflows[i][j]->w /= stddev[i] * scale;
      }
    }
  }

  else { /* [min,max] -> [-1,1] */
    for (i = 0; i < NumMarked; i++) {
      u = MarkedUnits[i];

      MarkedBias[i]->w -= (max[i] + min[i]) / 0.5 / (max[i] - min[i]);

      for (j = 0; j < NumInflows[i]; j++) {
        MarkedInflows[i][j]->w /= 0.5 * (max[i] - min[i]);
      }
    }
  }

  /* Add logbook entry and save the normalized net */
  AddLogEntry(ann->logbook, argv, argc); 
  SaveNet(Net, ann); 

  /* Clean up */
  FreeNet(ann);
  FreeNetWork(net);

  free(MarkedUnits);
  free(MarkedBias);
  free(NumInflows);
  FREE2D(MarkedInflows, NumMarked)

  free(max); free(min);
  free(sum); free(sum2);
  free(mean); free(stddev);

  return SYSTEM_OK_EXIT;
}

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  NormGroup.c                                      */
/* --------------------------------------------------------------------------*/
