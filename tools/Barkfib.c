/* ------------------------------------------------------------------------- */
/*                          Bark Scaled Filterbank                           */
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include "Constants.h"
#include "System.h"
#include "Command.h"
#include "AudioData.h"
#include "ParamData.h"
#include "Signal.h"
#include "Speetures.h"


void 
usage(void) {
  printf("USAGE: Barkfib [options] audiofile\n");
  printf("       Options                                                        Default\n");
  printf("         -S              Treat 'audiofile' as an inputfile script     (off)\n");
  printf("         -T level        Trace level (level=1,2 or 3)                 (0)\n");
  printf("         -F format       Audio format (raw, au, cmu or kth)           (kth)\n");
  printf("         -x ext          Extension for output files                   (fib)\n");
  printf("         -d ext          Directory for output files                   (current)\n");
  printf("         -q ext          Extension of input files\n");
  printf("         -p dir          Directory holding input files\n");
  printf("         -O format       Output format (binary, ascii etc.)           (binary)\n");
  printf("       Options specifying the filterbank characteristics\n");
  printf("         -n num_filters  Set number of filters                        (16)\n");
  printf("         -R ceil floor   Set low and high cut-frequencies [Hz]     (0 Nyquist)\n");
  printf("         -l length       Frame length [ms]                            (10)\n");
  printf("         -w length       Analyze window size [ms]                     (25)\n");
  printf("         -P coeff        Pre emphasis coeff                           (0.97)\n");
  printf("         -L cep-lift     Cepstrum lifting parameter                   (22.0)\n");
  printf("         -e              Add energy feature                           (off)\n");
  printf("         -0              Add zeroth cepstrum feature                  (off)\n");
  printf("         -r timeconst    Running average subtraction                  (off)\n");
  printf("         -m              Use Mel scale instead of Bark\n");
  printf("         -c N            Compute N cepstrum coeff's instead of filterbank\n");
  printf("       Options used to complement/override the file header information\n");
  printf("         -f freq         Sample frequenzy [Hz]\n");
  printf("         -h size         Size of file header                          (0)\n");
  printf("         -b              Force little endian                          (off)\n");
  printf("         -B              Force big endian                             (off)\n");

  exit(SYSTEM_ERR_EXIT);
}


