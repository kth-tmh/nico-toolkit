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

#ifndef AUDIODATA_LOADED
#define AUDIODATA_LOADED

#include <stdio.h>

#define SUNAUMagicNum 0x2e736e64
#define WinAUMagicNum 0x52494646

#define AUDIO_CHUNK_SIZE 32768

/* Theese are the supported data formats for audio files */
typedef enum {
  UNDEFINED_AU=-1,
  BINARY_AU=1,    /*  short int's                                    */
  KTH_AU=2,       /*  KTH StdHead format                             */
  CMU_AU=3,       /*  CMU format                                     */
  SUN_AU=4,       /*  .au format                                     */
  NIST_AU=5,      /*  NIST audio format                              */
  WIN_AU=6        /*  Windows .wav audio format                      */
} AudioFileType;


typedef struct {
  FILE         *fp;
  char         *fn;
  AudioFileType format;
  int           subformat;
  int           little_endian;   /* is set for little endian byte order */
  int           header_size;
  int           samp_freq;
  int           NumSamples;  /* -1 indicates "unspecified" */

  char         *mode;        /* currently only "r" or "w" are allowed */
  int           rewindable;  /* set if possible to rewind and alter header */
  int           eof;
} AUDIO_FILE;


struct AudioChunk;
typedef struct AudioChunk {
  int num_samp;
  short *audio;
  struct AudioChunk *next;
} AudioChunk;


typedef struct {
  AudioFileType format;
  int           subformat;
  int           header_size;
  int           little_endian;
  int           samp_freq;
  int           NumSamples;  /* -1 indicates "unspecified" */

  int           num_chunks;
  AudioChunk   *first_chunk, *last_chunk;
} AUDIO_BUFFER;


/* Conversion between the string and the code description of audio formats */
AudioFileType Str2AudioType(char *);
char *AudioType2Str(AudioFileType);


/* ------------------------------------------------------------------------- */
/*            Use theese to load or save a whole audiofile                   */
/* ------------------------------------------------------------------------- */

void LoadAudio(char *fn, AUDIO_BUFFER *);

void SaveAudio(char *fn, AUDIO_BUFFER *);

void FreeAudio(AUDIO_BUFFER *);


/* ------------------------------------------------------------------------- */
/*           Here follows the general audio I/O functions                    */
/* ------------------------------------------------------------------------- */

/* Uses heuristic information to guess the audio file type */
AudioFileType GuessAudioFileType(char *fn);

AUDIO_FILE *OpenAudioFile(char *fn, AudioFileType, char *mode);

/* Opens 2-byte audiofiles */
AUDIO_FILE *OpenRawAudioFile(char *fn, 
                             int samp_freq, int little_endian_flag, 
                             int header_size, char *mode);

int AudioReadFile(AUDIO_FILE *, short *Buffer, int NumSamples);
int AudioWriteFile(AUDIO_FILE *, short *Buffer, int NumSamples);

int AudioFileEOF(AUDIO_FILE *);

void CloseAudioFile(AUDIO_FILE *);



/* ------------------------------------------------------------------------- */
/* Info about an open AUDIO_FILE is returned from theese functions           */
/* ------------------------------------------------------------------------- */

AudioFileType GetAudioFileType(AUDIO_FILE *);

char *GetAudioFileMode(AUDIO_FILE *);

int GetAudioFileNumSamples(AUDIO_FILE *);


#endif

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  AudioData.h                                      */
/* --------------------------------------------------------------------------*/
