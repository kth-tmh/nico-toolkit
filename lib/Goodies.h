/* ------------------------------------------------------------------------- */
/*                  Goodies - Nice formatted printing                        */
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



/* Version of tolower() for Swedish alpahbeth */
char sw_tolower(char c);


/* Swedish 'tolower' on a whole string */
void sw_strtolower(char *word);


/* Version of toupper() for Swedish alpahbeth */
char sw_toupper(char c);


/* Swedish 'toupper' on a whole string */
void sw_strtoupper(char *word);



/*     Print the N*N float-matrix "matrix" with x- and y-categories          */
/*     specified by x_labels and y_labels.                                   */
/*     The floats are printed with format "format" (for example "%5.1").     */
/*     Output to 'stream'.                                                   */

void PrintMatrixWithCats(float **matrix, int N, char *format,
                         char **x_labels, char **y_labels, 
                         FILE *stream);



/* ------------         Printing inside a border         ------------------- */


/*                    Initialise border printing                            */
/*    The border will be of width 'width'.                                  */
void InitBorderPrint(int width, char *left, char line, char *right);


/*    Use this function to print the first and last line.                   */
/*    'title' will be printed centered in the line of stars                 */
void BfprintfTitle(FILE *stream, char *format, ...);


/*    Use this function to print the intermediate lines.                    */
/*    'text' will be printed left-aligned                                   */
void BfprintfL(FILE *stream, char *format, ...);


/*    Use this function to print the intermediate lines.                    */
/*    'text' will be printed center-aligned                                 */
void BfprintfC(FILE *stream, char *format, ...);


/*    Use this function to print the intermediate lines.                    */
/*    'text' will be printed right-aligned                                  */
void BfprintfR(FILE *stream, char *format, ...);


/* --------------------------------------------------------------------------*/
/*           END OF FILE :  Goodies.h                                        */
/* --------------------------------------------------------------------------*/


