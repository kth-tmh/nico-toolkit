/* ------------------------------------------------------------------------- */
/*                         Mel Filterbank generation                         */
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
#include <string.h>
#include <math.h>
#include <unistd.h>
#include "System.h"
#include "Command.h"
#include "AudioData.h"
#include "ParamData.h"
#include "Signal.h"
#include "Constants.h"
#include "Math.h"


static int trace = 0;
static int frame_len = 10;
static int win_len = 25;
static int sample_offset = 0;
static int NUM_MEL_FILTERS = 16;
static int ANALYZE_SIZE;
static int STEP_SIZE;
static int FFT_SIZE;
static int SAMP_FREQ = 16000;
static int freq_scale_type = 1; /* Mel scale */
static float HzFloor = 0.0, HzCeil = 8000.0;

static int energy_flag = 0;

/* Variables defining the filterbank */
static FFTInitializer *fib_fft_init;

static float *hamming_window;

static float *cut_frqs = NULL;

static int *cut_fft_points = NULL;
static int fft_size;
static int analyze_size;
static int nr_chans;
 
static float *shapes1 = NULL;
static float *shapes2 = NULL;

static float pre_emp_factor = 0.97;
static int pre_emp_flag = 0;


/*  ------------   Initialization of filter bank routines  ---------------- */
/*                                                                          */
/* analyze_size : size of the fft-window that's not zero-padded             */
/* nr_chans :     the number of output filter bank channels                 */
/* samp_freq :    sample frequency in Herz                                  */
/* scale_type :   0 for linear scale, 1 for mel-scale and 2 for bark-scale  */
/* HerzFloor :    lower cut frequency (in Hz) for the first filter          */
/* HerzCeil :     upper cut frequency (in Hz) for the last filter           */
/*                                                                          */
/* The size of the fftwindow is returned                                    */
/* ------------------------------------------------------------------------ */

int 
InitFilterbank(int ANALYZE_SIZE, int NR_CHANS, 
                    float samp_freq, int scale_type, 
                    float HerzFloor, float HerzCeil) {
  int i, j, num_cut_freq;
  float freq_scale;

  /* Low level signal processing initializations */
  if (fib_fft_init) FreeFFTInitializer(fib_fft_init);
  fib_fft_init = fft842init(REALFFT, ANALYZE_SIZE);
  fft_size = 2 * fib_fft_init->size;

  freq_scale = samp_freq / fft_size;
  analyze_size = ANALYZE_SIZE;
  nr_chans = NR_CHANS;

  hamming_window = InitHamming(analyze_size);

  /* ----  Compute the cut-frequencies using the selected scale  ---- */

  num_cut_freq = nr_chans + 2;

  if (!cut_frqs) CALLOC(cut_frqs, num_cut_freq, float)
  else REALLOC(cut_frqs, num_cut_freq, float)

  switch (scale_type) {
    case 0: {
      float Hz;
      float Hz_step = (HerzCeil - HerzFloor) / (float)(num_cut_freq - 1.0);
      for (Hz = HerzFloor, i = 0 ; i < num_cut_freq; Hz += Hz_step, i++) {
        cut_frqs[i] = Hz;
      }
    }
    break;
    case 1: {
      float mel;
      float mel_floor = Mel(HerzFloor);
      float mel_step = (Mel(HerzCeil) - mel_floor) / 
         (float)(num_cut_freq - 1.0);
      for (mel = mel_floor, i = 0 ; i < num_cut_freq; mel += mel_step, i++) {
        cut_frqs[i] = InvMel(mel);
      }
    }
    break;
    case 2: {
      float bark;
      float bark_floor = Bark(HerzFloor);
      float bark_step = (Bark(HerzCeil) - bark_floor) / 
         (float)(num_cut_freq - 1.0);
      for (bark = bark_floor, i = 0 ; i < num_cut_freq; 
        bark += bark_step, i++) {
        cut_frqs[i] = InvBark(bark);
      }
    }
    break;
    default:
      ErrorExit(SYSTEM_ERR_EXIT, 
        "Attempt to call InitFilterbank with illegal scale_type (%i).\n", 
        scale_type);
  }


  /* ----  Set up the array cut_fft_points according to cut_frqs  ---- */

  if (!cut_fft_points) CALLOC(cut_fft_points, num_cut_freq, int)
  else REALLOC(cut_fft_points, num_cut_freq, int)

  for (i = 0; i < num_cut_freq; i++) {
    cut_fft_points[i] = (int)ceil(cut_frqs[i] / freq_scale);
    if (cut_fft_points[i] > fft_size / 2) {
      ErrorExit(SYSTEM_ERR_EXIT, 
        "fft_size, sample frequecy and cut frequency not compatible.\n");
    }
  }


  if (!shapes2) CALLOC(shapes2, analyze_size, float)
  else REALLOC(shapes2, analyze_size, float)

  if (!shapes1) CALLOC(shapes1, analyze_size, float)
  else REALLOC(shapes1, analyze_size, float)

  for (i = 0; i < num_cut_freq - 1; i++) {
    for (j = cut_fft_points[i]; j < cut_fft_points[i + 1]; j++) {
      switch (scale_type) {
      case 0:
	shapes1[j] = (float)(j - cut_fft_points[i]) /
	  (float)(cut_fft_points[i + 1] - cut_fft_points[i] * 
		  freq_scale);
	break;
      case 1:
	shapes1[j] = (Mel(j * freq_scale) - Mel(cut_fft_points[i] * 
						freq_scale)) /
	  (Mel(cut_fft_points[i + 1] * freq_scale) - 
	   Mel(cut_fft_points[i] * freq_scale));
	break;
      case 2:
	shapes1[j] = (Bark(j * freq_scale) - Bark(cut_fft_points[i] * 
						  freq_scale)) /
	  (Bark(cut_fft_points[i + 1] * freq_scale) - 
	   Bark(cut_fft_points[i] * freq_scale));
	break;
      }
      shapes2[j] = 1.0 - shapes1[j];
    }
  }

  return fft_size;
}  /*  end of InitFilterbank()  */


