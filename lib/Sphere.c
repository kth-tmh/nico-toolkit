/* ------------------------------------------------------------------------- */
/*       Modified version of the NIST sphere file format functions           */
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


/********************************************/
/** NIST Speech Header Resources (SPHERE)  **/
/** Release 1.5 (beta)                     **/
/** Stan Janet (stan@jaguar.ncsl.nist.gov) **/
/** October 1990                           **/
/********************************************/

/* LINTLIBRARY */

/** File: spinput.c **/

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <sys/types.h>

#include "Sphere.h"

int farray_fields;
struct field_t *farray[MAXFIELDS];
char *header = CNULL;

static int 
parse_header();
static char *
parse_line();

/*******************************************************************/
/* Reads a NIST header from file pointer "fp" into a buffer, then  */
/*    calls parse_header() to parse the buffer if "parse_flag" is  */
/*    set.                                                         */
/* On failure, "*error" is set to a string describing the error.   */
/*******************************************************************/

int 
spx_read_header(fp,header_size,parse_flag,error)
     FILE *fp;
     char **error;
     int *header_size, parse_flag;
{
  char *p;
  static struct fileheader_fixed fh;
  int hsize, hfields;

  if (fp == FPNULL) {
    *error = "File pointer is null";
    goto errexit;
  }
  if (ftell(fp) != 0L) {
    *error = "File pointer not at beginning of file";
    goto errexit;
  }
  if (fread((char *) &fh, sizeof fh, 1, fp) != 1) {
    *error = "Fread for fixed part of header failed";
    goto errexit;
  }
  if (fh.header_id[sizeof(fh.header_id) - 1] != '\n') {
    *error = "Bad header label line";
    goto errexit;
  }
  if (strncmp(fh.header_id,NISTLABEL,sizeof(NISTLABEL)-1) != 0) {
    *error = "Bad header label";
    goto errexit;
  }
  if (fh.header_size[sizeof(fh.header_size) - 1] != '\n') {
    *error = "Bad header size line";
    goto errexit;
  }
  p = fh.header_size;
  while ((p < &fh.header_size[sizeof(fh.header_size)-1]) && (*p == ' '))
    p++;
  if (! isdigit(*p)) {
    *error = "Bad header size specifier";
    goto errexit;
  }
  hsize = atoi(p);
  if (hsize < sizeof fh) {
    *error = "Specified header size is too small";
    goto errexit;
  }
  header = spx_malloc((u_int) (hsize - sizeof fh + 1));
  if (header == CNULL) {
    *error = "Malloc for header failed";
    goto errexit;
  }
  if (fread(header,hsize-sizeof fh,1,fp) != 1) {
    *error = "Can't read entire header into memory";
    goto errexit;
  }
  if (parse_flag && (parse_header(header,hsize,&hfields,error) < 0))
    goto errexit;

  spx_free(header);
  header = CNULL;
  if (header_size != INULL)
    *header_size = hsize;
  return 0;

errexit:
  if (header != CNULL) {
    spx_free(header);
    header = CNULL;
  }
  return -1;
}

/************************************************************/
/* Parses the bytes read from a speech file and inserts the */
/*    fields into "farray".                                 */
/* If the parsing finishes with success, the fields should  */
/*    then be copied into a header structure for the user.  */
/************************************************************/

static int 
parse_header(p,hsize,fields,error)
     register char *p;
     char **error;
     int hsize, *fields;
{
  register char *lim = p + (hsize - sizeof(struct fileheader_fixed));
  int i, remaining;

  farray_fields = 0;
  for (i = 0; i < MAXFIELDS; i++)
    farray[i] = FNULL;

  *lim = '\0';		/* by setting last character in buffer to NULL,   */
  *fields = 0;		/* index() can be used at any point in buffer w/o */
  /* accessing potentially-illegal addresses        */

  while (p < lim) {
    remaining = lim - p;
    if (remaining < sizeof(ENDSTR)-1) {
      *error = "Bad header end";
      return -1;
    }
    if (*p == COMMENT_CHAR) {
      while ((p < lim) && (*p != '\n'))
	p++;
      if (p < lim) p++;
    } else if (isalpha(*p)) {
      register char *t, *v;

      if ((strncmp(p,ENDSTR,sizeof(ENDSTR)-1) == 0) &&
	  ((remaining == sizeof(ENDSTR)-1) ||
	   (*(p+sizeof(ENDSTR)-1) == ' ') ||
	   (*(p+sizeof(ENDSTR)-1) == '\n')))
	return 0;
      t = index(p,' ');
      if (t == CNULL) {
	*error = "space expected after field name";
	return -1;
      }
      v = index(t+1,' ');
      if (v == CNULL) {
	*error = "space expected after type specifier";
	return -1;
      }
      p = parse_line(p,t,v,error);
      if (p == CNULL)
	return -1;
      ++*fields;
    } else {
      *error = "Bad character at beginning of line";
      return -1;
    }
  }
  return 0;
}

