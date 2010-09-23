/* ------------------------------------------------------------------------- */
/*                 Functions related to the operating system                 */
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

#ifndef SYSTEM_H_LOADED
#define SYSTEM_H_LOADED


/* The maximum expected length of a command line string */
#define MAXSYSTEMLINE 1024

void LoadArgFile(char *fn, char *options);


/* Returns an allocated string holding the full pathname */
char *MakeName(char *Directory, char *Name, char *Extension);


/* Exctracts the parts of the path.                                          */
/* Directory, Name and Extension must have been allocated                    */
void Extract(char *in, char *Directory, char *Name, char *Extension);


/* If Extension or Directory are not empty, they replace the respective part */
/* of the path in 'in'. A new string is allocated                            */
char *ModifyName(char *in, char *Extension, char *Directory);


/* Loads and allocates filenames from a script file. ModifyName is called    */
/* with 'Extension' and 'Directory'. for all files.                          */
/* The number of filenames read is returned in 'N'.                          */
char **LoadFileList(char *fn, char *Directory, char *Extension, int *N);


/* Randomize the order of the lists of strings */
/* If the the second list is NULL, the function operates only on the first */
void RandomizeLists(char **list1, char **list2, int n);

/* returns the last time the file was touched */
long TouchedTime(char *file);

/*     Get time of current process      */
void my_time(unsigned long *user, unsigned long *system);


/* Get a string with the time in hours, minues, sec's and parts of sec's */
char *MyRunTime();


/* Print time and date */
void print_time_and_date(FILE *stream);
void sprint_time_and_date(char *string);



/* ----------------          Errors and Warnings              ------------- */

#define SYSTEM_OK_EXIT 0
#define SYSTEM_ERR_EXIT -1

typedef void (NICOErrHandler) (int exit_value, char *msg);

typedef void (NICOWarnHandler) (char *msg);

/* Use this funktion to specify user exception handles */
void NICOExceptions(NICOErrHandler *, NICOWarnHandler *);

/* 'format' is a printf-type format string.                                 */
/* 'format' together with the arguments following it is the error message.  */
/* In this the baseline implementation, 'ErrorExit' simply prints the error */
/* message to stderr and exits with 'exit_value'.                           */
/* The function calls "NICOErrHandler" before returning                     */
void ErrorExit(int exit_value, char *format, ...);

/* Print a warning to the user                                              */
/* 'format' is a printf-type format string.                                 */
/* 'format' together with the arguments following it is the warning.        */
/* The function calls "NICOWarnHandler" before returning                    */
void EmitWarning(char *format, ...);


/* The Default Error Handler                                                */
void NICODefaultErr(int exit_value, char *msg);


/* The Default Warning Handler                                              */
void NICODefaultWarn(char *msg);


/* --------------  System independent read/write functions  --------------- */

char *ReadString(FILE *);
void WriteString(char *s, FILE *);


/* Returns 1 if the native byteorder is little endian (low-high) otherwize 0*/
int GetNativeByteOrder();


/* --------------
 * This set of functions read and write from/to external stoarage with the
 * byte order defined by the last argument. Reading and writing operations
 * are adjusted depending on the native byte order of the architecture. */

long ReadInt32(FILE *fp, int little_endian);

int ReadInts32(long *i, int n, FILE *fp, int little_endian);

short ReadInt16(FILE *fp, int little_endian);

int ReadInts16(short *i, int n, FILE *fp, int little_endian);



/* --------------
 * The following set of functions all assume that the external storage is 
 * big endian, but adjust the reading and writing operations depending on
 * the native byte order of the architecture.
 * It is assumed that a "float" is 4 bytes and a "double" is 8 bytes long. */

short ReadShort(FILE *);
int ReadShorts(short *, int n, FILE *);

void WriteShort(short i, FILE *);
int WriteShorts(short *i, int n, FILE *);

long ReadLong(FILE *);
int ReadLongs(long *, int n, FILE *);
void WriteLong(long i, FILE *);
int WriteLongs(long *i, int n, FILE *);

float ReadFloat(FILE *);
int ReadFloats(float *, int n, FILE *);
void WriteFloat(float i, FILE *);
int WriteFloats(float *i, int n, FILE *);

double ReadDouble(FILE *);
int ReadDoubles(double *, int n, FILE *);
void WriteDouble(double i, FILE *);
int WriteDoubles(double *i, int n, FILE *);



/* Some def's for convenient file handeling and memory allocation */

#define nFOPEN(FP, NAME, MODE)  { \
  if (!(FP = fopen(NAME, MODE))) { \
    ErrorExit(0, "Cannot open %s. (mode = %s).", NAME, MODE); \
  } \
}


#define nFCLOSE(FP)  { \
  if (fclose(FP) == EOF) { \
    ErrorExit(-1, "in file %s, line %i: Cannot close file pointer %s.", \
      __FILE__, __LINE__, #FP); \
  } \
}


#define MALLOC(PTR, N, TYPE) { \
  if (!(PTR = (TYPE *)malloc(N * sizeof(TYPE)))) { \
     ErrorExit(-1, \
       "in file %s, line %i: Cannot allocate %i objects of type %s for %s.", \
       __FILE__, __LINE__, N, #TYPE, #PTR); \
  } \
}


#define CALLOC(PTR, N, TYPE) { \
  if (!(PTR = (TYPE *)calloc(N, sizeof(TYPE)))) { \
     ErrorExit(-1, \
       "in file %s, line %i: Cannot allocate %i objects of type %s for %s.", \
       __FILE__, __LINE__, N, #TYPE, #PTR); \
   } \
}


#define STRDUP(NEW_STRING, OLD_STRING) { \
  if (!(NEW_STRING = (char *)calloc(strlen(OLD_STRING) + 1, sizeof(char)))) { \
     ErrorExit(-1, \
       "in file %s, line %i: Cannot allocate for string duplication to %s.", \
       __FILE__, __LINE__, #NEW_STRING); \
  } \
  strcpy(NEW_STRING, OLD_STRING); \
}


#define REALLOC(PTR, N, TYPE) { \
  if (!(PTR = (TYPE *)realloc(PTR, (N) * sizeof(TYPE)))) { \
    ErrorExit(-1, \
      "in file %s, line %i: Cannot reallocate %i objects of type %s for %s.", \
      __FILE__, __LINE__, N, #TYPE, #PTR); \
  } \
}


#define CALLOC2D(PTR, N, M, TYPE) { \
  int i; \
  if (!(PTR = (TYPE **)calloc(N, sizeof(TYPE *)))) { \
    ErrorExit(-1, "in file %s, line %i: Cannot allocate %i objects \
      of type (%s *) for 2D-array %s.", \
      __FILE__, __LINE__, N, #TYPE, #PTR); \
  } \
  for (i = 0; i < N; i++) { \
    if (!(PTR[i] = (TYPE *)calloc(M, sizeof(TYPE)))) { \
      ErrorExit(-1, "in file %s, line %i: Cannot allocate %i objects \
        of type %s for 2D-array %s[%i].", \
        __FILE__, __LINE__, M, #TYPE, #PTR, i); \
    } \
  } \
}


#define FREE(PTR) { \
  free(PTR); \
}


#define FREE2D(PTR, N) { \
  int i; \
  for (i = 0; i < N; i++) { \
    free(PTR[i]); \
  } \
  free(PTR); \
}

#endif

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  System.h                                         */
/* --------------------------------------------------------------------------*/
