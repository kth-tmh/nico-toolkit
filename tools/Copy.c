/* ------------------------------------------------------------------------- */
/*                             Copies an object.                             */
/* ------------------------------------------------------------------------- */
/*                                                                           */
/*   Copyright (C) 1995, 1996, 1997, 1998 Nikko Strom                        */
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
#include "RTDNN.h"
#include "System.h"
#include "Command.h"

void 
usage(void) {
  printf("USAGE: Copy Object NewName Net\n");
  printf("       Option                                                      Default\n");
  printf("       -c         Clone - don't copy outflowing connections        (off)\n");
  printf("       -s         Split - split outflowing connections             (off)\n");
  printf("       -m Group   Move copy to 'Group'                             (network)\n");

  exit(0);
}  


int
main(int argc, char *argv[]) {   
  char *NetName, *NewName, *Parent = NULL, *Object;
  Net  *net;
  int obj, np, new, copy_style;
  int split_flag = 0, clone_flag = 0, move_flag = 0;
  char sw;

  if (argc == 1) usage();

  InitCommand(argv, argc);
  while (is_option()) {
    switch (sw = GetOption()) {
      case 'c':  
        clone_flag = 1;
        break;
      case 's':  
        split_flag = 1;
        break;
      case 'm':  
        Parent = GetName("Parent for copied object"); 
        move_flag = 1;
        break;
      default: ErrorExit(SYSTEM_ERR_EXIT, "Unknown switch %c", sw);
    }
  }

  Object = GetName("Name of the object"); 
  NewName = GetName("New name"); 
  NetName = GetName("Name of the Net"); 
  EndCommand();
  
  if (split_flag && clone_flag) {
    ErrorExit(SYSTEM_ERR_EXIT, "-c option and -s option are mutually incompatible.");
  }

  net = LoadNet(NetName);  

  if ((obj = GetId(net, Object, -1) )== -1) {
    ErrorExit(SYSTEM_ERR_EXIT, "Object (%s) nonexistent.", Object);
  }
 
  if (move_flag) {
    if ((np = GetId(net, Parent, -1)) == -1) {
      ErrorExit(SYSTEM_ERR_EXIT, "Parent (%s) non-existent.", Parent);
    }
  }
  else np = net->rootgroup->id;

  if (split_flag) copy_style = 1;
  else if (clone_flag) copy_style = 2;
  else copy_style = 0;

  new = Copy(net, obj, copy_style);

  Rename(net, new, NewName);

  Join(net, np, new);
  
  OrderUnits(net);
  AddLogEntry(net->logbook, argv, argc); 
  SaveNet(NetName ,net);
  FreeNet(net); 

  return SYSTEM_OK_EXIT;
}

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  Copy.c                                           */
/* --------------------------------------------------------------------------*/
