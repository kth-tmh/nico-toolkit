/* ------------------------------------------------------------------------- */
/*   Back Propagation algorithm applied to Recurrent Time Delay Neural Net   */
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
#include <math.h>
#include <string.h>
#include <unistd.h>
#include "Math.h"
#include "RTDNN.h"
#include "Command.h" 
#include "System.h" 
#include "Simulation.h" 

/* --------------     Evaluation of validation set    ------------------  */

int *EvalDelay, *EvalIndex;
int EvalStreamIndex, NumEvalOutput;

/* Initialize for validation of a stream */
void 
InitFrameEval(Net *ann, NetWork *net, char *StreamName) {
  Stream *s;
  Unit *u;
  int i;

  i = GetId(ann, StreamName, STREAM);
  if (i == -1)
    ErrorExit(SYSTEM_ERR_EXIT, "No stream %s found in %s", StreamName, ann->Name);
  s = GETPTR(ann, i, Stream);

  NumEvalOutput = s->size;
  EvalStreamIndex = s->index;
  CALLOC(EvalDelay, NumEvalOutput, int) 
  CALLOC(EvalIndex, NumEvalOutput, int) 

  for (i = 0; i < ann->NumUnits; i++) { 
    u = ann->UTab[i];
    if (u->backtype != NONOUTPUT && u->link == s->id) {
      if (u->pos < 0 || u->pos >= NumEvalOutput)
        ErrorExit(SYSTEM_ERR_EXIT, "Bad validation stream");
      EvalDelay[u->pos] = net->Xlag[u->index];
      EvalIndex[u->pos] = u->index;
    }
  }
}


/* Return the number of correctly classified frames in the current utterance */
int 
FrameEval(NetWork *net) {
  int NumUnits = net->NumUnits;
  float *A = net->A + net->MaxDelay * NumUnits;
  float **D = net->ExtData;
  int T = net->T;
  int o = 0, t = 0, i, j;
  float max_o, max_t, tmp;
  int Hit = 0;

  for (i = 0; i < T; i++) { 
    max_o = max_t = -1000.0;

    /* Find the highest activity and target */
    for (j = 0; j < NumEvalOutput; j++) {

      if ((tmp = D[i][EvalStreamIndex + j]) > max_t) { 
        t = j;
        max_t = tmp;
      }

      if ((tmp = A[(i - EvalDelay[j]) * NumUnits + EvalIndex[j]]) > max_o) {
        o = j;
        max_o = tmp;
      }
    }

    if (max_t < 0.5 && o == t) Hit++;
  }

  return Hit;
}


/* -------------      Runtime Output of Search Progress     ----------------*/ 

void 
InitDiag(char *fn, int validation_flag, char *argv[], int argc) {
  FILE *fp;
  char time_and_date[256];
  char commandline[256];
  int k;

  strcpy(commandline, argv[0]);
  strcat(commandline, " ");
  for (k = 1; k < argc; k++) {
    strcat(commandline, argv[k]);
    if (k != argc - 1) strcat(commandline, " ");
  }
  sprint_time_and_date(time_and_date);

  FOPEN(fp, fn, "w")

  fprintf(fp, "******************** BackProp - Logfile *******************\n");
  fprintf(fp, "HOST   : %s  ", getenv("HOST"));
  fprintf(fp, "PID    : %i\n", (int)getpid());
  fprintf(fp, "Started: %s\n", time_and_date);
  fprintf(fp, "Command: %s\n", commandline);
  fprintf(fp, "***********************************************************\n");
  fprintf(fp, "Iteration   Global Error   Pruning");

  if (validation_flag) fprintf(fp, "        Validation\n");
  else  fprintf(fp, "\n");

  fprintf(fp, "      |            |       C%%   U%%");

  if (validation_flag) fprintf(fp, "     %%Correct   Error\n");
  else  fprintf(fp, "\n");

  FCLOSE(fp);
}


