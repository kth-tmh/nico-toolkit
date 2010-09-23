/* ------------------------------------------------------------------------- */
/*                        Parameter Data Input/Output                        */
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>
#include "System.h"
#include "ParamData.h"
#include "nMath.h"

/* Conversion from the string to the code description of param formats */
ParamFileType Str2ParamType(char *s) {
  if      (strcmp(s, "binary") == 0)   return BINARY;
  else if (strcmp(s, "ascii") == 0)    return ASCII;
  else if (strcmp(s, "kth") == 0)      return KTHPARAM;
  else if (strcmp(s, "nist") == 0)     return NISTPARAM;
  else if (strcmp(s, "htk") == 0)      return HTKPARAM;
  else if (strcmp(s, "codebook") == 0) return CODEBOOK;
  else if (strcmp(s, "classes") == 0)  return PARAMCLASSES;
  else return UNSPECPARAM;
}


/* Conversion from the code to the string description of param formats */
char *
ParamType2Str(ParamFileType t) {
  char *s;

  if      (t == BINARY)       STRDUP(s, "binary")
  else if (t == ASCII)        STRDUP(s, "ascii")
  else if (t == KTHPARAM)     STRDUP(s, "kth")
  else if (t == NISTPARAM)    STRDUP(s, "nist")
  else if (t == HTKPARAM)     STRDUP(s, "htk")
  else if (t == CODEBOOK)     STRDUP(s, "codebook")
  else if (t == PARAMCLASSES) STRDUP(s, "classes")
  else return NULL;

  return s;
}


PARAM_FILE *
OpenParameterFile(char *fn, ParamFileType format, 
		  int *SampleSize, char *mode) {
  ParamCodeBook *codebook;
  HTKmemory *memory;
  unsigned long magic;
  FILE *fp = NULL;
  PARAM_FILE *pfp;

  if (strcmp(mode, "r") == 0) {
    if (strcmp(fn, "stdin") == 0) fp = stdin;
    else {
      if (format == ASCII) {
	nFOPEN(fp, fn, "r");
      }
      else {
	nFOPEN(fp, fn, "rb");
      }
    }
  }
  else if (strcmp(mode, "w") == 0) {
    if (strcmp(fn, "stdout") == 0) fp = stdout;
    else  {
      if (format == ASCII) {
	nFOPEN(fp, fn, "w");
      }
      else {
	nFOPEN(fp, fn, "wb");
      }
    }
  }
  else ErrorExit(SYSTEM_ERR_EXIT, 
    "Only \"r\" and \"w\" are allowed modes for OpenParameterFile.");

  CALLOC(pfp, 1, PARAM_FILE)

  pfp->fp           = fp;
  pfp->NumSamples   = -1; 
  pfp->NumProcessed = 0; 
  pfp->format       = format;
  STRDUP(pfp->mode, mode);
  STRDUP(pfp->fn, fn);
  pfp->SampleSize   = *SampleSize;

  switch (pfp->format) {
  case BINARY:
    break;

  case ASCII:
    break;

  case HTKPARAM:
    if (strcmp(mode, "r") == 0) {
      long l;
      short s;

      CALLOC(pfp->subformat, 1, HTKmemory)
      memory = (HTKmemory *)pfp->subformat;

      /* Read the 12 byte HTK header */
      l = ReadLong(fp);       /* Number of samples */
      pfp->NumSamples = (int)l;

      l = ReadLong(fp);       /* Sample period (ns) */

      s = ReadShort(fp);      /* Num. bytes / sample */

      pfp->SampleSize = s / sizeof(float);

      s = ReadShort(fp);    /* Samplekind */
      if (s & 02000) ErrorExit(SYSTEM_ERR_EXIT, "HTK compressing is not implemented");
      memory->ParamKind = s;

      if (*SampleSize && pfp->SampleSize != *SampleSize) 
        ErrorExit(SYSTEM_ERR_EXIT, "Sample size %i expected. Got %i.",
          *SampleSize, pfp->SampleSize);
      *SampleSize = pfp->SampleSize;
    }
    else { /* mode == "w" */
      long l;
      short s;

      CALLOC(pfp->subformat, 1, HTKmemory);
      memory = (HTKmemory *)pfp->subformat;

      /* Write the 12 byte HTK header */
      WriteLong(pfp->NumSamples, fp); /* Number of samples */

      l = 100000; /* Sample period (ns) unknown here, but guessing 100000 ns */
      WriteLong(l, fp);

      s = pfp->SampleSize * sizeof(float);
      WriteShort(s, fp); /* Num. bytes / sample */

      pfp->SampleSize = s / sizeof(float);

      s = memory->ParamKind = 7; /* Samplekind unknown here, guessing MFCC */
      WriteShort(s, fp); 
    }
    break;

  case CODEBOOK:
    CALLOC(pfp->subformat, 1, ParamCodeBook)
    codebook = (ParamCodeBook *)pfp->subformat;

    if (strcmp(mode, "r") == 0) {
      magic = ReadLong(fp);
      if (magic != PARAMCODEBOOKMAGIC) ErrorExit(SYSTEM_ERR_EXIT,
	"Error reading magic number of codebook parameter file");

      pfp->SampleSize = ReadLong(fp);
      pfp->NumSamples = ReadLong(fp);
      codebook->size = ReadLong(fp);
      codebook->bit_size = ReadLong(fp);
      CALLOC(codebook->entry, codebook->size, float)
      ReadFloats(codebook->entry, codebook->size, fp);

      codebook->num_read_bits = 32; /* Force reading a new word */
    }
    else { /* mode == "w" */
      magic = PARAMCODEBOOKMAGIC;
      WriteLong(magic, fp);

      WriteLong(pfp->SampleSize, fp);
      WriteLong(pfp->NumSamples, fp); 

      codebook->num_read_bits = 0;
      codebook->last_word = 0;

      /* undefined here: */
      codebook->size = codebook->bit_size = -1;
    }
    break;

  case PARAMCLASSES:
    break;

    default: ErrorExit(SYSTEM_ERR_EXIT, "Illegal format in OpenParameterFile.");
  }

  return pfp;
}


