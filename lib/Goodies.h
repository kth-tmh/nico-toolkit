/* ------------------------------------------------------------------------- */
/*                  Goodies - Nice formatted printing                        */
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


