/* ------------------------------------------------------------------------- */
/*           Test the classification peformance on the frame level           */
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
#include <math.h>
#include <string.h>
#include <unistd.h>
#include "Goodies.h"
#include "ParamData.h"
#include "RTDNN.h"
#include "Command.h"
#include "System.h" 
#include "Simulation.h"
#include "Display.h"
 
void 
usage(void) {
  printf("USAGE: CResult [options] Net Input\n");
  printf("       Option                                                        Default\n");
  printf("       -S        Treat 'Input' as a script file holding inputfiles   (off)\n");
  printf("       -m object Mark a unit or a group to be tested         (all output units)\n");
  printf("       -c        Print confusion matrix                              (off)\n");
  printf("       -n N      Show \"within-top-N\" statistics                      (off)\n");
  printf("       -x label  Exclude frames were 'label' has highest target      (off)\n");
  exit(0);
}


int
main(int argc, char *argv[]) {       

  /* Command-line control varibles... */   
  int     NumFiles, NumUnits, NumD;
  char  **MarkList;
  char   *NetName, *Input;
  char  **InList, sw;

  int     NumMarked = 0;  /* number of selected units */
  int     scriptflag = 0; /* Use file script */
  int     conf_flag = 0;  /* print confusion matrix */
 
  /* For the list of labels to exclude... */
  char  **ExcludeList;
  int     NumExcl = 0;

  /* indices and names of selected units... */
  int    *Index, *DIndex; 
  int    *Delay;
  char  **Name;  
    
  float  *A;       /* activations */
  float **D;       /* targets */

  int   **C;       /* confusion matrix */
  int    *f;       /* frequency */
  int    *h;       /* hit's */
  int    *fa;      /* false alarms */
  int     Hit = 0; /* global hit-rate */

  int     N = 0;    /* N for top-N ( N=0 => off ) */
  int    **top_n = NULL;   /* within top-N stats */
  int     *global_top_n = NULL;
  float  *TopAct;   /* sorted list for top-N */
  int    *TopIndex; /*       - "" -          */

  NetWork *net;   
  Net    *ann;   
  Unit   *u;
  float   max_o, max_t, tmp;
  int     i, ii, j, k, o = 0, t = 0, n, obj, inc_flag, flag;
  int     T, TotT = 0;


  if (argc == 1) usage();

  CALLOC(MarkList, 128, char *)
  CALLOC(ExcludeList, 128, char *)

  InitCommand(argv, argc);
  while (is_option()) {
    switch (sw = GetOption()) {
      case 'S':
        scriptflag = 1;
        break;
      case 'c':
        conf_flag = 1;
        break;
      case 'n':
        N = GetInt("N in \"top-N\"", 1, 10000);
        break;
      case 'm': 
        MarkList[NumMarked] = GetName("-m option");
        NumMarked++;
        break;
      case 'x': 
        ExcludeList[NumExcl] = GetName("-m option");
        NumExcl++;
        break;
      default: ErrorExit(SYSTEM_ERR_EXIT, "Unknown switch %c.", sw);
    } 
  } 
  NetName = GetName("Name of Net"); 
  Input = GetName("Name of Input file or (with -S option) Script file");
  EndCommand();

  if (scriptflag) {
    InList = LoadFileList(Input, "", "", &NumFiles);
  }
  else {
    NumFiles = 1;
    CALLOC(InList, 1, char *)
    STRDUP(InList[0], Input)
  } 

  ann = LoadNet(NetName);
  net = Compile(ann);  
  NumUnits = net->NumUnits;
  NumD = net->NumExt;

  UnMarkAll(ann, -1);
  if (NumMarked != 0) {
    for (i = 0; i < NumMarked; i++) {
      obj = GetId(ann, MarkList[i], -1);
      if (GetIdType(ann, obj) != UNIT && GetIdType(ann, obj) != GROUP) {
        ErrorExit(SYSTEM_ERR_EXIT, "Only groups and units can be marked here.");
      }
      RecursiveMark(ann, obj, 1);
    }
  }
  else { /* No marked objects - mark all output units. */
    for (i = 0; i < ann->NumUnits; i++) {
      if (ann->UTab[i]->backtype != NONOUTPUT) {
        ann->UTab[i]->mark = 1;
      }
    }
  }

  /* Exclude units in the ExcludeList */
  for (i = 0; i < NumExcl; i++) { 
    obj = GetId(ann, ExcludeList[i], UNSPEC);
    if (GetIdType(ann, obj) != UNIT && GetIdType(ann, obj) != GROUP) {
      ErrorExit(SYSTEM_ERR_EXIT, "Only groups and units can be excluded here.");
    }
    RecursiveUnMark(ann, obj, 1);
  }

  /* Count the number of marked units */
  NumMarked = 0;
  for (i = 0; i < ann->NumUnits; i++) if (ann->UTab[i]->mark) NumMarked++;

  if (NumMarked == 0) ErrorExit(SYSTEM_ERR_EXIT, "No units selected for test.");

  if (N > NumMarked) 
    ErrorExit(SYSTEM_ERR_EXIT, "N in \"top-N\" larger than number of selected units.");

  CALLOC(Name, NumUnits, char *)
  CALLOC(Index, NumUnits, int)
  CALLOC(DIndex, NumUnits, int)
  CALLOC(Delay, NumUnits, int)
  CALLOC(TopIndex, NumUnits, int)
  CALLOC(TopAct, NumUnits, float)

  for (i = 0, j = 0; i < net->NumUnits; i++) {
    if (ann->UTab[i]->mark && ann->UTab[i]->backtype != NONOUTPUT) {
      u = ann->UTab[i];
      Index[j] = u->index;
      DIndex[j] = net->Xindex[u->index];
      Delay[j] = net->Xlag[u->index];
      Name[j] = u->Name;
      if (strcmp(Name[j], "") == 0) Name[j] = "<unnamed>";
      j++;
    }
  }
  NumMarked = j;

  REALLOC(Index, j, int)
  REALLOC(DIndex, j, int)
  REALLOC(Delay, j, int)
  REALLOC(TopIndex, j, int)
  REALLOC(TopAct, j, float)

  CALLOC2D(C, NumMarked,NumMarked, int)
  CALLOC(f, NumMarked, int)
  CALLOC(fa, NumMarked, int)
  CALLOC(h, NumMarked, int)
  if (N > 0) {
    CALLOC2D(top_n, NumMarked, N, int)
    CALLOC(global_top_n, N, int)
  }

  for (k = 0; k < NumFiles; k++) {
    LoadExtData(net, InList[k], 1, 1);
    Forward(net);
    A = net->A + net->MaxDelay * NumUnits;
    D = net->ExtData;
    T = net->T;

    for (i = 0; i < T; i++) { 
      max_o = max_t = -1000.0;
      inc_flag = 1; 

      /* find the highest activity and target */
      for (j = 0; j < NumMarked; j++) {

        if (i - Delay[j] >= T || i - Delay[j] < 0) {
          inc_flag = 0;
          break;
        }

        if ((tmp = D[i][DIndex[j]]) > max_t) { 
          t = j;
          max_t = tmp;
        }
        if ((tmp = A[(i - Delay[j]) * NumUnits + Index[j]]) > max_o) {
          o = j;
          max_o = tmp;
        } 
      }
      if (max_t < 0.5) inc_flag = 0;

      /* This is the "top-N" part... */
      if (N && inc_flag && max_t > 0.5) {
        /* i) sort the acts */
        for (j = 0; j < NumMarked; j++) {
          TopIndex[j] = j;
          TopAct[j] = A[(i - Delay[j]) * NumUnits + Index[j]];
          n = j;
          while (n > 0 && TopAct[n] > TopAct[n - 1]) {
            tmp = TopAct[n];
            TopAct[n] = TopAct[n - 1];
            TopAct[n - 1] = tmp;

            ii = TopIndex[n];
            TopIndex[n] = TopIndex[n - 1];
            TopIndex[n - 1] = ii;

            n--;
          }
        }
        /* ii) collect the top-N stats */
        for (j = 0, flag = 0; j < N; j++) {
          if (TopIndex[j] == t) flag = 1;
          if (flag) {
            top_n[t][j]++;
            global_top_n[j]++;
          }
        }
      }


      /* collect the "frame correct" stat's */
      if (inc_flag) {
        f[t]++;
        if (o == t) {
          Hit++;
          h[t]++;
        }
        else fa[o]++;
        C[t][o]++;
      }
      else TotT--; /* this frame will not contribute to the statistics! */
    }  
    TotT += T;
    FreeExtData(net);
  } 

  printf("NICO -- Frame-Level Classification Statistics\n\n"); 
  printf("Number of Frames: %i\n", TotT);
  printf("Frames Correct: %3.1f %%\n", ((float)100 * Hit) / (float)TotT);
  if (N) {
    for (j = 1; j < N; j++) {
      printf("   Top-%i: %5.1f %%\n", j + 1,
        global_top_n[j] == 0 ? 0.0 : (100.0 * global_top_n[j]) / (float)TotT);
    } 
  }
  printf("\n");

  printf("Class       Freq    Correct[%%]    False Alarms[%%]");
  if (N > 0) {
    printf("  "); 
    for (j = 1; j < N; j++) printf("Top-%-4i", j + 1); 
    printf("\n"); 
  }
  else printf("\n");
  for (i = 0; i < NumMarked; i++) {
    printf("%-9s%7i%11.1f%11.1f", Name[i], f[i],
           f[i]==0 ? 0.0 : (100.0 * h[i]) / f[i],
           f[i]==0 ? 0.0 : (100.0 * fa[i]) / f[i]);
    if (N) {
      printf("             ");
      for (j = 1; j < N; j++)  printf("%5.1f   ", 
        top_n[i][j]==0 ? 0.0 : (100.0 * top_n[i][j]) / f[i]); 
    }
    printf("\n");
  }
  
  if (conf_flag) {
    float **CC;
   
    CALLOC2D(CC, NumMarked, NumMarked, float)

    printf("\nConfusion Matrix\n");
    for (i = 0; i < NumMarked; i++) {
      for (j = 0; j < NumMarked; j++) {
        CC[i][j] = (f[i]==0 ? 0.0 : 100.0 * (float)C[i][j] / (float)f[i]);
      }
    }

    if (NumMarked > 20) 
      PrintMatrixWithCats(CC, NumMarked, "%.0f", Name, Name, stdout);
    else
      PrintMatrixWithCats(CC, NumMarked, "%.1f", Name, Name, stdout);

    FREE2D(CC, NumMarked);
  }

  printf("--\n");

  FreeNetWork(net);
  FreeNet(ann);

  FREE(f)
  FREE(fa)
  FREE(h)
  FREE2D(C, NumMarked)
  FREE(top_n)

  FREE(Name)
  FREE(Index)
  FREE(DIndex)
  FREE(TopIndex)
  FREE(TopAct)

  return SYSTEM_OK_EXIT;
}

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  CResult.c                                        */
/* --------------------------------------------------------------------------*/