void 
CloseParameterFile(PARAM_FILE *pfp) {
  ParamCodeBook *codebook;
  HTKmemory *memory;
  unsigned long magic = PARAMCODEBOOKMAGIC;
  long l;
  short s;

  switch (pfp->format) {

  case HTKPARAM:
    memory = (HTKmemory *)pfp->subformat;

    if (strcmp(pfp->mode, "w") == 0) {
      /* Check if it's possible to rewind */
      if (fseek(pfp->fp, 0L, SEEK_SET) == 0) {
        /* Fill in number of samples */
        WriteLong(pfp->NumProcessed, pfp->fp); 

        l = 100000; /* Sample period (ns) unknown here, guessing */
        WriteLong(l, pfp->fp);

        s = pfp->SampleSize * sizeof(float);
        WriteShort(s, pfp->fp); /* Num. bytes / sample */

        s = memory->ParamKind; 
        WriteShort(s, pfp->fp); 
      }
      else EmitWarning("Could not write num-samples");
    }

    FREE(memory);

    break;

  case BINARY:
    break;

  case ASCII:
    break;

  case CODEBOOK:
    codebook = (ParamCodeBook *)pfp->subformat;

    if (strcmp(pfp->mode, "w") == 0 && codebook->num_read_bits != 0) {
      long *l;

      codebook->last_word <<= (32 - codebook->num_read_bits);

      l = (long *)&codebook->last_word;
      WriteLong(*l, pfp->fp);

      /* Check if it's possible to rewind */
      if (fseek(pfp->fp, 0L, SEEK_SET) == 0) {
        /* Overwrite the header number of samples */
        WriteLong(magic, pfp->fp); 
        WriteLong(pfp->SampleSize,  pfp->fp); 
        WriteLong(pfp->NumProcessed, pfp->fp); 
      }
      else ErrorExit(SYSTEM_ERR_EXIT, "Could not rewind to fill in header");
    }

    FREE(((ParamCodeBook *)pfp->subformat)->entry);
    FREE(pfp->subformat);
    break;

  case PARAMCLASSES:
    break;
    
    default: ErrorExit(SYSTEM_ERR_EXIT, "Illegal format in CloseParameterFile.");
  }
  
  nFCLOSE(pfp->fp);

  FREE(pfp->fn);
  FREE(pfp->mode);
  FREE(pfp);
}


