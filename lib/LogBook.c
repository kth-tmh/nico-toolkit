/* ------------------------------------------------------------------------- */
/*                       Logbook for binary files                            */
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

#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>
#include <stdarg.h>
#include "System.h"
#include "LogBook.h"


/* Create a Log-book structure and initialize it with an empty string */
LogBook *CreateLogBook() {
  LogBook *book;
  
  CALLOC(book, 1, LogBook)
  book->size = 1;

  CALLOC(book->book, 1, char)
  book->book[0] = '\0';

  return book;
}


/*  Use this function to insert a comment into the logbook.         */
/*  Promts are treated differently by the display functions.        */
/*  Here are the prompts recognised by them in order of proirity:   */
/*      0. COMMAND>           !Used exclusively by AddLogEntry      */
/*      1. user warning!>                                           */
/*           WARNING>                                               */
/*           COMPLETION>      !see: RemoveCompletionNote            */
/*      2. user comment>                                            */
/*           BREAFING>                                              */
/*      3.   DETAILS>                                               */
/*           STATISTICS>                                            */
void 
AddLogComment(LogBook *book, char *prompt, char *format, ...) {
  va_list args;
  char s[1024], *p = s;

  if (strncmp(prompt, "user", 4) != 0) { /* Don't indent user comments */
    strcpy(p, "  ");
    p += 2;
  }
  strcpy(p, prompt);
  p += strlen(prompt);
  strcpy(p, " ");
  p++;

  va_start(args, format);
  vsprintf(p, format, args);
  va_end(args);

  REALLOC(book->book, book->size + strlen(s) + 3, char)

  strcat(book->book, s); 
  strcat(book->book, "\n");
  book->size = strlen(book->book) + 1;
}



/*   When the status is changed, use this function to remove the old    */
/*   COMPLETION-comment in the logbook.                                 */
/*   Only the last comment is removed and only if it is the last line   */
/*   of the logbook and has prompt: "  COMPLETION>"                     */
void 
RemoveCompletionNote(LogBook *book) {
  char *log = book->book;
  char *log_end;

  log_end = log + book->size - 3;
  while (log_end != log && *log_end != '\n') log_end--;
  log_end++;

  /* Remove COMPLETION notes only */
  if (strncmp(log_end, "  COMPLETION>", 13) != 0) return;
  else {
    book->size = (int)log_end - (int)log + 1;
    *log_end = '\0';
  }
}



/* This function is intended to be used with v=arg and c=argc to enter a */
/* commandline into the logbook                                          */
void 
AddLogEntry(LogBook *book, char *v[], int c) {
  int i, l;
  char prompt[] = "COMMAND>";

  l = strlen(prompt) + 1;
  REALLOC(book->book, book->size + l + 1, char)
  strcpy(book->book + (book->size - 1), prompt); 
  book->size += l; 
  book->book[book->size - 2] = ' ';

  for(i = 0; i < c; i++) {
    l = strlen(v[i]) + 1;
    REALLOC(book->book, book->size + l + 1, char)
    strcpy(book->book + (book->size - 1), v[i]); 
    book->size += l; 
    book->book[book->size - 2] = ' ';
  }
  book->book[book->size - 2] = '\n'; 
  book->book[book->size - 1] = '\0';
}


LogBook *CopyLogBook(LogBook *old_book) {
  LogBook *book;

  CALLOC(book, 1, LogBook)
  STRDUP(book->book, old_book->book);
  book->size = old_book->size;

  return book;
}


void 
FreeLogBook(LogBook *book) {
  FREE(book->book);
  FREE(book);
}


void 
SaveLogBook(FILE *fp, LogBook *book) {
  WriteLong(book->size, fp);
  fwrite(book->book, sizeof(char) * book->size, 1, fp);
}


LogBook *LoadLogBook(FILE *fp) {
  LogBook *book;

  CALLOC(book, 1, LogBook)

  book->size = ReadLong(fp);
  CALLOC(book->book, book->size, char)
  fread(book->book, sizeof(char) * book->size, 1, fp);

  return book;
}


void 
ShowLogBook(FILE *stream, LogBook *book, int info_level) {
  char *pos = book->book, *end;
  char line[2048];
  while (*pos) {
    end = pos;
    while (!iscntrl(*end)) end++;
    strncpy(line, pos, (int)(end - pos));
    line[(int)(end - pos)] = '\0';
   
    if (info_level == 0 && strlen(line) > 76) {
       strcpy(&(line[76]), "...");
    }

    if (info_level >= 3 ||
 
        (info_level >= 2 && 
          (strstr(line, "BREAFING") || strstr(line, "user comment"))) ||

        (info_level >= 1 && 
          (strstr(line, "COMPLETION") || strstr(line, "WARNING") ||
           strstr(line, "user warning!"))) ||

        (info_level >= 0 && strstr(line, "COMMAND"))) {

      fprintf(stream, "%s\n", line);
    }

    while (iscntrl(*end) && *end != '\0') end++;
    pos = end;
  }
}



/* --------------------------------------------------------------------------*/
/*           END OF FILE :  Logbook.c                                        */
/* --------------------------------------------------------------------------*/