/*********************************************************************/
/* Parses a line from a speech file.                                 */
/* The arguments to parse_line() point into a line in the header     */
/*    buffer as follows:                                             */
/*                                                                   */
/*	field type value[;.....]\n                                   */
/* 	^    ^    ^                                                  */
/*	h    t    v                                                  */
/*********************************************************************/

static char *
parse_line(h,t,v,error)
     char *h, *t, *v, **error;
{
  struct field_t *f;
  int vtype, vlen;
  char *endofvalue = v + 1, *endoffieldname = h, *ptr;

  if (farray_fields >= MAXFIELDS) {
    *error = "too many fields";
    return CNULL;
  }
  *t = '\0';
  while (isalnum(*endoffieldname) || (*endoffieldname == '_'))
    endoffieldname++;
  if (endoffieldname != t) {
    *error = "space expected after field name";
    return CNULL;
  }
  if (*(t+1) != '-') {
    *error = "dash expected in type specifier";
    return CNULL;
  }
  switch (*(t+2)) {
  case 'i':
    vtype = T_INTEGER;
    while (isdigit(*endofvalue) || (*endofvalue == '-'))
      ++endofvalue;
    vlen = endofvalue - (v + 1);
    break;
  case 'r':
    vtype = T_REAL;
    while (isdigit(*endofvalue) ||
	   (*endofvalue == '.') ||
	   (*endofvalue == '-'))
      ++endofvalue;
    vlen = endofvalue - (v + 1);
    break;
  case 's':
    vtype = T_STRING;
    vlen = 0;
    ptr = t + 3;
    while (isdigit(*ptr))
      vlen = 10 * vlen + (*ptr++ - '0');
    if (! vlen) {
      *error = "bad string length";
      return CNULL;
    }
    if (ptr != v) {
      *error = "space expected after type specifier";
      return CNULL;
    }
    endofvalue = v + vlen + 1;
    break;
  default:
    *error = "unknown type specifier";
    return CNULL;
  }

  {
    /* Really only need the function call, but by null-terminating the     */
    /* string at (v+1), looking at a stack trace in "dbx" (a BSD Unix      */
    /* debugger) is easier. Otherwise, "dbx" will expect a null-terminator */
    /* and print the rest of the header block.                             */

    int ch = *(v + 1 + vlen);

    *(v + 1 + vlen) = '\0';
    f = spx_allocate_field_str(vtype,h,v+1,vlen);
    *(v + 1 + vlen) = ch;
  }

  if (f == FNULL) {
    *error = "Malloc for triple failed";
    return CNULL;
  }
  farray[farray_fields++] = f;

  switch (*endofvalue) {
  case COMMENT_CHAR:
  case '\n':
    return endofvalue + 1;
  case ' ':
    while (*endofvalue == ' ')
      ++endofvalue;
    if (*endofvalue == '\n')
      return endofvalue + 1;
    if (*endofvalue == COMMENT_CHAR) {
      char *eoln = index(endofvalue,'\n');
      if (eoln != CNULL)
	return eoln + 1;
    }
    *error = "bad character after triple and space(s)";
    return CNULL;
  }
  *error = "bad character after triple";
  return CNULL;
}

/********************************************************/

struct header_t *spx_allocate_header(fc,fv)
     int fc;
     struct field_t **fv;
{
  register struct header_t *h;

  h = (struct header_t *) spx_malloc((u_int) sizeof(struct header_t));
  if (h != HDRNULL) {
    h->fc = fc;
    h->fv = fv;
  }
  return h;
}

