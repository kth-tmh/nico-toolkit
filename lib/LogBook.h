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
