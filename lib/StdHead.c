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

#ifndef STDHEAD_LOADED
#define STDHEAD_LOADED

#define LINT_ARGS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "StdHead.h"
#include "System.h"


/* ======= "stdhead" internal variables ======== */

static BOOL     bLocalsSeen;
static BOOL     bGlobalsSeen;

static PDESC    Local_DP;
static PDESC    Global_DP;


/* ======= "stdhead" internal routines ========= */

static char *
mystrpos(char *src, char *obj) { /* Finds substring obj in src */
  char *ix, *s;

  while (*src) {
    s = src;
    for (ix = obj; (*ix == *s); ix++, s++);
    if (!(*ix)) return src;
    src++;
  }

  return NULL;
} /* mystrpos() */


static void 
mymemcpy(char *dest, char *src, int count) { /* Handles overlap */
  if (dest == src) return;
  if (dest < src) {
    for ( ; (count > 0); count--) *(dest++) = *(src++);
  }
  else {
    dest += count;
    src  += count;
    for ( ; (count > 0); count--) *(--dest) = *(--src);
  }
} /* mymemcpy() */


static PDESC load_header(FILE *hFile) {
  char  *buf_ptr;
  PDESC dp;
  int   header_size, bytes_read;

  CALLOC(dp, 1,HDESC)                        /* Allocate a struct hdesc */

  header_size = HD_DEFAULTSIZE;              /* Assume a default size header */
  CALLOC(buf_ptr, header_size + 1, char)

  bzero(buf_ptr, header_size + 1);           /* Zero-init to prevent garbage */

  rewind(hFile);                             /* Just in case */

  /* Read what might be the header */
  bytes_read = fread(buf_ptr, 1, header_size, hFile); 

  *(buf_ptr+bytes_read) = '\00';      /* Append a 0-byte to limit strncmp() */

  if (strncmp (buf_ptr, "head=", 5) == 0) { /* A header length IS specified */
    
    if (1 != sscanf(buf_ptr+5, "%d", &header_size)) /* Is it a valid number? */
      return NULL;                     /* No -"head=foot" or something; quit */
    /* Yes -repeat the ceremony for the actual size */
    else REALLOC(buf_ptr, header_size + 1, char)

    bzero(buf_ptr, header_size + 1);   /* Zeroinit to prevent garbage */

    rewind(hFile);                     /* Start over again */

    /* Read what might be the header */
    bytes_read = fread(buf_ptr, 1, header_size, hFile); 

    *(buf_ptr+bytes_read) = '\00';     /* Append a 0-byte to limit strncmp() */
  }

  dp->size = header_size;              /* Fill in the hdesc struct */
  dp->bufptr = buf_ptr;

  return dp;
} /* load_header() */


static BOOL getvar(char  *header, char  *var, char  *value, int max_len) {
  char  *p, *q;
  int   k;
  char  var_buf[HD_MAXLINE + 1];

  strcpy (var_buf, var);
  strcat (var_buf, "=");
  p = header;

  do
    p = mystrpos(p, var_buf);
  while (p != header && p && *(p - 1) != '\n');

  if (!p) return FALSE;

  p = mystrpos(p, "=") + 1;
  for (q = value, k = 0; *p != '\r' && k < max_len; *q++ = *p++, k++);

  *q = '\0';

  return TRUE;
} /* getvar() */


static char *
eq_pos(PDESC dp) { /* Find end of variables */
  char  *eq;

  eq = dp->bufptr;
  do
    eq = mystrpos(eq, "=\r");
  while (eq != dp->bufptr && eq && *(eq - 1) != '\n');

  return eq;
} /* eq_pos() */




/* ======= "stdhead" public routines =========== */


HANDLE LoadHeader(FILE *hFile) {       /* Returns a HANDLE to a HDESC struct */
  PDESC hDesc;

  hDesc = load_header(hFile);

  return (HANDLE)hDesc;
} /* LoadHeader() */


HANDLE MakeHeader(int size) {
  char *buf_ptr;
  PDESC dp;
  int header_size;

  CALLOC(dp, 1, HDESC)                       /* Allocate a struct hdesc */

  if (size <= 0)
    header_size = HD_DEFAULTSIZE;            /* Assume a default size header */
  else
    header_size = size;

  CALLOC(buf_ptr, header_size + 1, char)

  bzero(buf_ptr, header_size+1);             /* To prevent garbage in file */

  dp->size = header_size;
  dp->bufptr = buf_ptr;

  if (size != HD_DEFAULTSIZE)
    sprintf (buf_ptr, "head=%d\r\n=\r\n", header_size);
  else
    strcpy (buf_ptr, "=\r\n");

  return (HANDLE)dp;
} /* MakeHeader() */


int 
SaveHeader(HANDLE hd, FILE *hFile) {
  PDESC dp;
  char *p;

  dp = (PDESC) hd;

  p = dp->bufptr;               /* Append ^D^Z to the header before saving */
  p += strlen(p) + 1;
  *p++ = '\04';
  *p = '\032';

  if (fwrite(dp->bufptr, sizeof(char), dp->size, hFile) != dp->size)
    return (HD_FILEERR);

  return HD_OK;
} /* SaveHeader() */


int 
FreeHeader(HANDLE hd) {
  PDESC dp;

  dp = (PDESC)hd;
  FREE(dp->bufptr);
  FREE(dp);

  return HD_OK;
} /* FreeHeader() */


int 
GetHeaderSpace(HANDLE hd) {
  PDESC dp;
  int tmp;

  dp = (PDESC)hd;
  tmp = dp->size - strlen (dp->bufptr) - 3;

  return tmp;
} /* GetHeaderSpace() */