/**********************************************************/
/* Deallocates a header by freeing the structure that     */
/*    represents it.                                      */
/**********************************************************/

int 
spx_deallocate_header(h)
     struct header_t *h;
{
  if (h == HDRNULL)	return -1;	/* check sanity of arguments */

  spx_free((char *) h);
  return 0;
}

/**********************************************************/
/* Allocates room for a field with name "fieldname", type */
/*    "type" represented at address "v" and comprising    */
/*    "vlen" bytes.                                       */
/**********************************************************/

struct field_t *spx_allocate_field(type,fieldname,v,vlen)
     int type, vlen;
     char *fieldname, *v;
{
  static char buffer[1024];
  /* plenty big enough for storing ascii form of numbers */

  if (fieldname == CNULL) return FNULL;
  if (v == CNULL) return FNULL;

  switch (type) {
  case T_INTEGER:
    sprintf(buffer,"%ld",*(long *)v);
    break;
  case T_REAL:
    sprintf(buffer,"%f",*(double *)v);
    break;
  case T_STRING:
    if (vlen <= 0)
      return FNULL;
    return spx_allocate_field_str(type,fieldname,v,vlen);
    break;
  default:
    return FNULL;
  }
  return spx_allocate_field_str(type,fieldname,buffer,strlen(buffer));
}

/***************************************************************/

struct field_t *spx_allocate_field_str(type,fieldname,value,vlen)
     int type, vlen;
     char *fieldname, *value;
{
  register struct field_t *f;
  char *p1, *p2;

  if (vlen <= 0)
    return FNULL;

  f = (struct field_t *) spx_malloc((u_int) sizeof(struct field_t));
  if (f == FNULL)
    return FNULL;

  p1 = spx_malloc((u_int) (strlen(fieldname) + 1));
  if (p1 == CNULL) {
    spx_free((char *) f);
    return FNULL;
  }

  p2 = spx_malloc((u_int) (vlen + 1));
  if (p2 == CNULL) {
    spx_free((char *) f);
    spx_free(p1);
    return FNULL;
  }

  f->type = type;
  f->name = p1;
  f->data = p2;
  f->datalen = vlen;
  (void) strcpy(p1,fieldname);
  (void) bcopy(value,p2,vlen);
  p2[vlen] = '\0';

  return f;
}

/*******************************************************************/
/* Deallocates a field by freeing bytes used to store the field    */
/*    name and value, then freeing bytes that were allocated for   */
/*    the structure.                                               */
/*******************************************************************/

int 
spx_deallocate_field(fv)
     struct field_t *fv;
{
  if (fv == FNULL)	return -1;	/* check sanity of arguments */

  spx_free(fv->name);
  spx_free(fv->data);
  spx_free((char *) fv);
  return 0;
}

/******************************************************************/
/* Returns a pointer to a vector of field structures for the      */
/*    specified number of elements.                               */
/******************************************************************/

struct field_t **spx_get_field_vector(elements)
     int elements;
{
  if (elements <= 0)
    return FVNULL;
  return (struct field_t **)
    spx_malloc((u_int) (elements * sizeof(struct field_t *)));
}

/**********************************************************/
/* Copies field vector "src_fv" to field vector "dst_fv". */
/* The number of fields in the source vector must be      */
/*    specified by "elements", a positive number.         */
/**********************************************************/

int 
spx_copy_field_vector(src_fv, dst_fv, elements)
     struct field_t **src_fv, **dst_fv;
     int elements;
{
  int vbytes;

  if (elements <= 0)
    return -1;
  vbytes = elements * sizeof(struct field_t *);
  (void) bcopy((char *) src_fv, (char *) dst_fv, vbytes);
  return 0;
}
/********************************************/
/** NIST Speech Header Resources (SPHERE)  **/
/** Release 1.5 (beta)                     **/
/** Stan Janet (stan@jaguar.ncsl.nist.gov) **/
/** October 1990                           **/
/********************************************/

/* LINTLIBRARY */

/* File: spmalloc.c */


static int dealloc = 1;

char *
spx_malloc(u_int bytes) {
  if (bytes == 0)
    bytes = 1;
  return (char *)malloc(bytes);
}

void 
spx_free(char *p) {
  if (dealloc)
    (void) free(p);
}

FUNCTION void nicosp_set_dealloc(int n) {
  dealloc = n;
}

