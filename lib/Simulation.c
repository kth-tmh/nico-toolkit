/* ------------------------------------------------------------------------- */
/*               Forward and Backward evaluation of the RTDNN                */
/* ------------------------------------------------------------------------- */
/*                                                                           */
/*   Copyright (C) 1994, 1995, 1996, 1997, 1998 Nikko Strom                  */
/*                                                                           */
/*   All rights reserved.                                                    */
/*                                                                           */
/*   Developed by Nikko Strom at the Dept. of Speech, Music and Hearing,     */
/*   KTH (Royal Institute of Technology)                                     */
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
#include <math.h>  
#include <string.h>  
#include <ctype.h>  
#include <unistd.h>
#include "ParamData.h"
#include "System.h"
#include "nMath.h"
#include "RTDNN.h" 
#include "Simulation.h" 


/* A macro used by forward-functions */
#define FORWARD_SIGMOID(act, type) \
  { \
    switch (type) { \
      case TANHYP: \
        (act) = tanh(act);  \
        break; \
      case SIGMOID: \
        (act) = 1.0 / (1.0 + exp(-(act)));  \
        break; \
      case ARCTAN: \
        act = atan(act) / PIHALF;  \
        break; \
      case INVERTER: \
        act = 1.0 / act;  \
        break; \
      case EXPONENTIAL: \
        act = exp(act);  \
        break; \
      case BIAS: \
        act = 1.0; \
        break; \
      } \
    }


/* Two macros used by backward functions */

#define BACKWARD_SIGMOID(grad, act, type) \
  { \
    float __tmp__; \
    switch(type) { \
      case TANHYP: \
        (*(grad)) *= (1.0 + (act)) * (1.0 - (act)); \
        break; \
      case SIGMOID: \
        (*(grad)) *= (act) * (1.0 - (act)); \
        break; \
      case INVERTER: \
        (*(grad)) *= - (act) * (act); \
        break; \
      case EXPONENTIAL: \
        (*(grad)) *= (act); \
        break; \
      case ARCTAN: \
        __tmp__ = tan(act); \
        (*(grad)) *= 1.0 / (1.0 + __tmp__ * __tmp__) / PIHALF; \
        break; \
    } \
  }


#define INCREMENT_GRADIENT(grad, act, targ, E, backtype) \
  { \
      switch(backtype) { \
        case ZERO_ONE: \
          (*(grad)) += ZeroOne(act, targ, &(E)); \
          break; \
        case SYM_ZERO_ONE: \
          (*(grad)) += SymZeroOne(act, targ, &(E)); \
          break; \
        case OUTPUT: \
          (*(grad)) += L2(act, targ, &(E)); \
          break; \
        case L4OUTPUT: \
          (*(grad)) += L4(act, targ, &(E)); \
          break; \
        case L10OUTPUT: \
          (*(grad)) += L10(act, targ, &(E)); \
          break; \
        case L0OUTPUT: \
          (*(grad)) += L0(act, targ, &(E)); \
          break; \
        case SYMCROSSENTR: \
          (*(grad)) += SymCross(act, targ, &(E)); \
          break; \
        case CROSSENTR: \
          (*(grad)) += Cross(act, targ, &(E)); \
          break; \
        case CROSS01: \
          (*(grad)) += Cross01(act, targ, &(E)); \
          break; \
        case SYM_CROSS01: \
          (*(grad)) += SymCross01(act, targ, &(E)); \
          break; \
        case INTERACT: \
          (*(grad)) += targ; \
      } \
  }


/*   Numbers are divided by the number of units and connections in   */
/*   the network not to get too big numbers.                         */
static float ProcessedUnits = 0.0;
static float ProcessedConnections = 0.0;
static float PrunedUnits = 0.0;
static float PrunedConnections = 0.0;

void 
ResetPruneStats() {
  ProcessedUnits = 0.0;
  ProcessedConnections = 0.0;
  PrunedUnits = 0.0;
  PrunedConnections = 0.0;
}

void 
GetPruneStats(float *u, float *c) {
  *u = 100.0 * PrunedUnits / ProcessedUnits;
  *c = 100.0 * PrunedConnections / ProcessedConnections;
}


