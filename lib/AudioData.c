/* ------------------------------------------------------------------------- */
/*                          Audio File Input/Output                          */
/* ------------------------------------------------------------------------- */
/*                                                                           */
/*   Copyright (C) 1996, 1997, 1998 Nikko Strom                              */
/*                                                                           */
/*   All rights reserved.                                                    */
/*                                                                           */
/*   Developed by Nikko Strom at the Dept. of Speech, Music and Hearing,     */
/*   KTH (Royal Institute of Technology), Sweden.                            */
/*                                                                           */
/*   http://www.speech.kth.se                                                */
/*                                                                           */
/*   KTH                                                                     */
/*   Institutionen foer Tal, musik och hörsel                                */
/*   S-100 44 STOCKHOLM                                                      */
/*   SWEDEN                                                                  */
/*                                                                           */
/*   Project web site: http://nico.sourceforge.net/                          */
/*                                                                           */
/* ------------------------------------------------------------------------- */
/* Acknowledgemets to Craig Reese for the liner/mulaw conversion routines,   */
/* NIST for the sphere audio routines, Sten Ternström for the KTH StdHead    */
/* routines, and Erland Lewin (erl@speech.kth.se) for help with debugging.   */
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
#include <string.h>
#include <unistd.h>
#include "StdHead.h"
#include "Sphere.h"
#include "System.h"
#include "AudioData.h"

static int 
ulaw2linear(unsigned char ulawbyte);
static unsigned char linear2ulaw(int sample);

/* Conversion from the string to the code description of audio formats */
AudioFileType Str2AudioType(char *s) {
  if      (strcmp(s, "raw") == 0)      return BINARY_AU;
  else if (strcmp(s, "kth") == 0)      return KTH_AU;
  else if (strcmp(s, "cmu") == 0)      return CMU_AU;
  else if (strcmp(s, "au") == 0)       return SUN_AU;
  else if (strcmp(s, "nist") == 0)     return NIST_AU;
  else if (strcmp(s, "wav") == 0)      return WIN_AU;
  else return UNDEFINED_AU;
}

/* Conversion from the code to the string description of audio formats */
char *
AudioType2Str(AudioFileType t) {
  char *s;
  if      (t == BINARY_AU) STRDUP(s, "binary")
  else if (t == KTH_AU)  STRDUP(s, "kth")
  else if (t == CMU_AU)  STRDUP(s, "cmu")
  else if (t == SUN_AU)  STRDUP(s, "au")
  else if (t == NIST_AU) STRDUP(s, "nist")
  else if (t == WIN_AU)  STRDUP(s, "wav")
  else return NULL;
  return s;
}


/* Use heuristics to try to guess the audio file-type */
AudioFileType GuessAudioFileType(char *fn) {
  FILE *fp;
  long magic;

  FOPEN(fp, fn, "r")
  magic = ReadInt32(fp, 0);
  FCLOSE(fp)

  if (magic == SUNAUMagicNum) return SUN_AU;

  if (magic == WinAUMagicNum) return WIN_AU;

  if (strstr(fn, ".smp")) return KTH_AU;

  /* default */
  return BINARY_AU;
}


static void 
check_nist_int_entry(struct header_t *header_handle, 
                                 char *name, long value) {
  int status, len = sizeof(int);
  long real_value;

  status = nicosp_get_type(header_handle, name);
  if (status < T_INTEGER)
    ErrorExit(SYSTEM_ERR_EXIT, "%s not specified as integer in nist header", name);
  status = nicosp_get_data(header_handle, name, (char *)(&real_value), &len);
  if (status < 0) ErrorExit(SYSTEM_ERR_EXIT, "No sample rate specified in nist header");
  if (real_value != value) 
    ErrorExit(SYSTEM_ERR_EXIT, "Expected %s=%li -- got %li in nist header", 
              name, value, real_value);
}


