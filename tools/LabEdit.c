/* ------------------------------------------------------------------------- */
/*                             Edit label files.                             */
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>  
#include <unistd.h>  
#include "ParamData.h"
#include "LabelData.h"
#include "Command.h"
#include "System.h"

#define DEFAULT_SAMP_FREQ 16000.0
#define MAX_NUM_DIFFERENT_LABELS 1000

void 
usage(void) {
  printf("USAGE: LabEdit [options] InputFile\n");
  printf("       Options                                                         Default\n");
  printf("         -S              Treat 'InputFile' as an inputfile script      (off)\n");
  printf("         -T level        Level of trace output printed to stdout (1-3) (0)\n");
  printf("         -A script       Read options from 'script'\n");
  printf("         -x ext          Extension for output files                    (lab)\n");
  printf("         -d ext          Directory for output files                    (current)\n");
  printf("         -q ext          Extension of input files\n");
  printf("         -p dir          Directory holding input label files\n");
  printf("       Options defining the labelfile format\n");
  printf("         -F format       Input (mix, mix_w, timit, htk, atr1 or atr2)  (htk)\n");
  printf("         -O format       Output (mix, mix_w, timit, htk, atr1 or atr2)  (htk)\n");
  printf("         -t scale        Label time scaling, multiply times by 'scale' (1.0)\n");
  printf("         -a lag          Label time lag, add 'add' to times            (0.0)\n");
  printf("         -f samp_freq    Sample frequency [Hz]                         (16000)\n");
  printf("       Options for modifying the labels\n");
  printf("         -r name new     Rename all labels 'name' to 'new'\n");
  printf("         -m l1 l2 name   Merge label 'l1' followed by 'l2' to 'name'\n");
  printf("         -z              Remove all zero-length labels\n");
  printf("       Options for printing statistics\n");
  printf("         -l filename     List all outputted label names in 'filename'  (off)\n");
  printf("         -s filename     Stats about all outputted labels in 'filename'(off)\n");
  printf("         -0              Don't output label files\n");
  exit(SYSTEM_ERR_EXIT);
}


int 
insert_seen_label(char** list, int *freq, float *dur, float *dur2, 
                      int n, Label *new) {
  int i = n - 1;
  float d;

  if (n == MAX_NUM_DIFFERENT_LABELS) { 
    ErrorExit(SYSTEM_ERR_EXIT,
	      "Too many different labels (>%i)", MAX_NUM_DIFFERENT_LABELS);
  }

  for (i = 0; i < n; i++) if (strcmp(list[i], new->name) == 0) {
    freq[i]++;
    dur[i] += d = ((float)(new->end - new->start) / 
                  DEFAULT_SAMP_FREQ);
    dur2[i] += d * d;
    return n;
  }

  STRDUP(list[n], new->name)

  freq[n] = 1;
  dur[n] = d = ((float)(new->end - new->start)) / 
                  DEFAULT_SAMP_FREQ;
  dur2[n] = d * d;
  return n + 1;
}


