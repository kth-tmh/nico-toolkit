/* ------------------------------------------------------------------------- */
/*                        ParameterData Input/Output                         */
/* ------------------------------------------------------------------------- */
/*                                                                           */
/*   Copyright (C) 1994, 1995, 1996, 1997, 1998 Nikko Strom                  */
/*                                                                           */
/*   All rights reserved.                                                    */
/*                                                                           */
/*   Developed by Nikko Strom at the Dept. of Speech, Music and Hearing,     */
/*   KTH (Royal Institute of Technology)                                     */
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

#ifndef PARAMDEFS_LOADED
#define PARAMDEFS_LOADED

#include <stdio.h>

#define PARAMCODEBOOKMAGIC 1753589277

/* Theese are the supported data formats for parameter files */
typedef enum {
  UNSPECPARAM=-1,
  BINARY=0,     /*  4 byte samples, binary floats                  */
  ASCII=1,      /*  floats, text format (I/O by scanf and printf)  */
  KTHPARAM=2,   /*  the KTH, StdHead-format for paramfiles         */
  NISTPARAM=3,  /*  the NIST-format for paramfiles                 */
  HTKPARAM=4,   /*  the HTK-format for paramfiles                  */
  CODEBOOK=99,   /*  codebook format (non-standard)                 */
  PARAMCLASSES=98    /*  1-of-N classes format (non-standard)           */
} ParamFileType;


typedef struct {
  FILE         *fp;
  char         *fn;
  ParamFileType format;
  void         *subformat;   /* Used to specify format-specific info */
  int           SampleSize;  /* Floats/Sample */
  int           NumSamples;  /* -1 indicates "unknown" */
  int           NumProcessed;/* Number of read or written samples */
  char         *mode;        /* "r" or "w" are allowed */
} PARAM_FILE;



typedef struct ParamCodeBook {
  int size;
  int bit_size;
  int num_read_bits;
  unsigned long last_word;
  float *entry;
} ParamCodeBook;


/* We need this for the dela, delta-delta calculations of the HTK format */
typedef struct HTKmemory {
  int ParamKind; /* HTK param kind */
} HTKmemory;



/* Conversion between the code and the string description of param formats */
ParamFileType Str2ParamType(char *);
char *ParamType2Str(ParamFileType);


/* Use theese to load or save a whole parameterfile */
float **LoadParameters(char *fn, ParamFileType, int *SampleSize, 
                       int *NumSamples);

float **FilterLoadParams(char *command, ParamFileType, int *SampleSize, 
                       int *NumSamples);

void SaveParameters(char *fn, ParamFileType, int SampleSize, int NumSamples, 
                    float **Data);

void FilterSaveParams(char *command, ParamFileType, int SampleSize, 
                      int NumSamples, float **Data);

void FreeParameters(float **Data, int NumSamples);


/* These are the general I/O functions */
PARAM_FILE *OpenParameterFile(char *fn, ParamFileType, int *SampleSize, 
                              char *mode);

PARAM_FILE *PopenParameterFile(char *command, ParamFileType, int *SampleSize, 
                              char *mode);

float *ReadParameterSample(PARAM_FILE *);
void   WriteParameterSample(PARAM_FILE *, float *);

int ParameterFileEOF(PARAM_FILE *);

void CloseParameterFile(PARAM_FILE *);
void PcloseParameterFile(PARAM_FILE *);


/* Initializes the subformat info for the CODEBOOK-format */
/* Call this function after opening a codebook-file for writing */
void InitParamCodeBook(PARAM_FILE *, int size, float *entry);


/* Info about an open PARAM_FILE is returned from these functions */
ParamFileType GetParamFileType(PARAM_FILE *);
char *GetParamFileMode(PARAM_FILE *);
int GetParamFileNumSamples(PARAM_FILE *);
int GetParamFileSampleSize(PARAM_FILE *);


#endif

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  ParamData.h                                      */
/* --------------------------------------------------------------------------*/