void 
PrintFilterFreqs(FILE *stream) {
  int i;

  fprintf(stream, "chan nr   lower cut   mid frequency   upper cut\n");
  for (i = 0; i < nr_chans; i++) {
    fprintf(stream, "%5i      %7.1f        %7.1f     %7.1f\n", 
	    i + 1, cut_frqs[i], cut_frqs[i + 1], cut_frqs[i + 2]);
  }
}


/* -----  Filerbank analysis of the absolute-value-spectrum in 'data'.  ---- */
/*    The result is returned in the vectors 'fib1'and 'fib2'                 */
/*       WARNING:     'fib' must have been allocated before calling this     */
/*                    function                                               */
/* ------------------------------------------------------------------------- */

static void 
filterbank_from_power_spectrum(float *data, float *fib) {
  int i, j;
  int fft_point;
  float sum1, sum2;

  /* Convert from power to magnitudes */
  j = cut_fft_points[nr_chans + 1]; 
  for (fft_point = cut_fft_points[0]; fft_point < j; fft_point++) {
    data[fft_point] = sqrt(data[fft_point]);
  }

  /* Initialize sum1 for first filter */
  sum1 = 0.0;
  j = cut_fft_points[1]; /* First cut */
  for (fft_point = cut_fft_points[0]; fft_point < j; 
       fft_point++) {
    sum1 += data[fft_point] * shapes1[fft_point];
  }

  for (i = 0; i < nr_chans - 1; i++) {
    sum2 = sum1;
    sum1 = 0.0;
    j = cut_fft_points[i + 2]; /* Next cut */
    for (fft_point = cut_fft_points[i + 1]; fft_point < j; fft_point++) {
      sum2 += data[fft_point] * shapes2[fft_point];
      sum1 += data[fft_point] * shapes1[fft_point];
    }

    if (sum2 < SMALL_AMPL) fib[i] = SMALL_LOG;
    else fib[i] = 10.0 * log10((double)sum2);
  }

    /* Take care of the last filter */
  sum2 = sum1;
  j = cut_fft_points[nr_chans + 1]; /* Last cut */
  for (fft_point = cut_fft_points[nr_chans]; fft_point < j; fft_point++) {
    sum2 += data[fft_point] * shapes2[fft_point];
  }

  j = nr_chans - 1; /* fib index */
  if (sum2 < SMALL_AMPL) fib[j] = SMALL_LOG;
  else fib[j] = 10.0 * log10((double)sum2);
}