PARAM_FILE *
PopenParameterFile(char *command, ParamFileType format, 
                              int *SampleSize, char *mode) {
  ParamCodeBook *codebook;
  HTKmemory *memory;
  long magic;

  FILE *fp;
  PARAM_FILE *pfp;

  if (strcmp(mode, "w") != 0 && strcmp(mode, "r") != 0) ErrorExit(SYSTEM_ERR_EXIT, 
    "Only \"r\" and \"w\" are allowed modes for OpenParameterFile.");

  fp = popen(command, mode);
  if (!fp) ErrorExit(SYSTEM_ERR_EXIT, "Could not popen using command <%s>", command);

  CALLOC(pfp, 1, PARAM_FILE)

  pfp->fp         = fp;
  pfp->NumSamples = -1; 
  pfp->NumProcessed = 0; 
  pfp->format     = format;
  STRDUP(pfp->mode, mode);
  STRDUP(pfp->fn, command);
  pfp->SampleSize = *SampleSize;

  switch (pfp->format) {
  case BINARY:
    break;

  case ASCII:
    break;

  case HTKPARAM:
    if (strcmp(mode, "r") == 0) {
      long l;
      short s;

      /* Reading the 12 byte HTK header */
      CALLOC(pfp->subformat, 1, HTKmemory)
      memory = (HTKmemory *)pfp->subformat;

      l = ReadLong(fp);        /* Number of samples */
      pfp->NumSamples = l;
      l = ReadLong(fp);       /* Sample period (ns) */
      s = ReadShort(fp);      /* Num. bytes / sample */
      pfp->SampleSize = s / sizeof(float);

      s = ReadShort(fp);      /* Samplekind */
      if (s & 02000)
        ErrorExit(SYSTEM_ERR_EXIT, "HTK compressing is not implemented");
      memory->ParamKind = s;

      if (*SampleSize && pfp->SampleSize != *SampleSize) 
        ErrorExit(SYSTEM_ERR_EXIT, "Sample size %i expected. Got %i.",
          *SampleSize, pfp->SampleSize);
      *SampleSize = pfp->SampleSize;

    }
    else { /* mode == "w" */
      ErrorExit(SYSTEM_ERR_EXIT, "Writing HTK parameter files is not yet implemented");
    }
    break;

  case CODEBOOK:
    CALLOC(pfp->subformat, 1, ParamCodeBook)
    codebook = (ParamCodeBook *)pfp->subformat;

    magic = ReadLong(fp);
    if (magic != PARAMCODEBOOKMAGIC) ErrorExit(SYSTEM_ERR_EXIT, 
      "Error reading magic number of codebook parameter file");

    if (strcmp(mode, "r") == 0) {
      codebook->size = ReadLong(fp);
      codebook->bit_size = ReadLong(fp);
      CALLOC(codebook->entry, codebook->size, float)
      ReadFloats(codebook->entry, codebook->size, fp);

      codebook->num_read_bits = codebook->bit_size;
    }
    else {
      codebook->size = codebook->bit_size = -1;
    }
    break;

  case PARAMCLASSES:
    break;

    default: ErrorExit(SYSTEM_ERR_EXIT, "Illegal format in OpenParameterFile.");
  }

  return pfp;
}


void 
PcloseParameterFile(PARAM_FILE *pfp) {
  ParamCodeBook *codebook;
  HTKmemory *memory;

  pclose(pfp->fp);

  switch (pfp->format) {

  case BINARY:
    break;

  case HTKPARAM:
    memory = (HTKmemory *)pfp->subformat;
    FREE(memory);
    break;

  case ASCII:
    break;

  case CODEBOOK:
    codebook = (ParamCodeBook *)pfp->subformat;
    if (strcmp(pfp->mode, "w") == 0 && codebook->num_read_bits != 0) {
      long *l = (long *)&codebook->last_word;
      WriteLong(*l, pfp->fp);
    }
    FREE(((ParamCodeBook *)pfp->subformat)->entry);
    FREE(pfp->subformat);
    break;

  case PARAMCLASSES:
    break;


  default: ErrorExit(SYSTEM_ERR_EXIT, "Illegal format in CloseParameterSample.");
  }
  
  FREE(pfp->fn);
  FREE(pfp->mode);
  FREE(pfp);
}


