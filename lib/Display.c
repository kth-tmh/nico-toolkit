/* ------------------------------------------------------------------------- */
/*                       Displays of RTDNN parameters                        */
/* ------------------------------------------------------------------------- */
/*                                                                           */
/*   Copyright (C) 1994, 1995, 1996, 1997, 1998 Nikko Ström                  */
/*                                                                           */
/*   All rights reserved.                                                    */
/*                                                                           */
/*   Developed by Nikko Ström at the Dept. of Speech, Music and Hearing,     */
/*   KTH (Royal Institute of Technology)                                     */
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
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "System.h" 
#include "RTDNN.h" 
#include "Display.h" 


void 
ShowNet(Net net, int level) {
  printf("**************** NICO(Ver %4.2f) Display ******************\n", net.Version);
  printf("             Name          : %s\n",    net.Name);
  printf("             Streams       : %li\n",   net.NumStreams);
  printf("             Groups        : %li\n",   net.NumGroups);
  ShowUnits(net, level);
  printf("\n");
  ShowConnections(net, level);
  printf("***********************************************************\n");
}  


void 
ShowObject(Net net, int obj, int level) {
  switch(GetIdType(&net, obj)) {
    case GROUP:
      ShowGroup(net, obj, level);
    break;
    case UNIT:
      ShowUnit(net, obj, level);
    break;
    case STREAM:
      ShowStream(net, obj, level);
    break;
    default: ErrorExit(SYSTEM_ERR_EXIT, "Display of this object is not supported.\n");
  }  
}

void 
ShowGroup(Net net, int group, int level) { 
  int i, type, unn;
  int sub = 0, unit = 0, substr = 0;                           
  Group *g, *s;
  Stream *str;
  Unit *u;

  g = GETPTR(&net, group, Group);
  if (strcmp(g->Name,"") != 0) printf("Group: %s\n", g->Name);
  else printf("Unnamed Group\n");

  printf("Type: ");
  switch(g->type) {
    case PUBLIC:
    printf("public\n");
    break;
    case HIDDEN:
    printf("hidden\n");
    break;
    case RECEPT:
    printf("recept\n");
    break;
    case SIGNAL:
    printf("signal\n");
    break;
  }


  for (i = 0; i < g->NumMem; i++) {
    type = GetIdType(&net, g->MemTab[i]);
    switch(type) {
      case STREAM   : substr++;  break;
      case GROUP    : sub++; break;
      case UNIT     : unit++; break;
    }
  }  
  if (g->NumMem == 0) printf("  Empty\n");

  if (sub != 0) {
    unn = 0; 
    printf("  Sub Groups:\n    ");
    for (i = 0; i < g->NumMem; i++) {
      if (GetIdType(&net, g->MemTab[i]) == GROUP) {
        s = GETPTR(&net, g->MemTab[i], Group);
        if (strcmp(s->Name, "") == 0) unn++; 
        else if (level > 0) printf("%s ", s->Name);
      }
    }
    if (level == 0) printf("%3i Named ", sub - unn);
    if (unn != 0) printf("%3i Unnamed\n", unn);
    else printf("\n");
  } 

  if (unit != 0) {
    unn = 0; 
    printf("  Units:\n    ");
    for (i = 0; i < g->NumMem; i++) {
      if (GetIdType(&net, g->MemTab[i]) == UNIT) {
        u = GETPTR(&net, g->MemTab[i], Unit);
        if (strcmp(u->Name, "") == 0) unn++; 
        else if (level > 0) printf("%s ", u->Name);
      }
    }
    if (level == 0) printf("%3i Named ", unit - unn);
    if (unn !=0) printf("%3i Unnamed\n", unn);
    else printf("\n");
  }

  if (substr != 0) {
    if (level > 0) {
      printf("  Streams:\n    ");
      for (i = 0; i < g->NumMem; i++) {
        if (GetIdType(&net, g->MemTab[i]) == STREAM) {
          str = GETPTR(&net, g->MemTab[i], Stream);
          printf("%s ", str->Name);
        }
      }
      printf("\n");
    }
    else {
      printf("  %i Streams\n", substr);
    }
  }
}


void 
ShowUnit(Net net, int unit, int level) {
  Unit *u;

  u = GETPTR(&net, unit, Unit);
  if (strcmp(u->Name,"") != 0)printf("Unit           : %s\n", u->Name);
  else printf("Unnamed Unit\n");
  printf("Type : ");

  switch (u->type) {
    case BIAS   : printf("bias, "); break;
    case LINEAR : printf("linear, "); break;
    case TANHYP : printf("tanhyp, "); break;
    case SIGMOID: printf("sigmoid, "); break;
    case ARCTAN : printf("arctan, "); break;
    case INPUT  : printf("input, "); break;
    case MULTIC : printf("multic, "); break;
    case INVERTER    : printf("inverter, "); break;
    case EXPONENTIAL : printf("exponential, "); break;
    case ENVIRONMENT : printf("environment, "); break;
    case FILEFILTER  : printf("file-filter, "); break;
  }

  switch (u->backtype) {
    case OUTPUT    : printf("output (L2)\n"); break;
    case ZERO_ONE  : printf("output (0/1)\n"); break;
    case CROSSENTR : printf("output (Cross entropy)\n"); break;
    case CROSS01   : printf("output (0/1 - Cross entropy)\n"); break;
    case L4OUTPUT  : printf("output (L4)\n"); break;
    case L10OUTPUT : printf("output (L10)\n"); break;
    case L0OUTPUT  : printf("output (Absolute)\n"); break;
    case INTERACT  : printf("output (interact)\n"); break;
    case NOERROR   : printf("output (no error)\n"); break;
    default     : printf("\n");
  }

  printf("       ");

  printf("Backward threshold : %f\n", u->backward_prune_thresh);
  if (u->link != -1) printf("Linked to stream: %s (%li)\n",
                            GetIdName(&net, u->link), u->pos);

  if (level > 0) {
    printf("Inflowing ");
    ShowConnection(net, GetId(&net, "network", GROUP), u->id, level > 1 ? 1:0);
  }
}


