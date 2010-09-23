/* ------------------------------------------------------------------------- */
/*                        Signal processing routines                         */
/* ------------------------------------------------------------------------- */
/*                                                                           */
/*   Copyright (C) 1994, 1995, 1996, 1997, 1998 Nikko Strom                  */
/*                                                                           */
/*   All rights reserved.                                                    */
/*                                                                           */
/*   Developed by Nikko Strom at the Dept. of Speech, Music and Hearing,     */
/*   KTH (Royal Institute of Technology), Sweden.                            */
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
/*                                                                           */
/* The implementation of the core Fast Fourier Transform (FFT) algorithm was */
/* derived from translating a FORTRAN program published in "Programs for     */
/* Digital Signal Processing," edited by the DSP Committee, IEEE Acoustics,  */
/* Speech, and Signal Processing Society (IEEE Press, 1979), Chapter 1.2,    */
/* "Fast Fourier Transform Algorithms," p. 1.2-1 - 1.2-18.                   */
/*                                                                           */
/* ------------------------------------------------------------------------- */

#include        <stdlib.h>
#include        <stdio.h>
#include        <string.h>
#include        <ctype.h>
#include        <math.h>
#include        <unistd.h>
#include        "nMath.h"
#include        "System.h"
#include        "Signal.h"


/* Lookup-table for powes of 2*/
static unsigned short power_of_two[16] =
{1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768};


/*   ----------   Approximative Mel-scale   -----------------------------    */
/* See :                                                                     */
/*   --------------------------------------------------------------------    */

double 
Mel(double f) {
  return 2595.0 * log10(1.0 + f / 700.0);
}

double 
InvMel(double Mel) {
  return 700.0 * (pow(10.0, Mel / 2595.0) - 1.0);
}




/*   -------------   Approximative Bark-scale   -------------------------    */
/* See :                                                                     */
/*      Schroeder M. R., Atal B. S. & Hall J. L. (1979):                     */
/*      "Objective Measure of certain speech signal degradations based on    */
/*       masking..."                                                         */
/*      in: Frontiers of Speech Communication Research (Eds: B. Lindblom &   */
/*       S. Oehman), Academic Press, London, pp. 217-229.                    */
/*   --------------------------------------------------------------------    */

double 
Bark(double f) {
  return 7.0 * asinh(f / 650.0);
}

double 
InvBark(double bark) {
  return 650.0 * sinh(bark / 7.0);
}


/*   --------------   Hamming Window smoothing   -----------------------    */
/* See :                                                                    */
/* ---------------------------------------------------------------------    */

/* Returns a hamming-window of length 'analyze_size' */
float *
InitHamming(int analyze_size) {
  int i;
  float a, *hamming_window;

  CALLOC(hamming_window, analyze_size, float)

  a = 2.0 * PI / ((float)analyze_size - 1.0);

  for (i = 0; i < analyze_size; i++) {
    hamming_window[i] = 0.54 - 0.46 * cos(a * (float)i);
  }
 
  return hamming_window;
}

/* Apply the hamming-window of length 'hamming_size' to 'data'*/
void 
ApplyHamming(float *data, float *hamming_window, 
                  int data_size, int hamming_size) {
  int i;
  float *s = data;
  float *h = hamming_window;

  for (i = 0; i < hamming_size; i++, s++, h++) (*s) *= (*h);
  for (i = hamming_size; i < data_size; i++, s++) *s = 0.0; /* Pad with 0's */
}



/* --     Down-sample by simply taking the average of 1, 2, 3 or 4        -- */
/* --                    consecutive samples                              -- */
/* The speech input is passed in the short vector 'speech' of length 'length'*/
/* The result is returned in the float vector 'data'                         */
/*     WARNING:  The vector 'data' must have been allocated                  */
/*     NOTE :    This is a convienient way to create a float-vector from a   */
/*               speech data vector of short int's.                          */
/* ------------------------------------------------------------------------- */