/* ---------------  Filterbank analysis of the vectors 'data1' ------------- */
/*  The result is returned in the vector 'fib'                               */
/*  The log total energy is the return value.                                */
/*    WARNING:  The vector 'data' is altered by this function.               */
/*    WARNING:  'fib' must have been allocated before calling this function  */
/* ------------------------------------------------------------------------- */

float 
Filterbank(float *data, float *fib) { 
  float e;

  /* Remove the DC component */
  RemoveDC(data, analyze_size);

  /* Apply pre-emphasis on the data */
  if (pre_emp_flag) PreEmphasis(data, analyze_size, pre_emp_factor);

  e = GetEnergy(data, analyze_size);
  if (e < SMALL_AMPL) e = SMALL_LOG;
  else e = (float)log10((double)e);

  ApplyHamming(data, hamming_window, fft_size, analyze_size); 

  power842(data, fib_fft_init);

  filterbank_from_power_spectrum(data, fib);

  return e;
}



/* Initialize filterbank freq's, FFT and hamming window */
void 
InitMelfib(int down_sample_factor) {
  if (trace > 0) {
    printf("Using %i ", NUM_MEL_FILTERS);

    switch (freq_scale_type) {
    case 0:
      printf("linearly ");
      break;
    case 1:
      printf("Mel-");
      break;
    case 2:
      printf("Bark-");
      break;
    }
    printf("scaled filters from %.1f Hz to %.1f Hz.\n", HzFloor, HzCeil);
  }

  ANALYZE_SIZE =  (float)win_len / 1000.0 * SAMP_FREQ; 
  STEP_SIZE = (float)frame_len / 1000.0 * SAMP_FREQ;

  FFT_SIZE = InitFilterbank(ANALYZE_SIZE / down_sample_factor, 
                 NUM_MEL_FILTERS,
                 SAMP_FREQ / down_sample_factor, 
                 freq_scale_type, HzFloor, HzCeil);

  if (trace > 1) PrintFilterFreqs(stdout);

  if (trace > 0) {
    printf("Sample Frequency: %i Hz\n", SAMP_FREQ);
    printf("FFT Size: %i,   Window Size: %i,  Frame Step Size: %i\n", 
       FFT_SIZE, ANALYZE_SIZE, STEP_SIZE);
  }
}


