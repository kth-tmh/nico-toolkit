/* ------------------------------------------------------------------------- */
/*               Forward and Backward evaluation of the RTDNN                */
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

#ifndef ANN_SIMULATION_LOADED
#define ANN_SIMULATION_LOADED

#include "RTDNN.h"
#include "ParamData.h"

#define PIHALF 1.570796326694896

/*  This is the "hard" version of the network. It's better suited for this  */
/*  simulation module than the "Net"-struct.                                */ 
typedef struct NetWork {
  /* My size... */
  int    NumUnits;
  int    NumConnections;
  int    NumExt;
  int    NumStreams;
  int    MaxDelay;

  /* The state... */
  int    excited;
  int    has_data;

  /* The streams... */
  float      *lin_a, *lin_b;
  char      **stream_comp_name;
  ParamFileType *stream_format; /* 'ParamFileType' is defined in ParamData.h */
  char      **stream_dir;
  char      **stream_ext;
  StreamType *stream_mode;   /* 'StreamType' is defined in RTDNN.h */
  int        *stream_offset;
  int        *stream_size;
  char      **stream_name;
  char      **stream_filter;

  /* The constants... */
  int NumConstants;
  float *Constants;
  char **ConstName;

  /* The current data... */
  char   *CurrentData;  /* Name of the data (typically a filename) */
  int     T;            /* Num of frames */
  float **ExtData;      /* First index is the time (in frames) */
  float  *A;            /* Activations (time * num_units + unit_index) */
  float  *GA;           /* Error Gradient of 'A' */

  /* For each unit... */
  int    *type;
  int    *backtype;
  int    *Xindex;
  int    *Xlag;
  StreamType *link_mode;   /* 'StreamType' is defined in RTDNN.h */
  float  *backthresh;
  int    *NumInflows; 
  /* For 2D-arrays, the first index is the unit of inflow */
  int   **Inflows; 
  float **InW;
  float **InGW;
  float **Gain;
  float **Momentum;
  float **DeltaW;
  float **Plast; 
  Connection ***ConLink; 

} NetWork;



/* This function sets up the "hard" version of the network (A "NetWork")   */
/*             for forward and backward evaluation.                        */ 
NetWork *Compile(Net *net);

/* Free the "hard" version of the network */
void FreeNetWork(NetWork *);

/* If all_flag is set, update all con's, otherwize update marked con's only */
void UpdateNet(Net *, NetWork *, int all_flag);

/*   Initialize training parameters for BackPropagation training  */
void InitGainAndMomentum(NetWork *net, float gain, float momentum);

/*  Take a gradient descent step including momentum */
void GradStep(Net *ann, NetWork *net, int all_flag);

/*  Perform weight decay on connections  */
void WeightDecay(Net *ann, NetWork *net, float factor, int all_flag);

/* Loading and freeing the array of external data - inputs and targets */
void LoadExtData(NetWork *, char *Name, int load_targets, int check_range);
void FreeExtData(NetWork *);

/* Forward simulation... */
void InitForward(NetWork *);
void Forward(NetWork *);
void ForwardSection(NetWork *, int start_t, int end_t);
void ForwardSection_RT(NetWork *, int start_t, int end_t,
		       float *A, float **ExtData);
void FinishForward(NetWork *);

/* Backward simulation... */
void  InitBackward(NetWork *);
float Backward(NetWork *);
float BackwardSection(NetWork *, int start_t, int end_t);
float BackwardSectionAndUpdate(NetWork *net, int start_t, int end_t);
float FinishBackward(NetWork *);

/* Do only the error computation of output units of the backward phase */ 
float GetGlobalError(NetWork *);

/* Change the net->GA array to show the gradient with respect to the  */
/* activities of the units (instead of the "net" inputs to them).     */ 
void  FixGA(NetWork *net);

/* Restart pruning-stat's collection */
void ResetPruneStats();

/* On return, the arguments hold per cent pruned units and connections in
   the backward phase */
void GetPruneStats(float *backward_units, float *backward_connections);


/* ------------------------ Error functions -------------------------------- */
/* The first 2 parameters to an error function are the activation- and target*/
/* values respectively. The third parameter is a pointer to the total error. */

float L2(float x, float y, float *E);
float L4(float x, float y, float *E);
float L10(float x, float y, float *E);
float L0(float x, float y, float *E);
float SymCross(float x, float y, float *E);
float SymCross01(float x, float y, float *E);
float Cross(float x, float y, float *E);
float Cross01(float x, float y, float *E);
float ZeroOne(float x, float y, float *E);
float SymZeroOne(float x, float y, float *E);
float NoError(float x, float y, float *E);

#endif

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  Simulation.h                                     */
/* --------------------------------------------------------------------------*/

