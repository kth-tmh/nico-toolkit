/* ------------------------------------------------------------------------- */
/*                          Audio File Input/Output                          */
/* ------------------------------------------------------------------------- */
/*                                                                           */
/*   Copyright (C) 1996, 1997, 1998 Nikko Ström                              */
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
