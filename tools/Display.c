/* ------------------------------------------------------------------------- */
/*          Display objects of the Recurrent Time Delay Neural Net           */
/* ------------------------------------------------------------------------- */
/*                                                                           */
/*   Copyright (C) 1994, 1995, 1996, 1997, 1998 Nikko Strom                  */
/*                                                                           */
/*   All rights reserved.                                                    */
/*                                                                           */
/*   Developed by Nikko Strom at the Dept. of Speech, Music and Hearing,     */
/*   KTH (Royal Institute of Technology), Sweden.                            */
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
#include "Display.h"
#include "System.h"
#include "Command.h"

void 
usage(void) {
  printf("USAGE: Display [options] Name\n");
  printf("       Option                                                        Default\n");
  printf("       -o name      Display the object 'name'                        (off)\n");
  printf("       -g           Display groups                                   (off)\n");
  printf("       -s           Display streams                                  (off)\n");
  printf("       -c           Display connection stat's                        (off)\n");
  printf("       -C from to   As -c but selected con's                         (off)\n");
  printf("       -u           Display unit stat's                              (off)\n");
  printf("       -l           Display Log                                      (off)\n");
  printf("       -X level     Extended display level                           (0)\n");

  exit(0);
}  


int
main(int argc, char *argv[] ) {   
  int  i, obj;
  int  Lflag = 0, Gflag = 0, CCflag = 0, Cflag = 0;
  int Iflag = 0, Sflag = 0, Oflag = 0, Uflag = 0;
  int info_level = 0;
  int  from, to; 
  char sw, *Name, *Object = NULL;
  char *From = NULL, *To = NULL;
  Net  *net;

  if (argc == 1) usage();

  InitCommand(argv,argc);
  while (is_option()) {
    switch (sw = GetOption()) {
      case 'l':  
        Lflag = 1;
        break;
      case 'c':  
        Cflag = 1;
        break;
      case 'u':  
        Uflag = 1;
        break;
      case 'g':  
        Gflag = 1;
        break;
      case 's':  
        Sflag = 1;
        break;
      case 'o':  
        Oflag = 1;
        Object = GetName("Name of object"); 
        break;
      case 'X':  
        info_level = GetInt("Level of information printed", 1, 3); 
        break;
      case 'C':  
        CCflag = 1;
        From = GetName("Name of 'from-object'"); 
        To   = GetName("Name of 'to-object'"); 
     break;
      default:
        ErrorExit(SYSTEM_ERR_EXIT, "Unknown switch %c.", sw);
    }
  }
  Name = GetName("File Name of Net"); 
  EndCommand();
  net = LoadNet(Name); 

  if (!Lflag && !Gflag && !Cflag && !CCflag && !Iflag &&
      !Sflag && !Oflag && !Uflag) ShowNet(*net, info_level);

  if (Oflag) {
    printf("\n");
    if ((obj = GetId(net, Object, -1)) == -1) {
      printf("Object (%s) not found.\n", Object);
      exit(0);
    }
    ShowObject(*net, obj, info_level);
  }

  if (Gflag) {
    printf("\n");
    for (i=0; i<net->NumGroups; i++) {
      if (strcmp(net->GrTab[i]->Name,"") != 0) 
        ShowGroup(*net, net->GrTab[i]->id, info_level);
    }
  }

  if (Sflag) {
    printf("\n");
    for (i = 0; i < net->NumStreams; i++) 
      ShowStream(*net, net->StrTab[i]->id, info_level);
  }

  if (CCflag) {
    printf("\n");
    if ((from = GetId(net, From, -1)) == -1) {
      ErrorExit(SYSTEM_ERR_EXIT, "'from-object' (%s) not found.", From);
    }
    if ((to = GetId(net, To, -1)) == -1) {
      ErrorExit(SYSTEM_ERR_EXIT, "'to-object' (%s) not found.", To);
    }
    ShowConnection(*net, from, to, info_level);
  }

  if (Cflag) {
    printf("\n");
    ShowConnections(*net, info_level);
  }

  if (Uflag) {
    printf("\n");
    ShowUnits(*net, info_level);
  }

  if (Lflag) {
    printf("\n");
    ShowLogBook(stdout, net->logbook, info_level);
  }

  FreeNet(net); 

  return SYSTEM_OK_EXIT;
}

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  Display.c                                        */
/* --------------------------------------------------------------------------*/
