/* ------------------------------------------------------------------------- */
/*    Adds a comment to the loglist of a Recurrent Time Delay Neural Net     */
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


void 
usage(void) {
  printf("USAGE: Comment [option] Net\n");
  printf("       Option                                                        Default\n");  
  printf("       -w       Enter a \"warning!\" into the logbook                  \"comment\"\n");  

  exit(0);
}  


int
main(int argc, char *argv[] ) {   
  char s[1024];
  char *prompt;
  char *NetName, sw;
  Net  *net;

  if (argc == 1) usage();

  prompt = "user comment>"; 

  InitCommand(argv, argc);
  while (is_option()) {
    switch (sw = GetOption()) {
      case 'w':
      prompt = "user warning!>"; 
      break;
      default: ErrorExit(SYSTEM_ERR_EXIT, "Unknown switch %c", sw);
    }
  }
  NetName = GetName("Name of network definition file");
  EndCommand();

  net = LoadNet(NetName);
  printf("%s ", prompt); 
  scanf("%[^\n]", s);
  AddLogComment(net->logbook, prompt, s);

  SaveNet(NetName, net);
  FreeNet(net); 

  return SYSTEM_OK_EXIT;
}

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  Comment.c                                        */
/* --------------------------------------------------------------------------*/
