/* ------------------------------------------------------------------------- */
/*                           Set the type of units                           */
/* ------------------------------------------------------------------------- */
/*                                                                           */
/*   Copyright (C) 1994, 1995, 1996, 1997, 1998 Nikko Ström                  */
/*                                                                           */
/*   All rights reserved.                                                    */
/*                                                                           */
/*   Developed by Nikko Ström at the Dept. of Speech, Music and Hearing,     */
/*   KTH (Royal Institute of Technology), Sweden.                            */
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
#include "RTDNN.h"
#include "System.h"
#include "Command.h"
#include "Math.h"

void 
usage(void) {
  printf("USAGE: SetType [option] Unit Net\n");
  printf("       Option                                                        Default\n");
  printf("       -o      Change to an output unit                              (off)\n");
  printf("       -n      Change to a non-output unit                           (off)\n");
  printf("       -i      Change to an input unit                               (off)\n");
  printf("       -O efun Change to an output unit with specified error function\n");
  printf("               efun = ('0/1' L2, L4, L10, 'abs', 'cross0/1'\n"); 
  printf("               'interact', 'cross' or 'noerr'                        (off)\n");
  printf("       -t      Change to a tanhyp unit                               (off)\n");
  printf("       -a      Change to an arctan unit                              (off)\n");
  printf("       -s      Change to a sigmoid unit                              (off)\n");
  printf("       -l      Change to a linear unit                               (off)\n");
  printf("       -m      Change to a multiplication unit                       (off)\n");
  printf("       -d      Change to a inverter (1/x) unit                       (off)\n");
  printf("       -x      Change to a exponential e(x) unit                     (off)\n");
  printf("       -e      Change to a environment unit                          (off)\n");
  printf("       -f      Change to a filefilter unit                           (off)\n");

  exit(0);
}  


int
main(int argc, char *argv[]) {   
  char sw, *NetName, *Unit;
  char *etype;
  Net  *net;
  int type = -1, backtype = -1; 

  if (argc == 1) usage();

  InitCommand(argv, argc);
  while (is_option()) {
    switch (sw = GetOption()) {
      case 'i':  
        type = INPUT;
        break;
      case 'l':  
        type = LINEAR;
        break;
      case 'a':  
        type = ARCTAN;
        break;
      case 't':  
        type = TANHYP;
        break;
      case 's':  
        type = SIGMOID;
        break;
      case 'm':  
        type = MULTIC;
        break;
      case 'd':  
        type = INVERTER;
        break;
      case 'x':  
        type = EXPONENTIAL;
        break;
      case 'e':  
        type = ENVIRONMENT;
        break;
      case 'f':  
        type = FILEFILTER;
        break;
      case 'n':  
        backtype = NONOUTPUT;
        break;
      case 'o':  
        backtype = OUTPUT;
        break;
      case 'O':  
        etype = GetName("Error type");
        if      (strcmp(etype, "L2") == 0)    backtype = OUTPUT;
        else if (strcmp(etype, "0/1") == 0)   backtype = ZERO_ONE;
        else if (strcmp(etype, "L4") == 0)    backtype = L4OUTPUT;
        else if (strcmp(etype, "L10") == 0)   backtype = L10OUTPUT;
        else if (strcmp(etype, "abs") == 0)   backtype = L0OUTPUT;
        else if (strcmp(etype, "cross") == 0) backtype = CROSSENTR;
        else if (strcmp(etype, "cross0/1") == 0) backtype = CROSS01;
        else if (strcmp(etype, "interact") == 0) backtype = INTERACT;
        else if (strcmp(etype, "noerr") == 0) backtype = NOERROR;
        else {
          ErrorExit(SYSTEM_ERR_EXIT, "Unknown error-type %s", etype);
        }
        break;
     default: ErrorExit(SYSTEM_ERR_EXIT, "Unknown switch %c", sw);
    }
  }
  Unit    = GetName("Name of the unit\n"); 
  NetName = GetName("Name of the net\n"); 
  EndCommand();

  net = LoadNet(NetName); 

  SetUnitType(net, GetId(net, Unit, UNSPEC), type, backtype);

  AddLogEntry(net->logbook, argv, argc); 
  SaveNet(NetName,net);
  FreeNet(net); 

  return SYSTEM_OK_EXIT;
}

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  SetType.c                                        */
/* --------------------------------------------------------------------------*/