/*        Load external data associated with the filename "Name".       */
/*             The number of frames is returned in net->T.              */
void 
LoadExtData(NetWork *net, char *Name, int load_targets, int check_range) {
  int i, j, k, T;
  int allocation_done = 0;
  char *fn, command[1024];
  float **d;
  int big_data_warning = 0;
  int big_data_index = 0;
  float big_data_value = 0;

  FreeExtData(net);
  T = 0;

  for (i = 0; i < net->NumStreams; i++) {

    if (! load_targets &&
        (net->stream_mode[i] == TARGET || net->stream_mode[i] == INTERACTIVE)) 
      continue;

    if (net->stream_mode[i] == WRITE || 
        net->stream_mode[i] == NOACTION) continue;

    fn = ModifyName(Name, net->stream_ext[i], net->stream_dir[i]);

    if (net->stream_filter[i][0] == '\0') {
      d  = LoadParameters(fn, net->stream_format[i], 
                          &(net->stream_size[i]), &T);
    }
    else { /* a filter - use 'FilterLoadParams' */
      strcpy(command, net->stream_filter[i]);
      if (net->stream_mode[i] == WRITE) strcat(command, " > ");
      else strcat(command, " < ");
      strcat(command, fn);
      d = FilterLoadParams(command, net->stream_format[i], 
                           &(net->stream_size[i]), &T);
    }

    FREE(fn);

    if (!allocation_done) { /* Allocate memory on first pass only. */
      CALLOC2D(net->ExtData, T, net->NumExt, float)
      allocation_done = 1;
    } 
    for (k = 0; k < T; k++) {
      for (j = 0; j < net->stream_size[i]; j++) {
        /* Note! the input is linearly transformed by x/b-a before entered 
           into the ExtData-array */
        if (fabs(net->ExtData[k][net->stream_offset[i] + j] = 
              d[k][j] / net->lin_b[net->stream_offset[i] + j] -
		 net->lin_a[net->stream_offset[i] + j]) > 10.0) {
          big_data_warning = 1;
	  if (fabs(net->ExtData[k][net->stream_offset[i] + j]) > big_data_value) {
	    big_data_value = fabs(net->ExtData[k][net->stream_offset[i] + j]);
	    big_data_index = k;
	  }
	}
      }
    }
    FreeParameters(d, T);
  }
  STRDUP(net->CurrentData, Name);


  /* Allocate memory here if it's not been done yet. */
  if (!allocation_done) {
    CALLOC2D(net->ExtData, T, net->NumExt, float)
    allocation_done = 1;
  }

  if (check_range && big_data_warning) 
    EmitWarning("External data are of large magnitude (%f>10.0) at sample %d.", 
		big_data_value, big_data_index);

  net->T = T;
  net->excited = 0;
  net->has_data = 1;
}


void 
FreeExtData(NetWork *net) {
  if (!net->has_data) return;

  FREE2D(net->ExtData, net->T)

  FREE(net->CurrentData);
  net->T = 0;
  net->has_data = 0;
} 