/* Read speech from an audio-file and create a melfib repr. */
float **Melfib(AUDIO_FILE *afp, int *num_frames, int down_sample_factor) {
  int frame_count, allocated_frames, numread, eof_flag;
  int buftop, bufbot, bottom_sample, top_sample;
  short *buffer1, *buffer2, *tmp, *frame;
  float *data;
  float **fib;
  float e;

  CALLOC(data, FFT_SIZE, float)

  CALLOC(frame, FFT_SIZE * down_sample_factor, short)
  CALLOC(buffer1, ANALYZE_SIZE, short)
  CALLOC(buffer2, ANALYZE_SIZE, short)

  if (sample_offset > 0) {
    numread = AudioReadFile(afp, buffer1, sample_offset);
    if (numread != sample_offset) 
      ErrorExit(SYSTEM_ERR_EXIT, "Very short sample-file: %s", afp->fn);
  }

  allocated_frames = 1024;
  CALLOC(fib, allocated_frames, float *);

  /* Initialize the two buffers */
  numread = AudioReadFile(afp, buffer1, ANALYZE_SIZE);
  if (numread < ANALYZE_SIZE) {
    EmitWarning("Very short datafile");
    FREE(buffer1)
    FREE(buffer2)
    FREE(fib)
    FREE(data)
    FREE(frame)
    return NULL;
  }
  numread = AudioReadFile(afp, buffer2, ANALYZE_SIZE);
  if (numread < ANALYZE_SIZE) {
    EmitWarning("Very short datafile");
    FREE(buffer1)
    FREE(buffer2)
    FREE(fib)
    FREE(data)
    FREE(frame)
    return NULL;
  }
  bufbot = 0;
  buftop = 2 * ANALYZE_SIZE;
 
 
  frame_count = 0;
  eof_flag = 0;
  while (1 /* termination criterium inside the loop */) {
    if (frame_count == allocated_frames) {
      allocated_frames *= 2;
      REALLOC(fib, allocated_frames, float *)
    }

    /* Find start and end-samples for the frame */
    bottom_sample = frame_count * STEP_SIZE - ANALYZE_SIZE / 2;
    top_sample = bottom_sample + ANALYZE_SIZE;
    if (bottom_sample < 0) bottom_sample = 0;

    /* Advance the buffers if necessary */
    if (!eof_flag && top_sample > buftop) {
      tmp = buffer1; buffer1 = buffer2; buffer2 = tmp;
      numread = AudioReadFile(afp, buffer2, ANALYZE_SIZE);
      if (numread < ANALYZE_SIZE) {
        buftop += numread;
        eof_flag = 1;
      }
      else buftop += ANALYZE_SIZE;
      bufbot += ANALYZE_SIZE; 
    }

    if (bottom_sample + ANALYZE_SIZE / 2 > buftop) break;
     
    /* Copy the frame from the two buffers */
    if (top_sample < ANALYZE_SIZE) { /* Spec. case in the beginning of file */
      memcpy(frame, buffer1, 
             (top_sample - bottom_sample) * sizeof(short));
      memset(frame + (top_sample - bottom_sample), '\0',
             (ANALYZE_SIZE - top_sample + bottom_sample) * sizeof(short));
    }
    else if (eof_flag) { /* Spec. case in the end of file */
      if (bottom_sample < bufbot + ANALYZE_SIZE) {
        memcpy(frame, buffer1 + (bottom_sample - bufbot),
               (ANALYZE_SIZE + bufbot - bottom_sample) * sizeof(short));
        memcpy(frame + (ANALYZE_SIZE + bufbot - bottom_sample), buffer2,
               (top_sample - ANALYZE_SIZE - bufbot) * sizeof(short));
      }
    }
    else {
      memcpy(frame, buffer1 + (bottom_sample - bufbot),
             (ANALYZE_SIZE + bufbot - bottom_sample) * sizeof(short));
      memcpy(frame + (ANALYZE_SIZE + bufbot - bottom_sample), buffer2,
             (ANALYZE_SIZE - buftop + top_sample) * sizeof(short));
    }

    /* Downsample and convert to floats */
    DownSample(frame, data, ANALYZE_SIZE, down_sample_factor);

    /* Do filterbank analysis */
    CALLOC(fib[frame_count], NUM_MEL_FILTERS + (energy_flag ? 1 : 0), float);
    e = Filterbank(data, fib[frame_count]);
    /* If the flag is set, insert log energy at the last pos. of 'fib' */
    if  (energy_flag) fib[frame_count][NUM_MEL_FILTERS] = e;

    frame_count++;
  }

  FREE(data)
  FREE(buffer1)
  FREE(buffer2)
  FREE(frame)

  *num_frames = frame_count;
  REALLOC(fib, frame_count, float *)

  return fib;
}


void 
cepstrum(float *cep, int num_cep,
              float *fib, int num_fib,
	      int cep0th_flag, float cep_lift) {
  int i, j;
  float A = PI / (float)num_fib;
  float B = sqrt(2.0 / (float)num_fib);

  for (i = (cep0th_flag ? 0 : 1); i <= num_cep; i++) {
    float lift = 1.0 + cep_lift / 2.0 * sin(PI * (float)i / cep_lift);
    for (j = 0; j < num_fib; j++) {
      cep[i - (cep0th_flag ? 0 : 1)] +=
	fib[j] * (B * lift * cos(A * (float)i * ((float)j + 0.5)));
    }
  }
}