void 
DownSample(short *speech, float *data, int length, int factor) {
  short *i;
  float *d;
  int j;
  long tmp;

  switch (factor) {
    case 1:
      for (d = data, i = speech, j = length; j; d++, i++, j--) {
        *d = (float) *i;
      }
      break;
    case 2:
      for (d = data, i = speech, j = length / 2; j; d++, j--) {
	tmp = *i++;
	tmp += *i++;
	*d = (float)tmp / 2.0;
      }
      break;
    case 3:
      for (d = data, i = speech, j = length / 3; j; d++, j--) {
	tmp = *i++;
	tmp += *i++;
	tmp += *i++;
        *d = (float)tmp / 3.0;
      }
      break;
    case 4:
      for (d = data, i = speech, j = length / 4; j; d++, j--) {
	tmp = *i++;
	tmp += *i++;
	tmp += *i++;
	tmp += *i++;
        *d = (float)tmp / 4.0;
      }
      break;
    default:
      ErrorExit(SYSTEM_ERR_EXIT, 
"Sorry, only factor 1, 2, 3 and 4 down sampling implemented (You tried %i).\n",
         factor);
  }
}

/* Pre emphasis with factor 'factor' to the signal in 'data' of length 'n'  */
void 
PreEmphasis (float *data, int n, float factor) {
   int i;
   
   for (i = n - 1; i; i--) data[i] -= data[i - 1] * factor;
   data[0] *= 1.0 - factor;
}


/* Compute the energy in the signal */
float 
GetEnergy(float *signal, int length) {
  int i;
  float E = 0.0, *s;

  s = signal;
  for (i = 0; i < length; i++, s++) E += (*s) * (*s);

  return E;
}


/* Remove the DC component in the signal */
void 
RemoveDC(float *signal, int length) {
  int i;
  float sum = 0.0, *s, DC;

  s = signal;
  for (i = 0; i < length; i++, s++) sum += *s;
  DC = sum / (float)length;
  s = signal;
  for (i = 0; i < length; i++, s++) *s -= DC;
}




/* ------- FFT-routines with base-8, base-4 and base-2 subroutines  -------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*                 RADIX 2 ITERATION SUBROUTINE                         */
/* -------------------------------------------------------------------- */

static void 
r2tx(int nthpo, float *cr0, float *cr1, float *ci0, float *ci1) {
  register int k;
  register float r1, fi1;

  for (k = 0; k < nthpo; k += 2) {
    r1 = cr0[k] + cr1[k];
    cr1[k] = cr0[k] - cr1[k];
    cr0[k] = r1;
    fi1 = ci0[k] + ci1[k];
    ci1[k] = ci0[k] - ci1[k];
    ci0[k] = fi1;
  }
}


/* -------------------------------------------------------------------- */
/*               RADIX 4 ITERATION SUBROUTINE                           */
/* -------------------------------------------------------------------- */

static void 
r4tx(int nthpo, 
                float *cr0, float *cr1, float *cr2, float *cr3, 
                float *ci0, float *ci1, float *ci2, float *ci3) {
  register int k;
  register float r1, r2, r3, r4;
  register float fi1, fi2, fi3, fi4;

  for (k = 0; k < nthpo; k += 4) {
    r1 = cr0[k] + cr2[k];
    r2 = cr0[k] - cr2[k];
    r3 = cr1[k] + cr3[k];
    r4 = cr1[k] - cr3[k];
    fi1 = ci0[k] + ci2[k];
    fi2 = ci0[k] - ci2[k];
    fi3 = ci1[k] + ci3[k];
    fi4 = ci1[k] - ci3[k];
    cr0[k] = r1 + r3;
    ci0[k] = fi1 + fi3;
    cr1[k] = r1 - r3;
    ci1[k] = fi1 - fi3;
    cr2[k] = r2 - fi4;
    ci2[k] = fi2 + r4;
    cr3[k] = r2 + fi4;
    ci3[k] = fi2 - r4;
  }
}


/* -------------------------------------------------------------------- */
/*               RADIX 8 ITERATION SUBROUTINE                           */ 
/* -------------------------------------------------------------------- */

