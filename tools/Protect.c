/* ------------------------------------------------------------------------- */
/*                  Change protection  of a group of units                   */
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
#include <unistd.h>
#include "RTDNN.h"
#include "System.h"
#include "Command.h"
#include "nMath.h"


void 
usage(void) {
  printf("USAGE: Protect [options] GroupName Net\n");
  printf("       Option                                                        Default\n"); 
  printf("       -P        Protection against all connections                  (off)\n");
  printf("       -R        Change to reception group                           (off)\n");
  printf("       -S        Change to signal group                              (off)\n");
  printf("       -0        Change to no protection (public group)              (off)\n");

  exit(0);
}  


int
main(int argc, char *argv[]) {   
  int n; 
  char sw, *NetName, *GroupName;
  Net  *net;
  int Protection = PUBLIC; 

  if (argc == 1) usage();

  InitCommand(argv, argc);
  while (is_option()) {
    switch (sw = GetOption()) {
      case 'P':  
        Protection = HIDDEN;
        break;
      case 'R':  
        Protection = RECEPT;
        break;
      case 'S':  
        Protection = SIGNAL;
        break;
      case '0':  
        Protection = PUBLIC;
        break;
      default: ErrorExit(SYSTEM_ERR_EXIT, "Unknown switch %c", sw);
    }
  }
  GroupName = GetName("Name of the group"); 
  NetName = GetName("Name of the Net"); 
  EndCommand();

  net = LoadNet(NetName); 
  
  if ((n = GetId(net, GroupName, GROUP)) == -1) {
    ErrorExit(SYSTEM_ERR_EXIT, "Group %c not found.", GroupName);
  }
  ProtectGroup(net, n, Protection);

  AddLogEntry(net->logbook, argv, argc); 
  SaveNet(NetName, net);
  FreeNet(net); 

  return SYSTEM_OK_EXIT;
}

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  ProtectGroup.c                                   */
/* --------------------------------------------------------------------------*/
