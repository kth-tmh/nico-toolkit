/* ------------------------------------------------------------------------- */
/*                          Speech feature extraction                        */
/* ------------------------------------------------------------------------- */
/*                                                                           */
/*   Copyright (C) 1996, 1997, 1998 Nikko Strom                              */
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
#include "System.h"
#include "Signal.h"
#include "Speetures.h"
#include "Math.h"

/* ------------------  Bark Filter-bank implementation  -------------------- */

typedef struct FibStruct {
  int num_filters;
  int analyze_size;
  int fft_size;
  FFTInitializer *fft_init;
  float *hamming_window;
  int *cut_fft_points; 
  float *shapes;
} FibStruct;


static void 
GetBarkCuts(Speetures *S) {
  FibStruct *info = S->info;
  double freq_scale = (double)S->samp_freq / info->fft_size;/* Herz/fft-point */
  double bark, bark_floor, bark_step;
  int i, j;

  /* Compute the cut-frequencies and the respective fft-points */
  CALLOC(info->cut_fft_points, info->num_filters + 2, int);

  bark_floor = Bark((double) S->low_cut);
  bark_step = (Bark((double) S->high_cut) - bark_floor) / 
                 (double)(info->num_filters + 1.0);
  for (bark = bark_floor, i = 0 ; i < info->num_filters + 2; 
       bark += bark_step, i++) {
    info->cut_fft_points[i] = 
       (int)floor(0.01 + InvBark(bark) / freq_scale);

    if (info->cut_fft_points[i] >= info->fft_size)
      info->cut_fft_points[i] = info->cut_fft_points[i] - 1;
  }

  /* Set up the shapes array for the triangular filters */
  CALLOC(info->shapes, info->fft_size, float);

  for (i = 0; i < info->num_filters + 1; i++) {
    for (j = info->cut_fft_points[i]; j < info->cut_fft_points[i + 1]; j++) {

      info->shapes[j] = (float)
          (Bark((double) j * freq_scale) - 
           Bark((double) info->cut_fft_points[i] * 
                                  freq_scale)) /
          (Bark((double) info->cut_fft_points[i + 1] * freq_scale) - 
                         Bark((double) info->cut_fft_points[i] * freq_scale));
    }
  }
}


static void 
GetMelCuts(Speetures *S) {
  FibStruct *info = S->info;
  double mel, mel_floor, mel_step;
  int i, j;
  /* Herz/fft-point */
  double freq_scale = (double)S->samp_freq / info->fft_size;/* Herz/fft-point */

  /* Compute the cut-frequencies and the respective fft-points */
  CALLOC(info->cut_fft_points, info->num_filters + 2, int);

  mel_floor = Mel((double) S->low_cut);
  mel_step = (Mel((double) S->high_cut) - mel_floor) / 
                 (double)(info->num_filters + 1.0);

  for (mel = mel_floor, i = 0 ; i < info->num_filters + 2; 
       mel += mel_step, i++) {
    info->cut_fft_points[i] = 
       (int)floor(0.01 + InvMel((double) mel) / freq_scale);

    if (info->cut_fft_points[i] >= info->fft_size)
      info->cut_fft_points[i] = info->cut_fft_points[i] - 1;
  }

  /* Set up the shapes array for the triangular filters */
  CALLOC(info->shapes, info->fft_size, float);

  for (i = 0; i < info->num_filters + 1; i++) {
    for (j = info->cut_fft_points[i]; j < info->cut_fft_points[i + 1]; j++) {

      info->shapes[j] = (float)
          (Mel((double) j * freq_scale) - 
             Mel((double) info->cut_fft_points[i] * freq_scale))
                   /
          (Mel((double) info->cut_fft_points[i + 1] * freq_scale) - 
             Mel((double) info->cut_fft_points[i] * freq_scale));
    }
  }
}


static void 
InitFilterbank1(Speetures *S) {
  FibStruct *info;

  CALLOC(info, 1, FibStruct);
  S->info = (void *)info;

  info->num_filters = S->base_size;
  info->analyze_size = S->frame_window_size;
  info->fft_size = S->allocated_window_size;

  /* Initialize the FFT842 routine */
  info->fft_init = fft842init(REALFFT, info->analyze_size);

  /* Get the hamming window */
  info->hamming_window = InitHamming(info->analyze_size);
}


