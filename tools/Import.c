/* ------------------------------------------------------------------------- */
/*               Import an object from one network to another.               */
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
  printf("USAGE: Import Object SourceNet Net\n");
  printf("       Option                                                      Default\n");
  printf("       -m Group   Move 'Object' to 'Group' in 'Net'                (network)\n");

  exit(0);
}  


int
main(int argc, char *argv[]) {   
  char *SourceNet, *TargetNet, *Object, *Parent = NULL;
  Net  *net1, *net2;
  int obj, new, np, move_flag = 0;
  char sw;

  if (argc == 1) usage();

  InitCommand(argv, argc);
  while (is_option()) {
    switch (sw = GetOption()) {
      case 'm':  
        Parent = GetName("Parent for copied object"); 
        move_flag = 1;
        break;
      default: ErrorExit(SYSTEM_ERR_EXIT, "Unknown switch %c", sw);
    }
  }
  Object    = GetName("Name of the object"); 
  SourceNet = GetName("Name of the source net"); 
  TargetNet = GetName("Name of the net"); 
  EndCommand();

  net1 = LoadNet(SourceNet); 
  net2 = LoadNet(TargetNet);
 
  if ((obj = GetId(net1, Object, UNSPEC)) == -1)
    ErrorExit(SYSTEM_ERR_EXIT, "No Object %s in %s", Object, SourceNet);
 
  new = Import(net1, obj, net2);

  if (move_flag) {
    if ((np = GetId(net2, Parent, UNSPEC)) == -1) {
      ErrorExit(SYSTEM_ERR_EXIT, "Parent (%s) non-existent.", Parent);
    }
  }
  else np = net2->rootgroup->id;

  Join(net2, np, new);

  OrderUnits(net2);
  SortConnections(net2);

  AddLogEntry(net2->logbook, argv, argc); 

  SaveNet(TargetNet, net2);
  FreeNet(net1); 
  FreeNet(net2); 

  return SYSTEM_OK_EXIT;
}

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  Import.c                                         */
/* --------------------------------------------------------------------------*/
