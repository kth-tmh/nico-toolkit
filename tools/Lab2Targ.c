/* ------------------------------------------------------------------------- */
/*               Creates output target files from label files.               */
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
#include <string.h>  
#include <math.h>  
#include <unistd.h>  
#include "LabelData.h"
#include "ParamData.h"
#include "System.h"
#include "Command.h"

static int   NumInp, NumOut;    
static int   NumFeatures;
static int   implicit = 0;
static int   impl_n; 
static char  *impl_name;
static int   delay = 0;
static int   dc_win = 0;
static float scale = 160; 
static float offset = 0; 
static float highval = 1.0;
static float lowval = 0.0;
static int   T; 
static LabelList *list;


/* Simply look-up 'lab' in 'labels' and return the index. 
   'labels' is of size n. */
static int 
get_lab_index(char **labels, int n, char *lab) {
  int i;

  for (i = 0; i < n; i++) if (strcmp(labels[i], lab) == 0) return i;
  return -1;
}


/*   Creates desired output file from a labelfile.    */
/*     mode = 2 : peak at end of label                */
/*     mode = 0 : Output = 1 throughout label         */
static float **CreateOutput(LabelList *list, int T, 
                            char **labels, int num_labs) {
  float **Output;
  int i, j, k, flag; 
  int t, t0;
  char *lab;
  int end_frame, start_frame;

  if (implicit) {
    if ((impl_n = get_lab_index(labels, num_labs, impl_name)) == -1) { 
      ErrorExit(SYSTEM_ERR_EXIT, "Implicit label, %s, is not in labellist.", impl_name);
    }  
  }

  CALLOC2D(Output, T, num_labs, float)

  for (j = 0; j < T; j++) {
    for (k = 0; k < num_labs; k++) {
      Output[j][k] = lowval;
    }
  }

  for (k = 0; k < list->num_labels; k++) {
    t0 = list->list[k]->start;
    t = list->list[k]->end;
    lab = list->list[k]->name;
    i = get_lab_index(labels, NumOut, lab);
    if (t0 < 0) ErrorExit(SYSTEM_ERR_EXIT, "Label starttime negative!");
    if (t0 > t) ErrorExit(SYSTEM_ERR_EXIT, "Label starttime greater than endtime.");
    t0 -= offset;
    t -= offset;

    start_frame = (int)rint(t0 / scale) + delay;
    end_frame = (int)rint(t / scale) + delay;

    if (end_frame >= T) {
      EmitWarning("Label endtime out of bound (%i>%i).", 
        end_frame, T); 
      end_frame = T - 1;
    }  
    if (start_frame >= T) {
      EmitWarning("Label startime out of bound. (%i>%i)", 
        start_frame, T); 
      start_frame = T - 1;
    }  
    if (end_frame < 0) {
      EmitWarning("Label endtime out of bound (%i<0).", end_frame); 
      end_frame = 0;
    }  
    if (start_frame < 0) {
      EmitWarning("Label startime out of bound. (%i<0)", 
        start_frame, T); 
      start_frame = 0;
    }  

    if (i == -1) 
      EmitWarning("Label (%s) not included in labellistfile found.",
        lab); 
    else {   
      for (j = start_frame; j < end_frame; j++) 
        Output[j][i] = highval;
    }
  }

  if (implicit) {
    for (t = 0; t < T; t++) {  
      i = 0;
      flag = 0;
      while (flag == 0 && i < num_labs) {
        if (Output[t][i] != lowval) flag = 1;
        i++;
      }
      if (!flag) Output[t][impl_n] = highval;             
    }
  }

  return Output;
}  


/*   Creates desired output file from a labelfile.    */
/*     mode = 2 : peak at end of label                */
/*     mode = 0 : Output = 1 throughout label         */
static float **CreateSegmPeak(LabelList *list, int T, int num_labs) {
  float **Output;
  int j, k; 
  int t, t0;
  int end_frame, start_frame;

  CALLOC2D(Output, T, 1, float)

  for (j = 0; j < T; j++) Output[j][0] = lowval;

  for (k = 0; k < list->num_labels; k++) {
    t0 = list->list[k]->start;
    t = list->list[k]->end;
    if (t0 < 0) ErrorExit(SYSTEM_ERR_EXIT, "Label starttime negative!");
    if (t0 > t) ErrorExit(SYSTEM_ERR_EXIT, "Label starttime greater than endtime.");
    t0 -= offset;
    t -= offset;

    start_frame = (int)rint(t0 / scale) + delay;
    end_frame = (int)rint(t / scale) + delay;

    if (end_frame >= T) {
      EmitWarning("Label endtime out of bound (%i>%i)", 
        end_frame, T); 
      continue;
    }  
    if (start_frame >= T) {
      EmitWarning("Label startime out of bound. (%i>%i)",
        start_frame, T); 
      continue;
    }  
    if (end_frame < 0) {
      EmitWarning("Label endtime out of bound (%i<0).", end_frame); 
      continue;
    }  
    if (start_frame < 0) {
      EmitWarning("Label startime out of bound. (%i<0)",
        start_frame, T); 
      continue;
    }  

    if (k == 0) { /* At first label, peak also at the start */
      j = start_frame - dc_win; if (j < 0) j = 0;
      for (; j < start_frame + dc_win; j++) {
        if (j >= T) break;
        Output[j][0] = 0.5;  
      }
      Output[start_frame][0] = highval;  
    }
    j = end_frame - dc_win; if (j < 0) j = 0;
    for (; j <= end_frame + dc_win; j++) {
      if (j >= T) break;
      Output[j][0] = 0.5;  
    }
    Output[end_frame][0] = highval;  
  }

  return Output;
}  


