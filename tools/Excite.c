/* ------------------------------------------------------------------------- */
/*                 Excite a Recurrent Time Delay Neural Net                  */
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
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ParamData.h"
#include "RTDNN.h"
#include "Simulation.h"
#include "System.h" 
#include "Command.h"
 

static int *Index;
static char **Name;
static int NumToShow;
static int resolution = 2;
static int eflag;


void 
print_name_line(FILE *fp) {
  int i;

  for (i = 0; i < NumToShow; i++) {
    if (resolution == 2) fprintf(fp, "%5s ", Name[i]); 
    else fprintf(fp, "%9s ", Name[i]);
  }
  fprintf(fp, "\n");
}


void 
OutputStream(NetWork *net, Stream *stream, FILE *fp) { 
  int i, t;
  float **X = net->ExtData;
  float a;
  int T = net->T;

  for (t = 0; t < T; t++) {
    for (i = 0; i < stream->size; i++) {
      a = X[t][stream->index + i];
      if (resolution == 2) fprintf(fp, "%5.2f ", a);
      else fprintf(fp, "%9.6f ", a);
    }
    fprintf(fp, "\n");
  }
  fprintf(fp,"\n");
}


void 
OutputBinaryStream(NetWork *net, Stream *stream, FILE *fp) { 
  int i, t;
  float **X = net->ExtData;
  float a;
  int T = net->T;

  for (t = 0; t < T; t++) {
    for (i = 0; i < stream->size; i++) {
      a = X[t][stream->index + i];
      fwrite(&a, 1, sizeof(float), fp);
    }
  }
}


void 
OutputActs(NetWork *net, FILE *fp, 
                int cflag, int nflag, int name_freq) {  
  int i, t;
  float *A = net->A + net->MaxDelay * net->NumUnits;
  float a;
  int N = net->NumUnits;
  int T = net->T;

  /* Read the gradients instead of 'A' if this flag is set */
  if (eflag) A = net->GA;

  if (name_freq == 0 && nflag) print_name_line(fp);

  for (t = net->MaxDelay; t < T; t++) {
    if (cflag) fprintf(fp, "%6i ", t);
    if (nflag && name_freq != 0 && t % name_freq == 0) print_name_line(fp);

    for (i = 0; i < NumToShow; i++) {
      a = A[Index[i] + t * N];
      if (resolution == 2) fprintf(fp, "%5.2f ", a);
      else fprintf(fp, "%9.6f ", a);
    }
    fprintf(fp, "\n");
  }
  fprintf(fp,"\n");
}


void 
OutputBinaryActs(NetWork *net, FILE *fp) {  
  int i, t;
  float *A = net->A + net->MaxDelay * net->NumUnits;
  float a;
  int N = net->NumUnits;
  int T = net->T;

  /* Read the gradients instead of 'A' if this flag is set */
  if (eflag) A = net->GA + net->MaxDelay * net->NumUnits;

  for (t = 0; t < T; t++) {
    for (i = 0; i < NumToShow; i++) {
      a = A[Index[i] + t * N];
      fwrite(&a, 1, sizeof(float), fp);
    }
  }
}


void 
OutputWinner(NetWork *net, FILE *fp, 
                  int cflag, int nflag, int name_freq) {  
  int i, t, maxi = 0;
  float max, this;
  float *A = net->A + net->MaxDelay * net->NumUnits;
  int N = net->NumUnits;
  int T = net->T;

  /* Read the gradients instead of 'A' if this flag is set */
  if (eflag) A = net->GA + net->MaxDelay * net->NumUnits;

  if (name_freq == 0 && nflag) print_name_line(fp);

  for (t = 0; t < T; t++) {
    if (cflag) fprintf(fp,"%6i ", t);
    if (nflag && name_freq != 0 && t % name_freq == 0) print_name_line(fp);

    max = 0.0;
    for (i = 0; i < NumToShow; i++) {
      this = A[Index[i] + t * N];
      if (this > max) {
        max  = this;
        maxi = i;
      }
    }
    for (i = 0; i < NumToShow; i++) 
      if (i == maxi) fprintf(fp, "1 "); else fprintf(fp, "0 ");
    fprintf(fp, "\n");
  }
}