FUNCTION int nicosp_get_dealloc() {
  return dealloc;
}

/********************************************/
/** NIST Speech Header Resources (SPHERE)  **/
/** Release 1.5 (beta)                     **/
/** Stan Janet (stan@jaguar.ncsl.nist.gov) **/
/** October 1990                           **/
/********************************************/

/* LINTLIBRARY */

/** File: spoutput.c **/

#define NAMEWIDTH	30
/** Field names are printed to this width, left-justified, in */
/** nicosp_print_lines().                                         */


/******************************************************************/
/* Prints the specified header to stream fp in the standard       */
/* format that will allow "nicosp_open_header() to read it back       */
/* in later.                                                      */
/* More specifically:                                             */
/* writes "header_end" after the last field, then adds some       */
/* newlines (so pagers can be used to look at the file a          */
/* screenful at a time without seeing the samples themselves),    */
/* and then pads the header to a multiple of PAD_MULT bytes.      */
/* That constant is #define'd in sysparam.h, currently at 1024.   */
/* Padding is accomplished via ftell(), so it won't work on       */
/* systems where ftell() doesn't return byte offsets.             */
/******************************************************************/

FUNCTION int nicosp_write_header(fp,h,hbytes,databytes)
     register FILE *fp;
     struct header_t *h;
     long *hbytes, *databytes;
{
  register int c;
  long d=0, hpos=0, p, pbytes;
  static struct fileheader_fixed fh;

  if (fp == FPNULL)	return -1;	/* check sanity of arguments */
  if (h == HDRNULL)	return -1;
  if (hbytes == LNULL)	return -1;
  if (databytes == LNULL)	return -1;

  clearerr(fp);
  if (fseek(fp,(long)sizeof(struct fileheader_fixed),0) == -1)
    return -1;
  if (nicosp_format_lines(h,fp) < 0)
    return -1;
  (void) fprintf(fp,"%s\n",ENDSTR);
  d = ftell(fp);

  for (c=0; c < PAD_NEWLINES; c++)	/* pad with newlines always */
    (void) putc('\n',fp);
  pbytes = PAD_MULT - (ftell(fp) % PAD_MULT);
  for (p=0; p < pbytes; p++)		/* pad to multiple of PAD_MULT */
    (void) putc(PAD_CHAR,fp);
  hpos = ftell(fp);
  if (hpos % PAD_MULT != 0)
    return -1;

  /** now that we know how big the header is, seek to fixed part and write */
  /** NIST label and header size in bytes as first two lines. */
  if (fseek(fp,0L,0) == -1) return -1;
  sprintf((char *) &fh,"%*s\n%*ld\n",
	  sizeof(fh.header_id)-1,NISTLABEL,
	  sizeof(fh.header_size)-1,hpos);
  (void) fwrite((char *) &fh,sizeof(struct fileheader_fixed),1,fp);
  (void) fflush(fp);

  if (fseek(fp,hpos,0) == -1) return -1;
  if (ferror(fp)) return -1;

  *hbytes = hpos;		/* on success, return #bytes in header */
  *databytes = d;		/* and #bytes before padding           */
  return 0;
}

/*********************************************************************/
/* Dumps header fields to stream fp in a readable form:              */
/*        fieldnumber: fieldtype fieldname "field_value"             */
/*   where fieldtype is either 's', 'i' or 'r'.                      */
/* Field numbers start at 0.                                         */
/* Beware printing headers with unprintable characters in data       */
/*   fields using this function.                                     */
/*********************************************************************/

FUNCTION int nicosp_print_lines(h,fp)
     register FILE *fp;
     struct header_t *h;
{
  register int i, fc;
  register struct field_t **fv;
  int len, j;
  char *p;

  if (h == HDRNULL)	return -1;	/* check sanity of arguments */
  if (fp == FPNULL)	return -1;