void 
ShowStream(Net net, int s, int level) {
  Stream *str;
  int i;
 
  str = GETPTR(&net, s, Stream);
  printf("Stream     :  %s\n", str->Name);
  printf("  size     :  %4li\n", str->size); 
  printf("  path     :  %s\n", str->path); 
  printf("  extension:  %s\n", str->ext); 

  printf("  format   :  "); 
  printf("%s\n", ParamType2Str(str->format));

  if (*(str->Filter)) printf("  filter   :  %s\n", str->Filter);

  printf("  mode     :  "); 
  switch (str->type) {
    case READ:        printf("read (r)\n");        break;
    case TARGET:      printf("target (t)\n");      break;
    case WRITE:       printf("write (w)\n");       break;
    case INTERACTIVE: printf("interactive (i)\n"); break;
    case NOACTION:    printf("no-action (0)\n");   break;
    default: printf("unknown\n");
  }

  if (level > 1) {
    if (str->CompName[0][0] != '\0') { /* if first comp has non-empty name */
      printf("Names            A             B\n"); 
      for (i = 0; i < str->size; i++) 
        printf("%-10s %13f %13f\n", str->CompName[i], str->a[i], str->b[i]);
    }
    else {
      printf("     A             B\n"); 
      for (i = 0; i < str->size; i++) 
        printf("%13f %13f\n", str->a[i], str->b[i]);
    }
  }
}


void 
ShowCStat(Net net, int all_flag, int level) {
  int i, j, num_cons = 0;
  Connection *c;
  Unit *from_u, *to_u;
  int delay_freq[1024];
  int min_delay = 10000, max_delay = -10000;

  for (j = 0; j < 1024; j++) delay_freq[j] = 0;

  for (i = 0; i < net.NumConnections; i++) {
    c = net.CTab[i];
    if (!all_flag && !c->mark) continue;

    if (c->delay > max_delay) max_delay = c->delay;
    if (c->delay < min_delay) min_delay=c->delay;

    delay_freq[c->delay + 512]++;
    num_cons++;
  }

  printf("Connections : %6i\n", num_cons);

  if (level >= 1) {
    printf("Dynamic Profile:\n");
    printf("Delay    :");
    for (i = min_delay; i <= max_delay; i++) printf(" %i\t", i);
    printf("\n");
    printf("Freqency :");
    for (i = min_delay; i <= max_delay; i++) 
      printf(" %i\t", delay_freq[i + 512]);
    printf("\n");
  }
  if (level > 1) {
    for (i = 0; i < net.NumConnections; i++) {
      c = net.CTab[i];
      if (!all_flag && !c->mark) continue;
      from_u = GETPTR(&net, c->from, Unit);
      to_u = GETPTR(&net, c->to, Unit);
      printf("%10s ->%10s %15.3f       delay=%2li plast=%.1f\n",
        *(from_u->Name) == '\0' ? "<Unnamed>" : from_u->Name, 
        *(to_u->Name) == '\0' ? "<Unnamed>" : to_u->Name,
        c->w, c->delay, c->plast);
    }
  }
}


void 
ShowConnections(Net net, int level) {
  ShowCStat(net, 1, level);
}


void 
ShowConnection(Net net, int g1, int g2, int level) {
  SetAllConnectionMarks(&net, 0);
  MarkConnections(&net, g1, g2, 1);
  ShowCStat(net, 0, level);
}


void 
ShowUStat(Net net, int all_flag, int level) {
  int i;
  int inp = 0, lin = 0, thp = 0, sigm = 0, sym = 0, out = 0;
  int expo = 0, inver = 0, multic = 0;
  int env = 0, ffil = 0;
  Unit *u;

  for (i = 0; i < net.NumUnits; i++) {
    u = net.UTab[i];
    if (!all_flag && !(net.UTab[i]->mark)) continue;
    switch (u->type) {
      case INPUT  : inp++;  break;
      case LINEAR : lin++;  break;
      case TANHYP : thp++; break;
      case SIGMOID: sigm++; break;
      case ARCTAN : sym++;  break;
      case EXPONENTIAL : expo++;  break;
      case INVERTER    : inver++;  break;
      case MULTIC      : multic++;  break;
      case ENVIRONMENT : env++; break;
      case FILEFILTER  : ffil++; break;
      case BIAS: break; /* do nothing */
    }
    if (u->backtype != NONOUTPUT) out++;
  }

  printf("Units :%6li\n", net.NumUnits);
  printf("  Input     : %i   ", inp);
  printf("  Hidden    : %li   ", net.NumUnits - out - inp);
  printf("  Output    : %i\n", out);
  printf("  Tanhyp       :%6i\n", thp);
  if (sigm) 
  printf("  Sigmoid      :%6i\n", sigm);
  printf("  Arctan       :%6i\n", sym);
  printf("  Linear       :%6i\n", lin);
  printf("  Inverter     :%6i\n", inver);
  printf("  Multic       :%6i\n", multic);
  printf("  Exponential  :%6i\n", expo);
  printf("  Environment  :%6i\n", env);
  printf("  Filefilter   :%6i\n", ffil);
}


void 
ShowUnits(Net net, int level) {
  ShowUStat(net, 1, level);
}


/* --------------------------------------------------------------------------*/
/*           END OF FILE :  Display.c                                        */
/* --------------------------------------------------------------------------*/
