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


#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include "System.h"


static char to_lower_table[] = {
      0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 
      0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 
      0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 
      0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 
      ' ', '!', '"', '#', '$', '%', '&', '\'', 
      '(', ')', '*', '+', ',', '-', '.', '/', 
      '0', '1', '2', '3', '4', '5', '6', '7', 
      '8', '9', ':', ';', '<', '=', '>', '?', 
      '@', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 
      'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 
      'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 
      'x', 'y', 'z', '{', '|', '}', '^', '_', 
      '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 
      'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 
      'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 
      'x', 'y', 'z', '{', '|', '}', '~', 0x7F, 
      0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 
      0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 
      0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 
      0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F, 
      0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 
      0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 
      0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 
      0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 
      0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 
      0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 
      0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 
      0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, 
      0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 
      0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF, 
      0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 
      0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
};


static char to_upper_table[] = {
      0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 
      0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 
      0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 
      0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 
      ' ', '!', '"', '#', '$', '%', '&', '\'', 
      '(', ')', '*', '+', ',', '-', '.', '/', 
      '0', '1', '2', '3', '4', '5', '6', '7', 
      '8', '9', ':', ';', '<', '=', '>', '?', 
      '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 
      'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 
      'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 
      'X', 'Y', 'Z', '[', '\\', ']', '^', '_', 
      '`', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 
      'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 
      'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 
      'X', 'Y', 'Z', '[', '\\', ']', '~', 0x7F, 
      0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 
      0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 
      0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 
      0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F, 
      0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 
      0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 
      0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 
      0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 
      0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 
      0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 
      0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 
      0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, 
      0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 
      0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF, 
      0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 
      0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
};


/* Version of tolower() for Swedish alpahbeth */
char 
sw_tolower(char c) {
  return to_lower_table[(int)c];
}


/* Swedish 'tolower' on a whole string */
void 
sw_strtolower(char *word) {
  int i;
  for (i = 0; i < strlen(word); i++) word[i] = to_lower_table[(int)word[i]];
}


/* Version of toupper() for Swedish alpahbeth */
char 
sw_toupper(char c) {
  return to_upper_table[(int)c];
}


/* Swedish 'toupper' on a whole string */
void 
sw_strtoupper(char *word){
  int i;
  for (i = 0; i < strlen(word); i++) word[i] = to_upper_table[(int)word[i]];
}



/* Print the N*N float-matrix "matrix" with x- and y-categories         */
/* specified by x_labels and y_labels.                                  */
/* The floats are printed with format "format" (for example "%5.1").    */
/* Output to 'stream'.                                                  */

void 
PrintMatrixWithCats(float **matrix, int N, char *format,
                         char **x_labels, char **y_labels, 
                         FILE *stream) {
  int i, j;
  int len, x_max_len, y_max_len, a_max_len;
  char y_name_format[64], x_name_format[64], a_format[64], a[64];

  /* Find the size of the longest element in the matrix */
  a_max_len = -1;
  for (i = 0; i < N; i++)  for (j = 0; j < N; j++) {
    sprintf(a, format, matrix[i][j]);
    len = strlen(a);
    if (len > a_max_len) a_max_len = len;
  }

  /* Find the size of the longest x-category */
  for (i = 0, x_max_len = 0; i < N; i++) {
    if ((len = strlen(x_labels[i])) > x_max_len) x_max_len = len;
  }

  /* Find the size of the longest y-category */
  for (i = 0, y_max_len = 0; i < N; i++) {
    if ((len = strlen(y_labels[i])) > y_max_len) y_max_len = len;
  }

  /* Print x-labels vertically */
  for (i = 0; i < a_max_len - 1; i++) x_name_format[i] = ' ';
  x_name_format[i++] = '%';
  x_name_format[i++] = 'c';
  x_name_format[i++] = ' ';
  x_name_format[i++] = '\0';
  for (j = x_max_len - 1; j >= 0; j--) {
    for (i = 0; i < y_max_len + 2; i++) fprintf(stream, " ");
    for (i = 0; i < N; i++) {
      if ((len = strlen(x_labels[i])) > j) 
        fprintf(stream, x_name_format, x_labels[i][len-j-1]);
      else fprintf(stream, x_name_format, ' ');
    }
    fprintf(stream, "\n");
  }

  /* Print the matrix and y-labels */
  sprintf(y_name_format, "%%%is", y_max_len + 1);
  sprintf(a_format, "%%%is", a_max_len);
  for (i = 0; i < N; i++) {
    fprintf(stream, y_name_format, y_labels[i]);
    for (j = 0; j < N; j++) {
      fprintf(stream, " ");
      sprintf(a, format, matrix[i][j]);
      fprintf(stream, a_format, a);
    }
    fprintf(stream, "\n");
  }
}



