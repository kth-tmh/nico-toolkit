/* ------------------------------------------------------------------------- */
/*                        ParameterData Input/Output                         */
/* ------------------------------------------------------------------------- */
/*                                                                           */
/*   Copyright (C) 1994, 1995, 1996, 1997, 1998 Nikko Ström                  */
/*                                                                           */
/*   All rights reserved.                                                    */
/*                                                                           */
/*   Developed by Nikko Ström at the Dept. of Speech, Music and Hearing,     */
/*   KTH (Royal Institute of Technology)                                     */
/*                                                                           */
/*   Nikko Ström, nikko@speech.kth.se                                        */
/*                                                                           */
/*   KTH                                                                     */
/*   Institutionen för Tal, musik och hörsel                                 */
/*   S-100 44 STOCKHOLM                                                      */
/*   SWEDEN                                                                  */
/*                                                                           */
/*   http://www.speech.kth.se/                                               */
/*                                                                           */
/* ------------------------------------------------------------------------- */
/*                                                                           */
/*         This software is part of the NICO toolkit for developing          */
/*                  Recurrent Time Delay Neural Networks                     */
/*                                                                           */
/* Permission to use, copy, or modify these programs and their documentation */
/* for educational and research purposes only and without fee is hereby      */
/* granted, provided that this copyright and permission notice appear on all */
/* copies and supporting documentation.  For any other uses of this software,*/
/* in original or modified form, including but not limited to distribution   */
/* in whole or in part, specific prior permission from the copyright holder  */
/* must be obtained. The copyright holder makes no representations about the */
/* suitability of this software for any purpose. It is provided "as is"      */
/* without express or implied warranty.                                      */
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