/* This function sets up the "hard" version of the network (a "NetWork")   */
/*             for forward and backward evaluation.                        */ 
NetWork *
Compile(Net *ann) {
  NetWork *net;
  int i, j, m, n, num_constants;
  Unit *u, *u_to, *u_from;
  Connection *c;

  CALLOC(net, 1, NetWork)

  net->MaxDelay = GetRealDelays(ann);

  /* Copy stream information */
  n = net->NumStreams = ann->NumStreams;
  if (n == 0) n++;
  CALLOC(net->stream_format, n, ParamFileType);
  CALLOC(net->stream_offset, n, int);
  CALLOC(net->stream_size,   n, int);
  CALLOC(net->stream_dir,    n, char *);
  CALLOC(net->stream_ext,    n, char *);
  CALLOC(net->stream_name,   n, char *);
  CALLOC(net->stream_filter, n, char *);
  CALLOC(net->stream_mode,   n, StreamType);

  m = 0;
  for (i = 0; i < net->NumStreams; i++) {   
    net->stream_offset[i] = m;
    ann->StrTab[i]->index = m;
    m += ann->StrTab[i]->size;
    net->stream_size[i] = ann->StrTab[i]->size;
    STRDUP(net->stream_ext[i], ann->StrTab[i]->ext);
    STRDUP(net->stream_dir[i], ann->StrTab[i]->path);
    STRDUP(net->stream_name[i], ann->StrTab[i]->Name);
    STRDUP(net->stream_filter[i], ann->StrTab[i]->Filter);
    net->stream_format[i] = ann->StrTab[i]->format;
    net->stream_mode[i] = ann->StrTab[i]->type;
  }
  net->NumExt = m;

  if (!m) m++;
  CALLOC(net->lin_a, m, float)
  CALLOC(net->lin_b, m, float)
  CALLOC(net->stream_comp_name, m, char *)

  m = 0;
  for (i = 0; i < net->NumStreams; i++) {   
    for (j = 0; j < ann->StrTab[i]->size; j++, m++) {   
      net->lin_a[m] = ann->StrTab[i]->a[j];
      net->lin_b[m] = ann->StrTab[i]->b[j];
	STRDUP(net->stream_comp_name[m], ann->StrTab[i]->CompName[j]);
    }
  }

  net->NumConnections = n = ann->NumConnections;
  if (!n) n = 1; /* Never allocate zero objects */

  for (i = 0, num_constants = 0; i < ann->NumUnits; i++) {
    if (ann->UTab[i]->type == FILEFILTER || 
       ann->UTab[i]->type == ENVIRONMENT) { 
      num_constants++;
    }
  }
             
  net->NumUnits = n = ann->NumUnits;
  if (!n) n++; /* Never allocate zero objects */

  CALLOC(net->type, n, int)
  CALLOC(net->backtype, n, int)
  CALLOC(net->Xindex, n, int)
  CALLOC(net->Xlag, n, int)
  CALLOC(net->link_mode, n, StreamType);
  CALLOC(net->backthresh, n, float)
  CALLOC(net->NumInflows, n, int)
  CALLOC(net->Inflows, n, int *)

  CALLOC(net->InW, n, float *)
  CALLOC(net->InGW, n, float *)

  CALLOC(net->Gain, n, float *)
  CALLOC(net->Momentum, n, float *)
  CALLOC(net->DeltaW, n, float *)
  CALLOC(net->Plast, n, float *)

  CALLOC(net->ConLink, n, Connection **)

  /* Units... */
  for (i = 0; i < ann->NumUnits; i++) {
    u = ann->UTab[i];
    u->index = i;
    net->type[i] = u->type;
    net->backtype[i] = u->backtype;

    /* Special backtypes for [-1,1] units */
    if (u->backtype == CROSSENTR && (u->type == TANHYP || u->type == ARCTAN))
      net->backtype[i] = SYMCROSSENTR;
    if (u->backtype == CROSS01 && (u->type == TANHYP || u->type == ARCTAN))
      net->backtype[i] = SYM_CROSS01;
    if (u->backtype == ZERO_ONE && (u->type == TANHYP || u->type == ARCTAN))
      net->backtype[i] = SYM_ZERO_ONE;

    net->backthresh[i] = u->backward_prune_thresh;
    if (u->link != -1) {
      net->Xindex[i] = GETPTR(ann, u->link, Stream)->index + u->pos;
      net->Xlag[i] = u->mark; /* The delay computed in GetRealDelays() */
      net->link_mode[i] = GETPTR(ann, u->link, Stream)->type;
    }
  }

  net->NumConstants = num_constants;
  if (!num_constants) num_constants++; /* Don't allocate a size zero object */
  CALLOC(net->Constants, num_constants, float)
  CALLOC(net->ConstName, num_constants, char *)

  for (i = 0, num_constants = 0; i < ann->NumUnits; i++) {
    u = ann->UTab[i];
    if (u->type == FILEFILTER || u->type == ENVIRONMENT) {
      net->Xindex[u->index] = num_constants;
      STRDUP(net->ConstName[num_constants], u->Name);
      num_constants++;
    }
  }

  /* Count number of inflowing con's and store in "unit->mark" */ 
  for (i = 0; i < ann->NumUnits; i++) ann->UTab[i]->mark = 0;
  for (i = 0, n = 0; i < ann->NumConnections; i++) {
    c = ann->CTab[i];
    c->index = i;
    u = GETPTR(ann, c->to, Unit);
    u->mark++;
  }

  for (i = 0; i < ann->NumUnits; i++) {
    u = ann->UTab[i];
    n = u->mark; /* number of inflowing connections */
    if (n == 0) n = 1;
    CALLOC(net->Inflows[u->index], n, int)

    CALLOC(net->InW[u->index], n, float)
    CALLOC(net->InGW[u->index], n, float)

    CALLOC(net->Plast[u->index], n, float)
    CALLOC(net->Gain[u->index], n, float)
    CALLOC(net->Momentum[u->index], n, float)
    CALLOC(net->DeltaW[u->index], n, float)

    CALLOC(net->ConLink[u->index], n, Connection *)
  }

  /* Weights... */

  for (j = 0; j < ann->NumConnections; j++) {
    c = ann->CTab[j];
    u_to = GETPTR(ann, c->to, Unit);
    u_from = GETPTR(ann, c->from, Unit);
   
    net->InW[u_to->index][net->NumInflows[u_to->index]] = c->w;

    net->Plast[u_to->index][net->NumInflows[u_to->index]] = c->plast;

    net->ConLink[u_to->index][net->NumInflows[u_to->index]] = c;

    (net->NumInflows[u_to->index])++;
  }


  /* Relative unit-pointers... */
  for (j = 0; j < net->NumUnits; j++) {
    net->NumInflows[j] = 0;
  }
  for (j = 0; j < ann->NumConnections; j++) {
    c = ann->CTab[j];
    u_to = GETPTR(ann, c->to, Unit);
    u_from = GETPTR(ann, c->from, Unit);

    /* The "real" delay of the connection was computed in GetRealDelays()
       and is stored in:  c->mark */

    net->Inflows[u_to->index][net->NumInflows[u_to->index]++] = 
       u_from->index - u_to->index - net->NumUnits * c->mark;
  }

  return net;
}


/*      This is the inverse of the "Compile" function          */
/*    The weights in the "NetWork" are copied to the "Net"     */ 
void 
UpdateNet(Net *ann, NetWork *net, int all_flag) {
  Connection   *c;
  Unit         *u;
  Unit        **UTab = ann->UTab;
  Connection ***ConLink = net->ConLink;
  float       **InW = net->InW;
  int           NumUnits = net->NumUnits;
  int          *NumInflows = net->NumInflows;
  int           i, j;

  if (all_flag) {
    for (i = 0; i < net->NumUnits; i++) {
      u = UTab[i];
      for (j = 0; j < NumInflows[i]; j++) {
        ConLink[i][j]->w = InW[i][j];
      }
    }
  }

  else {
    for (i = 0; i < NumUnits; i++) {
      for (j = 0; j < NumInflows[i]; j++) {
        c = ConLink[i][j];
        if (c->mark) c->w = InW[i][j];
      }
    }
  }
}
 

/*   Initialize training parameters for BackPropagation training  */
void 
InitGainAndMomentum(NetWork *net, float gain, float momentum) {
  int i, j;

  for (i = 0; i < net->NumUnits; i++) {
    for (j = 0; j < net->NumInflows[i]; j++) {
      net->Gain[i][j]     = gain * net->Plast[i][j];
      net->Momentum[i][j] = momentum;
      net->DeltaW[i][j]   = 0.0;
    }
  }
}


