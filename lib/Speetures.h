/* ------------------------------------------------------------------------- */
/*                          Speech feature extraction                        */
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

#ifndef SPEETUTRES_H_LOADED
#define SPEETUTRES_H_LOADED

/* ---------                   Basic feature types             ------------- */
/*  (Boolean OR on the basic feature types to get composite feture types)    */
 
#define CEPSTRUM_FEATURES   1
#define FIB_FEATURES        2

/* Default scale is Bark, set this bit for Mel scale */
#define MEL_FEATURES        4

#define BARKFIB_FEATURES    FIB_FEATURES
#define BARKCEP_FEATURES    CEPSTRUM_FEATURES
#define MELFIB_FEATURES     (FIB_FEATURES | MEL_FEATURES)
#define MELCEP_FEATURES     (CEPSTRUM_FEATURES | MEL_FEATURES)


#define ENERGY_FEATURES   128

#define CEPITCH_FEATURES  256
#define COVPITCH_FEATURES 512
#define PITCH_FEATURES    (CEPITCH_FEATURES | COVPITCH_FEATURES)


/* ----  Flags specifying extra features to add to the base-features    ---- */

/* log Energy (set at least one if ENERGY_FEATURES is set in the base-type) */
#define ADD_LOG_E         1
#define ADD_DELTA1_LOG_E  2
#define ADD_DELTA2_LOG_E  4

/* Delta coeff's of the base features */
#define ADD_DELTA1       16
#define ADD_DELTA2       32
#define ADD_ANY_DELTA \
  (ADD_DELTA1_LOG_E | ADD_DELTA1 | ADD_DELTA2_LOG_E | ADD_DELTA2)

/* Works with pitch features */
#define ADD_VOICING      64


/* Works with cepstrum features only */
#define ADD_ZEROTH_CEP 1024


struct SpeechEntry;
typedef struct SpeechEntry {
  struct SpeechEntry *next;
  struct SpeechEntry *prev;
  short *speech;
  int ptr;
  int size;
  int allocated_size;
} SpeechEntry;

struct FrameEntry;
typedef struct FrameEntry {
  struct FrameEntry *next;
  struct FrameEntry *prev;
  float *frame;   /* The actual frame vector, eg, filterbank activities */
  float Energy;   /* Temporary storage for energy */
  float Pitch;    /* Temporary storage for pitch */
  int n;          /* The index in the utterance (i.e, the nth frame) */
	void *user;     /* Arbitrary value specified by the caller of pushFeatures */
} FrameEntry;


/* ------------------------------------------------------------------------- */
/*   This is the speech feature stucture. It contains two FIFOs: one for the */
/*   incoming speech (shorts) and another for the outgoing frames of speech  */
/*   features (float vectors).                                               */
/* ------------------------------------------------------------------------- */
typedef struct Speetures {
  /* FIFO for the input speech */
  int num_samples_stacked;
  SpeechEntry *first_speech;
  SpeechEntry *last_speech;
  SpeechEntry *used_speech; /* These can be reused (saves mallocs) */

  int base_size;    /* The number of features/frame in the base-features */

  /* Base feature type */
  int base_type;

  /* Flags specifying additional features to add to the base-features */
  int add_flags;

  /* The number of used features from the base-type size */
  /* (e.g. 12 cepstrum coeff's used from a fib of 20 filters */
  int used_size;

  /* Use the spectrum between these frequencies */
  float low_cut, high_cut;

  float pre_emp_factor;
  float cep_lift;

  int num_frames;   /* Counts the number of frames popped */
  int num_pushed_frames; /* Counts the number of frames pushed */
  int memory_size;  /* Number of frames in memory necessary for delta comp's */
  int status;       /* 0 when SpeechEnd has been recieved, 1 otherwize */

  int frame_size;   /* The number of features/frame */

  /* used for mean subtraction */
  float *running_average;
  float ra_timeconst;
  int mean_subtraction;

  /* Offsets for the different components of the feature vector */
  int energy_offset;
  int delta1_energy_offset;
  int delta2_energy_offset;

  int pitch_offset;

  int delta1_offset;
  int delta2_offset;


  int samp_freq;    /* samples/sec */
  int frame_step;   /* number of samples/frame */

  int frame_window_size;     /* window width in samples */
  int allocated_window_size; /* allocated window width in samples */
  float *frame_window;       /* analysis window */

  void *info;       /* A hook for feature type dependent data */

  /* FIFO for the output frames */
  int num_stacked_frames;
  FrameEntry *first_frame;
  FrameEntry *last_frame;
  FrameEntry *used_frames; /* These can be reused (saves mallocs) */

  FrameEntry *next_frame_to_pop;
} Speetures;


/*   Push "num_samples" short-samples of audio to the Speetures struct       */
/*   (samp_freq [Hz], frame_length [sec])                                    */
Speetures *InitSpeetures(int base_type, int base_size, int used_size,
                         int add_flags, 
                         float samp_freq, 
                         float frame_length,
                         float frame_width,
                         float low_cut, float high_cut);

/*   Push "num_samples" short-samples of audio to the Speetures struct       */
void PushSpeech(Speetures *S, short *speech, int num_samples);


/*  Alternative to PushSpeech if the basefeatures are read from a file.      */
/*  Pushing one frame of base features to the Speetures struct.              */
/*  "base-features" must be an allocated vector of the size S->frame_size    */
/*  All features but the delta & delta-delta should be computed already.     */
/*  WARNING: "base-features" must be allocated by the caller. The same       */
/*  allocated area is later returned by 'PopFeatures'. It is then the        */
/*  responsibility of the recipient to free this vector.                     */
/*  the user parameter is specified by the caller, and is returned by  */
/*	PopFeatures2 */
	
void PushFeatures(Speetures *S, float *base_features);
void PushFeatures2(Speetures *S, float *base_features, void *user);


/*   Call this function when there are no more samples to push               */
void SpeechEnd(Speetures *S);


/*  Alternative to "SpeechEnd" -- used when PushFeatures are used directly,  */
/*  without pushing speech.                                                  */
void BaseFrameEnd(Speetures *S);


/*   Returns N if there are N frames to pop on the FIFO, -1 if the utterance */
/*   is finished and 0 otherwize.                                            */
int ProbeFeatures(Speetures *S);


/*   Get an allocated vector of features from S                              */
/*   This vector should be freed by the caller */
/*   Returns NULL if there are no frames to pop on the FIFO                  */
/*   PopFeatures2 returns the user value specified by PushFeatures2 if       */
/*	 user != NULL */
float *PopFeatures(Speetures *S);
float *PopFeatures2( Speetures *S, void **user );

/*   Free memory allocated in the Speetures structure                        */
void FreeSpeetures(Speetures *S);


#endif

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  Speetures.h                                      */
/* --------------------------------------------------------------------------*/
