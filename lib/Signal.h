/* ------------------------------------------------------------------------- */
/*                        Signal processing routines                         */
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

/* Used by the fft842 routines */
#define  pi2  6.283185307179586
#define  p7   0.707106781186548
#define  ln2  0.6931471805599453


/* The different types of fft */
/* The particular type affects the data stored in the FFTInitializer */
typedef enum FFTType {
  COMPLEXFFT,      /* Complex valued input and output */
  REALFFT,         /* Real valued input, complex, power or db output */
  CEPSTRUMFFT      /* Real valued input, cepstrum output */
} FFTType;


/* Use fft842init to get an allocated  FFTInitializer */
typedef struct FFTInitializer {
  FFTType type;
  int size;                   /* This is the fft-size - for real ffts it's half
                                 the number of samples */
  float *sintab1, *costab1;
  float *sintab2, *costab2;
  float *sintab3, *costab3;

  float *buffer;
} FFTInitializer;


/*   ----------   Approximative Mel-scale   -----------------------------    */
/*   I'm using the same approximation as the HTK toolkit.                    */
/*   --------------------------------------------------------------------    */
double Mel(double f);
double InvMel(double Mel);


/*   -------------   Approximative Bark-scale   -------------------------    */
/* See :                                                                     */
/*      Schroeder M. R., Atal B. S. & Hall J. L. (1979):                     */
/*      "Objective Measure of certain speech signal degradations based on    */
/*       masking..."                                                         */
/*      in: Frontiers of Speech Communication Research (Eds: B. Lindblom &   */
/*       S. Oehman), Academic Press, London, pp. 217-229.                    */
/*   --------------------------------------------------------------------    */
double Bark(double f);
double InvBark(double bark);



/*   --------------   Hamming Window smoothing   -----------------------    */
/* See :                                                                    */
/* ---------------------------------------------------------------------    */

/* Returns a hamming-window of length 'analyse_size' */
float *InitHamming(int analyse_size);

/* Apply the hamming-window of length 'hamming_size' to 'data'*/
void ApplyHamming(float *data, float *hamming_window,
                  int data_size, int hamming_size);

/* Compute the energy in the signal */
float GetEnergy(float *signal, int length);


/* Remove any DC component in the signal */
void RemoveDC(float *signal, int length);


/* -----  Down-sample by simply taking the average of 1, 2, 3 or 4    -----  */
/*                          consecutive samples                              */
/* The speech input is passed in the short vector 'speech' of length 'length'*/
/*   The result is returned in the float vector 'data'                       */
/*       WARNING:  The vector 'data' must have been allocated                */
/*       NOTE :    This is a convienient way to create a float-vector from a */
/*                 speech data vector of short int's.                        */
/* ------------------------------------------------------------------------- */
void DownSample(short *speech, float *data, int length, int factor);


/* Pre emphasis with factor 'factor' to the signal in 'data' of length 'n'  */
void PreEmphasis(float *data, int n, float factor);


/* -------------------------------------------------------------------- */
/*                                                                      */
/*   Initializes the FFT. n (fft size) must be an integer power of 2    */
/*   See also the FFTInitializer-struct and the typedef of FFTType.     */
/*                                                                      */
/* -------------------------------------------------------------------- */
FFTInitializer *fft842init(FFTType, int size);



/* -------------------------------------------------------------------- */
/*                                                                      */
/*   Free memory allocated by the fft initializer                       */
/*                                                                      */
/* -------------------------------------------------------------------- */
void FreeFFTInitializer(FFTInitializer *init);



/* -------------------------------------------------------------------- */
/*                                                                      */
/*   This function replaces the vector z=x+iy by its  finite            */
/*   discrete, complex Fourier transform if in=0.  The inverse          */
/*   transform is calculated for in=1.  It performs as many base        */
/*   8 iterations as possible and then finishes with a base             */
/*   4 iteration or a base 2 iteration if needed.                       */
/*                                                                      */
/* -------------------------------------------------------------------- */

void fft842(float *x, float *y, FFTInitializer *init, int inverse_flag);


/* -------------------------------------------------------------------- */
/*                                                                      */
/*  This function replaces the real vector argument by its finite       */
/*  discrete, complex Fourier transform.                                */
/*  The returned vector is on the form:                                 */
/*       [re0, re1,... , re(n/2 - 1), im0, im1,... ,im(n/2 - 1)] .      */
/*  (The n/2:th value is thrown away)                                   */
/*                                                                      */
/* -------------------------------------------------------------------- */
void real842(float *real, FFTInitializer *init);


/* -------------------------------------------------------------------- */
/*                                                                      */
/*  This function replaces the real vector argument by its power        */ 
/*  spectrum.                                                           */
/*  The n/2 first values of the returned vector are defined and are the */
/*  n/2 first values of the power spectrum.                             */
/*                                                                      */
/* -------------------------------------------------------------------- */
void power842(float *real, FFTInitializer *init);


/* -------------------------------------------------------------------- */
/*                                                                      */
/*  This function replaces the real vector argument by the db-values    */
/*  of it's power spectrum.                                             */
/*  The n/2 first values of the returned vector are defined and are the */
/*  n/2 first values of the db-values of the power spectrum.            */
/*                                                                      */
/* -------------------------------------------------------------------- */
void db842(float *real, FFTInitializer *init);


/* -------------------------------------------------------------------- */
/*                                                                      */
/*  This function replaces the real vector argument by the cepstrum     */
/*  transform.                                                          */
/*  The n/4 first values of the returned vector are defined and are the */
/*  n/4 first values of the cepstrum.                                   */
/*                                                                      */
/* -------------------------------------------------------------------- */
void cepstrum842(float *real, FFTInitializer *init);


/* --------------------------------------------------------------------------*/
/*           END OF FILE :  Signal.h                                         */
/* --------------------------------------------------------------------------*/