void 
OutputNBest(NetWork *net, FILE *fp, int n, int cflag) {  
  int i, t, j;
  float *A = net->A + net->MaxDelay * net->NumUnits;
  int N = net->NumUnits;
  int T = net->T;
  float *max, temp;
  int *maxi, tempi;

  /* Read the gradients instead of 'A' if this flag is set */
  if (eflag) A = net->GA + net->MaxDelay * net->NumUnits;

  CALLOC(maxi, NumToShow + 1, int)
  CALLOC(max, NumToShow + 1, float)

  for (t = 0; t < T; t++) {
    if (cflag) fprintf(fp, "%6i ", t);
    for (i = 0; i < NumToShow; i++) {
      max[i]  = A[Index[i] + t * N];
      maxi[i] = i;
      if (i != 0) for (j = i; j; j--) {
        if (max[j-1] < max[j]) {
          temp     = max[j-1]; 
          max[j-1] = max[j];
          max[j]   = temp;
          tempi     = maxi[j-1]; 
          maxi[j-1] = maxi[j];
          maxi[j]   = tempi;
        }
        else continue;
      }
    }
    for (i = 0; i < n; i++) fprintf(fp, "%s\t", Name[maxi[i]]);
    fprintf(fp, "\n");
  }
  free(max);   
  free(maxi);   
}


void 
usage(void) {
  printf("USAGE: Excite [options] Net Input\n");
  printf("       Option                                                        Default\n");
  printf("       -S        Treat 'Input' as a script file holding inputfiles   (off)\n");
  printf("       -x ext    Extension for outputfiles                           (act)\n");
  printf("       -d dir    Directory to store output files in                  (current)\n");
  printf("       -m object Mark a unit, group or stream to be shown            (outputs)\n");
  printf("       -e        Show error gradients (also does '-x grad')          (act's)\n");
  printf("       -w        'Winner takes all', highest activity =1.0, others =0.0\n");
  printf("       -b num    For each frame, show the names of the n highest outputs\n");
  printf("       -c        Add a column, counting framenumber.                 (off)\n");
  printf("       -B        Output binary floats (4 bytes)                      (text)\n");
  printf("       -X stream Output a stream                                     (off)\n");
  printf("       -n freq   Print the names of the units every 'freq' frames    (off)\n");
  printf("       -h        Select high (6 figures) resolution                  (2 figures)\n");
  printf("       -T level  Trace level                                         (0)\n");
  exit(0);
}


