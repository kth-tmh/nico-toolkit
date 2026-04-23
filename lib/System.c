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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#include <math.h>
#include <fcntl.h>
#include <sys/times.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>
#include <stdarg.h>
#include "System.h"
#include "nMath.h"

#define MAXSYSTEMLINE 1024

static long clock_tick = -1;

/* file name parsing */

char *
MakeName(char *d, char *n, char *x) {   
  static char line[MAXSYSTEMLINE];
  char *r; 

  if (*d == '\0') strcpy(line, n);
  else {
    strcpy(line,d);
    if (line[strlen(line) - 1] != '/') strcat(line, "/");
    strcat(line,n);
  }
  if (*x != '\0') { 
    if (*x != '.') strcat(line,".");
    strcat(line, x); 
  }
  CALLOC(r, strlen(line) + 1, char)
  strcpy(r, line);
  return r;
}


/* Extract the directory, filename and file extension from a path */
/* dir, name, and ext has to have been allocated before the call */
void 
Extract(char *in, char *dir, char *name, char *ext) {                
  int i, j, k;
  
  k = strlen(in) - 1;      

  for (i = k; i >= 0 && in[i] != '.'; i--);
  if (i == -1) i = k + 1;
  memcpy(ext, in + i + 1, sizeof(char) * (k - i + 1)); 
  ext[k - i + 1] = '\0'; 

  for (j = k; j >= 0 && in[j] != '/'; j--);
  memcpy(name, in + j + 1, sizeof(char) * (i - j - 1));
  name[i - j - 1] = '\0'; 

  memcpy(dir, in, sizeof(char) * (j + 1));
  dir[j + 1] = '\0'; 
}


char *
ModifyName(char *in, char *X, char *P) {
  static char p[MAXSYSTEMLINE];
  static char n[MAXSYSTEMLINE];
  static char x[MAXSYSTEMLINE];   

  Extract(in, p, n, x);
  if (*P != '\0') strcpy(p, P);
  if (*X != '\0') strcpy(x, X);
  return MakeName(p, n, x);
}
 

char **
LoadFileList(char *fn, char *P, char *X, int *N) {  
  FILE *fp;  
  static char line[MAXSYSTEMLINE];
  static char p[MAXSYSTEMLINE];
  static char n[MAXSYSTEMLINE];
  static char x[MAXSYSTEMLINE];
  char **List, *c;

  *N = 0;
  nFOPEN(fp, fn, "r")
  CALLOC(List, 1, char *);
  while (!feof(fp)) {
    fgets(line, 256, fp);
    c = line; 
    while (!isspace((int)*c) && !iscntrl((int)*c)) c++; *c = '\0'; /* rm linefeed */
    Extract(line, p, n, x);
    if (*X != '\0') strcpy(x, X);
    if (*P != '\0') strcpy(p, P);
    List[*N] = MakeName(p, n, x);
    (*N)++;  
    REALLOC(List, (*N + 1), char *);
    fscanf(fp," \n"); 
  } 
  nFCLOSE(fp);
  return List;
}               


/* Randomize the order of the lists of strings */
/* If the the second list is NULL, the function operates only on the first */
void 
RandomizeLists(char **list1, char **list2, int n) {
  int i, x;
  char *tmp;

  Randomize();

  for (i = 0; i < n; i++) {
    x = (int)floor(RectRand() * n);
    if (x == n) x = n - 1; /* not very likely - but possible */

    tmp = list1[i];
    list1[i] = list1[x];
    list1[x] = tmp;

    if (list2) {
      tmp = list2[i];
      list2[i] = list2[x];
      list2[x] = tmp;
    }
  }
}


/* returns the last time the file was touched */
long
TouchedTime(char *file) {
  time_t t;
  struct stat s;

  stat(file, &s);

  t =  s.st_mtime > s.st_ctime ? s.st_mtime : s.st_ctime;

  return (long)t;
}


