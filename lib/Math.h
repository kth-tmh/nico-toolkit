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

#include <math.h>

#define FLOAT_PRECISION 1e-6

#ifdef M_PI
#define PI M_PI
#endif

#ifndef PI
#define PI 3.1415927
#endif

#ifdef M_LN2
#define log2(x) (log(x)/M_LN2)
#endif

#ifndef log2
#define log2(x) (log(x)/0.6931471805599453)
#endif

#ifndef RAND_MAX
#define RAND_MAX 0x8FFF 
#endif


/* Loarithm values less than log(SMALL_AMPL) are approximated by SMALL_LOG
 * These values are used by the SafeLog() function. */
#define SMALL_AMPL 1e-10 
#define SMALL_LOG -23.025851


int AlmostEqual(float a, float b);

/* Take the log of x, but return SMALL_LOG if x < SMALL_AMPL */
float SafeLog(float x);

/* Add two log probabilities */
float LogAdd(float a, float b);

/* Initialize random generartor with a random seed */
void Randomize(); 

/* Returns a uniform [0;1] distributed random nimber */
float RectRand(); 

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  Math.h                                           */
/* --------------------------------------------------------------------------*/