int 
GetHeaderStr(HANDLE hd, char *var, char *value, int max_len) {
/* Returns 0,1,2,4 or < 0 for errors */
/*
  int           hd;
  char          *var;       // pointer to string naming variable
  char          *value;     // pointer to receiving string buffer
  int           max_len;    // max string length to be returned, excluding \0
*/
  PDESC dp;

  *value = '\00';                              /* Clear destination string */
  dp = (PDESC)hd;                              /* Cast handle to pointer */

  if (getvar(dp->bufptr, var, value, max_len)) /* Seek variable in user file */
    return HD_FOUND;

  if (bLocalsSeen)                 /* If not found, try local defaults */
    if (getvar(Local_DP->bufptr, var, value, max_len))
      return HD_LOCALFOUND;

  if (bGlobalsSeen)               /* If still not found, try global defaults */
    if (getvar(Global_DP->bufptr, var, value, max_len))
      return HD_GLOBALFOUND;

  return HD_OK;                   /* Nothing found anywhere */
} /* GetHeaderStr() */


char *
GetHeaderText(HANDLE hd) {
  PDESC dp;
  char  *eq;

  dp = (PDESC)hd;               /* Cast handle to pointer */
  eq = eq_pos(dp);

  if (!eq) return (NULL);       /* Abort if missing */

  return eq + 3;
} /* GetHeaderText() */


/* Retrieves long named by var */
int 
GetHeaderLong(HANDLE hd, char *var, LONG *pLong) {
  int   err;
  LONG  tmp;
  char  buf[41];

  err = GetHeaderStr(hd, var, buf, 40);
  if (err <= 0) return err;

  if (sscanf (buf, "%ld", &tmp) == 1) {
    *pLong = tmp;

    return err;
  }

  return HD_NOTINT;
} /* GetHeaderLong() */


/* Retrieves double named by var */
int 
GetHeaderDouble(HANDLE hd, char *var, DOUBLE *pDouble) {
  int           err;
  double        tmp;
  char          buf[41];

  err = GetHeaderStr(hd, var, buf, 40);
  if (err <= 0)
    return err;

  if (sscanf (buf, "%lf", &tmp) == 1) {
    *pDouble = tmp;

    return err;
  }

  return HD_NOTDOUBLE;
} /* GetHeaderDouble() */


/* Installs/replaces/removes a header entry */
int 
SetHeaderStr(HANDLE hd, char *var, char *value) {
  char  buf[HD_MAXLINE + 1];             /* Line buffer */
  int   len, old_line_len, new_line_len; /* Line lengths */
  char  *eq;                             /* Position of last equals sign */
  char  *ins;                            /* Insertion point for new variable */
  char  *p, *r;
  PDESC dp;
  int   ret_val;

  ret_val = HD_OK;                       /* Assume no errors */
  dp = (PDESC)hd;                        /* Cast handle to pointer */

  if (!var || !(*var)) return HD_BADVAR;           /* Bad input */

  /* Compute old and new line lengths */
  if (getvar(dp->bufptr, var, buf, HD_MAXLINE))    /* Does variable exist? */
    old_line_len = strlen(var) + strlen(buf) + 3;  /* Add 3 for "=\r\n" */
  else
    old_line_len = 0;

  if (value && *value)
    new_line_len = strlen(var) + strlen(value) + 3;
  else
    new_line_len = 0;

  eq = eq_pos(dp);                     /* Find end of variables */
  if (!eq)
    return HD_BADHEADER;               /* Abort if missing */

                                       /* Check that there will be room: */
                                       /* Add 3 for "\00\04\032" */
  if (new_line_len - old_line_len + strlen (dp->bufptr) + 3 > dp->size)
    return HD_OVERFLOW;

  *eq = '\0';                          /* beyond "\n=" is off-bounds for now */

  strcpy (buf, var);
  strcat (buf, "=");
  r = dp->bufptr;
  do
    r = mystrpos(r, buf);
  while (r != dp->bufptr && r && *(r - 1) != '\n');

  *eq = '=';                                   /* Re-enable rest of header   */
  ins = eq;                                    /* ins = pos for new variable */

  if (r) {                                     /* If extant, delete old entry*/
    p = mystrpos(r, "\n") + 1;                 /* p -> 1st char on next line */
    strcpy (r, p);                             /* Overwrite it */
    ins -= (p - r);                            /* Adjust pointer to last '=' */
    ret_val = HD_OK;
  }


  if (value && *value) {                       /* If there's a new value: */
    strcat (buf, value);                       /* concat the value,       */
    strcat (buf, "\r\n");                      /* and CRLF.               */
    len = strlen (buf);
    if (r)                                     /* If variable already exists,*/
      ins = r;                                 /*   leave it in place */
    mymemcpy(ins + len, ins, strlen (ins) + 3);/* Move up rest of the header */
    mymemcpy(ins, buf, len);                   /* Insert buf */
    ret_val = HD_OK;
  }

  return ret_val;
} /* SetHeaderStr() */


int 
SetHeaderText(HANDLE hd, char *szText) {
  PDESC dp;
  char  *eq;
  int   max, len;

  dp = (PDESC)hd;
  eq = eq_pos(dp);
  if (!eq)
    return HD_BADHEADER;

  eq += 3;
  len = strlen (szText);
  max = strlen (eq) + GetHeaderSpace(hd);
  if (len > max)
    len = max;

  mymemcpy(eq, szText, len);
  *(eq + len) = '\00';

  return len;
} /* SetHeaderText() */

#endif

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  StdHead.c                                        */
/* --------------------------------------------------------------------------*/