  clearerr(fp);
  fv = h->fv;
  fc = h->fc;
  for (i=0; i < fc; i++) {
    len = strlen(fv[i]->name);
    if (len <= NAMEWIDTH)
      (void) fprintf(fp,"%5d: %c %-*s ",i,spx_tp(fv[i]->type),NAMEWIDTH,fv[i]->name);
    else
      (void) fprintf(fp,"%5d: %c %s ",i,spx_tp(fv[i]->type),fv[i]->name);
    for (j=0, p=fv[i]->data; j < fv[i]->datalen; j++, p++)
      if (isprint(*p) || (*p == '\n') || (*p == '\t'))
	(void) putc(*p,fp);
      else
	putc(0, fp);
	(void) fprintf(fp,"%o",*p);
    (void) fprintf(fp,"\n");
  }
  if (ferror(fp)) {
    clearerr(fp);
    return -1;
  }
  return 0;
}

/********************************************************************/
/* Write header fields to stream fp in NIST SPHERE format.          */
/* Format is basically: name -type value.                           */
/* See documentation for the complete header grammar.               */
/********************************************************************/

FUNCTION int nicosp_format_lines(h,fp)
     struct header_t *h;
     register FILE *fp;
{
  int i, j, fc;
  char *p;
  register struct field_t **fv;

  if (h == HDRNULL)	return -1;	/* check sanity of arguments */
  if (fp == FPNULL)	return -1;

  clearerr(fp);
  fv = h->fv;
  fc = h->fc;
  for (i=0; i < fc; i++) {
    (void) fprintf(fp,"%s -%c",fv[i]->name,spx_tp(fv[i]->type));
    if (fv[i]->type == T_STRING)
      (void) fprintf(fp,"%d",strlen(fv[i]->data));
    (void) fprintf(fp," ");
    for (j=0, p=fv[i]->data; j < fv[i]->datalen; j++, p++)
      (void) putc(*p,fp);
    (void) fprintf(fp,"\n");
  }
  if (ferror(fp)) {
    clearerr(fp);
    return -1;
  }
  return 0;
}

/************************************************************/
/** Returns a character that represents the type associated */
/** with the integer parameter, '?' for bad type.           */
/************************************************************/

int 
spx_tp(ftype)
     register int ftype;
{
  register int result;

  switch (ftype) {
  case T_INTEGER:
    result = 'i'; break;
  case T_REAL:
    result = 'r'; break;
  case T_STRING:
    result = 's'; break;
  default:
    result = '?'; break;
  }

  return result;
}

/**************************************************************/
/* Copies stream fp to stream outfp through EOF.              */
/* Returns -1 if an error occurs (either reading or writing). */
/**************************************************************/

FUNCTION int nicosp_fpcopy(fp,outfp)
     register FILE *fp, *outfp;
{
  register int c;

  while ((c = getc(fp)) != EOF)
    if (putc(c,outfp) == EOF)
      return -1;
  return (ferror(fp) || ferror(outfp)) ? -1 : 0;
}
/********************************************/
/** NIST Speech Header Resources (SPHERE)  **/
/** Release 1.5 (beta)                     **/
/** Stan Janet (stan@jaguar.ncsl.nist.gov) **/
/** October 1990                           **/
/********************************************/

/* LINTLIBRARY */

/** File: nicosp_utils.c **/

extern int farray_fields;
extern struct field_t *farray[];

/***************************************************************/
/* Reads an existing header in from file pointer "fp".         */
/* The file pointer is assumed to be positioned at the         */
/*    beginning of a speech file with a header in NIST SPHERE  */
/*    format.                                                  */
/* On success, "fp" is positioned at the end of the header     */
/*    (ready to read samples) and a pointer to a header        */
/*    structure is returned.                                   */
/* On failure, argument "error" will point to a string         */
/*    describing the problem.                                  */
/* If "parse_flag" is false (zero), the fields in the header   */
/*    will not be parsed and inserted into the header          */
/*    structure; the structure will contain zero fields.       */
/*    This is useful for operations on files when the contents */
/*    of the header are not important, for example when        */
/*    stripping the header.                                    */
/***************************************************************/

sphere_header *nicosp_open_header(FILE *fp, char **error, int parse_flag) {
  register struct header_t *h;
  int header_size, i;
  struct field_t **fv;

  if (fp == FPNULL)	return HDRNULL;	/* check sanity of arguments */

  if (ftell(fp) != 0L)	return HDRNULL; /* fp must be at beginning of file */

  if (spx_read_header(fp,&header_size,parse_flag,error) < 0)
    return HDRNULL;

  if ((! parse_flag) || (farray_fields == 0))
    fv = FVNULL;
  else {
    fv = spx_get_field_vector(farray_fields);
    if (fv == FVNULL) {
      for (i=0; i<farray_fields; i++)
	(void) spx_deallocate_field(farray[i]);
      return HDRNULL;
    }
    (void) spx_copy_field_vector(farray, fv, farray_fields);
  }

  h = spx_allocate_header(farray_fields,fv);
  if (h == HDRNULL)
    for (i=0; i<farray_fields; i++)
      (void) spx_deallocate_field(farray[i]);
  return h;
}

