/* ------------------------------------------------------------------------- */
/*              Definitions and functions related to the RTDNN               */
/* ------------------------------------------------------------------------- */
/*                                                                           */
/*   Copyright (C) 1994, 1995, 1996, 1997, 1998 Nikko Strom                  */
/*                                                                           */
/*   All rights reserved.                                                    */
/*                                                                           */
/*   Developed by Nikko Strom at the Dept. of Speech, Music and Hearing,     */
/*   KTH (Royal Institute of Technology)                                     */
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

#define RTDNN_MAGIC 5738919 
#define RTDNN_VERSION 1.10

#ifndef RTDNN_LOADED
#define RTDNN_LOADED

#include "ParamData.h"
#include "LogBook.h"


/* Types of groups */
typedef enum {PUBLIC=0, HIDDEN=1, RECEPT=2, SIGNAL=3} GroupType;

/* Forward types for units */
typedef enum {
  INPUT=0,
  TANHYP=1,
  ARCTAN=2,
  LINEAR=3,
  BIAS=6,
  FILEFILTER=7,
  ENVIRONMENT=8, 
  SIGMOID=9,   /* Old sigmoid function with range [0;1] */
  INVERTER=10,
  EXPONENTIAL=11,
  MULTIC=12
} ForwardType;

/* Backward types for units */
typedef enum {
  OUTPUT=0,
  NONOUTPUT=1,
  L4OUTPUT=2,
  L10OUTPUT=3,
  L0OUTPUT=4,
  CROSSENTR=5,
  ZERO_ONE=6,
  CROSS01=8,
  /* Special types start at 100 */
  SYMCROSSENTR=133, /* Used internally for [-1,1] units */
  SYM_ZERO_ONE=134, 
  SYM_CROSS01=135, 
  INTERACT=100,
  NOERROR=101
} BackwardType;


/* Types of streams */
typedef enum {
  NOACTION=-1,
  READ=1,
  TARGET=2,
  WRITE=3,
  INTERACTIVE=4,
} StreamType;


/* Identifier types */
typedef enum {
  DELETED=-10000,
  UNSPEC=-1,
  UNIT=0,
  GROUP=1,
  CONNECTION=2,
  STREAM=4 
} IdentifierType;


typedef struct Group {
  char      *Name;
  long        id;
  GroupType  type;
  long        mark;
  long        NumParents;
  long        NumMem;
  long       *MemTab;
} Group;


typedef struct Unit {
  char         *Name;
  long           id;
  ForwardType   type;
  BackwardType  backtype;
  float         backward_prune_thresh;
  long           mark;
  long           NumParents;
  long           index;
  long           link;     /* This is the id of a stream or -1 for "no link". */
  long           pos;      /* Used if linked to a stream */
} Unit;


/* For GAUSS units the contribution from the weight is: w*(o-m)*(o-m),
   for multic units the weigths aren't used, and for the other units 
   it's simply: o*w. */
typedef struct Connection {
  long            to;
  long            from;
  long            delay;
  float           w;
  float           plast;   /* Plasticity - scaling of gradient */
  long            mark;
  long            index;
} Connection;


typedef struct IdEntry {
  char          *Name;
  IdentifierType type;
  void          *ptr;
} IdEntry;


typedef struct Stream {
  char          *Name;
  long            id;
  StreamType     type;
  ParamFileType  format; /* File Formats are defined in ParamData.h */
  char          *ext;
  char          *path;
  long           size;  /* Floats/Frame */

  /* Defines linear transform from data to activities, i - a * b. */
  float *a, *b; 

  /* Name for each component of the stream-vector */
  char **CompName;
 
  char *Filter;  /* Filefilter for pre/post processing of the data */
  long            index;
  long            NumParents;
  long            mark;  
} Stream;


typedef struct Net {
  char           *Name;
  float           Version; 

  long            NumId;
  long            AllocatedId;
  long            NumGroups;
  long            NumTemplates;
  long            NumConnections, NumAllocatedCons, NumSortedCons;
  long            NumStreams;

  long            NumUnits;
  long            NumInput; 
  long            NumTanhyp; 
  long            NumArctan; 
  long            NumLinear;
  long            NumOutput;

  long            MaxDelay;

  Group        **GrTab;
  Unit         **UTab;
  Connection   **CTab; 
  Stream       **StrTab;
  IdEntry       *IdTab;

  Group         *rootgroup;
  Unit          *biasunit;

  LogBook       *logbook;
} Net;