/*     Open an AUDIO_FILE *                                           */
/* Currently only mode = "r" or mode = "w" is allowed                 */
AUDIO_FILE *
OpenAudioFile(char *fn, AudioFileType format, char *mode) { 
  FILE *fp = NULL;
  AUDIO_FILE *afp;

  if (strcmp(mode, "r") == 0) {
    if (strcmp(fn, "stdin") == 0) fp = stdin;
    else FOPEN(fp, fn, mode);
  }
  else if (strcmp(mode, "w") == 0) {
    if (strcmp(fn, "stdout") == 0) fp = stdout;
    else FOPEN(fp, fn, mode);
  }
  else ErrorExit(SYSTEM_ERR_EXIT, 
		 "Only \"r\" and \"w\" are allowed modes for OpenAudioFile.");

  CALLOC(afp, 1, AUDIO_FILE);

  afp->fp         = fp;
  afp->NumSamples = -1; 
  afp->format     = format;

  STRDUP(afp->mode, mode);
  STRDUP(afp->fn, fn);

  /* Default values... */
  afp->subformat     = 0;
  afp->header_size   = 0;
  afp->little_endian = 0;
  afp->rewindable    = 0;
  afp->eof           = 0;

  switch (afp->format) {
  case BINARY_AU:
    /* Nothing to do. */
    break;

  case KTH_AU:
    if (strcmp(mode, "r") == 0) {
      double sf;
      char file[64];
      HANDLE header_handle;

      header_handle = LoadHeader(afp->fp);
      afp->header_size = 1024;

      GetHeaderStr(header_handle, "file", file, 64);
      if (strcmp(file, "samp") != 0)
        ErrorExit(SYSTEM_ERR_EXIT,
      "This is not a StdHead sample file (header variable file != \"samp\")");

      if (GetHeaderDouble(header_handle, "sftot", &sf))
        afp->samp_freq = (int)sf;
      else afp->samp_freq = 16000;
    }
    else { /* mode = "w" */
      HANDLE header_handle;

      header_handle = MakeHeader(1024);
      SetHeaderStr(header_handle, "file", "samp");
      SetHeaderStr(header_handle, "msb", "first");
      SetHeaderStr(header_handle, "nchans", "1");
      SetHeaderStr(header_handle, "preemph", "no");
      SetHeaderStr(header_handle, "sftot", "16000");
      SetHeaderStr(header_handle, "range", "-32768,32767");
      SetHeaderStr(header_handle, "born", "NICO Toolkit");

      SaveHeader(header_handle, afp->fp);
    }
    break;

  case NIST_AU:
    if (strcmp(mode, "r") == 0) {
      int status, len;
      long sf;
      char byteform[4];
      struct header_t *header_handle;
      char *err_msg;

      header_handle = nicosp_open_header(afp->fp, &err_msg, 1);
      afp->header_size = 1024;

      /* Get the sample rate from the header */
      status = nicosp_get_type(header_handle, "sample_rate");
      if (status != T_INTEGER)
        ErrorExit(SYSTEM_ERR_EXIT, "Sample rate not specified as integer in nist header");
      len = sizeof(long);
      status = nicosp_get_data(header_handle, "sample_rate", (char *)(&sf), &len);
      if (status < 0) ErrorExit(SYSTEM_ERR_EXIT, "No sample rate specified in nist header");
      afp->samp_freq = sf;

      check_nist_int_entry(header_handle, "sample_n_bytes", 2);
      check_nist_int_entry(header_handle, "channel_count", 1);
      check_nist_int_entry(header_handle, "sample_sig_bits", 16);

      /* Get the byte format from the header */
      status = nicosp_get_type(header_handle, "sample_byte_format");
      if (status != T_STRING)
        ErrorExit(SYSTEM_ERR_EXIT, "Byte format not specified as string in nist header");
      len = 3;
      status = nicosp_get_data(header_handle, "sample_byte_format", byteform, &len);
      byteform[len] = '\0';
      if (status < 0 || strcmp("10", byteform) == 0)
        afp->little_endian = 0;
      else if (strcmp("01", byteform) == 0) 
        afp->little_endian = 1;
      else 
        ErrorExit(SYSTEM_ERR_EXIT, "Unknown sample byte format (%s) specified in nist header",
		  byteform);
    }
    else { /* mode = "w" */
      ErrorExit(SYSTEM_ERR_EXIT, "Writing NIST audio files not implemented");
    }
    break;

  /* Read the Sun AU file format, as documented in 
   * http://www.tnt.uni-hannover.de/soft/audio/tools/ff_convert/aff/ap03.html
   */
  case SUN_AU:
    if (strcmp(mode, "r") == 0) {
	int magic, size, format, samp_freq, channelCount;
			
	magic = ReadInt32(fp, 0);

	if (magic != SUNAUMagicNum)
	  ErrorExit(SYSTEM_ERR_EXIT, "Not a valid .au-file - magic number didn't match.");
			
	afp->header_size = ReadInt32(fp, 0);

	size = ReadInt32(fp, 0);
			
	format = ReadInt32(fp, 0);
	if (format != 3 && format != 1) 
	  ErrorExit(SYSTEM_ERR_EXIT,
          "Only format 1 (mulaw) and 3 (lin16) are implemented for .au-files.");

	afp->subformat = format;
			
	if (size == -1) afp->NumSamples = -1;
	else {
          if (format == 1) afp->NumSamples = size;
          else if (format == 3) afp->NumSamples = size / 2;
	}

	samp_freq = ReadInt32(fp, 0);
	afp->samp_freq = samp_freq;
			
	/* read the number of channels */
	channelCount = ReadInt32(fp, 0);
	if (channelCount != 1) 
	  ErrorExit(SYSTEM_ERR_EXIT, "Can only read files with one single audio channel.");

	/* Position the file pointer at the beginning of the sample data */
	fseek(fp, afp->header_size, SEEK_SET);
    }
    else { /* mode = "w" */
      ErrorExit(SYSTEM_ERR_EXIT, "Writing .au files is not implemented");
    }
    break;

  /* The Windows audio format (.wav) */
  case WIN_AU:
    if (strcmp(mode, "r") == 0) {
	int magic, size, format, samp_freq, channelCount, num_samp_bits;

	magic = ReadInt32(fp, 0);
	if (magic != WinAUMagicNum) /* RIFF*/
	  ErrorExit(SYSTEM_ERR_EXIT, "Not a valid .wav-file - magic number didn't match.");

	ReadInt32(fp, 0);

	magic = ReadInt32(fp, 0);
	if (magic != 0x57415645) /* WAVE */
	  ErrorExit(SYSTEM_ERR_EXIT, "Not a valid .wav-file - bad header (missing \"WAVE\").");

	magic = ReadInt32(fp, 0);
	if (magic != 0x666d7420) /* fmt */
	  ErrorExit(SYSTEM_ERR_EXIT, "Not a valid .wav-file - bad header(missing \"fmt \").");

        afp->header_size = 28 + ReadInt32(fp, 1);

	format = ReadInt16(fp, 1);
	if (format != 7 && format != 1) 
	  ErrorExit(SYSTEM_ERR_EXIT,
          "Only format 7 (mulaw) and 1 (lin16) are implemented for .wav-files.");

	afp->subformat = format;

	/* read the number of channels */
	channelCount = ReadInt16(fp, 1);
	if (channelCount != 1) 
	  ErrorExit(SYSTEM_ERR_EXIT, "Can only read files with one single audio channel.");

	samp_freq = ReadInt32(fp, 1);
	afp->samp_freq = samp_freq;

        /* Skip a few bytes */
	ReadInt32(fp, 1); ReadInt16(fp, 1);

        num_samp_bits = ReadInt16(fp, 1);

        if ((num_samp_bits == 8 && (format != 7 && format != 6)) ||
	    (num_samp_bits == 16 && format != 1))
	  ErrorExit(SYSTEM_ERR_EXIT,
          "Only format 7 (mulaw) and 1 (lin16) are implemented for .wav-files.");

	magic = ReadInt32(fp, 0);
	if (magic != 0x64617461) /* data */
	  ErrorExit(SYSTEM_ERR_EXIT, "Not a valid .wav-file - bad header(missing \"data\").");

	size = ReadInt32(fp, 1);
	if (size == -1) afp->NumSamples = -1;
	else {
          if (format == 6 || format == 7) afp->NumSamples = size;
          else if (format == 1) afp->NumSamples = size / 2;
	}

	afp->little_endian = 1;
    }
    else { /* mode = "w" */
      ErrorExit(SYSTEM_ERR_EXIT, "Writing .wav files is not implemented");
    }
    break;

  case CMU_AU:
    if (strcmp(mode, "r") == 0) {
      short CMU_head_size, CMU_version, CMU_num_chan, CMU_samp_rate;
      int CMU_num_samp;

      CMU_head_size = ReadInt16(fp, 1);
      if (CMU_head_size != 12) 
        ErrorExit(SYSTEM_ERR_EXIT, "Can only read 12-byte CMU header.");

      CMU_version = ReadInt16(fp, 1);

      CMU_num_chan = ReadInt16(fp, 1);
      if (CMU_num_chan != 1) 
        ErrorExit(SYSTEM_ERR_EXIT, "Can only read 1-channel CMU data.");

      CMU_samp_rate = ReadInt16(fp, 1);
      afp->samp_freq = CMU_samp_rate;

      CMU_num_samp = ReadInt32(fp, 1);
      afp->NumSamples = CMU_num_samp;

      afp->little_endian = 1;
    }
    else { /* mode = "w" */
      ErrorExit(SYSTEM_ERR_EXIT, "Writing CMU files not implemented");
    }
    break;

  default:
    ErrorExit(SYSTEM_ERR_EXIT, "Fileformat not recognized in OpenAudioFile");
  }

  return afp;
}


