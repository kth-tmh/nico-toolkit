/* ------------------------------------------------------------------------- */
/*                               Math routines                               */
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
#include <math.h>  
#include <time.h> 
#include <unistd.h>
#include "Math.h"


int 
AlmostEqual(float a, float b) {
  return (fabs((a - b) / (a + b)) < FLOAT_PRECISION);
}


float 
SafeLog(float f) {
  if (f > SMALL_AMPL) return log(f);
  else return SMALL_LOG;
}


/* Add numbers in the log domain */
float 
LogAdd(float a, float b) {
  if (a < b) {
    return b + SafeLog(1.0 + exp(a - b));
  }
  else {
    return a + SafeLog(1.0 + exp(b - a));
  }
}


/* ---                  Random number generation                         --- */
#ifdef _TIME_INCLUDED
void 
Randomize(void) {
/* Initialize (randomly) the random number generator */
  struct timeval t;
  struct timezone z;
  gettimeofday(&t,&z);

  srand48((unsigned int)t.tv_usec);
}
#else
void 
Randomize(void) {
  srand48(getpid());
}
#endif

float 
RectRand(void) {
/* Returns a square distributed random value [0,1] */ 
   return (float)drand48();
}


/* --------------------------------------------------------------------------*/
/*           END OF FILE :  Math.c                                           */
/* --------------------------------------------------------------------------*/
