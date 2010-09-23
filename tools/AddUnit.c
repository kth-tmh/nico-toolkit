/* ------------------------------------------------------------------------- */
/*              Adds units to a Recurrent Time Delay Neural Net              */
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
#include "RTDNN.h"
#include "System.h"
#include "Command.h"
#include "nMath.h"

void 
usage(void) {
  printf("USAGE: AddUnit [options] Name Net\n");
  printf("       Option                                                      Default\n");
  printf("       -u number Add multiple unnamed units. 'Name' is the parent  (off)\n");
  printf("       -S File   Add multiple named units, 'Name' is the parent    (off)\n");
  printf("       -i        Add input unit                                    (hidden)\n");
  printf("       -o        Add output unit                                   (hidden)\n");
  printf("       -s        Add sigmoid unit                                  (tanhyp)\n");
  printf("       -t        Add arctan unit                                   (tanhyp)\n");
  printf("       -l        Add linear unit                                   (tanhyp)\n");
  printf("       -f        Add filefilter unit                               (tanhyp)\n");
  printf("       -e        Add environment unit                              (tanhyp)\n");
  printf("       -r float  Bound on random init of bias                      (0.10)\n");

  exit(0);
}  


int
main(int argc, char *argv[]) {   
  Net *net;
  Unit *u;
  char **LabList;
  char *ScriptFile = NULL, *NetName, *Name;
  char sw;
  int i, n, p;
  int multiple = 0, script = 0;
  int type = -1; 
  int backtype = NONOUTPUT; 
  float bound = 0.10;

  if (argc == 1) usage();

  InitCommand(argv, argc);
  while (is_option()) {
    switch (sw = GetOption()) {
      case 'u':  
        multiple = 1;
        n = GetInt("Number of units", 0, 10000);
        break;
      case 'S':  
        script = 1;
        ScriptFile = GetName("Name of script file holding unit names");
        break;
      case 'i':  
        type = INPUT;
        break;
      case 's':  
        type = SIGMOID;
        break;
      case 't':  
        type = ARCTAN;
        break;
      case 'l':  
        type = LINEAR;
        break;
      case 'f':  
        type = FILEFILTER;
        bound = 0.0;
        break;
      case 'e':  
        type = ENVIRONMENT;
        bound = 0.0;
        break;
      case 'o':  
        backtype = OUTPUT;
        break;
      case 'r':  
        bound = GetFloat("Bound for random bias", 0.0, 10.0);
        break;
      default: ErrorExit(SYSTEM_ERR_EXIT, "Unknown switch %c.", sw);
    }
  }
  if (multiple) Name = GetName("Name of the unit");
  else  Name = GetName("Name of the parent-group");
  NetName = GetName("Name of the Net"); 
  EndCommand();

  /* Default for is TANHYP */
  if (type == -1) type = TANHYP;

  Randomize();

  net = LoadNet(NetName);

  if (script) {
    p = GetId(net, Name, UNSPEC);
    if (GetIdType(net, p) != GROUP) {
      ErrorExit(SYSTEM_ERR_EXIT, "Parent to multiple units must be a group.");
    }
    LabList = LoadObjectList(ScriptFile, &n);
    for (i = 0; i < n; i++) {
      u = AddNamedUnit(net, LabList[i], type, backtype, 0.0);
      Join(net, p, u->id);
 
      if (bound != 0.0) {
        if (type != INPUT && type != BIAS) {
          AddConnection(net, u->id, net->biasunit->id, 0,
            (RectRand() - 0.5) * 2 * bound);
        }
      }
    }
  }
  else if (multiple) {
    p = GetId(net, Name, UNSPEC);
    if (GetIdType(net, p) != GROUP) {
      ErrorExit(SYSTEM_ERR_EXIT, "Parent to multiple units must be a group.");
    }
    AddUnits(net, p, type, backtype, n, bound);
  }
  else {
    u = AddNamedUnit(net, Name, type, backtype, bound);
    Join(net, net->rootgroup->id, u->id);
  }

  OrderUnits(net);
  SortConnections(net);

  AddLogEntry(net->logbook, argv, argc); 
  SaveNet(NetName, net);
  FreeNet(net); 

  return SYSTEM_OK_EXIT;
}

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  AddUnit.c                                        */
/* --------------------------------------------------------------------------*/