float *
ReadParameterSample(PARAM_FILE *pfp) {
  ParamCodeBook *codebook;
  float *sample = NULL;
  int i;
  unsigned long index, bitmask;
  HTKmemory *memory;

  if (strcmp(pfp->mode, "r") != 0) 
    ErrorExit(SYSTEM_ERR_EXIT, 
      "Attempt to call ReadParameterSample with a mode=\"w\"-filepointer.");

  switch (pfp->format) {
  case BINARY:
    CALLOC(sample, pfp->SampleSize, float)
    if ((i = ReadFloats(sample, pfp->SampleSize, pfp->fp)) != pfp->SampleSize)
      ErrorExit(SYSTEM_ERR_EXIT, 
		"Failing reading sample %d from Parameterfile: %s (%d/%d read).", 
		pfp->NumProcessed + 1, pfp->fn, i, pfp->SampleSize);
    break;

  case HTKPARAM:
    memory = (HTKmemory *)pfp->subformat;
    CALLOC(sample, pfp->SampleSize, float);

    if ((i = ReadFloats(sample, pfp->SampleSize, pfp->fp)) != pfp->SampleSize)
      ErrorExit(SYSTEM_ERR_EXIT, "Failing reading from Parameterfile: %s (%d/%d read).", 
		pfp->fn, i, pfp->SampleSize);
    break;

  case ASCII:
    CALLOC(sample, pfp->SampleSize, float)
    for (i = 0; i < pfp->SampleSize; i++) {
      if (feof(pfp->fp)) ErrorExit(SYSTEM_ERR_EXIT, 
         "Truncated sample at end of: %s", pfp->fn);
      fscanf(pfp->fp, " %f ", &(sample[i])); 
    }
    break;

  case CODEBOOK:
    CALLOC(sample, pfp->SampleSize, float)
    codebook = (ParamCodeBook *)pfp->subformat;

    /* Leaves only the 'codebook->bit_size' leftmost bits in 'bitmask' */
    bitmask = 0xFFFFFFFF << (32 - codebook->bit_size);

    for (i = 0; i < pfp->SampleSize; i++) {

      if (codebook->num_read_bits == 32) {
        long *l = (long *)&(codebook->last_word);
        if (ReadLongs(l, 1, pfp->fp) != 1) {
	  ErrorExit(SYSTEM_ERR_EXIT, "Failing reading from Parameterfile: %s.", pfp->fn);
        }
        codebook->num_read_bits = 0;
      }

      index = codebook->last_word & bitmask;
      index >>= (32 - codebook->bit_size);
      sample[i] = codebook->entry[index];
      codebook->last_word <<= codebook->bit_size;
      codebook->num_read_bits += codebook->bit_size;
    }
    break;

  case PARAMCLASSES:
    CALLOC(sample, pfp->SampleSize, float)
      index = ReadLong(pfp->fp);
    if (index < 0 || index >= pfp->SampleSize) {
      ErrorExit(SYSTEM_ERR_EXIT, "Failing reading from Parameterfile: %s.", pfp->fn);
    }
    sample[index] = 1.0;
    break;
    
    default: ErrorExit(SYSTEM_ERR_EXIT, "Illegal format in ReadParameterSample.");
  }

  pfp->NumProcessed++;

  return sample;
}