/* --- Id-table Handeling --- */
void *GetPtr(Net *net, long id);
#define GETPTR(net, id, type) ((type *)GetPtr(net, id))

IdentifierType GetIdType(Net *net, long id);

char *GetIdName(Net *net, long id);

/* If "type" is UNSPEC (-1), any type is accepted. */
long GetId(Net *net, char *Name, IdentifierType type);  


/* --- Net Building --- */
Net *CreateNet(); 
void FreeNet(Net *net);

Group    *AddNamedGroup         (Net *net, char *Name, GroupType type);
Unit     *AddNamedUnit          (Net *net, char *Name, 
                                 ForwardType type, BackwardType backtype,
                                 float RandBound);

Stream *AddStream(Net *net, char *Name, StreamType, long size);

void AddUnits          (Net *net, long parent, 
                        ForwardType type, BackwardType backtype, 
                        long n, float RandBound);
void AddGroups         (Net *net, long parent, GroupType type, long n);

void  Join  (Net *net, long parent, long child);
void  UnJoin(Net *net, long parent, long child);
void  Move  (Net *net, long parent, long newparent, long child);


/* -------------------------------------------------
   copy_style=0: copy all con's
   copy_style=1: split weights of outflowing con's
   copy_style=2: don't copy outflowing con's
   ------------------------------------------------- */ 
long Copy(Net *net1, long id1, long copy_style);


long Import(Net *net1, long id1, Net *net2);

void DeleteObject(Net *net, long object, long recursive);
void DeleteConnection(Net *, Connection *);

void Rename(Net *net, long id, char *Name); 

void ReHash(Net *net);

/* NewId may be NULL in which case c.from and c.to are 
 * unchanged for all connections */
void ReHashConnections(Net *, long *NewId);

void SetUnitType(Net *net, long unit, ForwardType type, BackwardType backtype);

void ProtectGroup(Net *net, long group, GroupType Protection);

void SetBackwardPrune(Net *net, long unit, float prune);


/* --- Connections --- */
long GetRealDelays(Net *ann);

void OrderUnits(Net *net);

void SortConnections(Net *net);

Connection *AddConnection(Net *net, long to, long from, long delay, 
                          float weight);

void Connect(Net *net, long g1, long g2, long Delay, float RandBound);

void ConstConnect(Net *net, long g1, long g2, long Delay, float weight);
 
void SparseConnect(Net *net, long g1, long g2, long delay, float RandBound,
                   float Fraction);

void MetricConnect(Net *net, long g1, long g2,
                   long delay, float RandBound,
                   float fraction, float spread, long cyclic);

long UnConnect(Net *net, long g1, long g2, long Delay);


/* --- Stream linking --- */
long LinkUnit(Net *net, long stream, long pos, long unit); 


/* --- Storage --- */

/* Save a network to a file
 * This function automatically compresses .Z and .gz files. */ 
void SaveNet(char *fn, Net *net);

/* Save a RTDNN def file to a stream */
void WriteNet(Net *net, FILE *fp);

/* Load a RTDNN def file by file name.
 * This function automatically extracts .Z and .gz compressed files. */ 
Net *LoadNet(char *fn);

/* Load a RTDNN def file from a stream */
Net *ReadNet(FILE *fp);

/* --- Marking objects --- */
void UnMarkAll      (Net *net, IdentifierType type);
void Mark           (Net *net, long object, long val);
void UnMark         (Net *net, long object, long val);

void RecursiveMark  (Net *net, long object, long val);
void RecursiveUnMark(Net *net, long object, long val);

void SetAllConnectionMarks(Net *net, long val);
void MarkConnections(Net *net, long from, long to, long val);
void UnMarkConnections(Net *net, long from, long to, long val);


#endif

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  RTDNN.h                                          */
/* --------------------------------------------------------------------------*/