static void 
InitBarkFib(Speetures *S) {
  InitFilterbank1(S);
  GetBarkCuts(S);
}


static void 
InitMelFib(Speetures *S) {
  InitFilterbank1(S);
  GetMelCuts(S);
}


static float *
FilterbankFeatures(Speetures *S, float *Energy) {
  int i, j;
  int fft_point, next_cut;
  float sum1, sum2, d;
  float *fib;
  float E;

  FibStruct *info =     (FibStruct *)S->info;
  float *data =         S->frame_window;

  int fft_size =        info->fft_size;
  int analyze_size =    info->analyze_size; 
  int *cut_fft_points = info->cut_fft_points;
  float *shapes =       info->shapes;

  /* Remove the DC component */
  RemoveDC(data, analyze_size);

  /* Get log energy if reqiured */  
  if (S->base_type & ENERGY_FEATURES) {
    E = GetEnergy(data, analyze_size);

    E = SafeLog((double)E);
    *Energy = E;
  }

  /* Apply pre-emphasis on the data */
  PreEmphasis(data, analyze_size, S->pre_emp_factor);

  /* Hamming window the data */
  ApplyHamming(data, info->hamming_window, fft_size, analyze_size); 

  /* This is the FFT */
  power842(data, info->fft_init);

  /* Necessary to allocate at least base_size */
  if (S->frame_size < S->base_size) i = S->base_size; else i = S->frame_size;
  CALLOC(fib, i, float);

  /* Initialize sum1 for first filter */
  sum1 = 0.0;
  next_cut = cut_fft_points[1];
  for (fft_point = cut_fft_points[0]; fft_point < next_cut; fft_point++) {
    d = data[fft_point];
    d = sqrt(d);
    sum1 += d * shapes[fft_point];
  }

  for (i = 0; i < info->num_filters - 1; i++) {
    sum2 = sum1;
    sum1 = 0.0;
    next_cut = cut_fft_points[i + 2];

    for (fft_point = cut_fft_points[i + 1]; fft_point < next_cut; fft_point++) {
      d = data[fft_point];
      d = sqrt(d);
      sum2 += d * (1.0 - shapes[fft_point]);
      sum1 += d * shapes[fft_point];
    }

    fib[i] = SafeLog((double)sum2);
  }

  /* Take care of the last filter */
  sum2 = sum1;
  next_cut = cut_fft_points[info->num_filters + 1]; /* Last cut */

  for (fft_point = cut_fft_points[info->num_filters]; 
       fft_point < next_cut; fft_point++) {
    d = data[fft_point]; 
    d = sqrt(d);
    sum2 += d * (1.0 - shapes[fft_point]);
  }

  j = info->num_filters - 1; /* fib index */
  fib[j] = SafeLog((double)sum2);

  return fib;
}


static void 
FreeFibStruct(FibStruct *b) {
  if (b->hamming_window) FREE(b->hamming_window);
  if (b->cut_fft_points) FREE(b->cut_fft_points);
  if (b->shapes) FREE(b->shapes);

  if (b->fft_init) FreeFFTInitializer(b->fft_init);

  FREE(b);
}


/* -----------  Bark Scaled Cepstrum Features implementation  -------------- */

typedef struct CepStruct {
  FibStruct *fib;
  float **coeff;
  int num_coeff;
} CepStruct;


static void 
InitBarkCep(Speetures *S) {
  CepStruct *info;
  int i, j, N;
  int num_coeff;
  float A, B, lift;

  InitBarkFib(S);
  CALLOC(info, 1, CepStruct);
  info->fib = S->info;
  N = info->fib->num_filters;
  S->info = (void *)info;
  
  num_coeff = info->num_coeff = S->used_size;
  CALLOC2D(info->coeff, num_coeff, S->base_size, float);

  A = PI / (float)N;
  B = sqrt(2.0 / (float)N);

  for (i = 1; i <= num_coeff; i++) {
    lift = 1.0 + S->cep_lift / 2.0 * sin(PI * (float)i / S->cep_lift);

    for (j = 1; j <= N; j++) {
      info->coeff[i - 1][j - 1] = 
         B * lift * cos(A * (float)i * ((float)j - 0.5));
    }
  }
}


