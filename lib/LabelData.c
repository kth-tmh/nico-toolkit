/* ------------------------------------------------------------------------- */
/*                        Tools to handle label files                        */
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
#include "System.h" 
#include "LabelData.h" 
 

LabelFormat Str2LabelFormat(char *s) {
  if      (strcmp(s, "mix") == 0)    return MIX;
  else if (strcmp(s, "mix_w") == 0)  return MIX_W;
  else if (strcmp(s, "htk") == 0)    return HTK;
  else if (strcmp(s, "timit") == 0)  return TIMIT;
  else if (strcmp(s, "atr1") == 0)   return ATR1;
  else if (strcmp(s, "atr2") == 0)   return ATR2;
  else return UNDEF_LABFORMAT;
}


char *
LabelFormat2Str(LabelFormat t) {
  char *s;
  if      (t == MIX)   STRDUP(s, "mix")
  else if (t == MIX_W) STRDUP(s, "mix_w")
  else if (t == HTK)   STRDUP(s, "htk")
  else if (t == TIMIT) STRDUP(s, "timit")
  else if (t == ATR1)  STRDUP(s, "atr1")
  else if (t == ATR2)  STRDUP(s, "atr2")
  else return NULL;
  return s;
}


/*                   Create a new empty label-list                         */
LabelList *CreateLabels(int samp_freq) {
  LabelList *list;

  CALLOC(list, 1, LabelList)

  list->samp_freq = samp_freq;
  list->num_labels = 0; 
  list->num_allocated = 128;

  CALLOC(list->list, list->num_allocated, Label *)

  return list;
}


/*                Add a new label at the end of the list                  */
Label *AddLabel(LabelList *list) {
  Label *label;
  
  CALLOC(label, 1, Label)

  if (list->num_labels == list->num_allocated) {
    list->num_allocated *= 2;
    REALLOC(list->list, list->num_allocated, Label *)
  }

  list->list[list->num_labels] = label;
  list->num_labels++;

  return label;
}


/*    Insert a new label before label number n (labels indexed from zero)  */
Label *InsertLabel(LabelList *list, int n) {
  Label *label;
  int i;

  if (n > list->num_labels - 1) ErrorExit(SYSTEM_ERR_EXIT, 
    "Attempt to insert a label outside the list");

  CALLOC(label, 1, Label)

  if (list->num_labels == list->num_allocated) {
    list->num_allocated *= 2;
    REALLOC(list->list, list->num_allocated, Label *)
  }

  for (i = list->num_labels; i > n; i--) list->list[i] = list->list[i - 1];
  list->num_labels++;

  list->list[n] = label;

  return label;
}


static void 
free_label(Label *lab) {
  FREE(lab->name);
  FREE(lab);
}


/*       Remove (and free) label number n  (labels indexed from zero)       */
void 
RemoveLabel(LabelList *list, int n) {
  int i;

  free_label(list->list[n]);

  list->num_labels--;

  for (i = n; i < list->num_labels; i++) list->list[i] = list->list[i + 1];
}


/*             Free all memory allocated by a label-list                  */
void 
FreeLabels(LabelList *list) {
  int i;

  for (i = 0; i < list->num_labels; i++) free_label(list->list[i]);
  FREE(list->list);
  FREE(list);
}


/* Remove all labels with zero-length */
/* The number of removals is returned */
int 
RemoveNullLabels(LabelList *list) {
  int i = 0, num = 0;

  while (i < list->num_labels) {
    while (i < list->num_labels &&
           list->list[i]->start == list->list[i]->end) {
      RemoveLabel(list, i);
      num++;
    }
    i++;
  }
  return num;
}


/*         Remove all labels in the unwanted-list                        */
/*         The number of removals is returned                            */
int 
RemoveLabels(LabelList *list, char **unwanted, int n) {
  int i, j, deleted, num = 0;

  i = 0;
  while (i < list->num_labels) {
    deleted = 0;
    for (j = 0; j < n; j++) {
      if (strcmp(unwanted[j], list->list[i]->name) == 0) {
        RemoveLabel(list, i);
        deleted = 0;
        num++;
        break;
      }
    }
    if (!deleted) i++;
  }
  return num;
}


/*  Change all instances of 'original[i]' to 'changed[i]' for all (n) i.   */
/*  The number of changes is returned                                      */
int 
ChangeLabels(LabelList *list, char **original, char **changed, int n) {
  int i, j, num = 0;

  for (i = 0; i < list->num_labels; i++) {
    for (j = 0; j < n; j++) {
      if (strcmp(original[j], list->list[i]->name) == 0) {
        FREE(list->list[i]->name);
        STRDUP(list->list[i]->name, changed[j]);
        num++;
        break;
      }
    }
  }
  return num;
}


