/* ------------------------------------------------------------------------- */
/*               Filter style forward evaluation of RTDNN                    */
/* ------------------------------------------------------------------------- */
/*                                                                           */
/*   Copyright (C) 1997, 1998 Nikko Ström                                    */
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

#include <stdlib.h> 
#include <stdio.h> 
#include <math.h>  
#include <string.h>  
#include <ctype.h>  
#include <unistd.h>
#include "System.h"
#include "RTDNN.h" 
#include "Simulation.h" 
#include "RTSim.h" 

RTSimulator *CompileRTSim(Net *ann) {
  NetWork *net;
  int i;

  /* change target mode to "interactive" */
  for (i = 0; i < ann->NumStreams; i++) {
    if (ann->StrTab[i]->type == TARGET) ann->StrTab[i]->type = INTERACTIVE;
  }

  net = Compile(ann);
  return MakeRTSim(net);
}


RTSimulator *MakeRTSim(NetWork *net) {
  RTSimulator *work;
  int i, j, k;
  int type, num_out, num_inp;

  CALLOC(work, 1, RTSimulator);
  work->net = net;

  /* get the sizes of the arrays */
  work->num_units = net->NumUnits;
  work->max_delay = net->MaxDelay;

  work->buf_size = work->max_delay + 2;


  net->excited = 0;
  net->has_data = 0;
  net->T = work->buf_size;


  /* count the input- and output streams */
  work->num_input_streams = work->num_output_streams = 0;
  for (i = 0; i < net->NumStreams; i++) {
    int type;

    type = net->stream_mode[i];
    if (type == WRITE || type == INTERACTIVE) 
      work->num_output_streams++;
    if (type == READ) 
      work->num_input_streams++;
  }
  
  /* allocate arrays */
  CALLOC(work->input_stream_size, work->num_input_streams, int);
  CALLOC(work->input_stream_name, work->num_input_streams, char *);
  CALLOC(work->input_component_name, work->num_input_streams, char **);

  CALLOC(work->output_stream_size, work->num_output_streams, int);
  CALLOC(work->output_stream_name, work->num_output_streams, char *);
  CALLOC(work->output_component_name, work->num_output_streams, char **);


  /* fill in stream info. */
  work->num_input_streams = work->num_output_streams = 0;
  for (i = 0; i < net->NumStreams; i++) {
    int type;

    type = net->stream_mode[i];

    if (type == WRITE || type == INTERACTIVE) {
      k = work->num_output_streams;

      work->output_stream_name[k] = net->stream_name[i];
      work->output_stream_size[k] = net->stream_size[i];
      CALLOC(work->output_component_name[k], net->stream_size[i], char *);

      for (j = 0; j < net->stream_size[i]; j++) {
	work->output_component_name[k][j] =
	  net->stream_comp_name[net->stream_offset[i] +j];
      }
      work->num_output_streams++;
    }
    else if (type == READ) {
      k = work->num_input_streams;

      work->input_stream_name[k] = net->stream_name[i];
      work->input_stream_size[k] = net->stream_size[i];
      CALLOC(work->input_component_name[k], net->stream_size[i], char *);

      for (j = 0; j < net->stream_size[i]; j++) {
	work->input_component_name[k][j] =
	  net->stream_comp_name[net->stream_offset[i] + j];
      }

      work->num_input_streams++;
    }
  }


  CALLOC2D(work->ExtData, work->buf_size, net->NumExt, float);
  CALLOC(work->A, net->NumUnits * (net->T + 2 * net->MaxDelay), float);

  /* set the pointers to the input/output */
  CALLOC(work->input_stream, work->num_input_streams, float *);
  CALLOC(work->output_stream, work->num_output_streams, float *);

  for (i = 0, num_out = num_inp = 0; i < net->NumStreams; i++) {
    type = net->stream_mode[i];

    if (type == WRITE || type == INTERACTIVE) {
      work->output_stream[num_out++] = 
	work->ExtData[0] + net->stream_offset[i];
    }
    else if (type == READ) {
      work->input_stream[num_inp++] = 
	work->ExtData[net->MaxDelay + 1] + net->stream_offset[i];
    }
  }

  return work;
}


void 
RTSimInputSize(RTSimulator *work, int *num_streams, int **stream_sizes) {
  *num_streams = work->num_input_streams;
  *stream_sizes = work->input_stream_size;
}