static void 
InitMelCep(Speetures *S) {
  CepStruct *info;
  int i, j, N;
  int num_coeff;
  float A, B, lift;
	
  CALLOC(info, 1, CepStruct);

  InitMelFib(S);
  info->fib = S->info;
  S->info = (void *)info;
  N = info->fib->num_filters;

  num_coeff = info->num_coeff = S->used_size;
  CALLOC2D(info->coeff, num_coeff, S->base_size, float);

  A = PI / (float)N;
  B = sqrt(2.0 / (float)N);

  for (i = 1; i <= num_coeff; i++) {
    lift = 1.0 + S->cep_lift / 2.0 * sin(PI * (float)i / S->cep_lift);
    
    for (j = 1; j <= N; j++) {
      info->coeff[i - 1][j - 1] = 
	B * lift * cos(A * (float)i * ((float)j - 0.5));
    }
  }
}


static float *
CepstrumFeatures(Speetures *S) {
  CepStruct *info;
  float *fib, E;
  float *c, *cepstrum, *cep, *f;
  int num_coeff;
  int i, j, N;

  info = S->info;      /* save this pointer */
  S->info = info->fib; /* set up for fib features */
  fib = FilterbankFeatures(S, &E);
  S->info = info;      /* back to cep features */
  num_coeff = info->num_coeff;
  N = S->base_size;

  /* Necessary to allocate at least base_size */
  if (S->frame_size < N) i = N; else i = S->frame_size;
  CALLOC(cepstrum, i, float);

  cep = cepstrum;
  for (i = 0; i < num_coeff; i++, cep++) {
    c = info->coeff[i];

    for (j = 0, f = fib; j < N; j++, c++, f++) {
      *cep += (*c) * (*f);
    }
  }

  /* Zeroth coefficient */
  if (S->add_flags & ADD_ZEROTH_CEP) {
    float zeroth = 0.0; 
    float scale = sqrt(2.0 / (float)info->fib->num_filters);

    for (j = 0, f = fib; j < N; j++, f++) zeroth += *f;
    cepstrum[S->energy_offset] = zeroth * scale;
  }

  /* Copy energy feature */
  if (S->add_flags & ADD_LOG_E) {
    cepstrum[S->energy_offset] = E;
  }

  FREE(fib);

  return cepstrum;
}


static void 
FreeCepStruct(CepStruct *b) {
  FREE2D(b->coeff, b->num_coeff);
  FreeFibStruct(b->fib);
  FREE(b);
}


/* ---------  End of section with feature-type specific functions  --------- */


/*  Extract features from the analysis-window in S and return an allocated   */
/*  vector with the features                                                 */
/*  Delta features are NOT computed in this function                         */
static float *
ExtractFeatures(Speetures *S) {
  float *features = NULL;
  float Energy;
  int mel_flag = (S->base_type & MEL_FEATURES);

  if (S->base_type & FIB_FEATURES && !mel_flag) {
    features = FilterbankFeatures(S, &Energy);
    if (S->base_type & ENERGY_FEATURES) {
      features[S->energy_offset] = Energy;
    }
  }
  else if (S->base_type & FIB_FEATURES && mel_flag) {
    features = FilterbankFeatures(S, &Energy);
    if (S->base_type & ENERGY_FEATURES) {
      features[S->energy_offset] = Energy;
    }
  }
  else if (S->base_type & CEPSTRUM_FEATURES) {
    features = CepstrumFeatures(S);
  }
  else { 
    ErrorExit(SYSTEM_ERR_EXIT, "The base-type %i is not implemeted in Speetures.c", 
	      S->base_type);
  }

  if (S->base_type & CEPITCH_FEATURES) {
    ErrorExit(SYSTEM_ERR_EXIT, "Pitch features are not yet implemented");
  }
  else if (S->base_type & COVPITCH_FEATURES) {
    ErrorExit(SYSTEM_ERR_EXIT, "Pitch features are not yet implemented");
  }

  return features;
}


