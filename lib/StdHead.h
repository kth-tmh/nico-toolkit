/* ------------------------------------------------------------------------- */
/*   The original Stdhead-software was written by Sten Ternstrom and is      */
/*   public domain.                                                          */
/*   This version of Stdhead is modified to be incorporated in               */
/*   the NICO toolkit and is NOT public domain.                              */
/* ------------------------------------------------------------------------- */
/*                                                                           */
/*   Copyright (C) 1995, 1996, 1997, 1998 Nikko Ström                        */
/*                                                                           */
/*   All rights reserved.                                                    */
/*                                                                           */
/*   Modification by Nikko Ström at the Dept. of Speech, Music and Hearing,  */
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

