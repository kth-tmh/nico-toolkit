/* ------------------------------------------------------------------------- */
/*                       Logbook for binary files                            */
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



#ifndef LOGBOOK_LOADED
#define LOGBOOK_LOADED


typedef struct LogBook {
  char *book;
  int size;
} LogBook;




/* Create a log-book structure and initialize it with an empty string */
LogBook *CreateLogBook();


/* Add the command line into the log-book of a net */
void AddLogEntry(LogBook *, char *v[], int c);


/*  Use this function to insert a comment into the logbook          */
/*  promts are treated differently by 'ShowLogBook'.                */
/*  Here are the prompts recognised by them in order of proirity:   */
/*      0. COMMAND>           !Used exclusively by AddLogEntry      */
/*      1. user warning!>                                           */
/*           WARNING>                                               */
/*           COMPLETION>      !see: RemoveCompletionNote            */
/*      2. user comment>                                            */
/*           BREAFING>                                              */
/*      3.   DETAILS>                                               */
/*           STATISTICS>                                            */
void AddLogComment(LogBook *, char *prompt, char *format, ...);


/* Deletes the last line in the log-book if it has a "COMPLETION>" prompt */
void RemoveCompletionNote(LogBook *);


void FreeLogBook(LogBook *book);


LogBook *CopyLogBook(LogBook *old_book);


void SaveLogBook(FILE *, LogBook *);


LogBook *LoadLogBook(FILE *);


void ShowLogBook(FILE *, LogBook *, int info_level);



#endif /* ifndef LOGBOOK_LOADED */

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  Logbook.h                                        */
/* --------------------------------------------------------------------------*/