/* This is the regression formula used in the HTK tool-kit 
   with the default: delwin=2 */
#define REGRESSION mid[j] = (-2.0 * l2[i] - l1[i] + r1[i] + 2.0 * r2[i]) / 10.0;

/* Compute all delta features
   NOTE: the first and last two frames are not computed -- their delta
   features are defined to be zero (should be an OK approximation if 
   it's silent there).  */ 
static void 
DoDelta1(Speetures *S, FrameEntry *f) {
  FrameEntry *frame = f;
  float *l1, *l2, *mid, *r1, *r2;
  int i, j;

  if (!(S->add_flags & ADD_ANY_DELTA)) return;
  else if (S->num_stacked_frames < 4) {
    for (i = 0, j = S->delta1_offset; i < S->used_size; i++, j++) {
      frame->frame[j] = 0.0;
    }
  }
  else {
    frame = f->prev->prev;
    
    l2 = frame->prev->prev->frame;
    l1 = frame->prev->frame;
    mid = frame->frame;
    r1 = frame->next->frame;
    r2 = frame->next->next->frame;
    
    /* base features delta */
    if (S->add_flags & ADD_DELTA1) {
      for (i = 0, j = S->delta1_offset; i < S->used_size; i++, j++) {
	REGRESSION 
      }
    }
    
    /* energy delta */
    if (S->add_flags & ADD_DELTA1_LOG_E) {
      i = S->energy_offset;
      j = S->delta1_energy_offset;
      REGRESSION
    }
  }
}


/* Compute all delta-delta features
   NOTE: the first and last four frames are not computed -- their delta-delta
   features are defined to be zero (should be an OK approximation if 
   it's silent there).  */ 
static void 
DoDelta2(Speetures *S, FrameEntry *f) {
  FrameEntry *frame = f;
  float *l1, *l2, *mid, *r1, *r2;
  int i, j;

  if (!(S->add_flags & ADD_DELTA2)) return;
  else if (f->n < 7 || S->num_stacked_frames < 6) {
    for (i = 0, j = S->delta2_offset; i < S->used_size; i++, j++) {
      frame->frame[j] = 0.0;
    }
  }
  else {
    frame = f->prev->prev->prev->prev;

    l2 = frame->prev->prev->frame;
    l1 = frame->prev->frame;
    mid = frame->frame;
    r1 = frame->next->frame;
    r2 = frame->next->next->frame;

    /* base features delta-delta */
    if (S->add_flags & ADD_DELTA2) {
      for (i = S->delta1_offset, j = S->delta2_offset; 
	   i < S->used_size + S->delta1_offset; i++, j++) {
	REGRESSION 
      }
    }

    /* energy delta-delta */
    if (S->add_flags & ADD_DELTA2_LOG_E) {
      i = S->delta1_energy_offset;
      j = S->delta2_energy_offset;
      REGRESSION 
    }
  }
}

#undef REGRESSION


/*   Push a new frame on the FIFO of S */
/*   Can be an alternative to PushSpeech if the basefeatures are read from   */
/*   a file. Pushing one frame of base features to the Speetures struct      */
/*   WARNING: 'features' must be allocated but NOT freed by the caller.      */
void 
PushFeatures( Speetures *S, float *features ) {
  PushFeatures2( S, features, NULL );
}

void 
PushFeatures2(Speetures *S, float *features, void *user) {
  FrameEntry *frame;

  /* Get an allocated FrameEntry */
  if (S->used_frames != NULL) {
    frame = S->used_frames;
    S->used_frames = frame->next;
  }
  else {
    CALLOC(frame, 1, FrameEntry);
  }

  /* Move the actual features to the frame */
  frame->frame = features;

  frame->user = user;
	
  /* link the new entry last in the FIFO */
  frame->prev = S->last_frame;
  if (S->last_frame) S->last_frame->next = frame;
  S->last_frame = frame;
  frame->next = NULL;

  /* If the FIFO is empty, the new entry is the first in line */
  if (!S->first_frame) {
    S->first_frame = frame;
    S->next_frame_to_pop = frame;
  }

  frame->n = S->num_pushed_frames++;

  DoDelta1(S, frame);
  DoDelta2(S, frame);

  S->num_stacked_frames++;
}


