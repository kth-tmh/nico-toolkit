/* ------------------------------------------------------------------------- */
/*                             Copies an object.                             */
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