/*   Merge all instances of 'merge1[i]' followed by 'merge2[i]' into       */
/*   'merged[i]' for all (n) i.                                            */
/*   The number of merges is returned                                      */
int 
MergeLabels(LabelList *list, char **merge1, char **merge2, 
                char **merged, int n) {

  int i, j, num = 0;

  i = 0;
  while (i < list->num_labels - 1) {
    for (j = 0; j < n; j++) {
      if (strcmp(merge1[j], list->list[i]->name) == 0 &&
          strcmp(merge2[j], list->list[i + 1]->name) == 0) {
        RemoveLabel(list, i);
        FREE(list->list[i]->name);
        STRDUP(list->list[i]->name, merged[j]);
        num++;
        break;
      }
    }
    i++;
  }

  return num;
}


/*  Remove all labels that are not in the 'AllowedList' of size n.         */
/*  If 'warnings' is set, a warning will be printed for each removal.      */
/*  The number of removals is returned                                     */
int 
RestrictLabels(LabelList *list, char **allowed, int n, int warnings) {
  int i, j, found, num = 0;

  i = 0;
  while (i < list->num_labels) {
    found = 0;
    for (j = 0; j < n; j++) {
      if (strcmp(allowed[j], list->list[i]->name) == 0) {
        found = 1;
        num++;
        break;
      }
    }

    if (!found) {
      if (warnings) fprintf(stderr, "WARNING: Removing unknown label: %s",
                            list->list[i]->name);
      RemoveLabel(list, i);
    }
    else i++;
  }

  return num;
}


/* ---------------     Reading & writing of labelfiles   ------------------ */

static LabelList *read_label_list_file(FILE *fp, float scale) {
  LabelList *list;
  Label *label = NULL;
  char line[MAX_LINE_LENGTH];
  char label_name[MAX_LABEL_LENGTH];
  int from_time, to_time;
  list = CreateLabels(16000);

  while (fgets(line, MAX_LINE_LENGTH, fp) != NULL) {
    sscanf(line, "%i %i %s", &from_time, &to_time, label_name);

    label = AddLabel(list);
    STRDUP(label->name, label_name);
    label->start = (int)ceil(from_time / scale);
    label->end = (int)ceil(to_time / scale);
  }

  return list;
}


static LabelList *mixfile_to_wordlabels(FILE *fp) {
  LabelList *list;
  Label *label = NULL;
  char word[MAX_LABEL_LENGTH], *w;
  char line[MAX_LINE_LENGTH];
  int i, t, time, ct1_found = 0;

  list = CreateLabels(16000);

  time = 0;
  while (fgets(line, MAX_LINE_LENGTH, fp) != NULL) {
    if (strncmp(line, "CT 1", 4) == 0) 
      ct1_found = 1; /* No labels before 'CT 1' */

    else if (!ct1_found) continue;

    sscanf(line + 3, "%i", &t);
    if (t > time) time = t;

    if (!(w = strstr(line, ">w"))) continue; /* look for lines with a word */
    
    w += 2;
    sscanf(w, "%s", word);

    label = AddLabel(list);

    STRDUP(label->name, word);
    label->start = time;
    label->end = -1; /* Undefined for now */
  }

  if (label != NULL) {
    label->end = time;

    for (i = 0; i < list->num_labels - 1; i++) 
      list->list[i]->end = list->list[i + 1]->start;
    
    list->list[list->num_labels - 1]->end = 
      list->list[list->num_labels - 1]->start;
  }

  return list;
}


static LabelList *mixfile_to_labels(FILE *fp) {
  LabelList *list;
  Label *label;
  char label_name[MAX_LABEL_LENGTH], mix_label[16];
  char line[MAX_LINE_LENGTH];
  int pos, i, time, ct1_found = 0;

  list = CreateLabels(16000);

  while (fgets(line, MAX_LINE_LENGTH, fp) != NULL) {
    if (strncmp(line, "CT 1", 4) == 0) 
      ct1_found = 1; /* No labels before 'CT 1' */

    else if (!ct1_found) continue;

    if (strncmp(line, "FR ", 3) == 0) {
      char c;

      sscanf(line + 3, "%i %s", &time, mix_label);

      /* Use labels preceeded by $ or # only. */
      if (mix_label[0] != '#' && mix_label[0] != '$') continue; 
 
      pos = i = 0;
      c = mix_label[i];
      while (c == '#' || c == '$' || c == '"' || c == '\'' || c == '`' || 
             c == '+' || c == '?') {
        i++;
        c = mix_label[i];
      }

      for (; i <= strlen(mix_label); i++) 
        label_name[pos++] = mix_label[i];

      label = AddLabel(list);
      STRDUP(label->name, label_name);
      label->start = time;
      label->end = -1; 
    }
  }

  for (i = 0; i < list->num_labels - 1; i++) 
    list->list[i]->end = list->list[i + 1]->start;

  list->list[list->num_labels - 1]->end = 
    list->list[list->num_labels - 1]->start;

  return list;
}