/*******************************************************************/
/* Deletes all fields from the header pointed to by h.             */
/*******************************************************************/

FUNCTION int nicosp_clear_fields(h)
     register struct header_t *h;
{
  register int i, j, errors = 0;

  if (h == HDRNULL)	return -1;	/* check sanity of arguments */

  for (i=0, j = h->fc; i<j; i++) {
    if (spx_deallocate_field(h->fv[i]) < 0)
      errors++;
    h->fv[i] = FNULL;
  }
  if (h->fv != FVNULL)
    spx_free((char *) h->fv);
  h->fv = FVNULL;
  h->fc = 0;

  return errors ? -1 : 0;
}

/***********************************************************************/
/* Reclaims the space allocated for the header structure pointed to    */
/* by h. First reclaims all space allocated for the header's fields,   */
/* if any exist.                                                       */
/***********************************************************************/

FUNCTION int nicosp_close_header(h)
     register struct header_t *h;
{
  (void) nicosp_clear_fields(h);
  spx_free((char *) h);
  return 0;
}

/*********************************************************************/
/* Returns the number of fields stored in the specified header.      */
/*********************************************************************/

FUNCTION int nicosp_get_nfields(h)
     struct header_t *h;
{
  if (h == HDRNULL)	return -1;	/* check sanity of arguments */

  return h->fc;
}

/*********************************************************************/
/* Fills in an array of character pointers with addresses of the     */
/* fields in the specified header. No more than n pointers in the    */
/* array will be set.                                                */
/* Returns the number of pointers set.                               */
/*********************************************************************/

FUNCTION int nicosp_get_fieldnames(h,n,v)
     struct header_t *h;
     int n;
     char *v[];
{
  register struct field_t **fv;
  int i, fc;

  if (h == HDRNULL)
    return -1;	/* check sanity of arguments */
  if (v == (char **) NULL)
    return -1;

  fc = h->fc;
  fv = h->fv;
  for (i=0; i < fc && i < n; i++)
    v[i] = fv[i]->name;
  return i;
}

/***********************************************************************/
/* Returns the type and size (in bytes) of the specified header field. */
/* Types are T_INTEGER, T_REAL, T_STRING (defined in header.h).        */
/* The size of a T_INTEGER field is sizeof(long).                      */
/* The size of a T_REAL field is sizeof(double).                       */
/* The size of a string is variable and does not includes a            */
/*    null-terminator byte (null bytes are allowed in a string).       */
/***********************************************************************/

int 
nicosp_get_field(struct header_t *h, char *name, int *type, int *size) {
  register int i, fc;
  register struct field_t **fv;

  if (h == HDRNULL)	return -1;	/* check sanity of arguments */
  if (name == CNULL)	return -1;

  fc = h->fc;
  fv = h->fv;
  for (i=0; i < fc ; i++, fv++)
    if (strcmp(name,(*fv)->name) == 0) {
      switch ((*fv)->type) {
      case T_INTEGER:
	*size = sizeof(long);
	break;
      case T_REAL:
	*size = sizeof(double);
	break;
      case T_STRING:
	*size = (*fv)->datalen;
	break;
      default:
	return -1;
      }
      *type = (*fv)->type;
      return 0;
    }
  return -1;
}

/*********************************************************************/
/* Returns the type of the specified header field.                   */
/* Types are T_INTEGER, T_REAL, T_STRING (defined in header.h).      */
/*********************************************************************/

FUNCTION int nicosp_get_type(h,name)
     struct header_t *h;
     char *name;
{
  register int i, fc;
  register struct field_t **fv;

  if (h == HDRNULL)	return -1;	/* check sanity of arguments */
  if (name == CNULL)	return -1;

  fc = h->fc;
  fv = h->fv;
  for (i=0; i < fc ; i++, fv++)
    if (strcmp(name,(*fv)->name) == 0)
      switch ((*fv)->type) {
      case T_INTEGER:
      case T_REAL:
      case T_STRING:
	return (*fv)->type;
      default:
	return -1;
      }
  return -1;
}