static float **CreateFeatures(LabelList *list, int T, char **labels, 
                              int num_labs, float **fmat, int numf) {
  float **Output;
  int i, j, k; 
  int t, t0;
  char *lab; 
  int end, start;

  CALLOC2D(Output, T, num_labs, float)

  for (k = 0; k < list->num_labels; k++) {
    t0 = list->list[k]->start;
    t = list->list[k]->end;
    lab = list->list[k]->name;
    i = get_lab_index(labels, NumOut, lab);
    if (t0 < 0) ErrorExit(SYSTEM_ERR_EXIT, "Label starttime negative!");
    if (t0 > t) ErrorExit(SYSTEM_ERR_EXIT, "Label starttime greater than endtime.");
    t0 -= offset;
    t -= offset;
    start = (int)rint(t0 * scale) + delay;
    end = (int)rint(t * scale) + delay;

    if (start > T) {
      EmitWarning("Label starttime out of bound."); 
      start = T;
    }  
    if (end > T) {
      EmitWarning("Label endtime out of bound."); 
      end = T;
    }  
    
    if (i == -1)
      EmitWarning("Label not included in labellistfile found.");
    else {    
      for (j = start; j < end; j++) {
        for (k = 0; k < NumFeatures; k++) {
          Output[j][k] = fmat[k][i];
        }
      }
    }
  }

  return Output;
}  

  
void 
usage() {
  printf("USAGE: Lab2Targ [options] LabelListFile InputSize InputFile\n");
  printf("     Options                                                        Default\n");
  printf("       -S        Treat 'InputFile' as an inputfile script           (off)\n");
  printf("     Options specifying paths and extensions of datafiles\n");
  printf("       -x ext    Extension for outputfiles                          (targ)\n");
  printf("       -d dir    Directory for outputfiles                          (.)\n");
  printf("       -q ext      Extension of labelfiles                          (lab)\n");
  printf("       -p dir      Directory holding labelfiles                     (.)\n");
  printf("       -Q ext        Extension of inputfiles                        (none)\n");
  printf("       -P dir        Directory holding inputfiles                   (.)\n");
  printf("     Options for fileformats\n");
  printf("       -F format Output format (binary or ascii, htk, etc.)         (binary)\n");
  printf("       -L format Labelformat (mix, timit, htk, etc.)                (htk)\n");
  printf("       -I format Inputformat (binary, ascii, htk, etc.)             (binary)\n");
  printf("     Options specifying target generation\n");
  printf("       -e window Peak at borders with \"don't care\"-size 'window'    (off)\n");
  printf("       -f file   Features; feature-phoneme matrix in 'file'         (off)\n");
  printf("       -i label  Implicit label. If no other target >0.5,\n");
  printf("                 'label' vill be given target 1.0                   (off)\n");
  printf("       -D num    Delay targets 'num' frames                         (off)\n");
  printf("       -1 high   Value for 'on'                                     (1.0)\n");
  printf("       -0 low    Value for 'off'                                    (0.0)\n");
  printf("       -l scale  Label time scale (samples/frame)                   (160)\n");
  printf("       -o offset Label time offset (samples)                        (0)\n");

  exit(0);
}