static void 
r8tx(int nxtlt, int nthpo, int lengt, 
                 int nofpts, float *sint, float *cost,
                 float *cr0, float *cr1, float *cr2, float *cr3, 
                 float *cr4, float *cr5, float *cr6, float *cr7,
                 float *ci0, float *ci1, float *ci2, float *ci3, 
                 float *ci4, float *ci5, float *ci6, float *ci7) {
  float c1, c2, c3, c4, c5, c6, c7;
  float s1, s2, s3, s4, s5, s6, s7;
  float ar0, ar1, ar2, ar3, ar4, ar5, ar6, ar7;
  float ai0, ai1, ai2, ai3, ai4, ai5, ai6, ai7;
  float br0, br1, br2, br3, br4, br5, br6, br7;
  float bi0, bi1, bi2, bi3, bi4, bi5, bi6, bi7;
  register float tr, ti;
  register int j, k;

  for (j = 0; j < nxtlt; j++) {
    c1 = cost[j * nofpts / lengt];
    s1 = sint[j * nofpts / lengt];
    c2 = c1 * c1 - s1 * s1;
    s2 = c1 * s1 + c1 * s1;
    c3 = c1 * c2 - s1 * s2;
    s3 = c2 * s1 + s2 * c1;
    c4 = c2 * c2 - s2 * s2;
    s4 = c2 * s2 + c2 * s2;
    c5 = c2 * c3 - s2 * s3;
    s5 = c3 * s2 + s3 * c2;
    c6 = c3 * c3 - s3 * s3;
    s6 = c3 * s3 + c3 * s3;
    c7 = c3 * c4 - s3 * s4;
    s7 = c4 * s3 + s4 * c3;
    for (k = j; k < nthpo; k += lengt) {
      ar0 = cr0[k] + cr4[k];
      ar1 = cr1[k] + cr5[k];
      ar2 = cr2[k] + cr6[k];
      ar3 = cr3[k] + cr7[k];
      ar4 = cr0[k] - cr4[k];
      ar5 = cr1[k] - cr5[k];
      ar6 = cr2[k] - cr6[k];
      ar7 = cr3[k] - cr7[k];
      ai0 = ci0[k] + ci4[k];
      ai1 = ci1[k] + ci5[k];
      ai2 = ci2[k] + ci6[k];
      ai3 = ci3[k] + ci7[k];
      ai4 = ci0[k] - ci4[k];
      ai5 = ci1[k] - ci5[k];
      ai6 = ci2[k] - ci6[k];
      ai7 = ci3[k] - ci7[k];
      br0 = ar0 + ar2;
      br1 = ar1 + ar3;
      br2 = ar0 - ar2;
      br3 = ar1 - ar3;
      br4 = ar4 - ai6;
      br5 = ar5 - ai7;
      br6 = ar4 + ai6;
      br7 = ar5 + ai7;
      bi0 = ai0 + ai2;
      bi1 = ai1 + ai3;
      bi2 = ai0 - ai2;
      bi3 = ai1 - ai3;
      bi4 = ai4 + ar6;
      bi5 = ai5 + ar7;
      bi6 = ai4 - ar6;
      bi7 = ai5 - ar7;
      cr0[k] = br0 + br1;
      ci0[k] = bi0 + bi1;
      if (j > 0) {
        cr1[k] = c4 * (br0 - br1) - s4 * (bi0 - bi1);
        ci1[k] = c4 * (bi0 - bi1) + s4 * (br0 - br1);
        cr2[k] = c2 * (br2 - bi3) - s2 * (bi2 + br3);
        ci2[k] = c2 * (bi2 + br3) + s2 * (br2 - bi3);
        cr3[k] = c6 * (br2 + bi3) - s6 * (bi2 - br3);
        ci3[k] = c6 * (bi2 - br3) + s6 * (br2 + bi3);
        tr = p7 * (br5 - bi5);
        ti = p7 * (br5 + bi5);
        cr4[k] = c1 * (br4 + tr) - s1 * (bi4 + ti);
        ci4[k] = c1 * (bi4 + ti) + s1 * (br4 + tr);
        cr5[k] = c5 * (br4 - tr) - s5 * (bi4 - ti);
        ci5[k] = c5 * (bi4 - ti) + s5 * (br4 - tr);
        tr = -p7 * (br7 + bi7);
        ti = p7 * (br7 - bi7);
        cr6[k] = c3 * (br6 + tr) - s3 * (bi6 + ti);
        ci6[k] = c3 * (bi6 + ti) + s3 * (br6 + tr);
        cr7[k] = c7 * (br6 - tr) - s7 * (bi6 - ti);
        ci7[k] = c7 * (bi6 - ti) + s7 * (br6 - tr);
      } else {
        cr1[k] = br0 - br1;
        ci1[k] = bi0 - bi1;
        cr2[k] = br2 - bi3;
        ci2[k] = bi2 + br3;
        cr3[k] = br2 + bi3;
        ci3[k] = bi2 - br3;
        tr = p7 * (br5 - bi5);
        ti = p7 * (br5 + bi5);
        cr4[k] = br4 + tr;
        ci4[k] = bi4 + ti;
        cr5[k] = br4 - tr;
        ci5[k] = bi4 - ti;
        tr = -p7 * (br7 + bi7);
        ti = p7 * (br7 - bi7);
        cr6[k] = br6 + tr;
        ci6[k] = bi6 + ti;
        cr7[k] = br6 - tr;
        ci7[k] = bi6 - ti;
      }
    }
  }
}