/* Returns the cpu-time used by the process in ms. */
void 
my_time(unsigned long *user, unsigned long *system) {
  struct tms t;

  if (clock_tick == -1) clock_tick = sysconf(_SC_CLK_TCK);
  times(&t);
  *user = (unsigned long)t.tms_utime * 1000.0 / clock_tick;
  *system = (unsigned long)t.tms_stime * 1000.0 / clock_tick;
}


/* Get a string with the run-time in hours, minues, sec's and parts of sec's */
char *
MyRunTime() {
  unsigned long u, s;
  float seconds;
  int minutes = 0, hours = 0;
  char runtime[256], *retval;

  my_time(&u, &s);

  seconds = (float)u / 1000.0;
  if (seconds >= 60.0) {
    minutes = (int)(floor(seconds) / 60.0);
    seconds -= 60.0 * (float)minutes;
  }
  if (minutes >= 60) {
    hours = minutes / 60;
    minutes %= 60;
  }

  if (hours > 0) 
    sprintf(runtime, "%ih %imin %.1fsec", hours, minutes, seconds);
  else if (minutes > 0) 
    sprintf(runtime, "%imin %.1fsec", minutes, seconds);
  else 
    sprintf(runtime, "%.1fsec", seconds);

  STRDUP(retval, runtime)

  return retval;
}


void 
print_time_and_date(FILE *stream) {
  time_t timer = time(&timer);

  fprintf(stream, "%s", ctime(&timer));
}


void 
sprint_time_and_date(char *string) {
  time_t timer = time(&timer);

  sprintf(string, "%s", ctime(&timer));
  string[strlen(string) - 1] = '\0';
}


/* ----------------          Errors and Warnings              ------------- */

static NICOErrHandler *NICOCustomErrHandler = NICODefaultErr;

static NICOWarnHandler *NICOCustomWarnHandler = NICODefaultWarn; 

/* Use this funktion to specify user exception handles */
void 
NICOExceptions(NICOErrHandler *errh, NICOWarnHandler *warnh) {
  NICOCustomErrHandler = errh;
  NICOCustomWarnHandler = warnh;
}

/* 'format' is a printf-type format string.                                 */
/* 'format' together with the arguments following it is the error message.  */
/* In the baseline implementation, 'ErrorExit' simply prints the error      */
/* message to stderr and exits with 'exit_value'.                           */
void 
ErrorExit(int exit_value, char *format, ...) {
  va_list args;
  char msg[256];

  va_start(args, format);
  vsprintf(msg, format, args);
  va_end(args);

  (NICOCustomErrHandler)(exit_value, msg);
}


void 
NICODefaultErr(int exit_value, char *msg) {
  fprintf(stderr, "Error: %s\n", msg);
  exit(exit_value);
}



/* Print a warning to the user                                              */
/* 'format' is a printf-type format string.                                 */
/* 'format' together with the arguments following it is the warning.        */
void 
EmitWarning(char *format, ...) {
  va_list args;
  char msg[256];

  va_start(args, format);
  vsprintf(msg, format, args);
  va_end(args);

  (NICOCustomWarnHandler)(msg);
}


void 
NICODefaultWarn(char *msg) {
  fprintf(stderr, "Warning: %s\n", msg);
}



/* ----------------  System independent read/write functions  ---------------- */

char *
ReadString(FILE *fp) {
  unsigned char size0;
  int size;
  char s[512], *r;

  fread(&size0, sizeof(unsigned char), 1, fp);
  size = (int)size0;
  fread(s, sizeof(char), size, fp);

  STRDUP(r, s);
  return r;
}


void 
WriteString(char *s, FILE *fp) {
  int size;
  unsigned char size0;

  size = strlen(s) + 1;
  size0 = (unsigned char)size;
  fwrite(&size0, sizeof(unsigned char), 1, fp);

  fwrite(s, sizeof(char), size, fp);
}


static void 
swap_2bytes(char *bytes, int n) {
  int i;
  char c, *b = bytes;

  for (i = 0; i < n; i++) {
    c = *b;
    *b = *(b + 1);
    b++;
    *b = c;
    b++; 
  }
}