/* ------------          Printing inside a border        ------------------ */

static int border_width;
static char *left_border, *right_border;
static char border_line;

/*                    Initialise border printing                            */
/*    The border will be of width 'width'.                                  */
void 
InitBorderPrint(int width, char *left, char line, char *right) {
  border_width = width;
  STRDUP(left_border, left);
  STRDUP(right_border, right);
  border_line = line;
}


/*    Use this function to print the first and last line.                   */
/*    'title' will be printed centered in the line of stars                 */
void 
BfprintfTitle(FILE *stream, char *format, ...) {
  va_list args;
  char *line;
  char *title;
  int i, len, offset;

  CALLOC(line, border_width + 3, char)

  /* Get the title */
  CALLOC(title, 1024, char)
  va_start(args, format);
  vsprintf(title, format, args);
  va_end(args);

  for (i = 0; i < border_width; i++) line[i] = border_line;

  strncpy(line, left_border, strlen(left_border));
  strncpy(line + border_width - strlen(right_border), 
          right_border, strlen(right_border));

  if (!title) printf("%s\n", line);
  else {
    len = strlen(title);
    if (len == 0) printf("%s\n", line);
    else {
      if (len > border_width - 4) len = border_width - 4;

      offset = (border_width - len) / 2 - 2;

      line[offset] = ' ';
      strncpy(line + offset + 1, title, len);
      line[offset + len + 1] = ' ';

      fprintf(stream, "%s\n", line);
    }
  }

  FREE(line)
  FREE(title)
}


/*    Use this function to print the intermediate lines.                    */
/*    'text' will be printed left-aligned                                   */
void 
BfprintfL(FILE *stream, char *format, ...) {
  va_list args;
  char *text;
  char *line;
  int i;

  CALLOC(line, border_width + 3, char)

  /* Get the text */
  CALLOC(text, 1024, char)
  va_start(args, format);
  vsprintf(text, format, args);
  va_end(args);

  for (i = 1; i < border_width - 1; i++) line[i] = ' ';

  strncpy(line, left_border, strlen(left_border));
  strncpy(line + border_width - strlen(right_border), 
          right_border, strlen(right_border));

  strncpy(line + 4, text, 
      (strlen(text) + 6) > strlen(line) ? strlen(line) - 6 : strlen(text));

  fprintf(stream, "%s\n", line);

  FREE(line)
  FREE(text)
}


/*    Use this function to print the intermediate lines.                    */
/*    'text' will be printed center-aligned                                 */
void 
BfprintfC(FILE *stream, char *format, ...) {
  va_list args;
  char *line;
  char *text;
  int i, len, offset;

  CALLOC(line, border_width + 3, char)

  /* Get the text */
  CALLOC(text, 1024, char)
  va_start(args, format);
  vsprintf(text, format, args);
  va_end(args);

  for (i = 1; i < border_width - 1; i++) line[i] = ' ';

  strncpy(line, left_border, strlen(left_border));
  strncpy(line + border_width - strlen(right_border), 
          right_border, strlen(right_border));

  if (!text) printf("%s\n", line);
  else {
    len = strlen(text);
    if (len == 0) printf("%s\n", line);
    else {
      if (len > border_width - 4) len = border_width - 4;

      offset = (border_width - len) / 2;

      strncpy(line + offset, text, len);

      fprintf(stream, "%s\n", line);
    }
  }

  FREE(line)
  FREE(text)
}


/*    Use this function to print the intermediate lines.                    */
/*    'text' will be printed right-aligned                                  */
void 
BfprintfR(FILE *stream, char *format, ...) {
  va_list args;
  char *line;
  char *text;
  int i, len, offset;

  CALLOC(line, border_width + 3, char)

  /* Get the text */
  CALLOC(text, 1024, char)
  va_start(args, format);
  vsprintf(text, format, args);
  va_end(args);

  for (i = 1; i < border_width - 1; i++) line[i] = ' ';

  strncpy(line, left_border, strlen(left_border));
  strncpy(line + border_width - strlen(right_border), 
          right_border, strlen(right_border));

  if (!text) printf("%s\n", line);
  else {
    len = strlen(text);
    if (len == 0) printf("%s\n", line);
    else {
      if (len > border_width - 4) len = border_width - 4;

      offset = (border_width - len) - 2;

      strncpy(line + offset, text, len);

      fprintf(stream, "%s\n", line);
    }
  }
  FREE(line);
  FREE(text);
}


/* --------------------------------------------------------------------------*/
/*           END OF FILE :  Goodies.c                                        */
/* --------------------------------------------------------------------------*/