/*  Take a gradient descent step including momentum */
void 
GradStep(Net *ann, NetWork *net, int all_flag) {
  Connection   *c;
  Connection ***ConLink = net->ConLink;
  float       **DeltaW = net->DeltaW;
  float       **InGW = net->InGW;
  float       **Gain = net->Gain;
  float       **Momentum = net->Momentum;
  float       **InW = net->InW;
  int           NumUnits = net->NumUnits;
  int          *NumInflows = net->NumInflows;
  int           i, j;

  if (all_flag) {
    for (i = 0; i < NumUnits; i++) {
      for (j = 0; j < NumInflows[i]; j++) if (Gain[i][j] != 0.0) {
        DeltaW[i][j] = InGW[i][j] * Gain[i][j] +
            DeltaW[i][j] * Momentum[i][j];

        InW[i][j] += DeltaW[i][j];

        InGW[i][j] = 0.0;
      }
    }
  }
  else {
    for (i = 0; i < NumUnits; i++) {
      for (j = 0; j < NumInflows[i]; j++) {
        c = ConLink[i][j];
        if (c->mark && Gain[i][j] != 0.0) {
          DeltaW[i][j] = InGW[i][j] * Gain[i][j] +
              DeltaW[i][j] *  Momentum[i][j];

          InW[i][j] += DeltaW[i][j];

          InGW[i][j] = 0.0;
        }
      }
    }
  }
}
 

/*  Perform weight decay on connections  */
void 
WeightDecay(Net *ann, NetWork *net, float factor, int all_flag) {
  Connection   *c;
  Connection ***ConLink = net->ConLink;
  float       **Plast = net->Plast;
  float       **InW = net->InW;
  int           NumUnits = net->NumUnits;
  int          *NumInflows = net->NumInflows;
  int           i, j;

  if (all_flag) {
    for (i = 0; i < NumUnits; i++) {
      for (j = 0; j < NumInflows[i]; j++) {
        InW[i][j] *= (1.0 - factor * Plast[i][j]);
      }
    }
  }
  else {
    for (i = 0; i < NumUnits; i++) {
      for (j = 0; j < NumInflows[i]; j++) {
        c = ConLink[i][j];
        if (c->mark) {
          InW[i][j] *= (1.0 - factor * Plast[i][j]);
        }
      }
    }
  }
}


void 
FreeNetWork(NetWork *net) {
  if (net->has_data) FreeExtData(net);

  FREE2D(net->stream_name, net->NumStreams); 
  FREE2D(net->stream_dir, net->NumStreams)
  FREE2D(net->stream_ext, net->NumStreams)

  FREE(net->stream_offset); 
  FREE(net->stream_mode);  
  FREE(net->stream_format); 
  FREE(net->stream_size); 
  FREE(net->stream_filter); 
  FREE(net->lin_a); 
  FREE(net->lin_b); 

  FREE(net->type);
  FREE(net->backtype);
  FREE(net->backthresh);
  FREE(net->Xindex);
  FREE(net->Xlag);
  FREE(net->link_mode); 

  FREE2D(net->ConstName, net->NumConstants)

  FREE(net->NumInflows);

  FREE2D(net->Inflows, net->NumUnits)
  FREE2D(net->InW, net->NumUnits)
  FREE2D(net->InGW, net->NumUnits)
  FREE2D(net->Gain, net->NumUnits)
  FREE2D(net->Momentum, net->NumUnits)
  FREE2D(net->DeltaW, net->NumUnits)
  FREE2D(net->Plast, net->NumUnits)
}


void 
InitForward(NetWork *net) {
  int num_constants, i;
  float activity;
  char *value, *ptr;
  char DataDirectory[256], DataName[128], DataExtension[64];

  if (net->has_data == 0) {
    ErrorExit(SYSTEM_ERR_EXIT, "Cannot perform forward phase - no data.");
  } 

  /* Start with an allocated, zero-initialized activation matrix */
  if (net->A) FREE(net->A);
  CALLOC(net->A, net->NumUnits * (net->T + 2 * net->MaxDelay), float);

  /* Reset the gradient vector */
  if (net->GA) {
    FREE(net->GA);
    net->GA = NULL;
  }

  /* Set up the array of Constants */
  for (i = 0, num_constants = 0; i < net->NumUnits; i++) {
    if (net->type[i] == FILEFILTER) {
      Extract(net->CurrentData, DataDirectory, DataName, DataExtension);
      if (strstr(DataName, net->ConstName[num_constants])) 
        net->Constants[num_constants] = 1.0;
      else net->Constants[num_constants] = 0.0;
      num_constants++;
    }
    if (net->type[i] == ENVIRONMENT)  {
      value = getenv(net->ConstName[num_constants]);
      /* Activity 0.0 if the variable is not defined */
      if (!value) activity = 0.0; 
      else {
        activity = strtod(value, &ptr);
        if (ptr == value) { /* Value is not numeric... */
          for (i = 0; i < strlen(value); i++) {
            if (!isspace((int)value[i])) {
              ErrorExit(SYSTEM_ERR_EXIT, 
                "The value of environment variable `%s' is non-numeric.", 
                net->ConstName[num_constants]);
            }
          }
          activity = 1.0; /* Defined but empty => Activity 1.0 */
        }
      }
      net->Constants[num_constants] = activity;  
      num_constants++;
    }
  }
}