static void 
swap_4bytes(char *bytes, int n) {
  int i;
  char c, *b = bytes;

  for (i = 0; i < n; i++) {
    /* swap first with last byte */
    c = *b;
    *b = *(b + 3);
    *(b + 3) = c;
    /* swap middle two bytes */
    b++;
    c = *b;
    *b = *(b + 1);
    *(b + 1) = c;

    b += 3;
  }
}


static void 
swap_8bytes(char *bytes, int n) {
  int i;
  char c, *b = bytes;

  for (i = 0; i < n; i++) {
    /* swap first with last byte */
    c = *b;
    *b = *(b + 7);
    *(b + 7) = c;

    /* swap byte #2 with byte #7*/
    b++;
    c = *b;
    *b = *(b + 5);
    *(b + 5) = c;

    /* swap byte #3 with byte #6*/
    b++;
    c = *b;
    *b = *(b + 3);
    *(b + 3) = c;

    /* swap middle two bytes */
    b++;
    c = *b;
    *b = *(b + 1);
    *(b + 1) = c;

    b += 5;
  }
}


/* Returns 0 if the byte order is big endian, 1 if it is little endian */
static short lowbyteisoneandhighbyteiszero = 1;
int 
GetNativeByteOrder() {
  return (int)(*(char *)&lowbyteisoneandhighbyteiszero);
}

#define NATIVE_LITTLE_ENDIAN ((int)(*(char *)&lowbyteisoneandhighbyteiszero))
#define NATIVE_BIG_ENDIAN (!(int)(*(char *)&lowbyteisoneandhighbyteiszero))


long 
ReadInt32(FILE *fp, int little_endian) {
  long i;  
  fread(&i, 4, 1, fp);

  if ((NATIVE_LITTLE_ENDIAN && !little_endian) ||
      (NATIVE_BIG_ENDIAN && little_endian)) swap_4bytes((char *)&i, 1);

  return i;
}


int 
ReadInts32(long *i, int n, FILE *fp, int little_endian) {
  int m = fread(i, 4, n, fp);

  if ((NATIVE_LITTLE_ENDIAN && !little_endian) ||
      (NATIVE_BIG_ENDIAN && little_endian)) swap_4bytes((char *)i, n);

  return m;
}


short 
ReadInt16(FILE *fp, int little_endian) {
  short i;  
  fread(&i, 2, 1, fp);

  if ((NATIVE_LITTLE_ENDIAN && !little_endian) ||
      (NATIVE_BIG_ENDIAN && little_endian)) swap_2bytes((char *)&i, 1);

  return i;
}


int 
ReadInts16(short *i, int n, FILE *fp, int little_endian) {
  int m = fread(i, 2, n, fp);

  if ((NATIVE_LITTLE_ENDIAN && !little_endian) ||
      (NATIVE_BIG_ENDIAN && little_endian)) swap_2bytes((char *)i, n);

  return m;
}


/* --------------
 * The following set of functions all assume that the external storage is 
 * big endian, but adjust the reading and writing operations depending on
 * the native byte order of the architecture. */


/* if the native byte order is little endian, we need to swap bytes*/
#define SWAP_BYTE_ORDER ((int)(*(char *)&lowbyteisoneandhighbyteiszero))

long 
ReadLong(FILE *fp) {
  long i;  
  fread(&i, 4, 1, fp);

  if (SWAP_BYTE_ORDER) swap_4bytes((char *)&i, 1);

  return i;
}

int 
ReadLongs(long *i, int n, FILE *fp) {
  int m = fread(i, 4, n, fp);

  if (SWAP_BYTE_ORDER) swap_4bytes((char *)i, n);

  return m;
}


void 
WriteLong(long i, FILE *fp) {
  if (SWAP_BYTE_ORDER) swap_4bytes((char *)&i, 1);

  fwrite(&i, 4, 1, fp);

  if (SWAP_BYTE_ORDER) swap_4bytes((char *)&i, 1);
}