void 
CloseAudioFile(AUDIO_FILE *afp) { 
  switch (afp->format) {
  case BINARY_AU:
    break;
  case KTH_AU:
    break;
  case CMU_AU:
    break;
  case SUN_AU:
    break;
  case NIST_AU:
    break;
  case WIN_AU:
    break;
  default:
    ErrorExit(SYSTEM_ERR_EXIT, "Fileformat not recognized in CloseAudioFile");
  }

  FREE(afp->fn);
  FREE(afp->mode);
  FCLOSE(afp->fp);
  FREE(afp);
}


/* Read NumSamples samples into Buffer from an AUDIO_FILE *.        */
/* The number of samples actually read is returned.                 */
int 
AudioReadFile(AUDIO_FILE *afp, short *Buffer, int NumSamples) { 
  short *t = Buffer;
  unsigned char *bytebuf;
  int i = NumSamples;
  int num_read = 0;

  if (strcmp(afp->mode, "r") != 0)
    ErrorExit(SYSTEM_ERR_EXIT, "Attempt to read from an audio file with mode %s.\n", 
              afp->mode);

  switch (afp->format) {
  case BINARY_AU:
    num_read = ReadInts16(t, NumSamples, afp->fp, afp->little_endian); 
    break;

  case KTH_AU:
    num_read = ReadInts16(t, NumSamples, afp->fp, afp->little_endian); 
    break;

  case CMU_AU:
    num_read = ReadInts16(t, NumSamples, afp->fp, afp->little_endian); 
    break;

  case NIST_AU:
    num_read = ReadInts16(t, NumSamples, afp->fp, afp->little_endian); 
    break;

  case SUN_AU:
    switch (afp->subformat) {
    case 1: /* mu-law */
      CALLOC(bytebuf, NumSamples, unsigned char);
      num_read = fread(bytebuf, 1, NumSamples, afp->fp);
      for (i = 0; i < num_read; i++) t[i] = ulaw2linear(bytebuf[i]);
      FREE(bytebuf);
      break;
    case 3: /* lin16 */
      num_read = ReadInts16(t, NumSamples, afp->fp, afp->little_endian); 
      break;
    default:
      ErrorExit(SYSTEM_ERR_EXIT, "This au-subformat (%i) is not implemented.\n", 
		afp->subformat);
    }
    break;

  case WIN_AU:
    switch (afp->subformat) {
    case 7: /* mu-law */
      CALLOC(bytebuf, NumSamples, unsigned char);
      num_read = fread(bytebuf, 1, NumSamples, afp->fp);
      for (i = 0; i < num_read; i++) t[i] = ulaw2linear(bytebuf[i]);
      FREE(bytebuf);
      break;
    case 1: /* lin16 */
      num_read = ReadInts16(t, NumSamples, afp->fp, afp->little_endian); 
      break;
    default:
      ErrorExit(SYSTEM_ERR_EXIT, "This au-subformat (%i) is not implemented.\n", 
		afp->subformat);
    }
    break;

  default:
    ErrorExit(SYSTEM_ERR_EXIT, "Fileformat not recognized in AudioReadFile");
  }

  return num_read;
}