void 
OutDiag(char *fn, int i, float err, float bu, float bc,
             int validation_flag,
             float validation_score, float validation_error) {
  FILE *fp;

  if ((fp = fopen(fn, "a")) == NULL) return;
  fprintf(fp, "%7i%15.3e    %4.1f %4.1f", i, err, bc, bu);

  if (validation_flag) {
    if (validation_score >= 0.0) 
      fprintf(fp, "%9.1f %10.3e\n", validation_score, validation_error);
    else 
      fprintf(fp, "%9s %10.3e\n", "N/A", validation_error);      
  }
  else  fprintf(fp, "\n");

  FCLOSE(fp);
}


void 
AddProgressDot(char *fn) {
  FILE *fp;
  if ((fp = fopen(fn, "a")) == NULL) return;
  fprintf(fp, ".");
  FCLOSE(fp);
}


void 
EndProgressDots(char *fn) {
  FILE *fp;
  if ((fp = fopen(fn, "a")) == NULL) return;
  fprintf(fp, "\n");
  FCLOSE(fp);
}


void 
ProgressComment(char *fn, char *comment) {
  FILE *fp;

  if ((fp = fopen(fn, "a")) == NULL) return;

  fprintf(fp, "%s\n", comment);

  FCLOSE(fp);
}


void 
FinishDiag(char *fn, char *prologue) {
  FILE *fp;
  char time_and_date[256];

  sprint_time_and_date(time_and_date);

  if ((fp = fopen(fn, "a")) == NULL) return;
  fprintf(fp, "%s\n", prologue);
  fprintf(fp, "********          %s         ********\n", time_and_date);
  fprintf(fp, "***********************************************************\n");

  FCLOSE(fp);
}


void 
usage(void) {
  printf("USAGE: BackProp [options] Net Input\n");
  printf("       Option                                                           Default\n");
  printf("       -S            Treat 'Input' as an inputfile script               (off)\n");
  printf("       -s obj1 obj2  Select connections from obj1 to obj2               (all)\n");
  printf("       -g gain       Linear gain factor                                 (1e-3)\n");
  printf("       -m momentum   Momentum parameter                                 (0.9)\n");
  printf("       -w factor     Weight decay                                       (off)\n");
  printf("       -f num        Update frequency - update every 'num' frames       (Max+1)\n");
  printf("       -F min max    Random update after between min and max frames     (off)\n");
  printf("       -E            Epoch updating                                     (off)\n");
  printf("       -p name       Name of runtime error progress report file         (none)\n"); 
  printf("       -.            Show progress within epoch using dots in prog. file(off)\n"); 
  printf("       -T level      Level of detail in progress report file            (0)\n"); 
  printf("       -P name m n   Update progress every m:th and net every n:th iteration\n"); 
  printf("       -i iter       Number of iterations                               (100)\n");
  printf("       -V set stream Set Validation set/stream (1-of-N classification) (off)\n");
  printf("       -v set stream Set Validation set/stream (general case)          (off)\n");
  printf("       -D decay acc  Multiply gain with 'decay' after epochs where\n");
  printf("                     the validation set's global error is not\n");
  printf("                     improved, otherwize multiply by 2*acc'.            (off)\n");
  printf("       -C decay num  Multiply gain with 'decay' after epochs where\n");
  printf("                     the validation set's global error is not\n");
  printf("                     improved, but maximum 'num' times.                 (off)\n");
  printf("       -e crit       Error Criterium -- exit when global error < 'crit' (off)\n");
  printf("       -d            Store external data in RAM   (read from disc each epoch)\n");
  exit(0);
}