int 
main(int argc, char **argv) {
  /* Command line var's */
  char *Input, sw, *format_string;
  char **OutList, **InList;    
  char OutExt[64] = {"fib"}, OutDir[256] = {"."}; 
  char InExt[64] = {""}, InDir[256] = {""}; 
  int  NumFiles;
  int scriptflag = 0;
  int trace = 0;

  /* Filterbank var's */
  Speetures *speetures;
  float **fib;
  int base_type = BARKFIB_FEATURES;
  int num_frames, allocated_frames;
  int num_filters = 16;
  int TotN = 0; /* for stat's */
  int status, end_of_speech;
  float frame_length = 0.010; /* [sec] */
  float window_length = 0.025; /* [sec] */
  float HzFloor = 0.0, HzCeil = 8000.0;
  float pre_emp_coeff = 0.97;
  float cep_lift = 22.0;
  int add_flags = 0;
  int energy_flag = 0;
  int num_coeffs = 0; /* Number of cepstrum coeff's */
  float ra_timeconst = -1;

  /* Audio var's */
  AUDIO_FILE *afp;
  AudioFileType format = KTH_AU;
  ParamFileType out_format = BINARY;
  int little_endian_value = -1;
  int header_size = -1;
  float samp_freq = 16000.0;
  short *speech;
  int read_samples, chunc_size;
  int little_endian_flag = 0, header_size_flag = 0, samp_freq_flag = 0;

  int i;

  if (argc == 1) usage();

  InitCommand(argv, argc);
  while (is_option()) {
    switch (sw = GetOption()) {
      case 'n':
        num_filters = GetInt("Number of filters", 2, 300);
        break;
      case 'R':
        HzFloor = GetFloat("Low cut frequency [Hz]", 0.0, 5000.0);
        HzCeil = GetFloat("High cut frequency [Hz]", HzFloor, 20000.0);
        break;
      case 'd':
        strcpy(OutDir, GetName("Output path name"));
        break;  
      case 'x':
        strcpy(OutExt, GetName("Output file extension"));
        break;
      case 'p':
        strcpy(InDir, GetName("speech path name"));
        break;  
      case 'q':
        strcpy(InExt, GetName("speech file extension"));
        break;
      case 'h':
        header_size = GetInt("Size of speech file header (bytes)", 0, 10000);
        header_size_flag = 1;
        break;
      case 'B':
        little_endian_value = 1;
        little_endian_flag = 1;
        break;
      case 'b':
        little_endian_value = 0;
        little_endian_flag = 1;
        break;
      case 'S':
        scriptflag = 1;
        break;
       case 'T':
        trace = GetInt("Trace level", 1, 3); 
        break;
      case 'F':
        format_string = GetName("speech file format");
        format = Str2AudioType(format_string);
        if (format == -1) 
          ErrorExit(SYSTEM_ERR_EXIT, "The file format %s is not supported.\n",
            format_string);
        break;
      case 'O':
        format_string = GetName("param file format");
        out_format = Str2ParamType(format_string);
        if (out_format == -1) 
          ErrorExit(SYSTEM_ERR_EXIT, "The file format %s is not supported.\n",
            format_string);
        break;
      case 'f':
        samp_freq = GetFloat("Sample frequency [Hz]", 10.0, 64000.0); 
        samp_freq_flag = 1;
        break;
      case 'l':
        frame_length = GetFloat("Frame length [ms]", 1.0, 100.0); 
        frame_length /= 1000.0;
        break;
      case 'w':
        window_length = GetFloat("Window size [ms]", 1.0, 100.0); 
        window_length /= 1000.0;
        break;
      case 'e':
        energy_flag = 1;
        add_flags |= ADD_LOG_E;
        base_type |= ENERGY_FEATURES;
        break;
      case '0':
        energy_flag = 1;
        add_flags |= ADD_ZEROTH_CEP;
        base_type |= ENERGY_FEATURES;
	break;
      case 'c':
        num_coeffs = GetInt("Number of cepstrum coeff's", 1, 50); 
        base_type &= (0xFFFF ^ FIB_FEATURES); 
        base_type |= CEPSTRUM_FEATURES; 
        break;
      case 'm':
        base_type |= MEL_FEATURES; 
        break;
      case 'P':
        pre_emp_coeff = GetFloat("Pre emphasis coeff", 0.0, 2.0); 
        break;
       case 'L':
        cep_lift = GetFloat("Cep lift parameter", 0.0, 100.0); 
        break;
      case 'r':
        ra_timeconst = GetFloat("Running average time constant", 0.0, 1.0); 
        break;
      default:
        ErrorExit(SYSTEM_ERR_EXIT, "Unknown switch %s.\n", sw);
    } 
  }

  Input = GetName("Name of speech file or (with -S option) Script file");   
  EndCommand(); 

  if ((base_type & CEPSTRUM_FEATURES) && num_coeffs > num_filters)
    ErrorExit(SYSTEM_ERR_EXIT, "Can't compute % cepstrum coeff's from %i filters", 
      num_coeffs, num_filters);

  if (!samp_freq_flag && format == BINARY_AU)
    ErrorExit(SYSTEM_ERR_EXIT, "You must specify the sample frequency (-f option)\n");


  if (scriptflag) {
    InList = LoadFileList(Input, InDir, InExt, &NumFiles);
    OutList = LoadFileList(Input, OutDir, OutExt, &NumFiles);
  }
  else {
    NumFiles = 1;
    CALLOC(InList, 1, char *)
    CALLOC(OutList, 1, char *)

    if (strcmp(Input, "stdin") == 0) {
      InList[0] = "stdin";
      OutList[0] = "stdout";
    }
    else {
      InList[0] = ModifyName(Input, InExt, InDir);
      OutList[0] = ModifyName(InList[0], OutExt, OutDir);
    }
  }


  if (trace > 0 && scriptflag) {
    printf("%i data files found in script.\n", NumFiles);
  }


  chunc_size = 4096;
  CALLOC(speech, chunc_size, short);

  for (i = 0; i < NumFiles; i++) {
    if (trace > 2) printf("Barkfib creating: %s\n", OutList[i]);

    afp = OpenAudioFile(InList[i], format, "r");

    /* Force new values for entries in the audio file */
    if (little_endian_flag) afp->little_endian = little_endian_value;
    if (header_size_flag) {
      fseek(afp->fp, header_size - afp->header_size, SEEK_CUR);
      afp->header_size = header_size;
    }
    if (samp_freq_flag) afp->samp_freq = samp_freq;

    /* Default for upper cut freq is the Nyquist frequency */
    if (HzCeil == -1) HzCeil = afp->samp_freq / 2.0;

    /* Initialize a speech feature engine for this file */
    speetures = InitSpeetures(base_type, num_filters, 
      energy_flag + (base_type & CEPSTRUM_FEATURES) ? num_coeffs : num_filters,
      add_flags, afp->samp_freq, frame_length, window_length, HzFloor, HzCeil);

    if (ra_timeconst > 0.0) {
      speetures->mean_subtraction = 1; /* enable */
      speetures->ra_timeconst = ra_timeconst;
    }

    speetures->pre_emp_factor = pre_emp_coeff;
    speetures->cep_lift = cep_lift;

    allocated_frames = 1024;
    CALLOC(fib, allocated_frames, float *);
    num_frames = 0;

    status = 0;
    end_of_speech = 0;

    while (status != -1) {
      /* Read speech and push it */
      if (!end_of_speech) {
        read_samples = AudioReadFile(afp, speech, chunc_size);
        PushSpeech(speetures, speech, read_samples);

        if (AudioFileEOF(afp)) {
          CloseAudioFile(afp);
          end_of_speech = 1;
          SpeechEnd(speetures);
        }
      }

      /* Pop frames */
      while ((status = ProbeFeatures(speetures)) > 0) {
        if (allocated_frames == num_frames) {
          allocated_frames *= 2;
          REALLOC(fib, allocated_frames, float *);
        }
        if ((fib[num_frames++] = PopFeatures(speetures)) == NULL)
          ErrorExit(SYSTEM_ERR_EXIT, "Frame pop operation failed"); /* Serious! */
      }
    }

    TotN += num_frames;

    if (num_frames > 0) {
      if (out_format == HTKPARAM) { /* treat HTK files separately */
        HTKmemory *head;
        PARAM_FILE *pfp;
        int SampleSize = speetures->frame_size, j;
	pfp = OpenParameterFile(OutList[i], HTKPARAM, &SampleSize, "w");


        /* set up the paramkind of the HTK header */

        head = (HTKmemory *)pfp->subformat;
        if (base_type & CEPSTRUM_FEATURES) head->ParamKind = 6; /* MFCC */
        else head->ParamKind = 7; /* FBANK */

        if (energy_flag && (add_flags & ADD_ZEROTH_CEP))
	  head->ParamKind |= 020000; /* _0 */
        if (energy_flag && !(add_flags & ADD_ZEROTH_CEP))
	  head->ParamKind |= 000100; /* _E */


        /* save frames */
	for (j = 0; j < num_frames; j++) {
	  WriteParameterSample(pfp, fib[j]);
	}

        /* close the file
	 * the function will first rewind and fill in the header info */
	CloseParameterFile(pfp);
      }
      else {
	SaveParameters(OutList[i], out_format, speetures->frame_size, 
		       num_frames, fib);
      }

      FreeParameters(fib, num_frames);
    }

    FreeSpeetures(speetures);
  }

  /* Print stat's */
  if (trace > 0) {
    unsigned long user, system;
    float real_time, used_time = 0.0;

    printf("In total %.2f s of audio processed\n", 
       real_time = (float)TotN * frame_length);
    my_time(&user, &system);
    used_time = (float)user / 1000.0;
    printf("CPU-time: %.2f s => %.2f %% of real time\n", 
       used_time, 
       100.0 * used_time / real_time);
  }

  return SYSTEM_OK_EXIT;
}

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  Barkfib.c                                        */
/* --------------------------------------------------------------------------*/