/* -------------------------------------------------------------------- */
/*                                                                      */
/*   Initializes the fft.                                               */
/*   if 'n' (fft size) is not an integer power of 2, the size-element   */
/*   of the init-struct will be set to the smallest integer power of 2  */
/*   greater than n.                                                    */
/*                                                                      */
/* -------------------------------------------------------------------- */

FFTInitializer *
fft842init(FFTType type, int n) {
  FFTInitializer *init;
  int k, l, m;
  double a, p;

  m = (int)ceil(LOG2((float)n));
  l = power_of_two[m];
  p = pi2 / l;

  CALLOC(init, 1, FFTInitializer)
  init->type = type;
  init->size = l;

  CALLOC(init->sintab1, l, float);
  CALLOC(init->costab1, l, float);
  for (k = 0; k < l; k++) {
    a = k * p;
    init->sintab1[k] = sin(a);
    init->costab1[k] = cos(a);
  }
  
  if (type != COMPLEXFFT) {
    CALLOC(init->buffer, l, float);
    l /= 2;
    p = pi2 / l;
    init->size = l;
    CALLOC(init->sintab2, l, float);
    CALLOC(init->costab2, l, float);
    for (k = 0; k < l; k++) {
      a = k * p;
      init->sintab2[k] = sin(a);
      init->costab2[k] = cos(a);
    }
  }

  if (type == CEPSTRUMFFT) {
    l /= 2;
    p = pi2 / l;
    CALLOC(init->sintab3, l, float);
    CALLOC(init->costab3, l, float);
    for (k = 0; k < l; k++) {
      a = k * p;
      init->sintab3[k] = sin(a);
      init->costab3[k] = cos(a);
    }
  }

  return init;
}


void 
FreeFFTInitializer(FFTInitializer *init) {
  if (init->sintab1) FREE(init->sintab1)
  if (init->costab1) FREE(init->costab1)
  if (init->sintab2) FREE(init->sintab2)
  if (init->costab2) FREE(init->costab2)
  if (init->costab3) FREE(init->costab3)
  if (init->sintab3) FREE(init->sintab3)
  if (init->buffer) FREE(init->buffer)
  FREE(init)
}


/* -------------------------------------------------------------------- */
/*                                                                      */
/*   This function replaces the vector z=x+iy by its  finite            */
/*   discrete, complex fourier transform if in=0.  The inverse          */
/*   transform is calculated for in=1.  It performs as many base        */
/*   8 iterations as possible and then finishes with a base             */
/*   4 iteration or a base 2 iteration if needed.                       */
/*                                                                      */
/* -------------------------------------------------------------------- */