void 
ForwardSection(NetWork *net, int start_t, int end_t) {
  ForwardSection_RT(net, start_t, end_t, net->A, net->ExtData);
}


/* used by the RTSim functions */
void 
ForwardSection_RT(NetWork *net, int start_t, int end_t,
		  float *A, float **ExtData) {
  int i, j, t;
  float *a;
  int  *Inflows, NumInflows;
  float *InW;
  int NumUnits = net->NumUnits;
  int *Xindex = net->Xindex;
  int *Xlag = net->Xlag;
  int *type = net->type;
  int *backtype = net->backtype;
  float *lin_a = net->lin_a;
  float *lin_b = net->lin_b;
  int T = net->T;

  if (end_t > T) {
    ErrorExit(SYSTEM_ERR_EXIT, "Segment end too high in function ForwardSection.");
  } 
  if (start_t > end_t) {
    ErrorExit(SYSTEM_ERR_EXIT, "Segment start > segment end in function ForwardSection.");
  } 

  a = A + NumUnits * (net->MaxDelay + start_t);

  for (t = start_t; t < end_t; t++) { 
    /* This is the inner loop of the forward pass */
    for (i = 0; i < NumUnits; i++, a++) { 

      InW = net->InW[i];
      Inflows = net->Inflows[i];
      NumInflows = net->NumInflows[i];

      if (type[i] == INPUT) {
        if (t + Xlag[i] < 0 ) *a = 0.0;
        else if (t + Xlag[i] >= T) *a = 0.0;
        else *a = ExtData[t + Xlag[i]][Xindex[i]];
      }
      else if (type[i] == FILEFILTER || type[i] == ENVIRONMENT) {
        *a = net->Constants[Xindex[i]];
      }
      else if (type[i] == MULTIC) {
        for (j = NumInflows, *a = 1.0; j; j--, Inflows++) {
          *a *= a[*Inflows] * *(InW++);
        }
      }
      else {
        for (j = NumInflows, *a = 0.0; j; j--, Inflows++) {
          *a += a[*Inflows] * *(InW++);
        }
	/* initialize gradient vector with saturation cost */
	/* if (net->GA) *(ga++) = *a * *(sat_cost++); */
	/* TODO: 1) add sat_cost to Unit struct, 2) add it to the SetType command,
	         3) add it to the Net struct and 4) make Compile update it. */

        FORWARD_SIGMOID(*a, type[i]);
      }

      if (backtype[i] != NONOUTPUT && 
          (net->link_mode[i] == WRITE ||
           net->link_mode[i] == INTERACTIVE)) {
        if (t + Xlag[i] >= 0 && t + Xlag[i] < T) {
          ExtData[t + Xlag[i]][Xindex[i]] = 
            (*a + lin_a[Xindex[i]]) * lin_b[Xindex[i]];
        }
      }
    }
  }
}


void 
FinishForward(NetWork *net) {
  int i, j, t;
  float *a, *a_boundary;
  int t_boundary;
  int  *Inflows, NumInflows;
  float *InW;
  float **ExtData = net->ExtData;
  int NumUnits = net->NumUnits;
  int *Xindex = net->Xindex;
  int *Xlag = net->Xlag;
  int *type = net->type;
  int *backtype = net->backtype;
  float *lin_a = net->lin_a;
  float *lin_b = net->lin_b;
  int T = net->T;

  a = net->A + NumUnits * (net->MaxDelay + T);
  a_boundary = net->A + NumUnits * (2 * net->MaxDelay + T);
  t_boundary = net->MaxDelay + T;

  for (t = T; t < t_boundary; t++) { 
    for (i = 0; i < NumUnits; i++, a++) { 

      InW = net->InW[i];
      Inflows = net->Inflows[i];
      NumInflows = net->NumInflows[i];


      if (type[i] == INPUT) {
        if (t + Xlag[i] < 0 ) *a = 0.0;
        else if (t + Xlag[i] >= T) *a = 0.0;
        else *a = ExtData[t + Xlag[i]][Xindex[i]];
      }
      else if (type[i] == FILEFILTER || type[i] == ENVIRONMENT) {
        *a = net->Constants[Xindex[i]];
      }
      else if (type[i] == MULTIC) {
        for (j = NumInflows, *a = 1.0; j; j--, Inflows++) {
          if ((a + *Inflows) >= a_boundary) InW++;
          else *a *= a[*Inflows] * *(InW++);
        }
      }
      else {
        for (j = NumInflows, *a = 0.0; j; j--, Inflows++) {
          if ((a + *Inflows) >= a_boundary) InW++;
          else *a += a[*Inflows] * *(InW++);
        }
	/* initialize gradient vector with saturation cost */
	/* if (net->GA) ... */

        FORWARD_SIGMOID(*a, type[i]);
      }

      if (backtype[i] != NONOUTPUT && 
           (net->link_mode[i] == WRITE ||
            net->link_mode[i] == INTERACTIVE)) {
        if (t + Xlag[i] >= 0 && t + Xlag[i] < T) {
          ExtData[t + Xlag[i]][Xindex[i]] = 
            (*a + lin_a[Xindex[i]]) * lin_b[Xindex[i]];
        }
      }
    } 
  }
  net->excited = 1;
}