/*   Push "num_samples" short-samples of audio to the Speetures struct       */
/*   (samp_freq [Hz], frame_length [sec])                                    */
Speetures *InitSpeetures(int base_type, int base_size, int used_size,
                         int add_flags, 
                         float samp_freq, 
                         float frame_length,
                         float frame_width,
                         float low_cut, float high_cut) {
  Speetures *S;
  short *zero_speech;
  int n;

  CALLOC(S, 1, Speetures);
  S->base_type = base_type;
  S->base_size = base_size;
  S->used_size = used_size;
  S->add_flags = add_flags;
  S->samp_freq = samp_freq;
  S->low_cut = low_cut;
  S->high_cut = high_cut;
  S->status = 1;

  S->frame_step = (int)floor(0.01 + frame_length * samp_freq);
  S->frame_window_size = (int)floor(0.01 + frame_width * samp_freq); 

  /* these parameters get default values here -- can be altered later */
  S->pre_emp_factor = 0.97;
  S->cep_lift = 22.0;

  S->memory_size = 0;
  if (add_flags & (ADD_DELTA1_LOG_E | ADD_DELTA1)) S->memory_size = 4;
  if (add_flags & (ADD_DELTA2_LOG_E | ADD_DELTA2)) S->memory_size = 8;

  /* Allocate a window of the nearest power of 2 above the actual size */
  n = 1;
  while (n < S->frame_window_size) n *= 2;
  S->allocated_window_size = n;

  CALLOC(S->frame_window, S->allocated_window_size, float);

  S->frame_size = used_size;

  if ((base_type & ENERGY_FEATURES) && (add_flags & ADD_ZEROTH_CEP)) {
    S->energy_offset = S->frame_size;
    S->frame_size++;
  }

  if ((base_type & ENERGY_FEATURES) && (add_flags & ADD_LOG_E)) {
    S->energy_offset = S->frame_size;
    S->frame_size++;
  }

  if (base_type & PITCH_FEATURES) {
    S->pitch_offset = S->frame_size;
    S->frame_size++;
    if (add_flags & ADD_VOICING) {
      S->frame_size++;
    }
  }

  if (add_flags & ADD_DELTA1) {
    S->delta1_offset = S->frame_size;
    S->frame_size += used_size;
  }

  if ((base_type & ENERGY_FEATURES) && (add_flags & ADD_DELTA1_LOG_E)) {
    S->delta1_energy_offset = S->frame_size;
    S->frame_size++;
  }

  if (add_flags & ADD_DELTA2) {
    S->delta2_offset = S->frame_size;
    S->frame_size += used_size;
  }

  if ((base_type & ENERGY_FEATURES) && (add_flags & ADD_DELTA2_LOG_E)) {
    S->delta2_energy_offset = S->frame_size;
    S->frame_size++;
  }

  S->num_frames = 0;
  S->num_pushed_frames = 0;

  S->first_frame = S->last_frame = S->used_frames = NULL;
  S->num_stacked_frames = 0;

  S->first_speech = S->last_speech = S->used_speech = NULL;
  S->num_samples_stacked = 0;

  /* Push an initial 0-vector of samples to make the first sample start at 0 */
  n = (S->frame_window_size - S->frame_step) / 2;
  CALLOC(zero_speech, n, short);
  PushSpeech(S, zero_speech, n);
  FREE(zero_speech);

  /* Feature-type specific initializations */
  if (S->base_type & FIB_FEATURES) {
    if (S->base_type & MEL_FEATURES) InitMelFib(S);
    else InitBarkFib(S);
  }
  else if (S->base_type & CEPSTRUM_FEATURES) {
    if (S->base_type & MEL_FEATURES) InitMelCep(S);
    else InitBarkCep(S);
  }
  if (S->base_type & CEPITCH_FEATURES) {
    ErrorExit(SYSTEM_ERR_EXIT, "Pitch features are not yet implemented");
  }
  else if (S->base_type & COVPITCH_FEATURES) {
    ErrorExit(SYSTEM_ERR_EXIT, "Pitch features are not yet implemented");
  }

  /* Set up running average */
  CALLOC(S->running_average, S->frame_size, float);
  S->mean_subtraction = 0; /* disable by default */
  S->ra_timeconst = 0.95;

  return S;
}


