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

#ifndef LABELS_LOADED
#define LABELS_LOADED

#define MAX_LINE_LENGTH 1024
#define MAX_LABEL_LENGTH 128



/* Label file formats */
typedef enum {
  UNDEF_LABFORMAT = -1,
  MIX    = 1,
  MIX_W  = 2,
  HTK    = 3,
  TIMIT  = 4,
  ATR1   = 5,
  ATR2   = 6
} LabelFormat;


typedef struct Label {
  char *name;
  int   start;          /* start-time in frames */
  int   end;            /* end-time in frames */
} Label;


typedef struct LabelList {
  int samp_freq;        /* Sample frequenzy in Hz */
  int num_labels;       /* size of the 'list' */
  int num_allocated;

  Label **list;
} LabelList;


/* Conversion between the string and the code description of audio formats */
LabelFormat Str2LabelFormat(char *);
char *LabelFormat2Str(LabelFormat);


/* ----------   Main Creation/Destruction and I/O functions    ----------- */

/*                 Create a new empty label-list                           */
LabelList *CreateLabels(int samp_freq);

/* Add a new label at the end of the list */
Label *AddLabel(LabelList *);

/*    Insert a new label before label number n (labels indexed from zero)  */
Label *InsertLabel(LabelList *, int n);

/*       Remove (and free) label number n  (labels indexed from zero)      */
void RemoveLabel(LabelList *, int n);

/*              Free all memory allocated by a label-list                  */
void FreeLabels(LabelList *);


/*                 Read a label-list from a label-file                     */
LabelList *ReadLabels(FILE *, LabelFormat);

/*                 Write a label-file from a label-list                    */
void WriteLabels(FILE *, LabelList *, LabelFormat);



/* ----------     Globally operating editing functions        ------------ */

/*  Remove all labels with zero-length. The number of removals is returned */
int RemoveNullLabels(LabelList *);


/*  Remove all instances of all labels with the (n) names in 'unwanted'.   */
/*  The number of removals is returned                                     */
int RemoveLabels(LabelList *, char **unwanted, int n);


/*  Change all instances of 'original[i]' to 'changed[i]' for all (n) i.   */
/*  The number of changes is returned                                      */
int ChangeLabels(LabelList *, char **original, char **changed, int n);


/*   Merge all instances of 'merge1[i]' followed by 'merge2[i]' into       */
/*   'merged[i]' for all (n) i.                                            */
/*   The number of merges is returned                                      */
int MergeLabels(LabelList *, char **merge1, char **merge2, 
                char **merged, int n);


/*  Remove all labels that are not in the 'allowed' of size n.             */
/*  If 'warnings' is set, a warning will be printed for each removal.      */
/*  The number of removals is returned                                     */
int RestrictLabels(LabelList *, char **allowed, int n, int warnings);


#endif

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  LabelData.h                                      */
/* --------------------------------------------------------------------------*/
