/* ------------------------------------------------------------------------- */
/*                  Change protection  of a group of units                   */
/* ------------------------------------------------------------------------- */
/*                                                                           */
/*   Copyright (C) 1995, 1996, 1997, 1998 Nikko Ström                        */
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
#include <unistd.h>
#include "RTDNN.h"
#include "System.h"
#include "Command.h"
#include "Math.h"


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