/* Read NumSamples samples into Buffer from an AUDIO_FILE *.       */
/* The number of samples actually read is returned.                */
int 
AudioWriteFile(AUDIO_FILE *afp, short *Buffer, int NumSamples) { 
  short *t = Buffer;
  int num_written = 0;

  if (strcmp(afp->mode, "w") != 0)
    ErrorExit(SYSTEM_ERR_EXIT, "Attempt to write from an audio file with mode %s.\n", 
              afp->mode);

  switch (afp->format) {
  case BINARY_AU:
    num_written = WriteShorts(t, NumSamples, afp->fp);
    break;

  case KTH_AU:
    num_written = WriteShorts(t, NumSamples, afp->fp);
    break;

  case CMU_AU:
    ErrorExit(SYSTEM_ERR_EXIT, "Writing CMU files not implemented");
    break;

  case SUN_AU:
    ErrorExit(SYSTEM_ERR_EXIT, "Writing SUN .au files not implemented");
    break;

  case WIN_AU:
    ErrorExit(SYSTEM_ERR_EXIT, "WritingWindows .wav files not implemented");
    break;

  case NIST_AU:
    ErrorExit(SYSTEM_ERR_EXIT, "Writing NIST audio files not implemented");
    break;

  default:
    ErrorExit(SYSTEM_ERR_EXIT, "Fileformat not recognized in AudioWriteFile");
  }

  return num_written;
}