int
main(int argc, char *argv[]) {   
  Net *ann;
  NetWork *net;
  FILE *outfp;
  Stream *str; 
  Unit *u;   
  int    i, j, n = 0, obj;
  int    scriptflag = 0, winner = 0, streamflag = 0;
  int    nbest = 0, cflag = 0;
  int    nflag = 0, binaryflag = 0;
  int    trace_level = 0; 
  int    type, name_freq = 0;
  int    NumFiles, NumMarked = 0;
  char **MarkList;
  char  *fn;
  char  *NetName, *Input;
  char   OutExt[64] = {"act"}, OutDir[256] = {"."}; 
  char **InList;
  char  *StreamName = NULL;
  Stream *stream = NULL;
  char   sw; 
  unsigned long zerotime, runtime, systime; 

  if (argc == 1) usage();

  InitCommand(argv, argc);
  CALLOC(MarkList, 1, char *)
  while (is_option()) {
    switch (sw = GetOption()) {
      case 'S':
        scriptflag = 1;
        break;
      case 'B':
        binaryflag = 1;
        break;
      case 'X':
        streamflag = 1;
        StreamName = GetName("Name of output stream");
        break;
      case 'w':
        winner = 1;
        break; 
      case 'b':
        nbest = 1;
        n = GetInt("'n' in n-best", 1, 10);
        break; 
      case 'c':
        cflag = 1;
        break; 
      case 'e':
        eflag = 1;
        strcpy(OutExt, "grad");
        break; 
      case 'n':
        name_freq = GetInt("Frequency for printing unit names", 0, 10000);
        nflag = 1;
        break; 
      case 'h':
        resolution = 6;
        break; 
      case 'x':
        strcpy(OutExt, GetName("Output file extension"));
        break;
      case 'd':
        strcpy(OutDir, GetName("Output path name"));
        break;  
      case 'm': 
        MarkList[NumMarked] = GetName("-m option");
        NumMarked++;
        REALLOC(MarkList, NumMarked, char *)
        break;
      case 'T':
        trace_level = GetInt("Trace level", 1, 3);
        break; 
      default: ErrorExit(SYSTEM_ERR_EXIT, "Unknown switch %c.", sw);
    } 
  } 
  NetName = GetName("Name of Net"); 
  Input = GetName("Name of Input file or (with -S option) Script file");
  EndCommand();

  ann = LoadNet(NetName);

  if (streamflag) {
    obj = GetId(ann, StreamName, STREAM);
    if (obj == -1) ErrorExit(SYSTEM_ERR_EXIT, "(-X option) The name of a stream");
    stream = (Stream *)GetPtr(ann, obj);

    if (stream->type != INTERACTIVE && stream->type != WRITE) {
      stream->type = INTERACTIVE;
    }
  }

  net = Compile(ann);

  CALLOC(Index, net->NumUnits, int)
  CALLOC(Name, net->NumUnits, char *)

  NumToShow = 0;

  UnMarkAll(ann, -1);
  if (NumMarked != 0) {
    for (i = 0; i < NumMarked; i++) {
      obj = GetId(ann, MarkList[i], -1);
      type = GetIdType(ann, obj);
      if (type != UNIT && type != GROUP && type != STREAM) {
        ErrorExit(SYSTEM_ERR_EXIT, 
          "Only groups, units and streams can be marked for output.");
      }
      RecursiveMark(ann, obj, 1);
    }
    for (i = 0; i < ann->NumUnits; i++) {
      u = ann->UTab[i];
      if (u->mark) {
        if (u->Name[0] == 0) Name[NumToShow] = "<unnamed>";
        else Name[NumToShow] = u->Name;
        Index[NumToShow++] = u->index;
      }
      else if (u->link != -1) {
        str = GetPtr(ann, u->link);
        if (str->mark) {
          if (u->Name[0] == 0) Name[NumToShow] = "<unnamed>";
          else Name[NumToShow] = u->Name;
          Index[NumToShow++] = u->index;
        }
      }
    }
  }
  else { /* No marked objects - show all output units. */
    for (i = 0; i < ann->NumUnits; i++) {
      u = ann->UTab[i];
      if (u->backtype != NONOUTPUT) {
        if (u->link == -1) continue;
        str = GetPtr(ann, u->link);
        if (u->Name[0] == 0) Name[NumToShow] = "<unnamed>";
        else Name[NumToShow] = u->Name;
        Index[NumToShow++] = u->index;
      }
    }
  }
  
  if (NumToShow == 0) ErrorExit(SYSTEM_ERR_EXIT, "No units marked for output.");
  
  if (scriptflag) {
    InList = LoadFileList(Input, "", "", &NumFiles);
  }
  else {
    NumFiles = 1;
    CALLOC(InList, 1, char *)
    InList[0] = Input;
  } 

  if (streamflag) {
    for (j = 0; j < NumFiles; j++) {
      if (trace_level > 0) printf("Data       : %s\n", InList[j]);

      LoadExtData(net, InList[j], 0, 1);
      if (trace_level > 1) printf("Num samples: %i\n", net->T);

      my_time(&zerotime, &systime);
      Forward(net);
      my_time(&runtime, &systime);
      runtime -= zerotime;

      fn = ModifyName(InList[j], OutExt, OutDir);
      FOPEN(outfp, fn, "w")

      if (binaryflag) OutputBinaryStream(net, stream, outfp); 
      else OutputStream(net, stream, outfp); 
      FCLOSE(outfp);

      if (trace_level > 1) 
        printf("Runtime    : %.2f s\n\n", (float)runtime / 1000.0);
    }
  }
  else for (j = 0; j < NumFiles; j++) {
    if (trace_level > 0) printf("Data: %s\n", InList[j]);

    LoadExtData(net, InList[j], eflag ? 1 : 0, 1);
    if (trace_level > 1) printf("Num samples: %i\n", net->T);

    my_time(&zerotime, &systime);
    Forward(net);
    my_time(&runtime, &systime);
    runtime -= zerotime;

    if (eflag) {
      Backward(net);
      FixGA(net);
    }  

    fn = ModifyName(InList[j], OutExt, OutDir);
    FOPEN(outfp, fn, "w")

    if      (binaryflag) OutputBinaryActs(net, outfp); 
    else if (winner) OutputWinner(net, outfp, cflag, nflag, name_freq); 
    else if (nbest)  OutputNBest (net, outfp, n, cflag);
    else             OutputActs  (net, outfp, cflag, nflag, name_freq);
    FCLOSE(outfp);

    if (trace_level > 1) 
      printf("Runtime    : %.2f s\n\n", (float)runtime / 1000.0);
  }

#ifdef TRAILING_MEMFREE
  FreeNetWork(net);
  FreeNet(ann);
  FREE(Name);
  FREE(Index);
  FREE(InList);
  FREE(MarkList);
#endif

  return SYSTEM_OK_EXIT;
}

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  Excite.c                                         */
/* --------------------------------------------------------------------------*/
