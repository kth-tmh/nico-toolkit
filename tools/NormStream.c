/* ------------------------------------------------------------------------- */
/*    Set the values of a stream's linear transform using external data.     */
/* ------------------------------------------------------------------------- */
/*                                                                           */
/*   Copyright (C) 1995, 1996, 1997, 1998 Nikko Strom                        */
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

Net *ann;
NetWork *net;
int *mark_array;
int scriptflag=0, markflag=0, std_flag=0, reset_flag=0;


void 
usage(void) {
  printf("USAGE: NormStream [options] Net [Input]\n");
  printf("       Option                                               Default\n");
  printf("       -S             Treat 'Input' as an inputfile  script (off)\n");
  printf("       -s stream      Process the specified stream          (all streams)\n");
  printf("       -d mult        Center around mean\n");
  printf("                      mapping: [mean-s*mult, mean+s*mult] -> [-1,1]\n");
  printf("                                                            (min,max -> -1,1)\n");
  printf("       -0             Reset linear coeff's (no Input)\n");
  printf("       -c a b         Set coeff's to 'a' and 'b' (no Input)\n");

  exit(0);
}


void 
reset_lin_coeff() {
  int i, j;
  Stream *str;

  for (i = 0; i < ann->NumStreams; i++) {
    str = ann->StrTab[i];
    if (markflag && !mark_array[str->id]) continue;
    for (j = 0; j < str->size; j++) {
      str->a[j] = 0.0;
      str->b[j] = 1.0;
    }
  }
}

void 
set_coeffs(float a, float b) {
  int i, j;
  Stream *str;

  for (i = 0; i < ann->NumStreams; i++) {
    str = ann->StrTab[i];
    if (markflag && !mark_array[str->id]) continue;
    for (j = 0; j < str->size; j++) {
      str->a[j] = a;
      str->b[j] = b;
    }
  }
}


int
main(int argc, char *argv[]) {  
  Stream *str;
  char    sw;   
  char   *Net;
  char   *stream_name;
  char   *Input = NULL;    
  char  **InList;    
  int     NumFiles;
  int     i, j, k, str_id, TotT=0;
  int     const_flag = 0;
  float  *mean, *stddev, *min, *max, *sum, *sum2, var, this;
  float   scale = 1.0;
  float   const_a = 0.0, const_b = 1.0;

  if (argc == 1) usage();
  InitCommand(argv, argc);
  while (is_option()) {
    switch (sw = GetOption()) {
      case 'S':
        scriptflag = 1;
        break; 
      case 's':
        markflag = 1;
        GetName("Name of a stream"); /* Leave this for the next pass */
        break; 
      case 'c':
        const_flag = 1;
        const_a = GetFloat("Value of 'a'", -10000.0, 10000.0); 
        const_b = GetFloat("Value of 'b'", -10000.0, 10000.0); 
        break; 
      case 'd':
        std_flag = 1;
        if (reset_flag) ErrorExit(SYSTEM_ERR_EXIT, "Reset- and std-option are not compatible");
        scale = GetFloat("Multiplier for -d option", 0.0, 1000.0);
        break; 
      case '0':
        reset_flag = 1;
        if (std_flag) ErrorExit(SYSTEM_ERR_EXIT, "Reset and std-option are not compatible.");
        break; 
      default: ErrorExit(SYSTEM_ERR_EXIT, "Unknown switch %c.", sw);
    }
  } 
  Net = GetName("Name of Net");
  if (reset_flag || const_flag) {
    if (!IsEndCommand()) ErrorExit(SYSTEM_ERR_EXIT, "No input should be specified with the -c or -0 option.");
  }
  else { 
    Input = GetName("Name of Input file or (with -S option) Script file");
  }
  EndCommand();

  ann = LoadNet(Net);

  CALLOC(mark_array, ann->NumId, int)

  InitCommand(argv, argc);

  for (i = 1; i < argc - 2; i++) {
    if (is_option()) {
      if (GetOption() == 's') {
        markflag = 1;
        stream_name = GetName("Name of a stream");
        i++;
        if ((str_id = GetId(ann, stream_name, STREAM)) == -1) {
          ErrorExit(SYSTEM_ERR_EXIT, "Stream %s not found.", stream_name);
        }
        mark_array[str_id] = 1;
      }
    }
    else SkipArg();
  }

  reset_lin_coeff();
  if (const_flag) set_coeffs(const_a, const_b);

  if (reset_flag || const_flag) {
    AddLogEntry(ann->logbook, argv, argc); 
    SaveNet(Net, ann); 
    FreeNet(ann);
    return SYSTEM_OK_EXIT;
  }

  net = Compile(ann);

  if (scriptflag) {
    InList = LoadFileList(Input, "", "", &NumFiles);
  }
  else {
    NumFiles = 1;
    CALLOC(InList, 1, char *)
    STRDUP(InList[0], Input)
  }

  CALLOC(max, net->NumExt, float)
  CALLOC(min, net->NumExt, float)
  CALLOC(sum, net->NumExt, float)
  CALLOC(sum2, net->NumExt, float)
  CALLOC(mean, net->NumExt, float)
  CALLOC(stddev, net->NumExt, float)


  /* Sums are divided "on the fly" by the number of frames in the current file 
     - this is to avoid loosing accuracy when processing large databases  */
  for (k = 0; k < NumFiles; k++) {
    LoadExtData(net, InList[k], 1, 0);
    if (k == 0) { /* Initialize stat-arrays only for the first file */
      for (j = 0; j < net->NumExt; j++) {
        min[j] = max[j] = net->ExtData[0][j];
        sum[j] = sum2[j] = 0.0;
      }
    }  
    for (i = 0; i < net->T; i++) {
      for (j = 0; j < net->NumExt; j++) {
        this = net->ExtData[i][j];

        /* See comment above this paragraph */
        sum[j] += this / (float)net->T;
        sum2[j] += this * this / (float)net->T; 

        if      (this > max[j]) max[j] = this;
        else if (this < min[j]) min[j] = this;
      }
    }
    TotT += net->T;
  }

  if (std_flag) {
    for (j = 0; j < net->NumExt; j++) {
      /* Here sums are divided only by the number of files; Division by nymber 
         of frames was done in the previous loop */
      mean[j] = sum[j] / (float)NumFiles;
      var = sum2[j] / (float)NumFiles - mean[j] * mean[j];
      if (var <= 0.0) stddev[j] = 0.0; /* Shouldn't really be necessary */
      else stddev[j] = sqrt(var);
    }
    for (i = 0; i < ann->NumStreams; i++) {
      str = ann->StrTab[i];
      if (markflag && !mark_array[str->id]) continue;
      for (j = 0; j < str->size; j++) {
        str->b[j] = stddev[str->index + j] * scale;
        str->a[j] = mean[str->index + j] / str->b[j];
      }
    }
  }

  else { /* min/max */
    for (i = 0; i < ann->NumStreams; i++) {
      str = ann->StrTab[i];
      if (markflag && !mark_array[str->id]) continue;
      for (j = 0; j < str->size; j++) {
        str->b[j] = 2.0 / (max[str->index + j] - min[str->index + j]);
        str->a[j] = (max[str->index + j] + min[str->index + j]) / str->b[j];
      }
    }
  }

  AddLogEntry(ann->logbook, argv, argc); 
  SaveNet(Net, ann); 
  FreeNet(ann);
  FreeNetWork(net);
  free(mark_array);
  free(max); free(min);
  free(sum); free(sum2);
  free(mean); free(stddev);

  return SYSTEM_OK_EXIT;
}

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  NormStream.c                                     */
/* --------------------------------------------------------------------------*/