void 
WriteParameterSample(PARAM_FILE *pfp, float *sample) {
  ParamCodeBook *codebook;
  int i;

  if (strcmp(pfp->mode, "w") != 0) 
    ErrorExit(SYSTEM_ERR_EXIT, 
      "Attempt to call WriteParameterSample with a mode=\"r\"-filepointer.");

  switch (pfp->format) {
  case BINARY:
    if (WriteFloats(sample, pfp->SampleSize, pfp->fp) != pfp->SampleSize)
      ErrorExit(SYSTEM_ERR_EXIT, "Failing writing to Parameterfile: %s", pfp->fn);
    break;

  case HTKPARAM:
    if (WriteFloats(sample, pfp->SampleSize, pfp->fp) != pfp->SampleSize)
      ErrorExit(SYSTEM_ERR_EXIT, "Failing writing to Parameterfile: %s", pfp->fn);
    break;

  case ASCII:
    for (i = 0; i < pfp->SampleSize; i++) {
      fprintf(pfp->fp, "%-8.3f ", sample[i]); 
    }
    fprintf(pfp->fp, "\n");
    break;

  case CODEBOOK:
    codebook = (ParamCodeBook *)pfp->subformat;

    for (i = 0; i < pfp->SampleSize; i++) {
      float best_diff, this_diff;
      unsigned long index, best_index = -1;

      if (codebook->num_read_bits == 32) {
        long *l = (long *)&(codebook->last_word);
        WriteLong(*l, pfp->fp);
        codebook->num_read_bits = 0;
        codebook->last_word = 0;
      }

      best_diff = 10000.0;
      for (index = 0; index < codebook->size; index++) {
        if ((this_diff = fabs(sample[i] - codebook->entry[index])) 
            < best_diff) {
          best_index = index;
          best_diff = this_diff;
        }
      }
      index = best_index;

      codebook->last_word <<= codebook->bit_size;
      codebook->last_word |= index;
      codebook->num_read_bits += codebook->bit_size;
    }
    break;

  case PARAMCLASSES:
    for (i = 0; i < pfp->SampleSize; i++) {
      if (sample[i] != 0.0) {
	WriteLong(1.0, pfp->fp);
	break;
      }
    } 
    break;

    default: ErrorExit(SYSTEM_ERR_EXIT, "Illegal format in WriteParameterSample.");
  }

  pfp->NumProcessed++;
}


int 
ParameterFileEOF(PARAM_FILE *pfp) {
  int c;
  int flag;

  switch (pfp->format) {

  case CODEBOOK:
    flag = (pfp->NumProcessed == pfp->NumSamples);
    break;

  case ASCII:
    while (isspace(c = getc(pfp->fp)));
    flag = feof(pfp->fp);
    ungetc(c, pfp->fp);
    break;
    
  default:
    c = fgetc(pfp->fp);
    if (c == EOF) {
      flag = 1;
    }
    else {
      flag = feof(pfp->fp);
      ungetc(c, pfp->fp);
    }
    break;
  }

  return flag;
}


float **
LoadParameters(char *fn, ParamFileType format, int *SampleSize, int *NumSamples) { 
  PARAM_FILE *pfp;
  float **Data;
  int allocatedT;
  int t;

  pfp = OpenParameterFile(fn, format, SampleSize, "r");

  if (*SampleSize != 0) {
    if (*SampleSize != pfp->SampleSize) {
      ErrorExit(SYSTEM_ERR_EXIT, "Sample size %i expected -- got %i", 
        *SampleSize, pfp->SampleSize);
    }
  }
  else *SampleSize = pfp->SampleSize;

  allocatedT = 256;
  CALLOC(Data, allocatedT, float *)

  t = 0;
  while (!ParameterFileEOF(pfp)) {
    if (*NumSamples > 0 && t == *NumSamples) {
      ErrorExit(SYSTEM_ERR_EXIT, "Datafile too long: %s %i %i", pfp->fn);
    }
    if (t == allocatedT) {
      allocatedT *= 2;
      REALLOC(Data, allocatedT, float *)
    }
    Data[t++] = ReadParameterSample(pfp); 
  }

  if (*NumSamples > 0 && t != *NumSamples) {
    ErrorExit(SYSTEM_ERR_EXIT, "Datafile too short: %s", pfp->fn);
  }

  if (!t) allocatedT = 1; else allocatedT = t;
  REALLOC(Data, allocatedT, float *)

  CloseParameterFile(pfp);

  *NumSamples = t;

  return Data;
}