/*   Push "num_samples" short-samples of audio to the Speetures struct       */
void 
PushSpeech(Speetures *S, short *speech, int num_samples) {
  SpeechEntry *spentry;
  float *features;

  /* Get an allocated SpeechEntry */
  if (S->used_speech != NULL) {
    spentry = S->used_speech;
    S->used_speech = spentry->next;
    if (spentry->allocated_size < num_samples) {
      while (spentry->allocated_size < num_samples) 
        spentry->allocated_size *= 2;
      REALLOC(spentry->speech, spentry->allocated_size, short);
    }
  }
  else {
    CALLOC(spentry, 1, SpeechEntry);
    MALLOC(spentry->speech, num_samples, short);
    spentry->allocated_size = num_samples;
  }
  spentry->ptr = 0;
  spentry->size = num_samples;
  memcpy(spentry->speech, speech, num_samples * sizeof(short));

  /* link the new entry last in the FIFO */
  spentry->prev = S->last_speech;
  if (S->last_speech) S->last_speech->next = spentry;
  S->last_speech = spentry;
  spentry->next = NULL;

  /* If the FIFO is empty, the new entry is the first in line */
  if (!S->first_speech) S->first_speech = spentry;

  S->num_samples_stacked += num_samples;

  /* If we have enough speech, do the feature extraction and push a frame */
  while (S->num_samples_stacked >= S->frame_window_size) {
    int i, j;
    int frame_step = S->frame_step;
    int frame_window_size = S->frame_window_size;
    float *frame_window = S->frame_window;
    short *speech;

    /* Copy the speech to the analysis window */
    spentry = S->first_speech;
    speech = spentry->speech;

    for (j = 0, i = spentry->ptr; j < frame_window_size; i++, j++) {

      if (j == frame_step) {
	spentry->ptr = i; /* Start here next time */
      }

      if (i == spentry->size) {
        if (j <= frame_step) {
          spentry->ptr = -1; /* signal: this spentry ok to discard */
	}
        spentry = spentry->next; /* Start copying from next spentry */
        speech = spentry->speech;
        i = 0;
      }

      frame_window[j] = (float)speech[i];
    }

    /* Check if we can discard speech stack entries */ 
    spentry = S->first_speech;
    while (spentry->ptr == -1) {
      /* Put the next frame first in the FIFO */
      S->first_speech = spentry->next;
      if (S->first_speech) S->first_speech->prev = NULL;

      /* Move the SpeechEntry to the used-list */
      spentry->next = S->used_speech;
      S->used_speech = spentry;

      spentry = S->first_speech;      
    }

    /* Fill the remainder of the window with zeroes */
    memset(frame_window + frame_window_size, '\0', 
       (S->allocated_window_size - frame_window_size) * sizeof(float));

    S->num_samples_stacked -= frame_step;

    /* Do the feature extraction */
    features = ExtractFeatures(S);
    
    /* Push the features on the features FIFO */
    PushFeatures(S, features);
  }
}


/*   Call this function when there are no more samples to push               */
void 
SpeechEnd(Speetures *S) {
  short *speech;
  int n, window_tail, last_sample_size;

  S->status = 0;

  /* Push enough zero speech at the end of the utterance to give the
     last frame its analysis window */
  window_tail = (S->frame_window_size - S->frame_step);
  last_sample_size = (S->num_samples_stacked - window_tail) % S->frame_step;
  n = S->frame_step - last_sample_size + window_tail + 1;

  CALLOC(speech, n, short);
  PushSpeech(S, speech, n);
  FREE(speech);
}