void 
Forward(NetWork *net) {
  InitForward(net);
  ForwardSection(net, 0, net->T);
  FinishForward(net);
}


int nicosim_big_gradient_warning = 0;


void 
InitBackward(NetWork *net) {
  /* Start with an allocated and zero-initialized gradient vector */
  if (net->GA) FREE(net->GA);
  CALLOC(net->GA, net->NumUnits * (net->T + 2 * net->MaxDelay), float);
  nicosim_big_gradient_warning = 0;
}


float 
BackwardSection(NetWork *net, int start_t, int end_t) {
  int   i, j, t;
  unsigned long PrunedU = 0;
  unsigned long PrunedC = 0;
  int *Inflows, NumInflows;
  float *InW, *InGW;
  float *a, f, *ga;
  float E = 0.0;
  int T = net->T;
  float **Output = net->ExtData; 
  int NumUnits = net->NumUnits;
  int MaxDelay = net->MaxDelay; 
  int *type = net->type;
  int *backtype = net->backtype;
  int *Xindex = net->Xindex;
  int *Xlag = net->Xlag;

  if (end_t > T) {
    ErrorExit(SYSTEM_ERR_EXIT, "Segment end too high in function BackwardSection.");
  } 
  if (start_t > end_t) {
    ErrorExit(SYSTEM_ERR_EXIT, "Segment start > segment end in function BackwardSection.");
  } 

  a = net->A + (end_t + MaxDelay) * NumUnits - 1;
  ga = net->GA + (end_t + MaxDelay) * NumUnits - 1;

  for (t = end_t - 1; t >= start_t; t--) { 
    for (j = NumUnits - 1; j >= 0; j--, a--, ga--) {

      if (backtype[j] != NONOUTPUT &&
        /* Don't propagate error back if you don't know the target */
        t + Xlag[j] >= 0 && t + Xlag[j] < T) {

        f = Output[t + Xlag[j]][Xindex[j]];

        INCREMENT_GRADIENT(ga, *a, f, E, backtype[j]);
      } 

      BACKWARD_SIGMOID(ga, *a, type[j]);

      /* seat-belt */
      if (*ga > 10) {
        *ga = 10;
	if (!nicosim_big_gradient_warning) {
	  EmitWarning("Large unit gradient (>10.0)");
	}
        nicosim_big_gradient_warning = 1;
      }

      if (fabs(*ga) > net->backthresh[j]) {
        InW = net->InW[j];
        InGW = net->InGW[j];
        Inflows = net->Inflows[j];
        NumInflows = net->NumInflows[j];

        /* ---  This is the inner loop of the backward pass  --- */

        if (type[j] == MULTIC) {
          float tmp;
          for (i = NumInflows; i; i--, Inflows++) {
            tmp = *a * (*ga);
            ga[*Inflows] += tmp / a[*Inflows];
            InW++;
          }
        }
        else {
          for (i = NumInflows; i; i--, Inflows++) {
            *(InGW++) += a[*Inflows] * (*ga);
            ga[*Inflows] += *(InW++) * (*ga);
          }
        }
      }
      else {
        *ga = 0.0;
        PrunedU++;
        PrunedC += net->NumInflows[j];
      }
    }  
  }
  ProcessedUnits +=  (float)(end_t - start_t);
  ProcessedConnections +=  (float)(end_t - start_t);
  PrunedUnits += (float)PrunedU / (float)NumUnits;
  PrunedConnections += (float)PrunedC / (float)net->NumConnections;
  
  return E;
}