float **
FilterLoadParams(char *command, ParamFileType format, 
		 int *SampleSize, int *NumSamples) { 
  PARAM_FILE *pfp;
  float **Data;
  int t = 0;
  int allocatedT;

  pfp = PopenParameterFile(command, format, SampleSize, "r");

  allocatedT = 256;
  CALLOC(Data, allocatedT, float *)

  while (!ParameterFileEOF(pfp)) {
    if (*NumSamples > 0 && t == *NumSamples) {
      ErrorExit(SYSTEM_ERR_EXIT, "Datafile too long: %s", pfp->fn);
    }
    if (t == allocatedT) {
      allocatedT *= 2;
      REALLOC(Data, allocatedT, float *)
    }
    Data[t++] = ReadParameterSample(pfp); 
  }

  if (*NumSamples > 0 && t != *NumSamples) {
    ErrorExit(SYSTEM_ERR_EXIT, "Datafile too short: %s", pfp->fn);
  }

  if (!t) allocatedT = 1; else allocatedT = t;
  REALLOC(Data, allocatedT, float *)
 
  PcloseParameterFile(pfp);

  *NumSamples = t;

  return Data;
}


void 
SaveParameters(char *fn, ParamFileType format, 
	       int SampleSize, int NumSamples, float **Data) {
  PARAM_FILE *pfp;
  int i;
  
  pfp = OpenParameterFile(fn, format, &SampleSize, "w");

  for (i = 0; i < NumSamples; i++) {
    WriteParameterSample(pfp, Data[i]);
  }

  CloseParameterFile(pfp);
}


void 
FilterSaveParams(char *command, ParamFileType format, 
                      int SampleSize, int NumSamples, float **Data) {
  PARAM_FILE *pfp;
  int i;
  
  pfp = PopenParameterFile(command, format, &SampleSize, "w");

  for (i = 0; i < NumSamples; i++) {
    WriteParameterSample(pfp, Data[i]);
  }

  PcloseParameterFile(pfp);
}


void 
FreeParameters(float **Data, int NumSamples) {
  FREE2D(Data, NumSamples)
}


/* Initializes the subformat info for the CODEBOOK-format */
/* Call this function after opening a new codebook-file for writing */
void 
InitParamCodeBook(PARAM_FILE *pfp, int size, float *entry) {
  ParamCodeBook *codebook;
  int i, n;

  if (pfp->format != CODEBOOK) ErrorExit(SYSTEM_ERR_EXIT, 
    "InitParamCodeBook called with a non-codebook parameter-filepointer");

  if (strcmp(pfp->mode, "w") != 0) ErrorExit(SYSTEM_ERR_EXIT, 
    "InitParamCodeBook called with a file-mode other than \"w\"");

  codebook = (ParamCodeBook *)pfp->subformat;

  CALLOC(codebook->entry, size, float)
  codebook->size = size;

  for (i = 0; i < size; i++) {
    codebook->entry[i] = entry[i];
  }

  n = (int)ceil(LOG2(size));
  if (n > 32) ErrorExit(SYSTEM_ERR_EXIT, "The size of a codebook is limited to 0xFFFFFFFF");
  else if (n > 16) codebook->bit_size = 32;
  else if (n > 8) codebook->bit_size = 16;
  else if (n > 4) codebook->bit_size = 8;
  else if (n > 2) codebook->bit_size = 4;
  else if (n > 1) codebook->bit_size = 2;
  else codebook->bit_size = 1;

  WriteLong(codebook->size, pfp->fp);
  WriteLong(codebook->bit_size, pfp->fp);
  WriteFloats(codebook->entry, codebook->size, pfp->fp);
}


/* Info about an open PARAM_FILE is returned from these functions */
ParamFileType GetParamFileType(PARAM_FILE *pfp) {
  return pfp->format;
}


char *
GetParamFileMode(PARAM_FILE *pfp) {
  return pfp->mode;
}

int 
GetParamFileNumSamples(PARAM_FILE *pfp) {
  return pfp->NumSamples;
}

int 
GetParamFileSampleSize(PARAM_FILE *pfp) {
  return pfp->SampleSize;
}

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  ParamData.c                                      */
/* --------------------------------------------------------------------------*/
