/* ------------------------------------------------------------------------- */
/*       Modified version of the NIST sphere file format functions           */
/* ------------------------------------------------------------------------- */
/*                                                                           */
/*   Copyright (C) 1996, 1997, 1998 Nikko Ström                              */
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


/********************************************/
/** NIST Speech Header Resources (SPHERE)  **/
/** Release 1.5 (beta)                     **/
/** Stan Janet (stan@jaguar.ncsl.nist.gov) **/
/** October 1990                           **/
/********************************************/

#include <stdio.h>

#define TRUE			1
#define FALSE			0

#define MAXFIELDS		8000

#define FPNULL			((FILE *) NULL)
#define CNULL			((char *) NULL)
#define INULL			((int *) NULL)
#define LNULL			((long *) NULL)
#define FNULL			((struct field_t *) NULL)
#define FVNULL			((struct field_t **) NULL)
#define HDRNULL			((struct header_t *) NULL)

#define T_INTEGER		0
#define T_REAL			1
#define T_STRING		2

#define N_STDFIELDS		11

#define ERROR_EXIT_STATUS	1

struct field_t {
	int type;
	char *name;
	char *data;
	int datalen;
	char *comment;
};

typedef struct header_t {
	int fc;
	struct field_t **fv;
} sphere_header;

/* File: sysparams.h */

#define FUNCTION

#define MIN(x,y)                ((x) < (y) ? (x) : (y))
#define MAX(x,y)                ((x) > (y) ? (x) : (y))

#define LINESIZE                16384

#define COMMENT_CHAR            ';'
#define NISTLABEL               "NIST_1A"
#define ENDSTR                  "end_head"
#define HDR_ID_SIZE             8
#define HDR_SIZE_SIZE           8
#define PAD_NEWLINES            24
#define PAD_CHAR                ' '
#define PAD_MULT                1024

struct fileheader_fixed {
        char header_id[HDR_ID_SIZE];
        char header_size[HDR_SIZE_SIZE];
};

/* File: sysfunctions.h */


/* Support library functions */

int spx_read_header();
struct field_t **spx_get_field_vector();
int spx_copy_field_vector();
struct header_t *spx_allocate_header();
int spx_deallocate_header();
struct field_t *spx_allocate_field();
struct field_t *spx_allocate_field_str();
int spx_deallocate_field();
int spx_tp();

extern char *spx_malloc();
extern int spf_free();


/* User library functions */

sphere_header *nicosp_open_header(FILE *fp, char **error, int parse_flag);
sphere_header *nicosp_create_header();
int nicosp_close_header(sphere_header *);

FILE *nicosp_get_fp();
int nicosp_set_fp();

int nicosp_get_hbytes();
int nicosp_get_nfields();
int nicosp_get_fieldnames();

int nicosp_print_lines();
int nicosp_format_lines();
int nicosp_fpcopy();

int nicosp_get_field(struct header_t *h, char *name, int *type, int *size);
int nicosp_get_data(sphere_header *, char *name, char *buf, int *len);
int nicosp_get_type(sphere_header *, char *name);
int nicosp_get_size(sphere_header *, char *name);

int nicosp_clear_fields();
int nicosp_add_field();
int nicosp_delete_field();
int nicosp_change_field();

int nicosp_is_std();

extern char *std_fields[];

void nicosp_set_dealloc();
int nicosp_get_dealloc();
void spx_free(char *);