/* High level eof function for audio files */
int 
AudioFileEOF(AUDIO_FILE *afp) { 
  return feof(afp->fp);
}


/* ------------------------------------------------------------------------- */
/*               Top level audio-file read and write                         */
/*                                                                           */
/*     Use the following functions to load or save a whole audiofile         */
/* ------------------------------------------------------------------------- */

/* Load a whole audio file */
void 
LoadAudio(char *fn, AUDIO_BUFFER *buffer) { 
  AUDIO_FILE *afp;
  AudioChunk *chunk;
  int T;
 
  afp = OpenAudioFile(fn, buffer->format, "r");

  buffer->subformat     = afp->subformat;
  buffer->header_size   = afp->header_size;
  buffer->little_endian = afp->little_endian;
  buffer->samp_freq     = afp->samp_freq;
  buffer->NumSamples    = afp->NumSamples;

  if (afp->samp_freq > 0) buffer->samp_freq = afp->samp_freq;
  else afp->samp_freq = buffer->samp_freq;

  if (afp->NumSamples > 0) buffer->NumSamples = afp->NumSamples;
  else afp->NumSamples = buffer->NumSamples;

  T = 0;
  buffer->first_chunk = NULL;
  while (!AudioFileEOF(afp)) {
    CALLOC(chunk, 1, AudioChunk);
    CALLOC(chunk->audio, AUDIO_CHUNK_SIZE, short);

    T += chunk->num_samp = AudioReadFile(afp, chunk->audio, AUDIO_CHUNK_SIZE);
    if (!buffer->first_chunk) {
      buffer->first_chunk = buffer->last_chunk = chunk;
    }
    else {
      buffer->last_chunk->next = chunk;
      buffer->last_chunk = chunk;
    }
    chunk->next = NULL;
  }
  buffer->NumSamples += T;

  CloseAudioFile(afp);
}


