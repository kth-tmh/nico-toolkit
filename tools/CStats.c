/* ------------------------------------------------------------------------- */
/*     Show weight statistics of the Recurrent Time Delay Neural Net         */
/* ------------------------------------------------------------------------- */
/*                                                                           */
/*   Copyright (C) 2000 Nikko Ström                                          */
/*                                                                           */
/*   All rights reserved.                                                    */
/*                                                                           */
/*   http://www.speech.kth.se/NICO                                           */
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
#include <math.h>
#include "RTDNN.h"
#include "Display.h"
#include "System.h"
#include "Command.h"

void 
usage(void) {
  printf("USAGE: CStats [options] Name\n");
  printf("       Option                                                     Default\n");
  printf("       -C from to   As -c but selected con's                      (off)\n");
  printf("       -n bins      Number of histogram bins                      (10)\n");
  printf("       -i min max   Specify interval                              (auto)\n");
  printf("       -f           Show connection fraction (%%)                 (count)");

  exit(0);
}  


int
main(int argc, char *argv[] ) {   
  int  i, num_bins = 10, num_selected;
  float span = 0.0, max = 0.0, min = 0.0;
  float d_min, d_max;
  int *Bins;
  int CCflag = 0, Countflag = 1, Spanflag = 0;
  int  from, to; 
  char *From = NULL, *To = NULL;
  char sw;
  char *Name;
  Connection *c;
  Net  *net;

  if (argc == 1) usage();

  InitCommand(argv,argc);
  while (is_option()) {
    switch (sw = GetOption()) {
      case 'n':  
        num_bins = GetInt("Number of bins", 1, 1000);
        break;
      case 'C':  
        CCflag = 1;
        From = GetName("Name of 'from-object'"); 
        To   = GetName("Name of 'to-object'"); 
        break;
      case 'f':  
        Countflag = 0;
        break;
      case 'i':  
        Spanflag = 1;
	min = GetFloat("min (-i option)", -1000.0, 1000.0);
	max = GetFloat("max (-i option)", -1000.0, 1000.0);
	span = max - min;
	if (span <= 0.0) {
	  ErrorExit(SYSTEM_ERR_EXIT, "max must be > min (-i option).", sw);
	}
     break;
      default:
        ErrorExit(SYSTEM_ERR_EXIT, "Unknown switch %c.", sw);
    }
  }
  Name = GetName("File Name of Net"); 
  EndCommand();
  net = LoadNet(Name); 

  if (CCflag) {
    SetAllConnectionMarks(net, 0);
    printf("\n");
    if ((from = GetId(net, From, -1)) == -1) {
      ErrorExit(SYSTEM_ERR_EXIT, "'from-object' (%s) not found.", From);
    }
    if ((to = GetId(net, To, -1)) == -1) {
      ErrorExit(SYSTEM_ERR_EXIT, "'to-object' (%s) not found.", To);
    }

    MarkConnections(net, from, to, 1);
  }
  else {
    SetAllConnectionMarks(net, 1);
  }

  for (d_max = -1000.0, d_min = 1000.0, i = 0; i < net->NumConnections; i++) {
    c = net->CTab[i];
    if (c->mark) {
      if (c->w > d_max) d_max = c->w;
      if (c->w < d_min) d_min = c->w;
    }
  }

  if (!Spanflag) {
    min = d_min;
    max = d_max;
    span = max - min;
  }

  CALLOC(Bins, num_bins, int);
  for (num_selected = 0, i = 0; i < net->NumConnections; i++) {
    c = net->CTab[i];
    if (c->mark) {
      int bin;
      num_selected++;
      if (c->w == max) bin = num_bins - 1;
      else bin = (int)floor(((c->w - min) / span) * num_bins);
      if (bin >= 0 && bin < num_bins) Bins[bin]++;
    }
  }

  if (num_selected == 0) ErrorExit(SYSTEM_ERR_EXIT, "No connections selected.");

  printf("NICO -- connection weight histogram for %d connections\n\n", num_selected);
  printf("Min: %.6f Max %.6f\n\n", d_min, d_max);
  for (i = 0; i < num_bins; i++) {
    if (Countflag) {
      printf("%+2.3f < w < %+2.3f: %7d\n", 
	     min + span / (float)num_bins * (float)i,
	     min + span / (float)num_bins * (float)(i + 1),
	     Bins[i]);   
    }
    else {
      printf("%+2.3f < w < %+2.3f: %.6f\n", 
	     min + span / (float)num_bins * (float)i,
	     min + span / (float)num_bins * (float)(i + 1),	     
	     (float)Bins[i] / (float)num_selected);   
    }
  }
  printf("--\n");

  FREE(Bins);
  FreeNet(net);

  return SYSTEM_OK_EXIT;
}

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  CStats.c                                         */
/* --------------------------------------------------------------------------*/