/*********************************************************************/
/* Returns the size (in bytes) of the specified header field.        */
/* The size of a T_INTEGER field is sizeof(long).                    */
/* The size of a T_REAL field is sizeof(double).                     */
/* The size of a string is variable and does not includes a          */
/*    null-terminator byte (null bytes are allowed in a string).     */
/*********************************************************************/

FUNCTION int nicosp_get_size(h,name)
     struct header_t *h;
     char *name;
{
  register int i, fc;
  register struct field_t **fv;

  if (h == HDRNULL)	return -1;	/* check sanity of arguments */
  if (name == CNULL)	return -1;

  fc = h->fc;
  fv = h->fv;
  for (i=0; i < fc ; i++, fv++)
    if (strcmp(name,(*fv)->name) == 0)
      switch ((*fv)->type) {
      case T_INTEGER:
	return sizeof(long);
      case T_REAL:
	return sizeof(double);
      case T_STRING:
	return (*fv)->datalen;
      default:
	return -1;
      }
  return -1;
}

/***********************************************************************/
/* Returns the value of the specifed header field in "buf".            */
/* No more than "len" bytes are copied; "len" must be positive.        */
/* It really doesn't make much sense to ask for part of a long or      */
/*    double, but it's not illegal.                                    */
/* Remember that strings are not null-terminated.                      */
/***********************************************************************/

FUNCTION int nicosp_get_data(h,name,buf,len)
     struct header_t *h;
     char *name, *buf;
     int *len;
{
  register struct field_t **fv;
  register int i, fc;
  long n;
  double x;

  if (h == HDRNULL)	return -1;	/* check sanity of arguments */
  if (name == CNULL)	return -1;
  if (buf == CNULL)	return -1;
  if (len == INULL)	return -1;
  if (*len <= 0)		return -1;

  fc = h->fc;
  fv = h->fv;
  for (i=0; i<fc; i++, fv++)
    if (strcmp(name,(*fv)->name) == 0) {
      switch ((*fv)->type) {
      case T_INTEGER:
	n = atol((*fv)->data);
	*len = MIN(*len,sizeof(long));
	(void) bcopy((char *) &n, buf, *len);
	break;
      case T_REAL:
	x = atof((*fv)->data);
	*len = MIN(*len,sizeof(double));
	(void) bcopy((char *) &x, buf, *len);
	break;
      case T_STRING:
	*len = MIN(*len,(*fv)->datalen);
	(void) bcopy((*fv)->data, buf, *len);
	break;
      default:
	return -1;
      }
      return 0;
    }
  return -1;
}

/*******************************************************************/
/* Adds the field "name" to header specified by "h".               */
/* Argument "type" is T_INTEGER, T_REAL, or T_STRING.              */
/* Argument "p" is a pointer to a long integer, a double, or a     */
/*    character cast if necessary to a character pointer.          */
/* The specified field must not already exist in the header.       */
/*******************************************************************/

FUNCTION int nicosp_add_field(h,name,type,p)
     struct header_t *h;
     int type;
     char *name, *p;
{
  register struct field_t **fv, *nf;
  int size, i, fc;

  if (h == HDRNULL)		return -1;	/* check sanity of arguments */
  if (h->fc < 0)			return -1;
  if (name == CNULL)		return -1;
  if (p == CNULL)			return -1;
  if (spx_tp(type) == '?')	return -1;

  fc = h->fc;
  for (i=0; i < fc; i++)
    if (strcmp(name,h->fv[i]->name) == 0)
      return -1;

  switch (type) {
  case T_INTEGER:
    size = sizeof(long);
    break;
  case T_REAL:
    size = sizeof(double);
    break;
  default:
    size = strlen(p);
    break;
  }

  nf = spx_allocate_field(type,name,p,size);
  if (nf == FNULL)
    return -1;

  fv = spx_get_field_vector(fc + 1);
  if (fv == FVNULL) {
    (void) spx_deallocate_field(nf);
    return -1;
  }

  if (fc > 0) {
    (void) spx_copy_field_vector(h->fv, fv, fc);
    spx_free((char *) h->fv);
  }
  fv[h->fc++] = nf;
  h->fv = fv;
  return 0;
}