int 
main(int argc, char *argv[]) {   
  LabelList *list;
  FILE *fp;        
  char *Input, sw;
  int  NumFiles;
  char **OutList, **InList;    
  char OutExt[64]={"lab"}, OutDir[256]={"."}; 
  char InExt[64]={""}, InDir[256]={""}; 
  char *label_list_name = NULL, *label_stats_name = NULL;
  char *seen_label_list[MAX_NUM_DIFFERENT_LABELS];
  int freq[MAX_NUM_DIFFERENT_LABELS];
  float dur[MAX_NUM_DIFFERENT_LABELS];
  float dur2[MAX_NUM_DIFFERENT_LABELS];
  int num_seen_labels = 0;
  int  i, j;
  int remove_zero_labels = 0;
  int scriptflag = 0, label_list_flag = 0, label_stats_flag = 0;
  int no_output = 0, format = HTK, out_format = HTK; 
  int num_merge = 0, num_replace = 0;
  char *merge_into[500];
  char *merge_1st[500];
  char *merge_2nd[500];
  char *replace_from[500];
  char *replace_to[500];
  char *format_string;
  float time_scale = 1.0;
  float time_lag = 0.0;
  float samp_freq = DEFAULT_SAMP_FREQ;
  int trace_level = 0;

  if (argc == 1) usage();

  InitCommand(argv, argc);
  while (is_option()) {
    switch (sw = GetOption()) {
      case 'd':
        strcpy(OutDir, GetName("Output path name"));
        break;  
      case 'x':
        strcpy(OutExt, GetName("Output file extension"));
        break;
      case 'p':
        strcpy(InDir, GetName("Labfile path name"));
        break;  
      case 'q':
        strcpy(InExt, GetName("Labfile file extension"));
        break;
      case 'l':
        label_list_name = GetName("Name of label list");
        label_list_flag = 1;
        break;
      case 's':
        label_stats_name = GetName("Name of label stats file");
        label_stats_flag = 1;
        break;
      case 'm':
        merge_1st[num_merge] = GetName("First merge label");
        merge_2nd[num_merge] = GetName("Second merge label");
        merge_into[num_merge] = GetName("Merge into label");
        num_merge++;
        break;
      case 'r':
        replace_from[num_replace] = GetName("Replace from label");
        replace_to[num_replace] = GetName("Replace to label");
        num_replace++;
        break;
      case 'z':
        remove_zero_labels = 1;
        break;
      case 'F':
        format_string = GetName("Label file format");
        format = Str2LabelFormat(format_string);
        if (format == -1) 
          ErrorExit(SYSTEM_ERR_EXIT, "The file format %s is not supported.\n",
            format_string);
        break;
      case 'O':
        format_string = GetName("Label file format");
        out_format = Str2LabelFormat(format_string);
        if (format == -1) 
          ErrorExit(SYSTEM_ERR_EXIT, "The file format %s is not supported.\n",
            format_string);
        break;
      case 't': 
        time_scale = GetFloat("Time scale", 0.0, 1.0E10);
        break;
      case 'a': 
        time_lag = GetFloat("Time lag", -1E10, 1.0E10);
        break;
      case 'f': 
        samp_freq = GetFloat("Sample frequency", 100.0, 128000.0);
        break;
      case 'S':
        scriptflag = 1;
        break;
      case 'T':
        trace_level = GetInt("Trace level", 1, 3);
        break;
       case 'A':
         LoadArgFile(GetName("Script file for -A option"), "rmtSOFAlqpxd");
        break;
      case '0':
        no_output = 1;
        break;
      default:
        ErrorExit(SYSTEM_ERR_EXIT, "Unknown switch %s.\n", sw);
    } 
  }

  /* Modify the time scale if sample frequency is not DEFAULT_SAMP_FREQ Hz */
  time_scale = time_scale * samp_freq / DEFAULT_SAMP_FREQ;

  Input = GetName("Name of Input File or (with -S option) Script file");   
  EndCommand(); 

  if (scriptflag) {
    InList = LoadFileList(Input, InDir, InExt, &NumFiles);
    OutList = LoadFileList(Input, OutDir, OutExt, &NumFiles);
  }
  else {
    NumFiles = 1;
    CALLOC(InList, 1, char *)
    InList[0] = ModifyName(Input, InExt, InDir);

    CALLOC(OutList, 1, char *)
    OutList[0] = ModifyName(InList[0], OutExt, OutDir);
  }

  if (trace_level > 0 && scriptflag) {
    printf("found %d files in script\n", NumFiles);
  }

  for (i = 0; i < NumFiles; i++) {
    if (trace_level > 2) {
      printf("processing: %s\n", InList[i]);
    }

    FOPEN(fp, InList[i], "r");
    list = ReadLabels(fp, format);
    FCLOSE(fp);

    ChangeLabels(list, replace_from, replace_to, num_replace);

    MergeLabels(list, merge_1st, merge_2nd, merge_into, num_merge);

    if (label_list_flag || label_stats_flag) {
      for (j = 0; j < list->num_labels; j++) {
        num_seen_labels = insert_seen_label(seen_label_list, freq, dur, dur2, 
                                            num_seen_labels, list->list[j]);
      }
    }

    if (remove_zero_labels) RemoveNullLabels(list);

    if (time_scale != 1.0) for (j = 0; j < list->num_labels; j++) {
      list->list[j]->start *= time_scale;
      list->list[j]->end *= time_scale;
    }
    if (time_lag != 0.0) for (j = 0; j < list->num_labels; j++) {
      list->list[j]->start += time_lag;
      list->list[j]->end += time_lag;
    }

    if (!no_output) {
      FOPEN(fp, OutList[i], "w");
      WriteLabels(fp, list, out_format);
      FCLOSE(fp);
    }

    FreeLabels(list);
  }

  if (label_list_flag) {
    FOPEN(fp, label_list_name, "w");

    for (i = 0; i < num_seen_labels; i++) {
      fprintf(fp, "%s\n", seen_label_list[i]);
    }
    FCLOSE(fp);
  }

  if (label_stats_flag) {
    FOPEN(fp, label_stats_name, "w");

    fprintf(fp, "%8s %8s %8s %8s\n", "Label", "Freq.", "Mean dur", "Std dev"); 
    for (i = 0; i < num_seen_labels; i++) { 
      float m, v;

      m = dur[i] / (float)freq[i];
      v = dur2[i] / (float)freq[i] - m*m;
      fprintf(fp, "%8s %8i %8.3f %8.3f\n", 
        seen_label_list[i], freq[i], m, sqrt(v));
    }
    FCLOSE(fp);
  }

  return SYSTEM_OK_EXIT;
}             

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  LabEdit.c                                        */
/* --------------------------------------------------------------------------*/