int
main(int argc, char *argv[]) { 
  FILE *fp;  
  char *Input, sw;
  char *LabelListName, Features[256];
  int NumFiles;
  char *format_string;
  char **OutList, **InList, **LabList;    
  char OutExt[64] = {"targ"}, OutDir[256] = {"."}; 
  char LabExt[64] = {"lab"}, LabDir[256] = {"."}; 
  char InExt[64] = {""}, InDir[256] = {"."}; 
  int k, mode = 0, scriptflag = 0, features = 0; 
  float **Fmat;
  float **output;
  LabelFormat label_format = HTK;
  ParamFileType output_format = BINARY;
  ParamFileType input_format = BINARY;
  char **labels;

  if (argc == 1) usage();

  InitCommand(argv, argc);
  while (is_option()) {
    switch (sw = GetOption()) {
      case 'f':
        strcpy(Features, GetName("Feature-phoneme matrix file"));
        features = 1;
        break;  
      case 'd':
        strcpy(OutDir, GetName("Output path name"));
        break;  
      case 'x':
        strcpy(OutExt, GetName("Output file extension"));
        break;
      case 'p':
        strcpy(LabDir, GetName("Labfile path name"));
        break;  
      case 'q':
        strcpy(LabExt, GetName("Labefile file extension"));
        break;
      case 'P':
        strcpy(InDir, GetName("Inputfile path name"));
        break;  
      case 'Q':
        strcpy(InExt, GetName("Inputfile file extension"));
        break;
      case 'e':
        dc_win = GetInt("Don't care window size", 0, 15);
        mode = 2;
        break;
       case 'D': 
        delay = GetInt("Delay option", 0, 10) / scale;
        break;
       case '1': 
        highval = GetFloat("Value for 'on'", -5.0, 5.0);
        break;
        case '0': 
        lowval = GetFloat("Value for 'off'", -5.0, 5.0);
        break;
       case 'o': 
        offset = GetInt("Label time offset", -10000, 10000);
        break;
       case 'l': 
        scale = GetFloat("Label time scale", 0.0, 10000.0);
        break;
      case 'i':
        implicit = 1;
        impl_name = GetName("Implicit label");        
        break;
      case 'L':
        format_string = GetName("Label file format");
        label_format = Str2LabelFormat(format_string);
        if (label_format == -1) 
          ErrorExit(SYSTEM_ERR_EXIT, "The file format %s is not supported.",
            format_string);
        break;
      case 'F':
        format_string = GetName("param file format");
        output_format = Str2ParamType(format_string);
        if (output_format == -1) 
          ErrorExit(SYSTEM_ERR_EXIT, "The file format %s is not supported.",
            format_string);
        break;
      case 'I':
        format_string = GetName("param file format");
        input_format = Str2ParamType(format_string);
        if (input_format == -1) 
          ErrorExit(SYSTEM_ERR_EXIT, "The file format %s is not supported.",
            format_string);
         break;
      case 'S':
        scriptflag = 1;
        break;
      default:
        printf("Error: Unknown switch %c.", sw);
        usage();
    } 
  }

  LabelListName = GetName("Name of label-list file");
  NumInp = GetInt("Number of floats/frame in input file", 0, 1000);
  Input = GetName("Name of Input File or (with -S option) Script file");   
  EndCommand(); 


  if (scriptflag) {
    InList  = LoadFileList(Input, InDir,  InExt,  &NumFiles);
    OutList = LoadFileList(Input, OutDir, OutExt, &NumFiles);
    LabList = LoadFileList(Input, LabDir, LabExt, &NumFiles);
  }
  else {
    NumFiles = 1;

    CALLOC(InList, 1, char *)
    InList[0] =  ModifyName(Input, InExt, InDir);

    CALLOC(OutList, 1, char *)
    OutList[0] = ModifyName(InList[0], OutExt, OutDir);

    CALLOC(LabList, 1, char *)
    LabList[0] = ModifyName(InList[0], LabExt, LabDir);
  }

  labels = LoadObjectList(LabelListName, &NumOut);

  if (features) {
    NumFeatures = 0;
    Fmat = LoadParameters(Features, ASCII, &NumOut, &NumFeatures);
    for (k = 0; k < NumFiles; k++) { 
      FOPEN(fp, LabList[k], "r")
      list = ReadLabels(fp, label_format);
      FCLOSE(fp);

      T = 0;
      /* Just to get T */
      output = LoadParameters(InList[k], BINARY, &NumInp, &T);
      FREE2D(output, T);

      output = CreateFeatures(list, T, labels, NumOut, Fmat, NumFeatures);
      SaveParameters(OutList[k], output_format, NumOut, T, output);
      FREE2D(output, T);
    }
  }
  else {
    for (k = 0; k < NumFiles; k++) { 
      FOPEN(fp, LabList[k], "r")
      list = ReadLabels(fp, label_format);
      FCLOSE(fp);

      T = 0;
      /* Just to get T */
      output = LoadParameters(InList[k], input_format, &NumInp, &T); 
      FREE2D(output, T);

      if (mode == 2) {
        output = CreateSegmPeak(list, T, NumOut); 
        SaveParameters(OutList[k], output_format, 1, T, output);
      }
      else {
        output = CreateOutput(list, T, labels, NumOut); 

	/* Special case for the codebook file format */
        if (output_format == CODEBOOK) { 
          PARAM_FILE *pfp;
          float *entry;
          int t;

          pfp = OpenParameterFile(OutList[k], CODEBOOK, &NumOut, "w");

          /* Initialize the codebook */
          CALLOC(entry, 2, float)
          entry[0] = lowval;
	  entry[1] = highval;
          InitParamCodeBook(pfp, 2, entry);
          FREE(entry)

	  for (t = 0; t < T; t++) WriteParameterSample(pfp, output[t]);

          CloseParameterFile(pfp);
	}
	/* all other file formats */
        else {
          SaveParameters(OutList[k], output_format, NumOut, T, output);
        }
      }

      FREE2D(output, T);
    }
  }

  return SYSTEM_OK_EXIT;
}             


/* --------------------------------------------------------------------------*/
/*           END OF FILE :  Lab2Targ.c                                       */
/* --------------------------------------------------------------------------*/