void 
usage(void) {
  printf("USAGE: Melfib [options] audiofile\n");
  printf("       Options                                                       Default\n");
  printf("         -S              Treat 'audiofile' as an inputfile script    (off)\n");
  printf("         -T level        Trace level (level=1,2 or 3)                (0)\n");
  printf("         -F format       Audio format (raw, au, cmu, kth etc.)       (raw)\n");
  printf("         -x ext          Extension for output files                  (fib)\n");
  printf("         -d ext          Directory for output files                  (current)\n");
  printf("         -q ext          Extension of input files\n");
  printf("         -p dir          Directory holding input files\n");
  printf("         -O format       Output format (binary, ascii, htk etc.)     (binary)\n");
  printf("         -c N            Output N cep coeff's instead of filterbank  (off)\n");
  printf("         -0              Include 0th cepstrum coefficient            (off)\n");
  printf("         -L lift         Cepstrum lifting                            (0.0)\n");
  printf("       Options specifying the filterbank characteristics\n");
  printf("         -E              Log energy added at first position          (off)\n");
  printf("         -n num_filters  Set number of filters                       (16)\n");
  printf("         -t scale_type   Set frequency scaling (lin, mel or bark)    (mel)\n");
  printf("         -R ceil floor   Set lowest and highest cut-frequencies      (0 8000)\n");
  printf("         -P type         Set filter shape (square or triangle)       (triangle)\n");
  printf("         -e c            Apply pre emphasis with coeff 'c'           (0.97)\n");
  printf("         -l length       Frame length in ms                          (10)\n");
  printf("         -w length       Analyze window length in ms                 (25)\n");
  printf("         -o offset       Shift input 'offset' samples                (0)\n");
  printf("       Options used to complement/override the file header information\n");
  printf("         -h size         Size of file header                         (0)\n");
  printf("         -f samp_freq    Set sample frequency (after downsampling)   (16000)\n");
  printf("         -b              Swap byte order                             (off)\n");

  exit(0);
}