float 
BackwardSectionAndUpdate(NetWork *net, int start_t, int end_t) {
  int   i, j, t;
  unsigned long PrunedU = 0;
  unsigned long PrunedC = 0;
  int *Inflows, NumInflows;
  float *InW;
  float *Gain;
  float *Momentum;
  float *DeltaW;
  float *a, f, *ga;
  float E=0.0;
  int T = net->T;
  float **Output = net->ExtData; 
  int NumUnits = net->NumUnits;
  int MaxDelay = net->MaxDelay; 
  int *type = net->type;
  int *backtype = net->backtype;
  int *Xindex = net->Xindex;
  int *Xlag = net->Xlag;

  if (end_t > T) {
    ErrorExit(SYSTEM_ERR_EXIT, 
       "Segment end too high in function BackwardSectionAndUpdate.");
  } 
  if (start_t > end_t) {
    ErrorExit(SYSTEM_ERR_EXIT,
       "Segment start > segment end in function BackwardSectionAndUpdate.");
  }

  a = net->A + (end_t + MaxDelay) * NumUnits - 1;
  ga = net->GA + (end_t + MaxDelay) * NumUnits - 1;

  for (t = end_t - 1; t >= start_t; t--) { 
    for (j = NumUnits - 1; j >= 0; j--, a--, ga--) {

      if (backtype[j] != NONOUTPUT &&
        /* Don't propagate error back if you don't know the target */
        t + Xlag[j] >= 0 && t + Xlag[j] < T) {

        f = Output[t + Xlag[j]][Xindex[j]];

        INCREMENT_GRADIENT(ga, *a, f, E, backtype[j]);
      } 

      BACKWARD_SIGMOID(ga, *a, type[j]);

      /* seat-belt */
      if (*ga > 10) {
        *ga = 10;
	if (!nicosim_big_gradient_warning) {
	  EmitWarning("Large unit gradient (>10.0)");
	}
        nicosim_big_gradient_warning = 1;
      }

      if (fabs(*ga) > net->backthresh[j]) {
        InW = net->InW[j];
        Inflows = net->Inflows[j];
        NumInflows = net->NumInflows[j];

        Gain = net->Gain[j];
        Momentum = net->Momentum[j];
        DeltaW = net->DeltaW[j];

        /* ---  This is the inner loop of the backward pass  --- */

        if (type[j] == MULTIC) {
          float tmp;
          for (i = NumInflows; i; i--, Inflows++) {
            tmp = *a * (*ga);
            ga[*Inflows] += tmp / a[*Inflows];
            InW++;
          }
        }
        else {
          for (i = NumInflows; i; i--, Inflows++) {
            if (*Gain == 0.0) {
	      Gain++;
	      Momentum++;
	      InW++;
	      DeltaW++;
	    }
            else {
	      *DeltaW = (a[*Inflows] * (*ga)) * (*(Gain++)) 
		+ (*(Momentum++)) * (*DeltaW);
	      ga[*Inflows] += *(InW) * (*ga);
	      *(InW++) += *(DeltaW++);
	    }
          }
        }

      }
      else {
        *ga = 0.0;
        PrunedU++;
        PrunedC += net->NumInflows[j];
      }
    }
  }
  ProcessedUnits +=  (float)(end_t - start_t);
  ProcessedConnections +=  (float)(end_t - start_t);
  PrunedUnits += (float)PrunedU / (float)NumUnits;
  PrunedConnections += (float)PrunedC / (float)net->NumConnections;
  
  return E;
}


float 
FinishBackward(NetWork *net) {
  int i, j, t;
  int *Inflows, NumInflows;
  float *InW, *InGW;
  float *a, *ga;
  float E = 0.0;
  int NumUnits = net->NumUnits;
  int MaxDelay = net->MaxDelay; 
  int *type = net->type;

  a = net->A + MaxDelay * NumUnits - 1;
  ga = net->GA + MaxDelay * NumUnits - 1;

  for (t = MaxDelay - 1; t >= 0; t--) { 
    for (j = NumUnits - 1; j >= 0; j--, a--, ga--) {
      InW = net->InW[j];
      InGW = net->InGW[j];
      Inflows = net->Inflows[j];
      NumInflows = net->NumInflows[j];

      if (type[j] == MULTIC) {
        float tmp;
        for (i = NumInflows; i; i--, Inflows++) {
          if ( (a + *Inflows) < net->A  ) continue;
          tmp = *a * (*ga);
          ga[*Inflows] += tmp / a[*Inflows];
          InW++;
        }
      }
      else {
        for (i = NumInflows; i; i--, Inflows++) {
          if ( (a + *Inflows) < net->A  ) continue;
          *(InGW++) += a[*Inflows] * (*ga);
          ga[*Inflows] += *(InW++) * (*ga);
        }
      }
    }  
  }

  return E;
}


float 
Backward(NetWork *net) {
  float E = 0.0;

  if (net->excited != 1) {
    ErrorExit(SYSTEM_ERR_EXIT, "Cannot perform backward phase - network is not excited.");
  }
  InitBackward(net);
  E += BackwardSection(net, 0, net->T);
  E += FinishBackward(net);

  return E;
}


float 
GetGlobalError(NetWork *net) {
  int   j, t;
  float *a, f, *ga;
  float E = 0.0;
  int T = net->T;
  float **Output = net->ExtData; 
  int NumUnits = net->NumUnits;
  int MaxDelay = net->MaxDelay; 
  int *backtype = net->backtype;
  int *Xindex = net->Xindex;
  int *Xlag = net->Xlag;

  InitBackward(net);

  a = net->A + (T + MaxDelay) * NumUnits - 1;
  ga = net->GA + (T + MaxDelay) * NumUnits - 1;

  for (t = T - 1; t >= 0; t--) { 
    for (j = NumUnits - 1; j >= 0; j--, a--, ga--) {

      if (backtype[j] != NONOUTPUT &&
	/* Don't propagate error back if you don't know the target */
	t + Xlag[j] >= 0 && t + Xlag[j] < T) {

        f = Output[t + Xlag[j]][Xindex[j]];

        INCREMENT_GRADIENT(ga, *a, f, E, backtype[j]);
      }
    }
  }

  return E;
}


void 
FixGA(NetWork *net) {
/* Replaces the derivatives of 'net' in GA with derivatives of the units. */
  int  t, j;
  int NumUnits = net->NumUnits;
  float *a = net->A  + NumUnits * net->MaxDelay;
  float *ga = net->GA + NumUnits * net->MaxDelay;
  int *type = net->type;

  for (t = 0; t < net->T; t++) {
    for (j = 0; j < NumUnits; j++, ga++, a++) {
      switch (type[j]) {
        case TANHYP:
        *ga /= (1.0 + *a) * (1.0 - (*a));
        break;
        case SIGMOID:
        *ga /= (*a) * (1.0 - (*a));
        break;
        case INVERTER:
        *ga /= - (*a) * (*a);
        break;
        case EXPONENTIAL:
        *ga /= exp(*a);
        break;
        case ARCTAN:
        (*ga) /= 1.0 / tan(1.0 + (*a) * (*a)) / PIHALF;
        break;
      }
    }
  }
}


