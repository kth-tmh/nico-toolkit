/* ------------------------------------------------------------------------- */
/*                         Edit a stream definition.                         */
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
#include "ParamData.h"
#include "System.h"
#include "Command.h"

void 
usage(void) {
  printf("USAGE: EditStream [options] Stream Net\n");
  printf("       Option\n");
  printf("       -m mode      Change stream mode, mode ::= r | w | t | i | 0\n");
  printf("            (read, write, target, interactive or no-action)\n");
  printf("       -F format    Change format (ascii, binary, htk etc.)\n");
  printf("       -p path      Change default path for stream data\n");
  printf("       -x extension Change extension for stream data\n");
  printf("       -f filter    Change pipeline filter command\n");
  printf("       -N file      Load component names from rows of 'file'\n");
  printf("       -n           Interactively enter names for components\n");
  printf("*** If no options are specified, interactive mode is entered\n");

  exit(0);
}  


int
main(int argc, char *argv[]) {   
  Net  *net;
  Stream *s;
  char *NetName, *StreamName;
  char *Mode = NULL, *Format = NULL, *Path = NULL, *Ext = NULL, *Filter = NULL;
  StreamType mode = NOACTION;
  ParamFileType f = 0;
  int interactive_flag = 1, name_script_flag = 0, ask_for_names = 0;
  char newformat[256], newpath[256], newext[256], 
       newmode[256], newfilter[1024];
  char **NameList;
  char *NameScript = NULL;
  char Name[256];
  int   sw, id, i, n;

  if (argc == 1) usage();
  InitCommand(argv, argc);
  while (is_option()) {
    switch (sw = GetOption()) {
      case 'm':  
        Mode = GetName("Mode of stream"); 
        if (strlen(Mode) != 1) 
          ErrorExit(SYSTEM_ERR_EXIT, "Mode of the stream must be r, w, t, i or 0");
         switch (Mode[0]) {
            case 'r':
              mode = READ;
              break;
            case 'w':
              mode = WRITE;
              break;
            case 't':
              mode = TARGET;
              break;
            case 'i':
              mode = INTERACTIVE;
              break;
            case '0':
              mode = NOACTION;
              break;
            default: 
              ErrorExit(SYSTEM_ERR_EXIT, "Mode of the stream must be r, w, t, i or 0");
        }
        interactive_flag = 0;
        break;
      case 'F':  
        Format = GetName("Format of stream data"); 
        f = Str2ParamType(Format);
        if (f == -1) ErrorExit(SYSTEM_ERR_EXIT, "Unknown format %s.", Format);
        interactive_flag = 0;
        break;
      case 'p':  
        Path = GetName("Path for stream data"); 
        interactive_flag = 0;
        break;
      case 'x':  
        Ext = GetName("Extension for stream data"); 
        interactive_flag = 0;
	break;
      case 'f':  
        Filter = GetName("Pipeline filter"); 
        interactive_flag = 0;
        break;
      case 'N':  
        NameScript = GetName("Component name file"); 
        name_script_flag = 1;
        interactive_flag = 0;
        break;
      case 'n':  
        ask_for_names = 1;
        interactive_flag = 0;
        break;
      default: ErrorExit(SYSTEM_ERR_EXIT, "Unknown switch %c.\n", sw);
    }
  } 
  StreamName = GetName("Name of the stream"); 
  NetName = GetName("Name of the network"); 
  EndCommand();

  net = LoadNet(NetName); 
  id = GetId(net, StreamName, STREAM);
  if (id == -1) ErrorExit(SYSTEM_ERR_EXIT, "No stream '%s' in %s", StreamName, NetName);
  s = (Stream *)GetPtr(net, id);

  if (name_script_flag) {
    NameList = LoadObjectList(NameScript, &n);
    for (i = 0; i < s->size; i++) {
      STRDUP(s->CompName[i], NameList[i])
    }
  }
  else if (ask_for_names) {
    for (i = 0; i < s->size; i++) {
      printf("Old Name[%i]:%8s\n", i, s->CompName[i]);
      printf("New (or nothing to keep old) >"); fflush(stdout);
      fgets(Name, 256, stdin);
      if (strcmp("", Name) != 0) {
        AddLogComment(net->logbook, "  BREAFING>", 
           "Changing name[%i] \"%s\"==>\"%s\"", s->CompName[i], Name);
        if (s->CompName[i]) FREE(s->CompName[i]);
        STRDUP(s->CompName[i], Name);
      }
    }
  }

  if (!interactive_flag) {
    if (Path) s->path = Path;
    if (Ext) s->ext = Ext;
    if (Format) s->format = f;
    if (Mode) s->type = mode;
    if (Filter) s->Filter = Filter;
  }
  else { /* No options - prompt user to input changes from stdin */
    printf("*** Edit stream %s of network %s ***\n", StreamName, NetName);

    printf("Old format : %s\n", s->format == ASCII ? "asc" : "bin");
    printf("New format or nothing to keep old format >"); 
    fflush(stdout);

    fgets(newformat, 256, stdin);
    if (strcmp("", newpath) != 0) {
      f = Str2ParamType(newformat);
      if (f == -1) ErrorExit(SYSTEM_ERR_EXIT, "Unknown format %s.", newformat);

      AddLogComment(net->logbook, "  BREAFING>", 
        "Changing format to \"%s\"", newformat);
      s->format = f;
    }

    printf("Old path : %s\n", s->path);
    printf("New path (a path or nothing to keep old path) >"); fflush(stdout);
    fgets(newpath, 256, stdin);
    if (strcmp("", newpath) != 0) {
      AddLogComment(net->logbook, "  BREAFING>", 
         "Changing path \"%s\"==>\"%s\"", s->path, newpath);
      s->path = newpath;
    }

    printf("Old extension : %s\n", s->ext);
    printf("New extension (new extension or nothing to keep old extension) >");
    fflush(stdout);

    fgets(newext, 256, stdin);
    if (strcmp("", newext) != 0) {
      AddLogComment(net->logbook, "  BREAFING>", 
         "Changing extension \"%s\"==>\"%s\"", s->ext, newext);
      s->ext = newext;
    }

    printf("Old filter : %s\n", s->Filter);
    printf("New filter (new filter or nothing to keep old filter) >");
    fflush(stdout);

    fgets(newfilter, 1024, stdin);
    if (strcmp("", newext) != 0) {
      AddLogComment(net->logbook, "  BREAFING>", 
         "Changing filter \"%s\"==>\"%s\"", s->Filter, newfilter);
      s->Filter = newfilter;
    }

    printf("Old mode : ");
    switch (s->type) {
      case READ:        printf("read (r)\n");        break;
      case WRITE:       printf("write (w)\n");       break;
      case INTERACTIVE: printf("interactive (i)\n"); break;
      case TARGET:      printf("target (t)\n");      break;
      case NOACTION:    printf("no-action\n");      break;
    }
    printf("New mode (r, w, i, 0 or nothing to keep old mode) >"); 
    fflush(stdout);

    fgets(newmode, 256, stdin);
    if (strcmp("", newmode) != 0) {
      switch (newmode[0]) {
        case 'r':
          AddLogComment(net->logbook, "  BREAFING>", "Changing mode to: read");
        s->format = READ;
        break;

        case 'w':
          AddLogComment(net->logbook, "  BREAFING>", 
            "Changing mode to: write");
        s->format = WRITE;
        break;
  
        case 'i':
          AddLogComment(net->logbook, "  BREAFING>", 
            "Changing mode to: interactive");
      s->format = INTERACTIVE;
        break;
  
        case 't':
          AddLogComment(net->logbook,
             "  BREAFING>", "Changing mode to: target");
        s->format = TARGET;
        break;

        case '0':
          AddLogComment(net->logbook,
             "  BREAFING>", "Changing mode to: no-action");
        s->format = NOACTION;
        break;

        default:  ErrorExit(SYSTEM_ERR_EXIT, "Unknown mode %s.\n", newformat);
      }
    }
  } /* End of interactive session */

  AddLogEntry(net->logbook, argv, argc); 
  SaveNet(NetName, net);
  FreeNet(net); 

  return SYSTEM_OK_EXIT;
}

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  EditStream.c                                     */
/* --------------------------------------------------------------------------*/
