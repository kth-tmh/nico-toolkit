/* ------------------------------------------------------------------------- */
/*                 Routines related to the operating system                  */
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
#include <unistd.h>
#include "RTDNN.h"
#include "System.h"
#include "Command.h"

static char **ArgList;
static int  NumArgs;
static int  ArgP;

void 
InitCommand(char *argv[], int argc) {
  int i;

  CALLOC(ArgList, argc, char *)
  for (i = 0; i < argc; i++) ArgList[i] = argv[i];
  NumArgs = argc;
  ArgP = 1;
}
       

/* Return 1 if the next argument is an option, i.e., starts in one of
 * the delimiter cahracters of the argument. */
int 
is_option() {
  if (ArgP == NumArgs) return 0;
  return (ArgList[ArgP][0] == '-');
}
 

char 
GetOption() {
  char option, *argument;
  
  if (!is_option()) ErrorExit(SYSTEM_ERR_EXIT, "Option expected.\n");

  /* Assume that an arg. follows if there is no space after the first char */
  if (strlen(ArgList[ArgP]) > 2) {
    CALLOC(argument, strlen(ArgList[ArgP]), char)
    strcpy(argument, ArgList[ArgP] + 2);
    option = ArgList[ArgP][1];
    ArgList[ArgP] = argument;
    return option; 
  }
  /* This is the clean case with a space after the first char */
  else return ArgList[ArgP++][1];
}


/* Returns a pointer to the string after the '-' character.
 * Note that this does not advance the argument counter. */
char *
CheckLongOption() {
  if (!is_option()) ErrorExit(SYSTEM_ERR_EXIT, "Option expected.\n");
  return ArgList[ArgP] + 1;
}


/* Returns a pointer to the string after the '-' character.
 * Increments the argument counter. */
char *
GetLongOption() {
  if (!is_option()) ErrorExit(SYSTEM_ERR_EXIT, "Option expected.\n");
  return ArgList[ArgP++] + 1;
}


char *
GetName(char *name) {
  char *r;
 
  if (ArgP == NumArgs) ErrorExit(SYSTEM_ERR_EXIT, "%s expected.\n", name);
  STRDUP(r, ArgList[ArgP]);
  ArgP++;

  return r;
}     


int 
GetInt(char *who, int min, int max) {
  int r;
 
  if (ArgP == NumArgs) {
    ErrorExit(SYSTEM_ERR_EXIT, "%s expects an integer in the range [%i,%i].\n", 
       who, min, max);
  }
  r = atoi(ArgList[ArgP++]);
  if (r > max || r < min) {
    ErrorExit(SYSTEM_ERR_EXIT, "%s expects an integer in the range [%i,%i].\n", 
       who, min ,max);
  }
  return r;
}


float 
GetFloat(char *who, float min, float max) {
  float r;
 
  if (ArgP == NumArgs) {
    ErrorExit(SYSTEM_ERR_EXIT, "%s expects a float in the range [%f,%f].\n", who, min, max);
  }
  r = atof(ArgList[ArgP++]);
  if (r > max || r < min) {
    ErrorExit(SYSTEM_ERR_EXIT, "%s expects a float in the range [%f,%f].\n", who, min, max);
  }
  return r;
}

  
void 
SkipArg() {
  if (ArgP == NumArgs) {
    ErrorExit(SYSTEM_ERR_EXIT, " Too few arguments.\n");
  }
  ArgP++;
}



static void 
AcceptObj(FILE *fp, char *obj) {
  fscanf(fp,"%s", obj); 
}    


char **LoadObjectList(char *fn, int *N) {     
  int allocated = 128;  
  char **Objs, obj[1024];
  FILE *fp;

  CALLOC(Objs, allocated, char *)
  *N = 0;               

  FOPEN(fp, fn, "r")

  while (!feof(fp)) {  
    if (*N == allocated) {
      allocated *= 2;
      REALLOC(Objs, allocated, char *)
    }

    AcceptObj(fp, obj);
    STRDUP(Objs[*N], obj);
    (*N)++;

    fscanf(fp," \n"); 
  }  

  FCLOSE(fp);

  REALLOC(Objs, *N, char *)

  return(Objs);
}   


/* Load additional arguments from the script file 'fn' */
/* In the script file, options does not need the hyphen if 
   they are specified in the string 'options'. */
/* The options are inserted in the args-list at the current position. */
void 
LoadArgFile(char *fn, char *options) {
  FILE *fp;
  char **list;
  int num_allocated_args = 256;
  char line[1024], *a, tmp[4];
  int i, line_num=0, num_new_args=0;
  

  CALLOC(list, 256, char *)
  FOPEN(fp, fn, "r")

  while (fgets(line, 1024, fp)) {
    line_num++;
    a = strtok(line, " \t\n");
    if (!a) continue;

    /* is the first arg on the line a legal option ? */
    if (strpbrk(a, options) && strlen(a) == 1) { 
      strcpy(tmp, "-"); strcat(tmp, a); 
      STRDUP(list[num_new_args], tmp);
      num_new_args++;
    }
    else STRDUP(list[num_new_args], a);
    num_new_args++;
    /* read the rest of the args on the line */
    while ((a = strtok(NULL, " \t\n")) != NULL) {
      STRDUP(list[num_new_args], a); 
      num_new_args++;
      if (num_new_args > num_allocated_args - 2) {
        num_allocated_args *= 2;
        REALLOC(list, num_allocated_args, char *)
      }
    }
  }
  FCLOSE(fp)

  /* reallocate and insert the new args in ArgList */
  NumArgs += num_new_args;
  REALLOC(ArgList, NumArgs, char *)
  for (i = NumArgs - 1; i >= ArgP; i--) ArgList[i] = ArgList[i - num_new_args];
  for (i = 0; i < num_new_args; i++) ArgList[i + ArgP] = list[i];

  FREE(list);
}


int 
IsEndCommand() {
  return (ArgP == NumArgs);
}  


void 
EndCommand() {
  FREE(ArgList);
  if (!IsEndCommand()) ErrorExit(SYSTEM_ERR_EXIT, "Too many arguments.\n");
}  


/* --------------------------------------------------------------------------*/
/*           END OF FILE :  Command.c                                        */
/* --------------------------------------------------------------------------*/