void 
fft842(float *x, float *y, FFTInitializer *init, int in) {
  int l[15];
  int i, n2pow, n8pow, nthpo, fn;
  int lengt, nofpts;
  register int ij, ji, nxtlt;
  int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14;
  float r, fi;
  float *sint, *cost;

  if (init->type != COMPLEXFFT)
    ErrorExit(SYSTEM_ERR_EXIT, "Complex fft called after initializing for real fft");

  sint = init->sintab1;
  cost = init->costab1;
  nofpts = init->size;

  n2pow = (int)LOG2(nofpts);

  fn = nthpo = nofpts;
  if (in == 0) {
    for (i = 0; i < nthpo; i++) {
      y[i] = -y[i];
    }
  }
  n8pow = n2pow / 3;
  if (n8pow != 0) {
    for (i = 0; i < n8pow; i++) {
      nxtlt = power_of_two[n2pow - 3 * (i + 1)];
      lengt = 8 * nxtlt;
      r8tx (nxtlt, nthpo, lengt, nofpts, sint, cost,
            &x[0], &x[nxtlt], &x[2 * nxtlt], &x[3 * nxtlt],
            &x[4 * nxtlt], &x[5 * nxtlt], &x[6 * nxtlt], &x[7 * nxtlt],
            &y[0], &y[nxtlt], &y[2 * nxtlt], &y[3 * nxtlt],
            &y[4 * nxtlt], &y[5 * nxtlt], &y[6 * nxtlt], &y[7 * nxtlt]);
    }
  }
  switch (n2pow - 3 * n8pow) {
  case 0:
    break;
  case 1:
    r2tx (nthpo, &x[0], &x[1], &y[0], &y[1]);
    break;
  case 2:
    r4tx (nthpo, &x[0], &x[1], &x[2], &x[3],
          &y[0], &y[1], &y[2], &y[3]);
    break;
  default:
    ErrorExit(SYSTEM_ERR_EXIT, "Algorithm Error fft842");
  }
  for (i = 0; i < 15; i++) {
    if (i < n2pow) {
      l[i] = power_of_two[n2pow - i];
    } else {
      l[i] = 1;
    }
  }
  ij = 0;
  for (i1 = 0; i1 < l[14]; i1++) {
    for (i2 = i1; i2 < l[13]; i2 += l[14]) {
      for (i3 = i2; i3 < l[12]; i3 += l[13]) {
        for (i4 = i3; i4 < l[11]; i4 += l[12]) {
          for (i5 = i4; i5 < l[10]; i5 += l[11]) {
            for (i6 = i5; i6 < l[9]; i6 += l[10]) {
              for (i7 = i6; i7 < l[8]; i7 += l[9]) {
                for (i8 = i7; i8 < l[7]; i8 += l[8]) {
                  for (i9 = i8; i9 < l[6]; i9 += l[7]) {
                    for (i10 = i9; i10 < l[5]; i10 += l[6]) {
                      for (i11 = i10; i11 < l[4]; i11 += l[5]) {
                        for (i12 = i11; i12 < l[3]; i12 += l[4]) {
                          for (i13 = i12; i13 < l[2]; i13 += l[3]) {
                            for (i14 = i13; i14 < l[1]; i14 += l[2]) {
                              for (ji = i14; ji < l[0]; ji += l[1]) {
                                if (ij < ji) {
                                  r = x[ij];
                                  x[ij] = x[ji];
                                  x[ji] = r;
                                  fi = y[ij];
                                  y[ij] = y[ji];
                                  y[ji] = fi;
                                }
                                ij++;
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  if (in == 0) {
    for (i = 0; i < nthpo; i++) {
      y[i] = -y[i];
    }
  } else {
    for (i = 0; i < nthpo; i++) {
      x[i] /= (float) fn;
      y[i] /= (float) fn;
    }
  }
}


/* -------------------------------------------------------------------- */
/*                                                                      */
/*  This function replaces the real vector argument by its finite       */
/*  discrete, complex Fourier transform.                                */
/*  The returned vector is on the form:                                 */
/*       [re0, re1,... , re(n/2 - 1), im0, im1,... ,im(n/2 - 1)] .      */
/*  (The n/2:th value is thrown away)                                   */
/*                                                                      */
/* -------------------------------------------------------------------- */

void 
real842(float *real, FFTInitializer *init) {
  int i, j;
  int n, nhalf, nquart;
  float *re1, *im1, *re2, *im2;
  float *Re1, *Re2, *Im1, *Im2;
  float t1, t2;
  float h1r, h1i, h2r, h2i;
  float *costab, *sintab, *real_buffer;
 
  if (init->type != REALFFT) 
    ErrorExit(SYSTEM_ERR_EXIT, 
      "Real fft called after initializing for complex or cepstrum fft");

  sintab = init->sintab1;
  costab = init->costab1;
  real_buffer = init->buffer;
  n = init->size * 2; 
  nhalf = n / 2;
  nquart = nhalf / 2;


  /* Mixing the samples */
  for (i = 0, j = 1; i < nhalf; i++, j+=2) real_buffer[i] = real[j];
  for (i = nhalf, j = 0; i < n; i++, j+=2) real_buffer[i] = real[j];

  /* Set up the initializer for a complex fft */
  init->sintab1 = init->sintab2;
  init->costab1 = init->costab2;
  init->type = COMPLEXFFT;

  /* Do the complex fft */
  fft842(real_buffer, real_buffer + nhalf, init, 0);

  /* Change back to the original values */
  init->sintab1 = sintab;
  init->costab1 = costab;
  init->type = REALFFT;


  /* pointers into 'real_buffer' */
  re1 = real_buffer + 1;
  im1 = real_buffer + nhalf + 1;
  re2 = real_buffer + nhalf - 1;
  im2 = real_buffer + n - 1;

  /* pointers into 'real' */
  Re1 = real;
  Im1 = real + nhalf;
  Re2 = real + nhalf - 1;
  Im2 = real + n - 1;

  *Re1 = real_buffer[0] + real_buffer[nhalf];
  *Im1 = 0.0;
  Re1++; Im1++;

  /* Merging the even and odd sequencies */
  for (i = 1, sintab = init->sintab1 + 1, costab = init->costab1 + 1;
         i <= nquart; 
           i++, 
           re1++, im1++, re2--, im2--, 
           Re1++, Im1++, Re2--, Im2--,
           sintab++, costab++) {
    h1r =  0.5 * (*re1 + *re2);
    h1i =  0.5 * (*im1 - *im2);
    h2r =  0.5 * (*im1 + *im2);
    h2i = -0.5 * (*re1 - *re2);

    t1 = *costab * h2r - *sintab * h2i;
    t2 = *costab * h2i + *sintab * h2r;

    *Re1 = h1r + t1;
    *Im1 = h1i + t2;

    *Re2 = h1r - t1;
    *Im2 = -h1i + t2;
  }
}


/* -------------------------------------------------------------------- */
/*                                                                      */
/*  This function replaces the real vector argument by its power        */ 
/*  spectrum.                                                           */
/*  The n/2 first values of the returned vector are defined and are the */
/*  n/2 first values of the power spectrum.                             */
/*                                                                      */
/* -------------------------------------------------------------------- */

void 
power842(float *real, FFTInitializer *init) {
  int i, j;
  int n, nhalf, nquart;
  float *re1, *im1, *re2, *im2;
  float *Pow1, *Pow2;
  float t1, t2, x, y;
  float h1r, h1i, h2r, h2i;
  float *costab, *sintab, *real_buffer;

  if (init->type != REALFFT) 
    ErrorExit(SYSTEM_ERR_EXIT, 
      "Real fft called after initializing for complex or cepstrum fft");

  sintab = init->sintab1;
  costab = init->costab1;
  real_buffer = init->buffer;
  n = init->size * 2;
  nhalf = n / 2;
  nquart = nhalf / 2;

  /* Extracting the even and odd sequencies */
  for (i = 0, j = 1; i < nhalf; i++, j+=2) real_buffer[i] = real[j];
  for (i = nhalf, j = 0; i < n; i++, j+=2) real_buffer[i] = real[j];

  /* Set up the initializer for a complex fft */
  init->sintab1 = init->sintab2;
  init->costab1 = init->costab2;
  init->type = COMPLEXFFT;

  /* Do the complex fft */
  fft842(real_buffer, real_buffer + nhalf, init, 0);

  /* Change back to the original initializer */
  init->sintab1 = sintab;
  init->costab1 = costab;
  init->type = REALFFT;

  /* pointers into 'real_buffer' */
  re1 = real_buffer + 1;
  im1 = real_buffer + nhalf + 1;
  re2 = real_buffer + nhalf - 1;
  im2 = real_buffer + n - 1;

  /* pointers into 'real' */
  Pow1 = real;
  Pow2 = real + nhalf - 1;

  x = real_buffer[0] + real_buffer[nhalf];
  *Pow1 = x * x;
  Pow1++;

  /* Merging the even and odd sequencies */
  for (i = 1, sintab = init->sintab1 + 1, costab = init->costab1 + 1;
         i <= nquart; 
           i++, 
           re1++, im1++, re2--, im2--, 
           Pow1++, Pow2--,
           sintab++, costab++) {
    h1r =  0.5 * (*re1 + *re2);
    h1i =  0.5 * (*im1 - *im2);
    h2r =  0.5 * (*im1 + *im2);
    h2i = -0.5 * (*re1 - *re2);

    t1 = *costab * h2r - *sintab * h2i;
    t2 = *costab * h2i + *sintab * h2r;

    x = h1r + t1;
    y = h1i + t2;
    *Pow1 = x * x + y * y;

    x = h1r - t1;
    y = -h1i + t2;
    *Pow2 = x * x + y * y;
  }
}


/* -------------------------------------------------------------------- */
/*                                                                      */
/*  This function replaces the real vector argument by the db-values    */
/*  of it's power spectrum.                                             */
/*  The n/2 first values of the returned vector are defined and are the */
/*  n/2 first values of the db-values of the power spectrum.            */
/*                                                                      */
/* -------------------------------------------------------------------- */

void 
db842(float *real, FFTInitializer *init) {
  int i, nhalf = init->size; 
  float corr = 20.0 * log10((double)(2 * nhalf));
  float *xp;
  float db = 4.3429448190325183;

  power842(real, init);

  for (i = 0, xp = real; i < nhalf; i++, xp++) {
    if (*xp < SMALL_AMPL) *xp = db * SMALL_LOG - corr;
    else *xp = db * log(*xp) / 2.0;
  }
}


/* -------------------------------------------------------------------- */
/*                                                                      */
/*  This function replaces the real vector argument by the cepstrum     */
/*  transform.                                                          */
/*  The n/4 first values of the returned vector are defined and are the */
/*  n/4 first values of the cepstrum.                                   */
/*                                                                      */
/* -------------------------------------------------------------------- */

void 
cepstrum842(float *real, FFTInitializer *init) {
  float *sintab, *costab;

  if (init->type != CEPSTRUMFFT) 
    ErrorExit(SYSTEM_ERR_EXIT, 
      "Cepstrum fft called after initializing for complex or real fft");

  init->type = REALFFT; /* Change this value to please the db842 function */
  db842(real, init);

  /* Set up an initializer for the second fft of 1/2 the size */
  sintab = init->sintab1;
  costab = init->costab1;
  init->sintab1 = init->sintab2;
  init->costab1 = init->costab2;
  init->sintab2 = init->sintab3;
  init->costab2 = init->costab3;
  init->type = REALFFT;
  init->size /= 2;

  power842(real, init);

  /* Back to the original initializer */
  init->size *= 2;
  init->type = CEPSTRUMFFT;
  init->sintab3 = init->sintab2;
  init->costab3 = init->costab2;
  init->sintab2 = init->sintab1;
  init->costab2 = init->costab1;
  init->sintab1 = sintab;
  init->costab1 = costab;
}


/* --------------------------------------------------------------------------*/
/*           END OF FILE :  Signal.c                                         */
/* --------------------------------------------------------------------------*/