int
main(int argc, char **argv) {
  AUDIO_FILE *afp;
  float **out_data;

  char *Input, sw, *format_string;
  char **OutList, **InList;    
  char OutExt[64] = {"fib"}, OutDir[256] = {"."}; 
  char InExt[64] = {""}, InDir[256] = {""}; 
  char *FreqScaleType;
  int  NumFiles;

  int num_frames, TotN = 0;
  int scriptflag = 0;
  int i, j;

  AudioFileType format = BINARY_AU;
  ParamFileType out_format = BINARY;

  int down_sample_factor =  1;
  int little_endian_value    = -1;
  int header_size        = -1;
  int little_endian_flag = 0, header_size_flag = 0, samp_freq_flag = 0;
  int cepstrum_flag = 0, cep0th_flag = 0;

  float **cep_data;
  int num_cep = 0;
  float cep_lift = 0.0;

  if (argc == 1) usage();

  InitCommand(argv, argc);
  while (is_option()) {
    switch (sw = GetOption()) {
      case 'n':
        NUM_MEL_FILTERS = GetInt("Number of filters", 2, 300);
        break;
      case 't':
        FreqScaleType = 
          GetName("Type of frequency scaling (lin, mel or bark)");
        if      (strcmp(FreqScaleType, "lin") == 0)  freq_scale_type = 0;
        else if (strcmp(FreqScaleType, "mel") == 0)  freq_scale_type = 1;
        else if (strcmp(FreqScaleType, "bark") == 0) freq_scale_type = 2;
        else ErrorExit(SYSTEM_ERR_EXIT, "'%s' is not a frequency scale option.\n", 
          FreqScaleType);
        break;
      case 'R':
        HzFloor = GetFloat("Lowest cut frequency", 0.0, 5000.0);
        HzCeil = GetFloat("Lowest cut frequency", HzFloor, 20000.0);
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
        header_size = GetInt("Size of speech file header", 0, 10000);
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
        SAMP_FREQ = GetInt("Sample frequency", 0, 64000); 
        samp_freq_flag = 1;
        break;
      case 'l':
        frame_len = GetInt("Frame length", 0, 64000); 
        break;
      case 'w':
        win_len = GetInt("Analyze window length", 0, 64000); 
        break;
      case 'o':
        sample_offset = GetInt("Sample offset", 0, 1000); 
        break;
      case 'E':
        energy_flag = 1; 
        break;
      case '0':
        cep0th_flag = 1; 
        break;
      case 'c':
        cepstrum_flag = 1; 
        num_cep = GetInt("Number of cepstrum cefficients", 1, 1000); 
        break;
      case 'L':
        cep_lift = GetFloat("Cepstrum lift cefficient", 1.0, 1000.0); 
        break;
       case 'e':
        pre_emp_factor = GetInt("Pre emphasis coeficient", 0.0, 2.0);
        pre_emp_flag = 1; 
        break;
     default:
        ErrorExit(SYSTEM_ERR_EXIT, "Unknown switch %s.\n", sw);
    } 
  }

  Input = GetName("Name of speech file or (with -S option) Script file");   
  EndCommand(); 

  if (cepstrum_flag && num_cep > NUM_MEL_FILTERS)
    ErrorExit(SYSTEM_ERR_EXIT, 
     "Number of cepstrum coeff's must be smaller than number of filters");


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

  InitMelfib(down_sample_factor);

  if (trace > 0 && scriptflag) 
    printf("%i data files found in script.\n",NumFiles );

  for (i = 0; i < NumFiles; i++) {
    if (trace > 2) printf("Melfib creating: %s\n", OutList[i]);

    afp = OpenAudioFile(InList[i], format, "r");

    /* Force different values for entries in the audio file */
    if (little_endian_flag)   afp->little_endian   = little_endian_value;
    if (header_size_flag) afp->header_size = header_size;
    if (samp_freq_flag)   afp->samp_freq   = SAMP_FREQ;

    out_data = Melfib(afp, &num_frames, down_sample_factor);

    CloseAudioFile(afp);

    TotN += num_frames;

    if (cepstrum_flag) {
      CALLOC2D(cep_data, num_frames,
	       num_cep + (energy_flag ? 1 : 0) + (cep0th_flag ? 1 : 0),
	       float);
      for (j = 0; j < num_frames; j++) {
        cepstrum(cep_data[j], num_cep,
		 out_data[j], NUM_MEL_FILTERS, cep0th_flag, cep_lift);
        if (energy_flag) {
	  cep_data[j][num_cep + (cep0th_flag ? 1 : 0)] = 
	    out_data[j][NUM_MEL_FILTERS];
	}
      }
      FreeParameters(out_data, num_frames);
      out_data = cep_data;
    }


    if (out_data) {
      if (cepstrum_flag) {
	SaveParameters(OutList[i], out_format, 
		       num_cep + (energy_flag ? 1 : 0) + (cep0th_flag ? 1 : 0),  
		       num_frames, out_data);
      }
      else {
	SaveParameters(OutList[i], out_format, 
		       NUM_MEL_FILTERS + (energy_flag ? 1 : 0),  
		       num_frames, out_data);
      }
      FreeParameters(out_data, num_frames);
    }
  }

  if (trace > 0) {
    unsigned long user, system;
    float real_time, used_time = 0.0;

    printf("Totally %.2f s of audio processed\n", 
       real_time = (float)TotN * (float)STEP_SIZE / (float)SAMP_FREQ);
    my_time(&user, &system);
    printf("CPU-time: %.2f s => %.2f %% of real time\n", 
       used_time = (float)user / 1000.0, 
       100.0 * used_time / real_time);
  }

  return SYSTEM_OK_EXIT;
}

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  Melfib.c                                         */
/* --------------------------------------------------------------------------*/