float **GetRTSimInputVectors(RTSimulator *work) {
  return work->input_stream;
}


void 
RTSimOutputSize(RTSimulator *work, int *num_streams, int **stream_sizes) {
  *num_streams = work->num_output_streams;
  *stream_sizes = work->output_stream_size;
}


float **GetRTSimOutputVectors(RTSimulator *work) {
  return work->output_stream;
}


void 
RTSimInitialize(RTSimulator *work, char *file_name) {
  NetWork *net = work->net;
  int i;
  char *value, *ptr;
  float activity;

  work->stopped = 0;
  work->num_input = 0;
  work->num_output = 0;
  work->big_data_warning = 0;

  /* filefilter and environment units not handled yet (to do) */
}


int 
RTSimCheckInput(RTSimulator *work) {
  if (work->stopped) return 0;
  if (work->num_input - work->num_output >= work->max_delay) return 0;

  return 1;
}


int 
RTSimCheckOutput(RTSimulator *work) {
  if (work->stopped && work->num_input > work->num_output) return 1;
  if (work->num_input - work->num_output >= work->max_delay) return 1;

  return 0;
}


static void 
shift_forward(RTSimulator *work) {
  NetWork *net = work->net;
  float *tmp;
  int i;

  /* move the act array */
  memmove(work->A, work->A + net->NumUnits,
	  net->NumUnits * (net->T + 1 * net->MaxDelay) * sizeof(float));

  /* move the contents to the first ext-data vector */ 
  memcpy(work->ExtData[0], work->ExtData[1], net->NumExt * sizeof(float));

  /* shift all but the first and last ext-data vectors */
  tmp = work->ExtData[1];
  for (i = 1; i < net->MaxDelay; i++) {
    work->ExtData[i] = work->ExtData[i + 1];
  }
  work->ExtData[net->MaxDelay] = tmp;

  /* move the contents from the last ext-data vector */ 
  memcpy(work->ExtData[net->MaxDelay], work->ExtData[net->MaxDelay + 1],
	 net->NumExt * sizeof(float));
}


static void 
compute_acts(RTSimulator *work) {
  NetWork *net = work->net;
  int i, j;

  for (i = 0; i < net->NumStreams; i++) {
    if (net->stream_mode[i] == READ) {

      /* Linearly transform input */
      for (j = 0; j < net->stream_size[i]; j++) { 
	if (fabs(work->ExtData[net->MaxDelay + 1][net->stream_offset[i] + j] = 
		 work->ExtData[net->MaxDelay + 1][net->stream_offset[i] + j] / 
		 net->lin_b[net->stream_offset[i] + j] - 
		 net->lin_a[net->stream_offset[i] + j]) > 10.0)
	  work->big_data_warning = 1;
      }
    }
  }

  ForwardSection_RT(net, net->MaxDelay + 1, net->MaxDelay + 2,
		  work->A, work->ExtData);

  shift_forward(work);
}


int 
RTSimInput(RTSimulator *work) {
  if (!RTSimCheckInput(work)) return 1;

  compute_acts(work);

  work->num_input++;
  return 0;
}


void 
RTSimInputEnd(RTSimulator *work) {
  work->stopped = 1;
}


int 
RTSimOutput(RTSimulator *work) {
  if (!RTSimCheckOutput(work)) return 1;

  if (work->stopped) {
    compute_acts(work);
  }

  work->num_output++;
  return 0;
}


int 
RTSimTime(RTSimulator *work) {
  return work->num_output;
}


/* NOTE that the actual NetWork is NOT freed.
 * This is because it may be shared between several simularors. */
void 
FreeRTSim(RTSimulator *work) {
  int i;

  for (i = 0; i < work->num_input_streams; i++) {
    FREE(work->input_component_name[i]);
  }
  for (i = 0; i < work->num_output_streams; i++) {
    FREE(work->output_component_name[i]);
  }

  FREE(work->input_stream);
  FREE(work->output_stream);
  FREE(work->input_stream_size);
  FREE(work->output_stream_size);
  FREE(work->input_stream_name);
  FREE(work->output_stream_name);

  FREE2D(work->ExtData, work->buf_size);
  FREE(work->A);

  FREE(work);
}


/* --------------------------------------------------------------------------*/
/*           END OF FILE :  RTSim.c                                          */
/* --------------------------------------------------------------------------*/