int
main(int argc, char *argv[]) {       
  Net     *ann;
  NetWork *net;

  /* Input data */
  int      NumFiles;
  char   **InList;
  float ***ExtData = NULL;
  int     *Length = NULL;    

  /* Command line information */
  char   sw;
  char   *NetName;
  char   *Input;
  char   *Diagnostics = NULL;
  int     trace_level = 0;
  char    prog_comment[256];

  /* Parameters from the command line */
  float gain = 1e-3;
  float gain_decay = 1.0;
  float gain_acceleration = 1.0;
  int   max_gain_decays = 0;
  float momentum  = 0.9;
  float weight_decay_factor = 0.0;
  int   MaxIter = 100;
  int   max_section = 0, min_section = 0;
  int   section_span = 0;
  int   section_length = -1;
  float ErrCrit = 0.0;
  int   NetUpdate = 1, ProgUpdate = 1;
  int   eflag = 0, dflag = 0, scriptflag = 0;
  int   random_flag = 0, validation_flag = 0, classification_flag = 0;
  int   gain_decay_flag = 0, epoch_flag = 0, mark_flag = 0, progress_dots = 0;
  int   prefer_prim_mem = 0;
  int   earlyquit = 0;

  /* For selecting conections */
  char   *group1, *group2;
  int     g1, g2;

  /* For the validation procedure */
  char   *EvalSet = NULL, *EvalStream = NULL;
  char  **EvalList = NULL;
  int     NumEvalFiles;
  float   eval_score = 0.0, last_eval_score = 0.0;
  float   eval_error = 0.0, last_eval_error = 1.0e+30;
  int     NumFrames, NumCorrect;

  /* Misc. */
  int     i, k;
  int     start_t, end_t;
  float   bu, bc;
  char   *runtime;
  char    starttime[32];
  int     num_gain_decays = 0;
  float   GlobalError = 0.0, LocalError; 


  if (argc == 1) usage();

  InitCommand(argv, argc);
  while (is_option()) {
    switch (sw = GetOption()) {
    case 'S':
      scriptflag = 1;
      break; 
    case 'i':
      MaxIter = GetInt("-i option", 0, 1e6);
      break; 
    case 'g':
      gain = GetFloat("Learning Gain", 0.0, 1.0);
      break; 
    case 'm':
      momentum = GetFloat("Learning Momentum", 0.0, 10.0);
      break; 
    case 'w':
      weight_decay_factor = GetFloat("Weight decay factor", 0.0, 0.1);
      break; 
    case 'V':
      EvalSet = GetName("validation set");
      EvalStream = GetName("validation stream");
      validation_flag = 1; 
      classification_flag = 1;
      break; 
    case 'v':
      EvalSet = GetName("validation set");
      EvalStream = GetName("validation stream");
      validation_flag = 1; 
      break; 
    case 'D':
      gain_acceleration = GetFloat("Gain Acceleration", 1.0, 2.0);
      gain_decay = GetFloat("Gain Decay", 0.0, 1.0);
      max_gain_decays = -1;
      gain_decay_flag = 1;
      break; 
    case 'C':
      gain_decay = GetFloat("Gain Decay", 0.0, 1.0);
      max_gain_decays = GetInt("Max number of Gain Decays", 1, 100);
      gain_decay_flag = 1;
      break; 
    case 'f':
      section_length = GetInt("-f option", 1, 10000);
      break; 
    case 'E':
      epoch_flag = 1;
      break; 
    case 'F':
      min_section = GetInt("First argument of -F option", 1, 10000);
      max_section = GetInt("Second argument of -F option", 
                           min_section, 10000);
      section_span = max_section - min_section;
      random_flag = 1;
      break; 
    case 'e':  
      eflag = 1;
      ErrCrit = GetFloat("-e option",0.0,1e6);
      break; 
    case 's':  
      mark_flag = 1;
      GetName("from-object for s-option");
      GetName("to-object for s-option");
      break; /* Save this for the next pass */
    case 'p':  
      dflag = 1;
      Diagnostics = GetName("Progress Report File");
      break;
    case '.':  
      progress_dots = 1;
      break;
    case 'T':  
      trace_level = GetInt("level of detail of progress report", 0, 3);
      break;
    case 'P':  
      dflag = 1;
      Diagnostics = GetName("Progress Report File");
      ProgUpdate = GetInt("first arg of -P option", 1, 1e6);
      NetUpdate = GetInt("second arg of -P option", 1, 1e6);
      break;
    case 'd':  
      prefer_prim_mem = 1;
      break;
    default: ErrorExit(SYSTEM_ERR_EXIT, "Unknown switch %c.", sw);
    }
  } 
  NetName = GetName("Name of neural net definition file"); 
  Input = GetName("Name of Input file or (with -S option) Script file");
  EndCommand();

  if (progress_dots && ! dflag)
    ErrorExit(SYSTEM_ERR_EXIT, "Can't print progress dots without a progress report file");

  ann = LoadNet(NetName);
  AddLogEntry(ann->logbook, argv, argc); 
  AddLogComment(ann->logbook, "COMPLETION>", 
                "BackProp not completed. 0(%i) iterations", MaxIter); 
  SaveNet(NetName, ann);

  net = Compile(ann);

  if (section_length == -1) section_length = net->MaxDelay + 1;

  if (mark_flag) {
    UnMarkAll(ann, -1);
    InitCommand(argv, argc);
    for (i = 1; i < argc - 2; i++) {
      if (is_option()) {
        if (GetOption() == 's') {

          group1 = GetName("First arg of -s option");
          i++;
          if ((g1 = GetId(ann, group1, -1)) == -1) 
            ErrorExit(SYSTEM_ERR_EXIT, "Object %s does not exist.", group1);

          group2 = GetName("Second arg of -s option");
          i++;
          if ((g2 = GetId(ann, group2, -1)) == -1) 
            ErrorExit(SYSTEM_ERR_EXIT, "Object %s does not exist.", group2);
 
          MarkConnections(ann, g1, g2, 1);
        }
      }
      else SkipArg();
    }
  }

  if (validation_flag) {
    InitFrameEval(ann, net, EvalStream);

    if (scriptflag) EvalList = LoadFileList(EvalSet, "", "", &NumEvalFiles);
    else {
      NumEvalFiles = 1;
      CALLOC(EvalList, 1, char *)
      STRDUP(EvalList[0], EvalSet)
    }
  }

  if (scriptflag) InList = LoadFileList(Input, "", "", &NumFiles);
  else {
    NumFiles = 1;
    CALLOC(InList, 1, char *)
    STRDUP(InList[0], Input)
  }

  if (!prefer_prim_mem) RandomizeLists(InList, NULL, NumFiles);

  /* This is a little hack to read in all data only once.
     When it's preferred to store the data in primary memory,
     it does just that. */
  if (prefer_prim_mem) {
    CALLOC(ExtData, NumFiles, float **)
    CALLOC(Length, NumFiles, int)
  
    for (k = 0; k < NumFiles; k++) {
      LoadExtData(net, InList[k], 1, 1);
      ExtData[k] = net->ExtData;
      Length[k]  = net->T;
      free(net->CurrentData);
      net->has_data = 0; /* Prevents data from beeing freed. */
    }
  }

  sprint_time_and_date(starttime);

  if (dflag) {
    InitDiag(Diagnostics, validation_flag, argv, argc);
    ResetPruneStats();
  }

  InitGainAndMomentum(net, gain, momentum);

  for (i = 0; i < MaxIter; i++) { 
    GlobalError = 0.0;
    NumFrames = 0;

    if (!prefer_prim_mem) RandomizeLists(InList, NULL, NumFiles);

    for (k = 0; k < NumFiles; k++) {

      /* Get data either from primary memory or from disc */
      if (prefer_prim_mem) {
        net->ExtData = ExtData[k];
        net->T = Length[k];
        net->CurrentData = InList[k];
        net->excited = 0;
        net->has_data = 1;
      }
      else LoadExtData(net, InList[k], 1, 1);

      NumFrames += net->T;

      LocalError = 0.0;

      InitForward(net); 
      InitBackward(net);

      start_t = end_t = 0;
      do {
        if (random_flag) 
          section_length = min_section + 
            (int)((float)section_span * RectRand());
        end_t = start_t + section_length;
        if (epoch_flag) end_t = net->T;
        if (end_t > net->T) end_t = net->T;
        ForwardSection(net, start_t, end_t);

        if (!epoch_flag && section_length == 1 && !mark_flag &&
            weight_decay_factor == 0.0) {
          /* This function does weight-updating too */
          GlobalError += BackwardSectionAndUpdate(net, start_t, end_t);
        }
        else {
          LocalError += BackwardSection(net, start_t, end_t);
          if (!epoch_flag) {
            GradStep(ann, net, !mark_flag);
            if (weight_decay_factor != 0.0) 
              WeightDecay(ann, net, weight_decay_factor, !mark_flag);
	  }
        }
        start_t = end_t;
      } while (end_t != net->T);

      /* FinishForward(net);
      FinishBackward(net); */

      GlobalError += LocalError;
      LocalError /= (float)net->T;

      if (progress_dots && (NumFiles < 60 || k % (NumFiles / 60) == 0))
	AddProgressDot(Diagnostics);

      /* Prevent data from beeing freed */
      if (prefer_prim_mem) net->has_data = 0;
    }

    if (progress_dots) EndProgressDots(Diagnostics);

    GlobalError /= (float)NumFrames;


    if (epoch_flag) {
      GradStep(ann, net, !mark_flag);
      if (weight_decay_factor != 0.0) 
	WeightDecay(ann, net, weight_decay_factor, !mark_flag);
    }

    if (validation_flag) {
      NumCorrect = 0, 
      NumFrames = 0;
      eval_error = 0.0;

      for (k = 0; k < NumEvalFiles; k++) {
        LoadExtData(net, EvalList[k], 1, 1);
        NumFrames += net->T;
        Forward(net);
        eval_error += GetGlobalError(net);
        if (classification_flag) NumCorrect += FrameEval(net);
      }

      if (classification_flag) eval_score = 100.0 * (float)NumCorrect / (float)NumFrames;
      else eval_score = -1.0;
      eval_error /= (float)NumFrames;
    }
    else {
      eval_error = GlobalError;
      eval_score = 1.0 / eval_error;
    }

    if (dflag) {
      if (i % ProgUpdate == 0) {
        GetPruneStats(&bu, &bc);
        ResetPruneStats();
        OutDiag(Diagnostics, i + 1, GlobalError, bu, bc, 
                validation_flag, eval_score, eval_error);
      }
    }

    if (gain_decay_flag) {
      if ((!classification_flag && eval_error > last_eval_error) ||
	  (classification_flag && eval_score < last_eval_score)) {
	gain *= gain_decay;
	num_gain_decays++;
	
	sprintf(prog_comment, 
		"*** Reducing gain to %.2e. Reduction number %i (max %i)", 
		gain, num_gain_decays, max_gain_decays);
	ProgressComment(Diagnostics, prog_comment);
	
	if (num_gain_decays == max_gain_decays) {
	  earlyquit = 1; 
	  i = MaxIter; /* Terminates the iterations loop */
	}
	InitGainAndMomentum(net, gain, momentum);
      }
      else {
	gain *= gain_acceleration;
      }
    }
    last_eval_error = eval_error;
    last_eval_score = eval_score;

    if (i % NetUpdate == 0) {
      UpdateNet(ann, net, !mark_flag);
      RemoveCompletionNote(ann->logbook);
      AddLogComment(ann->logbook, "COMPLETION>", 
                    "BackProp not completed. %i(%i) iterations, E = %.3e", 
                    i + 1, MaxIter, GlobalError); 
      SaveNet(NetName, ann);
    }

    if (eflag) {
      if (GlobalError < ErrCrit) {
        RemoveCompletionNote(ann->logbook);
        AddLogComment(ann->logbook, "COMMENT>",
                      "Error Criterium met after %i iterations.", i); 
        earlyquit = 1; 
        i = MaxIter; /* Terminates the iterations loop */
      }
    }
  }

  if (dflag && i % ProgUpdate != 0) {
    GetPruneStats(&bu, &bc);
    OutDiag(Diagnostics, i + 1, GlobalError, bu, bc, 
            validation_flag, eval_score, eval_error);
  }
  if (dflag) {
    if (earlyquit) 
      FinishDiag(Diagnostics, 
        "******            Error criterium met            ******");
    else
      FinishDiag(Diagnostics,
        "******        BackProp terminated normally       ******");
  }

  runtime = MyRunTime();

  RemoveCompletionNote(ann->logbook);

  AddLogComment(ann->logbook, "COMPLETION>", 
           "BackProp terminated with E = %.3e after %s of user run-time",
           GlobalError, runtime); 

  UpdateNet(ann, net, !mark_flag);
  SaveNet(NetName, ann); 

  FreeExtData(net);
  FreeNet(ann);
  FreeNetWork(net);

  if (prefer_prim_mem) {
    FREE2D(ExtData, NumFiles);
    FREE(Length);
  }

  if (validation_flag) {
    FREE(EvalDelay);
    FREE(EvalIndex);
  }

  return SYSTEM_OK_EXIT;
}

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  BackProp.c                                       */
/* --------------------------------------------------------------------------*/

