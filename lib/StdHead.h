/* ------------------------------------------------------------------------- */
/*   The original Stdhead-software was written by Sten Ternstrom and is      */
/*   public domain.                                                          */
/* ------------------------------------------------------------------------- */
/*                                                                           */
/*   Copyright (C) 1995, 1996, 1997, 1998 Nikko Strom                        */
/*                                                                           */
/*   All rights reserved.                                                    */
/*                                                                           */
/*   Modification by Nikko Strom at the Dept. of Speech, Music and Hearing,  */
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


/* =======  Error return codes  ======= */

#define  HD_GLOBALFOUND 4       /* Value found in global defaults */
#define  HD_LOCALFOUND  2       /* Value found in local defaults */
#define  HD_FOUND       1       /* Value found in current header */
#define  HD_OK          0       /* Value not found; but no errors */
#define  HD_FILEERR     -1      /* File access error (invalid file handle?) */
#define  HD_MEMERR      -2      /* Memory allocation error */
#define  HD_NULLPTR     -4      /* A NULL char ptr was passed */
#define  HD_BADVAR      -5      /* SetHeaderStr: bad input */
#define  HD_NOTINT      -6      /* GetHeaderInt: value was not an integer */
#define  HD_NOTDOUBLE   -7      /* GetHeaderDouble: value was not a double */
#define  HD_BADHEADER   -8      /* Header could not be parsed */
#define  HD_OVERFLOW    -9      /* SetHeaderStr: no more room in header */


/* ======= Other #defines ======= */

#define  HD_DEFAULTSIZE  1024
#define  HD_MAXLINE      255
#define  HD_EMPTYHEADER  "=\r\n"

/* ======= Typedefs ======= */

typedef struct hdesc {
  int   size;
  char  *bufptr;
} HDESC;

typedef struct hdesc *PDESC;
typedef struct hdesc *HANDLE;
typedef long LONG;
typedef double DOUBLE;
typedef char BOOL;
#define FALSE 0
#define TRUE 1


/* ====== Forward declarations of public routines ====== */

HANDLE LoadHeader(FILE *hFile);
HANDLE MakeHeader(int size);
int SaveHeader(HANDLE hd, FILE *hFile);
int FreeHeader(HANDLE hd);

int GetHeaderSpace(HANDLE hd);

int GetHeaderStr(HANDLE hd, char *var, char *value, int max_len);
char *GetHeaderText(HANDLE hd);

int GetHeaderLong(HANDLE hd, char *var, LONG *pLong);
int GetHeaderDouble(HANDLE hd, char *var, DOUBLE *pDouble);

int SetHeaderStr(HANDLE hd, char *var, char *value);
int SetHeaderText(HANDLE hd, char *szText);


/* --------------------------------------------------------------------------*/
/*           END OF FILE :  StdHead.h                                        */
/* --------------------------------------------------------------------------*/

