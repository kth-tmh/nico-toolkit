/* ------------------------------------------------------------------------- */
/*               Filter style forward evaluation of RTDNN                    */
/* ------------------------------------------------------------------------- */
/*                                                                           */
/*   Copyright (C) 1997, 1998 Nikko Strom                                    */
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

#ifndef RTSIM_H_LOADED
#define RTSIM_H_LOADED

#include <stdio.h> 
#include "RTDNN.h" 
#include "Simulation.h" 

typedef struct RTSimulator {
  NetWork *net;

  float **ExtData;
  float *A;

  /* -- counts the successful calls to RTSimInput and RTSimOutput resp. --*/
  int num_input, num_output;

  /* -- set if there is no more input -- */
  int stopped;

  int buf_size; 
  int num_units;
  int max_delay;

  int num_input_streams;
  float **input_stream; /* write data pointers */
  int *input_stream_size;
  char **input_stream_name;
  char ***input_component_name;

  int num_output_streams;
  float **output_stream; /* read data pointers */
  int *output_stream_size;
  char **output_stream_name;
  char ***output_component_name;

  int big_data_warning;
} RTSimulator;



/* intializes an RTSim struct */
RTSimulator *CompileRTSim(Net *ann);

/* Intializes an RTSim struct */
RTSimulator *MakeRTSim(NetWork *net);


/* returns the sizes of the input streams */
/* WARNING: do not alter the values in the returned vectors */
void RTSimInputSize(RTSimulator *work, int *num_streams, int **stream_sizes);


/* Get the vectors to write input data in */
/* ( *data[stream][index] ) */
float **GetRTSimInputVectors(RTSimulator *work);


/* returns the sizes of the output streams */
/* WARNING: do not alter the values in the returned vectors */
void RTSimOutputSize(RTSimulator *work, int *num_streams, int **stream_sizes);


/* Get the vectors to read output data from */
/* ( *data[stream][index] ) */
float **GetRTSimOutputVectors(RTSimulator *work);


/* initialize the RTSim struct */
/* "file_name" is used only for the act's of FILEFILTER units */
void RTSimInitialize(RTSimulator *work, char *file_name);


/* returns non-zero iff the RTSim struct is ready for more input */
int RTSimCheckInput(RTSimulator *work);


/* excite the network with the input stream values in "input" */
int RTSimInput(RTSimulator *work);


/* signal that there is no more input */
void RTSimInputEnd(RTSimulator *work);


/* returns non-zero iff there are output waiting to be fetched by RTSimOutput */
int RTSimCheckOutput(RTSimulator *work);


/* fills the (already allocated) array "output" with the next output
   from the output streams of the network */
int RTSimOutput(RTSimulator *work);


/* returns the number of frames returned by RTSimOutput after the last 
   call to RTSimInitialize */
int RTSimTime(RTSimulator *work);


/* frees memory allocated by an RTSim struct
 * NOTE however that the actual NetWork is not freed
 * this is to allow different RTSimulators to share a network */
void FreeRTSim(RTSimulator *work);

#endif

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  RTSim.h                                          */
/* --------------------------------------------------------------------------*/