/***********************************************************/
/* Deletes field "name" from header specified by "h".      */
/* The field must exist in the header.                     */
/***********************************************************/

FUNCTION int nicosp_delete_field(h,name)
     struct header_t *h;
     char *name;
{
  struct field_t **tmp_fv, *nf;
  int i, new_fc;

  if (h == HDRNULL)	return -1;	/* check sanity of arguments */
  if (h->fc <= 0)		return -1;
  if (name == CNULL)	return -1;

  if (h->fc > 1) {
    tmp_fv = spx_get_field_vector(h->fc - 1);
    if (tmp_fv == FVNULL)
      return -1;
  } else
    tmp_fv = FVNULL;
  nf = FNULL;
  for (i=0, new_fc=0; i< h->fc; i++)
    if (strcmp(name,h->fv[i]->name) == 0) {
      if (nf != FNULL) {
	spx_free((char *) tmp_fv);
	return -1;
      }
      nf = h->fv[i];
    } else {
      if ((nf == FNULL) && (i == h->fc - 1)) {
	spx_free((char *) tmp_fv);
	return -1;
      }
      tmp_fv[new_fc++] = h->fv[i];
    }
  (void) spx_deallocate_field(nf);
  spx_free((char *) h->fv);
  h->fv = tmp_fv;
  --h->fc;
  return 0;
}

/***********************************************************/
/* Changes an existing field to a new type and/or value.   */
/* The field must already exist in the header.             */
/***********************************************************/

FUNCTION int nicosp_change_field(h,name,type,p)
     struct header_t *h;
     char *name, *p;
     int type;
{
  register int i, index, size;
  struct field_t *nf;

  if (h == HDRNULL)		return -1;	/* check sanity of arguments */
  if (name == CNULL)		return -1;
  if (p == CNULL)			return -1;
  if (spx_tp(type) == '?')	return -1;

  for (i=0, index = -1; i< h->fc; i++)
    if (strcmp(h->fv[i]->name,name) == 0) {
      if (index >= 0) return -1;
      index = i;
    }
  if (index < 0) return -1;

  switch (type) {
  case T_INTEGER:
    size = sizeof(long); break;
  case T_REAL:
    size = sizeof(double); break;
  default:
    size = strlen(p); break;
  }

  nf = spx_allocate_field(type,name,p,size);
  if (nf == FNULL) return -1;

  if (spx_deallocate_field(h->fv[index]) < 0) {
    (void) spx_deallocate_field(nf);
    return -1;
  }
  h->fv[index] = nf;
  return 0;
}

/******************************************************************/
/* Returns a pointer to an empty header.                          */
/* Use nicosp_add_field() to insert fields into it.                   */
/* Use nicosp_print_header() to print it in readable format.          */
/* Use nicosp_format_header() to print it to a file in NIST SPHERE    */
/*      format.                                                   */
/******************************************************************/

FUNCTION struct header_t *nicosp_create_header()
{
  return spx_allocate_header(0,FVNULL);
}

/*******************************************************************/
/* Returns TRUE if the specified field name is a "standard" field, */
/* FALSE otherwise.                                                */
/* Standard fields are listed in stdfield.c.                       */
/*******************************************************************/

FUNCTION int nicosp_is_std(name)
     register char *name;
{
  register char **f;

  if (name == CNULL)
    return FALSE;

  f = &std_fields[0];
  while (*f != CNULL)
    if (strcmp(name,*f++) == 0)
      return TRUE;

  return FALSE;
}

/********************************************/
/** NIST Speech Header Resources (SPHERE)  **/
/** Release 1.5 (beta)                     **/
/** Stan Janet (stan@jaguar.ncsl.nist.gov) **/
/** October 1990                           **/
/********************************************/

/* LINTLIBRARY */

/* File: stdfield.c */

char *std_fields[] = {
  "database_id",
  "database_version",
  "utterance_id",
  "channel_count",
  "sample_count",
  "sample_rate",
  "sample_min",
  "sample_max",
  "sample_n_bytes",
  "sample_byte_format",
  "sample_sig_bits",
  CNULL
};