int 
WriteLongs(long *i, int n, FILE *fp) {
  int m;

  if (SWAP_BYTE_ORDER) swap_4bytes((char *)i, n);

  m = fwrite(i, 4, n, fp);

  if (SWAP_BYTE_ORDER) swap_4bytes((char *)i, n);

  return m;
}


short 
ReadShort(FILE *fp) {
  short i;  
  fread(&i, 2, 1, fp);

  if (SWAP_BYTE_ORDER) swap_2bytes((char *)&i, 1);

  return i;
}


int 
ReadShorts(short *i, int n, FILE *fp) {
  int m = fread(i, 2, n, fp);
	
  if (SWAP_BYTE_ORDER) swap_2bytes((char *)i, n);

  return m;
}


void 
WriteShort(short i, FILE *fp) {
  if (SWAP_BYTE_ORDER) swap_2bytes((char *)&i, 1);

  fwrite(&i, sizeof(short), 1, fp);

  if (SWAP_BYTE_ORDER) swap_2bytes((char *)&i, 1);
}


int 
WriteShorts(short *i, int n, FILE *fp) {
  int m;

  if (SWAP_BYTE_ORDER) swap_2bytes((char *)i, n);

  m = fwrite(i, sizeof(short), n, fp);

  if (SWAP_BYTE_ORDER) swap_2bytes((char *)i, n);

  return m;
}


float 
ReadFloat(FILE *fp) {
  float f;  

  assert(sizeof(float) == 4);

  fread(&f, sizeof(float), 1, fp);

  if (SWAP_BYTE_ORDER) swap_4bytes((char *)&f, 1);

  return f;
}


int 
ReadFloats(float *f, int n, FILE *fp) {
  int m = fread(f, sizeof(float), n, fp);

  assert(sizeof(float) == 4);

  if (SWAP_BYTE_ORDER) swap_4bytes((char *)f, n);

  return m;
}


void 
WriteFloat(float f, FILE *fp) {
  assert(sizeof(float) == 4);

  if (SWAP_BYTE_ORDER) swap_4bytes((char *)&f, 1);

  fwrite(&f, sizeof(float), 1, fp);

  if (SWAP_BYTE_ORDER) swap_4bytes((char *)&f, 1);
}


int 
WriteFloats(float *f, int n, FILE *fp) {
  int m;

  assert(sizeof(float) == 4);

  if (SWAP_BYTE_ORDER) swap_4bytes((char *)f, n);

  m = fwrite(f, sizeof(float), n, fp);

  if (SWAP_BYTE_ORDER) swap_4bytes((char *)f, n);

  return m;
}


double 
ReadDouble(FILE *fp) {
  double i;  
  fread(&i, sizeof(double), 1, fp);

  if (SWAP_BYTE_ORDER) swap_8bytes((char *)&i, 1);

  return i;
}


int 
ReadDoubles(double *i, int n, FILE *fp) {
  int m = fread(i, sizeof(double), n, fp);

  assert(sizeof(double) == 8);

  if (SWAP_BYTE_ORDER) swap_8bytes((char *)i, n);

  return m;
}


void 
WriteDouble(double i, FILE *fp) {
  assert(sizeof(double) == 8);

  if (SWAP_BYTE_ORDER) swap_8bytes((char *)&i, 4);

  fwrite(&i, sizeof(double), 1, fp);

  if (SWAP_BYTE_ORDER) swap_8bytes((char *)&i, 4);
}


int 
WriteDoubles(double *i, int n, FILE *fp) {
  int m;
 
  assert(sizeof(double) == 8);

  if (SWAP_BYTE_ORDER) swap_8bytes((char *)i, n);

  m = fwrite(i, sizeof(double), n, fp);

  if (SWAP_BYTE_ORDER) swap_8bytes((char *)i, n);

  return m;
}


/* --------------------------------------------------------------------------*/
/*           END OF FILE :  System.c                                         */
/* --------------------------------------------------------------------------*/