/* ------------------------ Error functions -------------------------------- */
/* The first 2 parameters to an error function are the activation- and target*/
/* values respectively. The third parameter is a pointer to the total error. */
/* The function returns the derivative of the unit with respect to E.        */


float 
ZeroOne(float x, float y, float *E) {
/* This is the usual Euclidean error measure, */
/* but modified close to 1 and 0.              */
  float e;

  e = y - x;
  if (e < 0.0 && y >= 0.5) e = 0.0;
  else if (e > 0.0 && y <= 0.5) e = 0.0;
  else *E += e * e / 2.0;

  return e;
}


float 
SymZeroOne(float x, float y, float *E) {
/* The usual Euclidean error measure, */
/* but modified close to 1 and 0.     */
  float e;

  e = y - x;
  if (e < 0.0 && y >= 0.0) e = 0.0;
  else if (e > 0.0 && y <= 0.0) e = 0.0;
  else *E += e * e / 2.0;

  return e;
}


float 
L2(float x, float y, float *E) {
/* This is the L2 E-measure. */
  float e;

  e = y - x;
  *E += e * e / 2.0;

  return e;
}

float 
L4(float x, float y, float *E) {
/* This is the L4 E-measure. */
  float e, ee;

  e = y - x;
  ee = e * e;
  *E += ee * ee / 4.0;

  return ee * e;
}


float 
L10(float x, float y, float *E) {
/* This is the L10 E-measure. */
  float e, ee;

  e = y - x;
  ee = e * e * e;
  ee = ee * ee * ee;
  *E += ee * e / 10.0;

  return ee;
}


float 
L0(float x, float y, float *E) {
/* This is an absolute E-measure. */
  float e, ee;

  e = y - x;
  ee = fabs(y - x);
  *E += ee;

  return e/ee;
}


float 
SymCross(float x, float y, float *E) {
  /* This is the cross-entropy E-measure modified for [-1, 1] output. */
  float yy = (y + 1.0) / 2.0;
  float xx = (x + 1.0) / 2.0;

  if (yy == 0.0) {
    *E -= SafeLog(1.0 - xx);
    return 1.0 / (xx - 1.0);
  }
  else if (yy == 1.0) {
    *E -= SafeLog(xx);
    return 1.0 / xx;
  }
  else {
    *E -= yy * SafeLog(xx) + (1.0 - yy) * SafeLog(1.0 - xx);
    return  yy / xx - (1.0 - yy) / (1.0 - xx);
  }
}


float 
Cross(float x, float y, float *E) {
  /* This is the cross entropy E-measure */
  if (y == 0.0) {
    *E -= SafeLog(1.0 - x);
    return 1.0 / (x - 1.0);
  }
  else if (y == 1.0) {
    *E -= SafeLog(x);
    return 1.0 / x;
  }
  else {
    *E -= y * SafeLog(x) + (1.0 - y) * SafeLog(1.0 - x);
    return  y / x - (1.0 - y) / (1.0 - x);
  }
}


float 
SymCross01(float x, float y, float *E) {
  /* Modified close to 1 and 0.     */
  /* This is the cross-entropy E-measure modified for [-1, 1] output. */
  float yy = (y + 1.0) / 2.0;
  float xx = (x + 1.0) / 2.0;

  float e = yy - xx;
  if (e < 0.0 && yy >= 0.5) return 0.0;
  else if (e > 0.0 && yy <= 0.5) return 0.0;
  else {
    if (yy == 0.0) {
      *E -= SafeLog(1.0 - xx);
      return 1.0 / (xx - 1.0);
    }
    else if (yy == 1.0) {
      *E -= SafeLog(xx);
      return 1.0 / xx;
    }
    else {
      *E -= yy * SafeLog(xx) + (1.0 - yy) * SafeLog(1.0 - xx);
      return  yy / xx - (1.0 - yy) / (1.0 - xx);
    }
  }
}


float 
Cross01(float x, float y, float *E) {
  /* Modified close to 1 and 0.     */
  /* This is the cross entropy E-measure */
  float e = y - x;
  if (e < 0.0 && y >= 0.5) return 0.0;
  else if (e > 0.0 && y <= 0.5) return 0.0;
  else {
    if (y == 0.0) {
      *E -= SafeLog(1.0 - x);
      return 1.0 / (x - 1.0);
    }
    else if (y == 1.0) {
      *E -= SafeLog(x);
      return 1.0 / x;
    }
    else {
      *E -= y * SafeLog(x) + (1.0 - y) * SafeLog(1.0 - x);
      return  y / x - (1.0 - y) / (1.0 - x);
    }
  }
}


float 
NoError(float x, float y, float *E) {
  return 0.0;
}

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  Simulation.c                                     */
/* --------------------------------------------------------------------------*/