static LabelList *ATR1file_to_labels(FILE *fp) {
  LabelList *list;
  Label *label;
  char line[MAX_LINE_LENGTH];
  float start_time, end_time;
  char label_name[MAX_LABEL_LENGTH];

  list = CreateLabels(16000);

  while (fgets(line, MAX_LINE_LENGTH, fp) != NULL && line[0] != '#') {
    sscanf(line, "%f %s %f", &start_time, label_name, &end_time);

    label = AddLabel(list);
    STRDUP(label->name, label_name);
    label->start = (int)(start_time * 16);
    label->end = (int)(end_time * 16);
  }
  
  fprintf(stderr, "Warning: No \"#\" found in ATR ITL Labelfile.\n"); 

  return list;
}


static LabelList *ATR2file_to_labels(FILE *fp) {
  LabelList *list;
  Label *label;
  char line[MAX_LINE_LENGTH];
  char label_name[MAX_LABEL_LENGTH];
  float start_time, end_time;

  /* Skip the first level */
  while (fgets(line, MAX_LINE_LENGTH, fp) != NULL && line[0] != '#');
  if (line[0] != '#') {
    ErrorExit(SYSTEM_ERR_EXIT, 
       "Error: No \"#\" found in ATR ITL Labelfile - No second level.\n");
  }
  
  list = CreateLabels(16000);

  while (fgets(line, MAX_LINE_LENGTH, fp) != NULL && line[0] != '#') {
    sscanf(line, "%f %s %f", &start_time, label_name, &end_time);

    label = AddLabel(list);
    STRDUP(label->name, label_name);
    label->start = (int)(start_time * 16);
    label->end = (int)(end_time * 16);
  }
  
  fprintf(stderr, "Warning: No \"#\" found in ATR ITL Labelfile.\n"); 

  return list;
}


static void 
write_labelfile(FILE *fp, LabelList *list, float scale) {
  Label *label;
  int i;

  for (i = 0; i < list->num_labels; i++) {
    label = list->list[i];

    if (label->end != -1) 
      fprintf(fp, "%10i %10i %s\n", 
	      (int)ceil(label->start * scale), 
	      (int)ceil(label->end * scale),
	      label->name);
    else fprintf(fp, "%10i            %s\n", 
		 (int)ceil(label->start * scale),
		 label->name);
  }
}


/*  Read a label-list from a label-file  */
LabelList *ReadLabels(FILE *fp, LabelFormat format) {
  LabelList *list = NULL;

  switch (format) {
    case MIX:
      list = mixfile_to_labels(fp);
      break;
    case MIX_W:
      list = mixfile_to_wordlabels(fp);
      break;
    case HTK:
      list = read_label_list_file(fp, 625);
      break;
    case TIMIT:
      list = read_label_list_file(fp, 1);
      break;
    case ATR1:
      list = ATR1file_to_labels(fp);
      break;
    case ATR2:
      list = ATR2file_to_labels(fp);
      break;

    default: ErrorExit(SYSTEM_ERR_EXIT, "Unknown label format in ReadLabels");
  }

  return list;
}


/* Write a label-file from a label-list */
void 
WriteLabels(FILE *fp, LabelList *list, LabelFormat format) {
  switch (format) {
    case MIX:
      ErrorExit(SYSTEM_ERR_EXIT, "Writing mix-format is not yet supported");
    break;
    case MIX_W:
      ErrorExit(SYSTEM_ERR_EXIT, "Writing mix-format is not yet supported");
    break;
    case HTK:
      write_labelfile(fp, list, 625);
      break;
    case TIMIT:
      write_labelfile(fp, list, 1);
      break;
    case ATR1:
      ErrorExit(SYSTEM_ERR_EXIT, "Writing ATR-format is not yet supported");
      break;
    case ATR2:
      ErrorExit(SYSTEM_ERR_EXIT, "Writing ATR-format is not yet supported");
      break;

    default: ErrorExit(SYSTEM_ERR_EXIT, "Unknown label format in ReadLabels");
  }
}


/* --------------------------------------------------------------------------*/
/*           END OF FILE :  LabelData.c                                      */
/* --------------------------------------------------------------------------*/