/*   Alternative to "SpeechEnd" -- used when PushFeatures are used directly, */
/*   without pushing speech.                                                 */
void 
BaseFrameEnd(Speetures *S) {
  S->status = 0;
}


/*   Returns N if there are N frames to pop on the FIFO, -1 if the utterance */
/*   is finished and 0 otherwize.                                            */
int 
ProbeFeatures(Speetures *S) {
  if (S->status == 0 && S->num_stacked_frames == 0) /* No more frames to pop?*/
    return -1;
  else {
    if (S->num_stacked_frames - S->memory_size > 0) {
      /* return number of frames currently available */
      return S->num_stacked_frames - S->memory_size;
    }
    else {
      if (S->num_pushed_frames <= S->memory_size) /* At beg. of utterance */
        return 0;
      else if (S->status == 0)            /* At end of utterance */
        return S->num_stacked_frames;
      else return 0;              /* More to come, but you'll have to wait */
    }
  }
}


/*   Get an allocated vector of features from S                               */
/*   Returns NULL if there are no frames to pop on the FIFO                   */
/*   NOTE! it is the responsibilty of the calling function to free the        */
/*   returned vector.                                                         */ 
float *
PopFeatures(Speetures *S) {
  return PopFeatures2(S, NULL);
}


/*   Get an allocated vector of features from S                               */
/*   Returns NULL if there are no frames to pop on the FIFO                   */
/*   NOTE! it is the responsibilty of the calling function to free the        */
/*   returned vector.                                                         */ 
float *
PopFeatures2(Speetures *S, void **user) {
  FrameEntry *frame;
  float *features;

  if (S->status && S->num_stacked_frames <= S->memory_size) {
    return NULL; /* An error -- nothing to pop */
  }
	
  if ( S->status == 0 && S->num_stacked_frames == 0 )
    return NULL; /* End of Speech and last frame has been popped */
	
  /* Get the frame to pop */
  frame = S->next_frame_to_pop;
  S->next_frame_to_pop = frame->next;

  features = frame->frame;
	
  if ( user != NULL ) *user = frame->user;
	
  /* Put the next frame first in the FIFO */
  frame = S->first_frame;
  S->first_frame = frame->next;

  /* Move the old first-frame to the used-list */
  frame->next = S->used_frames;
  frame->frame = NULL;
  S->used_frames = frame;

  S->num_stacked_frames--;

  S->num_frames++;

  /* do running average normalization */
  if (S->mean_subtraction) {
    int j;
    float tc = S->ra_timeconst;
    float itc = 1.0 - tc;
    float *ma = S->running_average;

    if (1.0 / S->num_frames > itc) { /* initialize */
      itc = 1.0 / S->num_frames;
      tc = 1.0 - tc;
    }

    for (j = 0; j < S->frame_size; j++) {
      ma[j] *= tc;
      ma[j] += itc * features[j];
      features[j] -= ma[j];
    }
  }

  return features;
}


/*   Free memory allocated in the Speetures structure                        */
void 
FreeSpeetures(Speetures *S) {
  SpeechEntry *speech, *s;
  FrameEntry *frame, *f;

  speech = S->first_speech;
  while (speech) {
    s = speech->next;
    FREE(speech->speech);
    FREE(speech);
    speech = s;
  }

  speech = S->used_speech;
  while (speech) {
    s = speech->next;
    FREE(speech->speech);
    FREE(speech);
    speech = s;
  }

  frame = S->first_frame;
  while (frame) {
    f = frame->next;
    if (frame->frame) FREE(frame->frame);
    FREE(frame);
    frame = f;
  }

  frame = S->used_frames;
  while (frame) {
    f = frame->next;
    if (frame->frame) FREE(frame->frame);
    FREE(frame);
    frame = f;
  }

  FREE(S->frame_window);

  if (S->base_type & FIB_FEATURES) {
    FreeFibStruct(S->info);
  }
  else if (S->base_type & CEPSTRUM_FEATURES) {
    FreeCepStruct(S->info);
  }

  FREE(S->running_average);

  FREE(S);
}


/* --------------------------------------------------------------------------*/
/*           END OF FILE :  Speetures.c                                      */
/* --------------------------------------------------------------------------*/