/* Save a whole audio file */
void 
SaveAudio(char *fn, AUDIO_BUFFER *buffer) { 
  AUDIO_FILE *afp;
  AudioChunk *chunk;
  
  afp = OpenAudioFile(fn, buffer->format, "w");

  afp->subformat     = buffer->subformat;
  afp->header_size   = buffer ->header_size;
  afp->little_endian = buffer->little_endian;
  afp->samp_freq     = buffer->samp_freq;
  afp->NumSamples    = buffer->NumSamples;

  chunk = buffer->first_chunk;
  while (chunk) {
    AudioWriteFile(afp, chunk->audio, chunk->num_samp);
    chunk = chunk->next;
  }

  CloseAudioFile(afp);
}


/* Free mem allocated in an audio buffer */
void 
FreeAudio(AUDIO_BUFFER *buffer) {
  AudioChunk *chunk = buffer->first_chunk, *next;

  while (chunk) {
    next = chunk->next;
    FREE(chunk->audio);
    FREE(chunk);
    chunk = next;
  }

  FREE(buffer);
}



/* ------------------------------------------------------------------------- */
/* Info about an open AUDIO_FILE is returned from the following functions */
/* ------------------------------------------------------------------------- */

AudioFileType GetAudioFileType(AUDIO_FILE *afp) { 
  return afp->format;
}

char *
GetAudioFileMode(AUDIO_FILE *afp) { 
  return afp->mode;
}

int 
GetAudioFileNumSamples(AUDIO_FILE *afp) { 
  return afp->NumSamples;
}


/* ------------------------------------------------------------------------- */
/*             u-law conversion                                              */
/* ------------------------------------------------------------------------- */


#define ZEROTRAP    /* turn on the trap as per the MIL-STD */
#define BIAS 0x84   /* define the add-in bias for 16 bit samples */
#define CLIP 32635

static unsigned char linear2ulaw(int sample) {
  static int exp_lut[256] = {0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,
                             4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
                             5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
                             5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
                             6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                             6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                             6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                             6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7};
  int sign, exponent, mantissa;
  unsigned char ulawbyte;

  /* Get the sample into sign-magnitude. */
  sign = (sample >> 8) & 0x80;          /* set aside the sign */
  if (sign != 0) sample = -sample;              /* get magnitude */
  if (sample > CLIP) sample = CLIP;             /* clip the magnitude */

  /* Convert from 16 bit linear to ulaw. */
  sample = sample + BIAS;
  exponent = exp_lut[(sample >> 7) & 0xFF];
  mantissa = (sample >> (exponent + 3)) & 0x0F;
  ulawbyte = ~(sign | (exponent << 4) | mantissa);
#ifdef ZEROTRAP
  if (ulawbyte == 0) ulawbyte = 0x02;   /* optional CCITT trap */
#endif

  return(ulawbyte);
}

/*
** This routine converts from ulaw to 16 bit linear.
**
** Craig Reese: IDA/Supercomputing Research Center
** 29 September 1989
**
** References:
** 1) CCITT Recommendation G.711  (very difficult to follow)
** 2) MIL-STD-188-113,"Interoperability and Performance Standards
**     for Analog-to_Digital Conversion Techniques,"
**     17 February 1987
**
** Input: 8 bit ulaw sample
** Output: signed 16 bit linear sample
*/

static int 
ulaw2linear(unsigned char ulawbyte) {
  static int exp_lut[8] = {0,132,396,924,1980,4092,8316,16764};
  int sign, exponent, mantissa, sample;

  ulawbyte = ~ulawbyte;
  sign = (ulawbyte & 0x80);
  exponent = (ulawbyte >> 4) & 0x07;
  mantissa = ulawbyte & 0x0F;
  sample = exp_lut[exponent] + (mantissa << (exponent + 3));
  if (sign != 0) sample = -sample;

  return(sample);
}

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  AudioData.c                                      */
/* --------------------------------------------------------------------------*/
