/* ------------------------------------------------------------------------- */
/*                      Functions related to the RTDNN                       */
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

#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include "System.h"
#include "nMath.h"
#include "RTDNN.h" 

/* -------------               Id-table Handeling           -------------- */

static void 
InitId(Net *net) {
  net->AllocatedId = 256;
  CALLOC(net->IdTab, net->AllocatedId, IdEntry)
  net->NumId = 0;
} 


static long 
CreateId(Net *net, char *Name, IdentifierType type, void *ptr) {
  IdEntry *id;

  if (net->NumId == net->AllocatedId) {
    net->AllocatedId *= 2;
    REALLOC(net->IdTab, net->AllocatedId, IdEntry)
  }

  id = net->IdTab + net->NumId;
  (net->NumId)++;
  id->type = type;
  id->ptr = ptr;
  id->Name = Name;

  return net->NumId - 1;
}


static long 
check_if_integer(char *s) {
  long i;
 
  for (i = 0; i < strlen(s); i++) if (s[i] < '0' || s[i] > '9') return 0;
  return 1;
}


static long 
GetId2(Net *, char *Name, long identifier, IdentifierType);
/* Call this fn if Name starts with a numeric index */
static long 
GetId3(Net *net, char *Name, long identifier, IdentifierType type) {
  long i, n, val = UNSPEC, NumMem, *MemTab;
  IdEntry *id;
  char *this_, *next, *r, *rr, delimiter;

  id = net->IdTab + identifier;

  if (GetIdType(net, identifier) == GROUP) {
    NumMem = GETPTR(net, identifier, Group)->NumMem;
    MemTab = GETPTR(net, identifier, Group)->MemTab;
  }
  else return UNSPEC;

  STRDUP(this_, Name);

  r = strchr(this_, '#');
  rr = strchr(this_, '.');

  if (!r && ! rr) {
    if (!check_if_integer(this_)) 
      ErrorExit(SYSTEM_ERR_EXIT, "Integer expected (got: %s)", this_);
    n = atoi(this_);
    if (n <= 0) 
      ErrorExit(SYSTEM_ERR_EXIT, "Indexing starts from 1 (got: %i)", n);

    FREE(this_);
    if (n > NumMem) {
      return UNSPEC;
    }
    else {
      return MemTab[n - 1];
    }
  }

  if (!r || (rr && rr < r)) {
    delimiter = '.';
    *rr = '\0';
    next = rr + 1;
  }
  else {
    delimiter = '#';
    *r = '\0';
    next = r + 1;
  }

  /* Now 'this' is the index into 'MemTab' and 'next' is the name
     after the delimiter and 'delimiter' is . or # */

  if (!check_if_integer(this_)) 
    ErrorExit(SYSTEM_ERR_EXIT, "Integer expected (got: %s)", this_);
  n = atoi(this_);
  if (n <= 0) 
    ErrorExit(SYSTEM_ERR_EXIT, "Indexing starts from 1 (got: %i)", n);

  if (delimiter == '#') {
    val = GetId3(net, next, MemTab[n - 1], type);
  }
  else {
    id = net->IdTab + MemTab[n - 1];

    if (id->type == GROUP) {
      n = ((Group *)id->ptr)->NumMem;
      MemTab = ((Group *)id->ptr)->MemTab;
    }
    else {
      FREE(this_);
      return UNSPEC;
    }

    for (i = 0; i < n; i++) {
      if ((val = GetId2(net, next, MemTab[i], type)) != UNSPEC) {
        FREE(this_);
        return val; 
      }
    }
  }

  FREE(this_);
  return val;
}


static long 
GetId2(Net *net, char *Name, long identifier, IdentifierType type) {
  long i, val, NumMem, *MemTab;
  IdEntry *id;
  char *this_, *next, *r, *rr, delimiter;
  
  id = net->IdTab + identifier;

  if (strcmp(Name, id->Name) == 0 && (type == UNSPEC || id->type == type))
        return identifier;

  if (GetIdType(net, identifier) == GROUP) {
    NumMem = GETPTR(net, identifier, Group)->NumMem;
    MemTab = GETPTR(net, identifier, Group)->MemTab;
  }
  else return UNSPEC;

  STRDUP(this_, Name);

  r = strchr(this_, '#');
  rr = strchr(this_, '.');
  if (!r && !rr) return UNSPEC;

  if (!r || (rr && rr < r)) {
    delimiter = '.';
    *rr = '\0';
    next = rr + 1;
  }
  else {
    delimiter = '#';
    *r = '\0';
    next = r + 1;
  }

  if (strcmp(this_, id->Name) != 0) return UNSPEC;

  if (delimiter == '#') {
    if ((val = GetId3(net, next, identifier, type)) != UNSPEC) {
      FREE(this_);
      return val; 
    }
  }
  else for (i = 0; i < NumMem; i++) {
    id = net->IdTab + MemTab[i];

    if ((val = GetId2(net, next, MemTab[i], type)) != UNSPEC) {
      FREE(this_);
      return val; 
    }
  }

  FREE(this_);
  return UNSPEC;
}


/*    'Name' can have the index forms: g.sub or g#47     */
long 
GetId(Net *net, char *Name, IdentifierType  type) {
  long i, val;
  
  i = 0;
  while (i < net->NumId) {
    if ((val = GetId2(net, Name, i, type)) != UNSPEC) return val;
    i++;
  }
  return UNSPEC;
}


IdentifierType GetIdType(Net *net, long id) {
  return net->IdTab[id].type;
}


char *
GetIdName(Net *net, long id) {
  return net->IdTab[id].Name;
}


void *
GetPtr(Net *net, long id) {
  return net->IdTab[id].ptr;
}



/* ------------              Network Building              ------------ */

/* Refresh info about nuber of different unit-types */
static void 
ComputeUnitTypesNumbers(Net *net) {
  long sigmo = 0;
  long symsig = 0;
  long input = 0;
  long output = 0;
  long linear = 0;
  /* long multic = 0; */
  long i;

  for (i = 0; i < net->NumUnits; i++) {
    if (net->UTab[i]->type == INPUT) input++;
    else if (net->UTab[i]->type == TANHYP) sigmo++;
    else if (net->UTab[i]->type == LINEAR) linear++;
    else if (net->UTab[i]->type == ARCTAN) symsig++;
    if (net->UTab[i]->backtype != NONOUTPUT) output++;
  }
  net->NumInput = input;
  net->NumTanhyp = sigmo;
  net->NumArctan = symsig;
  net->NumLinear = linear;
  net->NumOutput = output;
}


static Unit *
CreateUnit(Net *net, char *Name, ForwardType type) {
  Unit *u;

  CALLOC(u, 1, Unit)

  STRDUP(u->Name, Name);
  u->type     = type;
  u->backtype = NONOUTPUT;
  u->id       = CreateId(net, Name, UNIT, (void *)u);
  u->backward_prune_thresh = +1.0e-8;
  u->link     = -1;
  u->mark     = 0;
  u->NumParents = 0;
  return u;
}


static void 
FreeUnit(Unit *u) {
  FREE(u->Name);
  FREE(u);
}


static Connection *
CreateConnection(Net *net) {
  Connection *c;

  CALLOC(c, 1, Connection)

  c->mark     = 0;
  c->plast    = 1.0;
  c->w        = 0.0;

  c->index = net->NumConnections;
  net->CTab[c->index] = c;
  net->NumConnections++;

  return c;
}


/* I use this compare for qsort and bsearch */ 
/* Returns 0 if a==b, positive if a>b and negative if a<b */
static int 
ConCompare(const void *A, const void *B) {
  Connection *a = *(Connection **)A;
  Connection *b = *(Connection **)B;

  if (a == b) return 0; /* a and b are the same connection */

  /* First check the from-node */
  if (a->from > b->from) return 1;
  if (a->from < b->from) return -1;

  /* Then the to-node */
  if (a->to > b->to) return  1;
  if (a->to < b->to) return -1;

  /* Finally check the time delay */
  if (a->delay > b->delay) return 1;
  if (a->delay < b->delay) return -1;

  /* Two different connections with the same from and to node and delay */
  return 0;
}


static void 
FreeConnection(Connection *c) {
  FREE(c);
}


static Group *CreateGroup(Net *net, char *Name, GroupType type) {
  Group *g;

  CALLOC(g, 1, Group)

  STRDUP(g->Name, Name);
  g->type = type;
  g->id = CreateId(net, Name, GROUP, (void *)g);
  g->mark = 0;
  g->NumParents = 0;
  g->NumMem = 0;

  CALLOC(g->MemTab, 1, long)

  return g;
}


void 
FreeGroup(Group *g) {
  FREE(g->MemTab);
  FREE(g->Name);
  FREE(g);
}


static Stream *CreateStream(Net *net, char *Name, StreamType type) {
  Stream *str;

  CALLOC(str, 1, Stream)

  STRDUP(str->Name, Name);
  str->id = CreateId(net, Name, STREAM, (void *)str);
  str->format = BINARY;
  STRDUP(str->ext,  "data");
  STRDUP(str->path, ".");
  str->size = 0;
  str->type = type;

  CALLOC(str->a, 1, float)
  CALLOC(str->b, 1, float)
  CALLOC(str->CompName, 1, char *)

  str->NumParents = 0;

  return str;
}


void 
FreeStream(Stream *str) {
  FREE2D(str->CompName, str->size);

  FREE(str->Name);
  FREE(str->ext);
  FREE(str->path);
  FREE(str->a);
  FREE(str->b);
  FREE(str->Filter);
  FREE(str);
}


Net *CreateNet() {
  Net *net;
  Unit *u;

  CALLOC(net, 1, Net)

  net->Version        = RTDNN_VERSION;
  net->NumUnits       = 0;
  net->NumInput       = 0;
  net->NumTanhyp      = 0;
  net->NumArctan      = 0;
  net->NumOutput      = 0;  
  net->NumGroups      = 0;  
  net->NumConnections = 0;  
  net->NumStreams     = 0;
  net->MaxDelay       = 0;

  InitId(net);

  CALLOC(net->UTab, 1, Unit *)
  CALLOC(net->GrTab, 1, Group *)
  CALLOC(net->StrTab, 1, Stream *)

  net->NumSortedCons = 0;

  net->NumAllocatedCons = 512;
  CALLOC(net->CTab, net->NumAllocatedCons, Connection *)

  net->rootgroup = AddNamedGroup(net, "network", PUBLIC);

  u = AddNamedUnit(net, "bias", BIAS, NONOUTPUT, 0.0);
  Join(net, net->rootgroup->id, u->id);
  net->biasunit = u;

  net->logbook = CreateLogBook();

  return net;
}


void 
FreeNet(Net *net) {
  long i;

  for (i = 0; i < net->NumUnits; i++) FreeUnit(net->UTab[i]);
  FREE(net->UTab); 

  for (i = 0; i < net->NumConnections; i++) FreeConnection(net->CTab[i]);
  FREE(net->CTab); 

  for (i = 0; i < net->NumStreams; i++) FreeStream(net->StrTab[i]);
  FREE(net->StrTab);
 
  for (i = 0; i < net->NumGroups; i++) FreeGroup(net->GrTab[i]);
  FREE(net->GrTab); 

  FreeLogBook(net->logbook);

  FREE(net->IdTab);
  
  FREE(net->Name);
  
  FREE(net);
}


void 
DeleteObject(Net *net, long obj, long recursive) {
  long i;

  if (GetIdType(net, obj) == GROUP && 
      GETPTR(net, obj, Group) == net->rootgroup)
     ErrorExit(SYSTEM_ERR_EXIT, "The root-group \"network\" cannot be deleted");

  if (GetIdType(net, obj) == UNIT && GETPTR(net, obj, Unit) == net->biasunit)
     ErrorExit(SYSTEM_ERR_EXIT, "The bias-unit cannot be deleted");

  if (recursive) {
    if (GetIdType(net, obj) == GROUP) {
      Group *g;

      g = GETPTR(net, obj, Group);
      for (i = 0; i < g->NumMem; i++) DeleteObject(net, g->MemTab[i], 1);
    }
  }
  net->IdTab[obj].type = DELETED;

  ComputeUnitTypesNumbers(net);
}


void 
Rename(Net *net, long obj, char *Name) {
  switch (GetIdType(net, obj)) {
    case UNIT:
      if (GETPTR(net, obj, Unit) == net->biasunit)
         ErrorExit(SYSTEM_ERR_EXIT, "The bias-unit cannot be renamed");

      GETPTR(net, obj, Unit)->Name = Name;
      net->IdTab[obj].Name = Name;
    break;

    case GROUP:
      if (GETPTR(net, obj, Group) == net->rootgroup)
        ErrorExit(SYSTEM_ERR_EXIT, "The root-group \"network\" cannot be renamed");

      GETPTR(net, obj, Group)->Name = Name;
      net->IdTab[obj].Name = Name;
    break;

    case STREAM:
      GETPTR(net, obj, Stream)->Name = Name;
      net->IdTab[obj].Name = Name;
    break;

    default:
      ErrorExit(SYSTEM_ERR_EXIT, "Only units, groups and streams can be renamed");
  }
}


void 
ReHashConnections(Net *net, long *NewId) {
  Connection *c;
  int i, j;

  for (i = 0, j = 0; i < net->NumConnections; i++) {
    c = net->CTab[i];
    if (c != NULL) {
      if (NewId != NULL) {
	c->from = NewId[c->from];
	c->to = NewId[c->to];
      }
      net->CTab[j] = c;
      c->index = j;
      j++;
    }
  }
  if (i != j) {
    net->NumConnections = j;
    if (j == 0) j = 1; /* Always allocate space for at least one object */
    REALLOC(net->CTab, j, Connection *)
  }
}


void 
DeleteConnection(Net *net, Connection *c) {
  net->CTab[c->index] = NULL;
  FreeConnection(c);
}


void 
ReHash(Net *net) {
  IdEntry    *id;
  Unit       *u;
  Group      *g;
  Stream     *s;
  Connection *c;
  long *NewId;
  long i,j,k;

  CALLOC(NewId, net->NumId, long)

  /* Delete connections with a 'loose end'. */
  for (i = 0, j = 0; i < net->NumConnections; i++) {
    c = net->CTab[i];
    if (c && (net->IdTab[GETPTR(net, c->from, Unit)->id].type == DELETED ||
        net->IdTab[GETPTR(net, c->to, Unit)->id].type == DELETED)) {
          DeleteConnection(net, c);
    }
  }

  /* Set up the new id's */
  for (i = 0, j = 0; i < net->NumId; i++) {
    id = net->IdTab + i;
    if (id->type != DELETED) {
      NewId[i] = j;
      j++;
    }
  }

  /* Handle deleted members of groups */
  for (k = 0; k < net->NumGroups; k++) {
    g = net->GrTab[k];
    for (i = 0, j = 0; i < g->NumMem; i++) {
      if (net->IdTab[g->MemTab[i]].type != DELETED) {
        g->MemTab[j] = NewId[g->MemTab[i]];
        j++;
      }
    }
    if (i != j) {
      g->NumMem = j;
      if (g->NumMem == 0) j = 1; else j = g->NumMem;
      REALLOC(g->MemTab, j, long)
    }
  } 

  /* Handle connections */
  ReHashConnections(net, NewId);

  /* Handle units */
  for (i = 0, j = 0; i < net->NumUnits; i++) {
    u = net->UTab[i];
    if (net->IdTab[u->id].type == DELETED) {
      FreeUnit(u);
    }
    else {
      u->id = NewId[u->id];
      if (u->link != -1) {
        /* If the stream is deleted - unlink the unit from it.*/
        if (net->IdTab[u->link].type == DELETED) u->link = -1;
        else u->link = NewId[u->link];
      }
      net->UTab[j] = u;
      j++;
    }
  }
  if (i != j) {
    net->NumUnits = j;
    if (net->NumUnits == 0) j = 1; else j = net->NumUnits;
    REALLOC(net->UTab, j, Unit *)
  }

  /* Handle deleted groups */
  for (i = 0, j = 0; i < net->NumGroups; i++) {
    g = net->GrTab[i];
    if (net->IdTab[g->id].type == DELETED) {
      if (g->NumMem != 0) {
        for (k = 0; k < g->NumMem; k++) {
          if (net->IdTab[g->MemTab[k]].type != DELETED) {
            ErrorExit(SYSTEM_ERR_EXIT, "Attempt to delete nonempty group %s", g->Name);
          }
        }
      }
      FreeGroup(g);
    }
    else {
      g->id = NewId[g->id];
      net->GrTab[j] = g;
      j++;
    }
  }
  if (i != j) {
    net->NumGroups = j;
    if (net->NumGroups == 0) j = 1; else j = net->NumGroups;
    REALLOC(net->GrTab, j, Group *)
  }

  /* Handle deleted streams */
  for (i = 0, j = 0; i < net->NumStreams; i++) {
    s = net->StrTab[i];
    if (net->IdTab[s->id].type == DELETED) {
      FreeStream(s);
    }
    else {
      s->id = NewId[s->id];
      net->StrTab[j] = s;
      j++;
    }
  }
  if (i != j) {
    net->NumStreams = j;
    if (net->NumStreams == 0) j = 1; else j = net->NumStreams;
    REALLOC(net->StrTab, j, Stream *) 
  }

  /* Set up the id-entries */
  for (i = 0, j = 0; i < net->NumId; i++) {
    id = net->IdTab + i;
    if (id->type != DELETED) {
      memcpy(net->IdTab + j, net->IdTab + i, sizeof(IdEntry));
      j++;
    }
  }
  net->NumId = j;
  if (net->NumId == 0) j = 1; else j = net->NumId;
  REALLOC(net->IdTab, j, IdEntry) 

  FREE(NewId);
}


Group *AddNamedGroup(Net *net, char *Name, GroupType type) {
  Group *g;

  net->NumGroups++;
  g = CreateGroup(net, Name, type);

  REALLOC(net->GrTab, net->NumGroups, Group *)

  net->GrTab[net->NumGroups - 1] = g;

  return g;
}

         
Unit *AddNamedUnit(Net *net, char *Name, 
                   ForwardType type, BackwardType backtype,
                   float RandBound) {
  Unit *u;

  net->NumUnits++;
  u = CreateUnit(net, Name, type);

  REALLOC(net->UTab, net->NumUnits, Unit *)

  net->UTab[net->NumUnits - 1] = u;
  u->backtype = backtype;

  /* Add bias-connection by default to all non-input units */
  if (type != INPUT && RandBound > 0.0)
    AddConnection(net, u->id, net->biasunit->id, 0, 
        (RectRand() - 0.5) * 2.0 * RandBound);

  return u;
}

         
Stream *AddStream(Net *net, char *Name, StreamType type, long size) {
  Stream *str;
  long i;

  net->NumStreams++;
  str = CreateStream(net, Name, type);

  REALLOC(net->StrTab, net->NumStreams, Stream *)
  net->StrTab[net->NumStreams - 1] = str;

  /* An empty list of components */
  CALLOC(str->CompName, size, char *)
  for (i = 0; i < size; i++) {
    CALLOC(str->CompName[i], 1, char)
  }

  STRDUP(str->Filter, "\0")

  str->size = size;

  /* The linear transormation of the stream */
  CALLOC(str->a, size, float)
  CALLOC(str->b, size, float)

  if (type == READ) { /* default for input-streams: identity mappping */
    for (i = 0; i < size; i++) {
      str->a[i] = 0.0;
      str->b[i] = 1.0;
    }
  }
  else { /* default for non-read streams: map [-1, 1] to [0, 1] */
    for (i = 0; i < size; i++) {
      str->a[i] = 1.0;
      str->b[i] = 0.5;
    }
  }

  return str;
}

         
void 
AddUnits(Net *net, long group, ForwardType type, BackwardType backtype, 
              long n, float RandBound) {
  long i;
  Unit *u;
  Group *g = NULL;
  char *Name;

  if (GetIdType(net, group) == GROUP) {
    g = GETPTR(net, group, Group);
    if (g->NumMem != 0) ErrorExit(SYSTEM_ERR_EXIT,
      "Cannot add multiple unnamed units to nonempty group.");
  }
  else {
    ErrorExit(SYSTEM_ERR_EXIT, 
      "Only groups can be parents to multiple unnamed units.");
  }
  for (i = n; i; i--) {
    CALLOC(Name, 1, char)
    Name[0] = '\0';

    u = CreateUnit(net, Name, type);
    u->backtype = backtype;

    net->NumUnits++;
    REALLOC(net->UTab, net->NumUnits, Unit *)

    net->UTab[net->NumUnits - 1] = u;

    g->NumMem++;
    REALLOC(g->MemTab, g->NumMem, long)
    g->MemTab[g->NumMem - 1] = u->id;

    if (type != INPUT && RandBound > 0.0) {
        AddConnection(net, u->id, GetId(net, "bias", UNIT), 0, 
             (RectRand() - 0.5) * 2.0 * RandBound);
    }
  }

  ComputeUnitTypesNumbers(net);
}


void 
AddGroups(Net *net, long group, GroupType type, long n) {
  Group *g;
  Group *parent;
  char *Name;
  long i;

  if (GetIdType(net, group) != GROUP)
    ErrorExit(SYSTEM_ERR_EXIT, "Multiple groups must have a group-parent.");

  parent = GETPTR(net, group, Group);
  if (parent->NumMem != 0) 
    ErrorExit(SYSTEM_ERR_EXIT, "Cannot add multiple unnamed objects to nonempty group.");

  for (i = n; i; i--) {
    CALLOC(Name, 1, char)

    Name[0] = '\0';
    g = AddNamedGroup(net, Name, type);
    parent->NumMem++;
    REALLOC(parent->MemTab, parent->NumMem, long)

    parent->MemTab[parent->NumMem - 1] = g->id;
  }
}


/*   Add 'child' to the list of members to 'parent'   */
void 
Join(Net *net, long parent, long child) {

  /* Two special cases */
  if (GetIdType(net, child) == GROUP && 
      GETPTR(net, child, Group) == net->rootgroup)
    ErrorExit(SYSTEM_ERR_EXIT, "The root-group \"network\" cannot be joined");
  if (GetIdType(net, child) == UNIT && 
      GETPTR(net, child, Unit) == net->biasunit)
    ErrorExit(SYSTEM_ERR_EXIT, "The  bias-unit cannot be joined");

  
  /* Deal with the parent first */
  if (GetIdType(net, parent) == GROUP) {
    Group *g;
    
    g = GETPTR(net, parent, Group);
    if (g->NumMem != 0 && strcmp(GetIdName(net, g->MemTab[0]),"") == 0)
      ErrorExit(SYSTEM_ERR_EXIT, "Cannot mix named and unnamed objects.");

    REALLOC(g->MemTab, g->NumMem + 1, long)

    g->MemTab[g->NumMem] = child;

    g->NumMem++;
  }
  else {
    ErrorExit(SYSTEM_ERR_EXIT, "the \"parent\" %s is not a group.", GetIdName(net, parent));
  }

  /* Then take care of the child */
  switch(GetIdType(net, child)) {
    case UNIT:
      GETPTR(net, child, Unit)->NumParents++;
      break;
    case GROUP:
      GETPTR(net, child, Group)->NumParents++;
      break;
    case STREAM:
      GETPTR(net, child, Stream)->NumParents++;
      break;
    default:
      ErrorExit(SYSTEM_ERR_EXIT, "Can't join this type of object\n");
  }
}


/*   Remove 'child' from the list of members to 'parent'      */
/*   NOTE: any child must be a member of at least one parent  */
void 
UnJoin(Net *net, long parent, long child) {
  long i, n;
  Unit *u; Group *g; Stream *str;

  /* A special case */
  if (GetIdType(net, child) == UNIT && 
      GETPTR(net, child, Unit) == net->biasunit)
    ErrorExit(SYSTEM_ERR_EXIT, "The  bias-unit cannot be unjoined");

  /* Deal with the child first */
  switch(GetIdType(net, child)) {
    case UNIT:
      (u = GETPTR(net, child, Unit))->NumParents--;
      if (u->NumParents == 0) 
        ErrorExit(SYSTEM_ERR_EXIT, "Cannot unjoin this object from its last parent");
      break;

    case GROUP:
      (g = GETPTR(net, child, Group))->NumParents--;
      if (g->NumParents == 0) 
        ErrorExit(SYSTEM_ERR_EXIT, "Cannot unjoin this object from its last parent");
      break;

    case STREAM:
      (str = GETPTR(net, child, Stream))->NumParents--;
      if (str->NumParents == 0) 
        ErrorExit(SYSTEM_ERR_EXIT, "Cannot unjoin this object from its last parent");
      break;

    default:
      ErrorExit(SYSTEM_ERR_EXIT, "Can't unjoin this type of object\n");
  }


  /* Then take care of the parent */
  if (GetIdType(net, parent) == GROUP) {
    Group *g;
    
    if (strcmp(GetIdName(net, child), "") == 0) {
      ErrorExit(SYSTEM_ERR_EXIT, "Cannot unjoin unnamed objects.");
    }

    g = GETPTR(net, parent, Group);

    i = 0;
    while (i < g->NumMem && g->MemTab[i] != child) i++;
    if (g->MemTab[i] != child) {
      ErrorExit(SYSTEM_ERR_EXIT, "%s is not in group %s", GetIdName(net, child),
                GetIdName(net, parent));
    }

    for (; i < g->NumMem - 1; i++) g->MemTab[i] = g->MemTab[i + 1];
    g->NumMem--;
    if (g->NumMem == 0) n = 1; else n = g->NumMem; 
    REALLOC(g->MemTab, n, long)
  }
  else {
    ErrorExit(SYSTEM_ERR_EXIT, "the \"parent\" %s is not a group.", GetIdName(net, parent));
  }
}


/* Moving is of course equivalent to Join + Unjoin */
void 
Move(Net *net, long parent, long newparent, long child) {
  Join(net, newparent, child);
  UnJoin(net, parent, child);
}


/* Copy an object from one net to another */
static long 
Copy2(Net *net1, long id1, Net *net2) {
  Group      *g, *gc;
  Stream     *str, *strc;
  Unit       *u, *uc;
  long i, type;

  type = GetIdType(net1, id1);

  switch(type) {
    case GROUP:
      g  = GETPTR(net1, id1, Group);
      gc = AddNamedGroup(net2, g->Name, g->type);
      g->mark = gc->id; /* This is a temporary link used for connection. */

      if (g->NumMem == 0) i = 1; else i = g->NumMem;
      CALLOC(gc->MemTab, i, long)

      gc->NumMem = g->NumMem;
      for (i = 0; i < g->NumMem; i++) {
        gc->MemTab[i] = Copy2(net1, g->MemTab[i], net2);
        gc->NumParents = 1;
      }

      return gc->id;

    case STREAM:
      str  = GETPTR(net1, id1, Stream);
      strc = AddStream(net2, str->Name, str->type, str->size);

      STRDUP(strc->ext, str->ext)
      STRDUP(strc->path, str->path)
      STRDUP(strc->Filter, str->Filter)
      strc->size    = str->size;
      strc->format  = str->format;

      CALLOC(strc->CompName, str->size, char *)
      CALLOC(strc->a, str->size, float)
      CALLOC(strc->b, str->size, float)
      for (i = 0; i < str->size; i++) {
        STRDUP(strc->CompName[i], str->CompName[i])
        strc->a[i] = str->a[i];
        strc->b[i] = str->b[i];
      }

      str->mark = strc->id; /* This is a temporary link used for connection. */

      return strc->id;

    case UNIT:
      u = GETPTR(net1, id1, Unit);
      if (u == net1->biasunit) return net2->biasunit->id;

      uc = AddNamedUnit(net2, u->Name, u->type, u->backtype, 0.0);
      u->mark = uc->id; /* This is a temporary link used for connection. */

      uc->link = -u->link - 10; /* Be careful with this mark! */
      uc->pos = u->pos;
      uc->backward_prune_thresh = u->backward_prune_thresh;
      uc->NumParents = 1;

      return uc->id;
 
    default: 
      ErrorExit(SYSTEM_ERR_EXIT, "Copying of this object-type is not supported.");
  }

  return -1; /* this never happens */
}


long 
Import(Net *net1, long id1, Net *net2) {
  long i, id2, from, to;
  Connection *c, *cc;
  Unit *u, *uf, *ut;
  Stream *str;

  UnMarkAll(net1, UNSPEC);

  id2 = Copy2(net1, id1, net2);

   /* 'Copy2' sets the created unit's links to '-link-10' */
   for (i = 0; i < net2->NumUnits; i++) {
    u = net2->UTab[i];
    if (u->link <= -9) {  
      if (u->link == -9) u->link = -1;
      else {
        str = GETPTR(net1, -u->link - 10, Stream);
        if (str->mark) u->link = str->mark;
        else u->link = str->id;
      }
    }
  }

  for (i = 0; i < net1->NumConnections; i++) {
    c = net1->CTab[i];
    uf = GETPTR(net1, c->from, Unit);
    ut = GETPTR(net1, c->to, Unit);

    if ((uf->mark || uf->type == BIAS) && ut->mark) {

      if (uf->type == BIAS) from = net2->biasunit->id;
      else from = uf->mark;

      to = ut->mark;

      cc = AddConnection(net2, to, from, c->delay, c->w);
      cc->plast = c->plast;
    }
  }

  return id2;
}


long 
Copy(Net *net, long id1, long copy_style) {
  long i, id2, from, to;
  Connection *c, *cc;
  Unit *u, *uf, *ut;
  long old_num_c = net->NumConnections;

  UnMarkAll(net, UNSPEC);

  id2 = Copy2(net, id1, net);

  /* 'Copy2' sets the created unit's links to '-link-10' */
  for (i = 0; i < net->NumUnits; i++) {
    u = net->UTab[i];
    if (u->link <= -9) u->link = -u->link - 10;
  }

  switch (copy_style) {
    case 0: /* Copy all con's */
    for (i = 0; i < old_num_c; i++) {
      c = net->CTab[i];
      uf = GETPTR(net, c->from, Unit);
      ut = GETPTR(net, c->to, Unit);
      if ((uf->mark || ut->mark) && c->mark != -1) {
        if (uf->mark) from = uf->mark; else from = c->from;
        if (ut->mark) to = ut->mark; else to = c->to;
        cc = AddConnection(net, to, from, c->delay, c->w);
        cc->plast = c->plast;
        cc->mark = -1;
      }
    }
    break;

    case 1: /* split the weight */
    for (i = 0; i < old_num_c; i++) {
      c = net->CTab[i];
      uf = GETPTR(net, c->from, Unit);
      ut = GETPTR(net, c->to, Unit);
      if ((uf->mark || ut->mark) && c->mark != -1) {
        if (ut->mark) to = ut->mark; else to = c->to;
        if (uf->mark) {
          from = uf->mark;
          if (!(ut->mark)) c->w /= 2.0; /* split */
        }
        else from = c->from;
        cc = AddConnection(net, to, from, c->delay, c->w);
        cc->plast  = c->plast;
        cc->mark = -1;
      }
    }
    break;

    case 2: /* Don't copy outflowing con's */
    for (i = 0; i < old_num_c; i++) {
      c = net->CTab[i];
      uf = GETPTR(net, c->from, Unit);
      ut = GETPTR(net, c->to, Unit);
      if (ut->mark && c->mark != -1) {
        if (uf->mark) from = uf->mark; else from = c->from;
        if (ut->mark) to = ut->mark; else to = c->to;
        cc = AddConnection(net, to, from, c->delay, c->w);
        cc->plast  = c->plast;
        cc->mark = -1;
      }
    }
    break;  
  }

  return id2;
}


void 
SetUnitType(Net *net, long unit, ForwardType type, BackwardType backtype) {
  Unit *u;
  Group *g;
  long i;

  if (GetIdType(net, unit) == UNIT) {
    u = GETPTR(net, unit, Unit);

    if (u == net->biasunit) 
      ErrorExit(SYSTEM_ERR_EXIT, "Can't change the type of the bias-unit");

    if (type != -1) u->type = type;
    if (backtype != -1) u->backtype = backtype;
  }
  else if (GetIdType(net, unit) == GROUP) {
    g = GETPTR(net, unit, Group);
    for (i = 0; i < g->NumMem; i++)
      SetUnitType(net, g->MemTab[i], type, backtype);
  }
  
  ComputeUnitTypesNumbers(net);
}


void 
SetBackwardPrune(Net *net, long unit, float prune) {
  Unit *u;
  Group *g;
  long i;

  if (GetIdType(net, unit) == UNIT) {
    u = GETPTR(net, unit, Unit);
    u->backward_prune_thresh = prune;
  }
  else if (GetIdType(net, unit) == GROUP) {
    g = GETPTR(net, unit, Group);
    for (i = 0; i < g->NumMem; i++) SetBackwardPrune(net, g->MemTab[i], prune);
  }
}



/*  Mark units and connections with the delays used in the real simulations  */
/*               Return the maximum total delay for units                    */
long 
GetRealDelays(Net *ann) {
  Connection *c;
  Unit *u_from, *u_to;
  long i, n = 0, this_delay, finished;
  long max_delay = 0;

  UnMarkAll(ann, UNSPEC);

  do {
    finished = 1;
    for (i = 0; i < ann->NumConnections; i++) {
      c = ann->CTab[i];
      u_from = GETPTR(ann, c->from, Unit);
      u_to = GETPTR(ann, c->to, Unit);
      this_delay = u_from->mark - u_to->mark + c->delay;


      if (this_delay < 0) {
        u_to->mark += this_delay;
        if (u_to->mark > max_delay) max_delay = u_to->mark;
        this_delay = 0;
        finished = 0;
      }
      
      if (this_delay > max_delay) max_delay = this_delay;
      c->mark = this_delay;
    }

    n++;
    if (n > ann->NumUnits)
      ErrorExit(SYSTEM_ERR_EXIT, "Zero-time connection-loop detected");

  } while (!finished);

  ann->MaxDelay = max_delay;
  return max_delay;
}


/*   Order the units in UTab in such a way that zero-time connections are   */
/*   from lower index to higher. If this not possible, exit with error      */
void 
OrderUnits(Net *net) {
  Connection *c;
  Unit *u_from, *u_to;
  Unit **NewUTab;
  long i, j, finished, max_depth, *distr, *index;
  long old_mark, new_mark;

  GetRealDelays(net);

  /* For each unit, calculate the connection-depth... */  
  UnMarkAll(net, UNIT);

  max_depth = 0;
  do {
    finished = 1;
    for (i = 0; i < net->NumConnections; i++) {
      c = net->CTab[i];

      /* Only bother about real (see GetRealDelays) zero-delay con's */
      if (c->mark == 0) {
        u_to = GETPTR(net, c->to, Unit);
        u_from = GETPTR(net, c->from, Unit);
        old_mark = u_from->mark;
        new_mark = u_to->mark + 1;
        if (new_mark > old_mark) {
          u_from->mark = new_mark;
          finished = 0;
          if (new_mark > max_depth) max_depth = new_mark;
        }
      }
    }
    if (max_depth > net->NumUnits) 
      ErrorExit(SYSTEM_ERR_EXIT, "Zero-time connection-loop detected");
  } while (!finished);

  /* ...then sort (distribution counting) units according to this depth. */
  CALLOC(distr, max_depth + 2, long)
  CALLOC(index, max_depth + 2, long)

  CALLOC(NewUTab, net->NumUnits, Unit *)

  for (i = 0; i < net->NumUnits; i++) {
    u_from = net->UTab[i];
    distr[u_from->mark]++;
  }

  index[max_depth] = 0;
  for (i = max_depth - 1; i >= 0; i--) index[i] = index[i + 1] + distr[i + 1];

  for (i = 0; i < net->NumUnits; i++) {
    u_from = net->UTab[i];
    j = index[u_from->mark]++;
    NewUTab[j] = net->UTab[i];
  }

  FREE(distr);
  FREE(index);
  FREE(net->UTab);
  net->UTab = NewUTab;
}


Connection *AddConnection(Net *net, long to, long from, long delay, 
                          float weight) {
  Connection *c;
  Connection pattern, *cptr, **cptr2;
  Unit *u;

  /* If already there, find the connection in a binary search in the list
     of connections. The list (CTab) is sorted by SortConnections */
  pattern.from = from;
  pattern.to = to;
  pattern.delay = delay;
  cptr = &pattern;
  cptr2 = bsearch((void *)(&cptr),
                  (void *)(net->CTab), net->NumSortedCons, 
                  sizeof(Connection *), ConCompare);
  if (cptr2) return *cptr2;

  if (net->NumConnections == net->NumAllocatedCons) {
    net->NumAllocatedCons *= 2;
    REALLOC(net->CTab, net->NumAllocatedCons, Connection *)
  }

  c = CreateConnection(net);

  u = GETPTR(net, to, Unit);
  if (u->type == BIAS || u->type == INPUT || u->type == ENVIRONMENT ||
      u->type == FILEFILTER) {
    ErrorExit(SYSTEM_ERR_EXIT, "Attempt to connect to a clamped unit");
  }

  u = GETPTR(net, from, Unit);
  c->to = to;
  c->from = from;
  c->delay = delay;
  c->w = weight;

  return c;
}


/* Sort the connections in the CTab list */
void 
SortConnections(Net *net) {
  qsort((void *)(net->CTab), net->NumConnections, 
        sizeof(Connection *), ConCompare);
  net->NumSortedCons = net->NumConnections;
}


/*  Sparsely Connect from object g1 to object g2 with probabilty:           */
/*  'Fraction %'  for each possible connection.                             */
void 
SparseConnect(Net *net, long g1, long g2, long delay, float RandBound,
                   float Fraction) {
  float f = Fraction / 100.0;
  long i;
  Group *to, *from;
  float w;
  
  if (GetIdType(net, g2) == GROUP) {
    to = GETPTR(net, g2, Group);
    if (to->type == SIGNAL || to->type == HIDDEN) return;
    for (i = 0; i < to->NumMem; i++) {
      SparseConnect(net, g1, to->MemTab[i], delay, RandBound, Fraction);
    }
  }
  else if (GetIdType(net, g1) == GROUP) {
    from = GETPTR(net, g1, Group);
    if (from->type == RECEPT || from->type == HIDDEN) return;
    for (i = 0; i < from->NumMem; i++) {
      SparseConnect(net, from->MemTab[i], g2, delay, RandBound, Fraction);
    }
  }
  else if (GetIdType(net, g1) == UNIT && GetIdType(net, g2) == UNIT) {
    w = (RectRand() * 2.0 - 1.0) / 2.0 * RandBound;
    if (RectRand() < f) AddConnection(net, g2, g1, delay, w);
  }
}


void 
MetricConnect(Net *net, long g1, long g2,
                   long delay, float RandBound,
                   float fraction, float spread, long cyclic) {
  float f = fraction / 100.0;
  float sigma = 1.0 / spread;
  long i, j, diff;
  Group *to, *from;
  float w, d, p;
  
  if (GetIdType(net, g2) != GROUP || GetIdType(net, g1) != GROUP) {
    ErrorExit(SYSTEM_ERR_EXIT, "Metric connection only possible between two groups");
  }

  from = GETPTR(net, g1, Group);
  to = GETPTR(net, g2, Group);

  for (i = 0; i < to->NumMem; i++) {
    if (GetIdType(net, to->MemTab[i]) != UNIT)
      ErrorExit(SYSTEM_ERR_EXIT, "Metric connection only possible between groups of units");

    for (j = 0; j < from->NumMem; j++) {
      if (GetIdType(net, from->MemTab[j]))
        ErrorExit(SYSTEM_ERR_EXIT, "Metric connection only possible between groups of units");

      /* Compute the distance between the units */
      diff = i * from->NumMem - j * to->NumMem;
      d = fabs((float)diff / (float)from->NumMem);
      if (cyclic) {
        if (d > to->NumMem / 2.0) {
          d = to->NumMem - d;
	}
      }

      /* The connection probability is a function of the distance */
      p = f * exp(-sigma * d);
      w = (RectRand() - 0.5) / 2.0 * RandBound;
      if (RectRand() < p) 
        AddConnection(net, from->MemTab[i], to->MemTab[j], delay, w);
    }
  }
}

/*  Connect from object g1 to object g2 with the constant weight w */  
void 
ConstConnect(Net *net, long g1, long g2, long delay, float w) {
  long i;
  Group *to, *from;
  
  if (GetIdType(net, g2) == GROUP) {
    to = GETPTR(net, g2, Group);
    if (to->type == SIGNAL || to->type == HIDDEN) return;
    for (i = 0; i < to->NumMem; i++) {
      ConstConnect(net, g1, to->MemTab[i], delay, w);
    }
  }
  else if (GetIdType(net, g1) == GROUP) {
    from = GETPTR(net, g1, Group);
    if (from->type == RECEPT || from->type == HIDDEN) return;
    for (i = 0; i < from->NumMem; i++) {
      ConstConnect(net, from->MemTab[i], g2, delay, w);
    }
  }
  else if (GetIdType(net, g1) == UNIT && GetIdType(net, g2) == UNIT) {
    AddConnection(net, g2, g1, delay, w)->plast = 0.0;
  }
}

/*  Connect from object g1 to object g2 */  
void 
Connect(Net *net, long g1, long g2, long delay, float RandBound) {
  long i;
  Group *to, *from;
  float w;
  
  if (GetIdType(net, g2) == GROUP) {
    to = GETPTR(net, g2, Group);
    if (to->type == SIGNAL || to->type == HIDDEN) return;
    for (i = 0; i < to->NumMem; i++) {
      Connect(net, g1, to->MemTab[i], delay, RandBound);
    }
  }
  else if (GetIdType(net, g1) == GROUP) {
    from = GETPTR(net, g1, Group);
    if (from->type == RECEPT || from->type == HIDDEN) return;
    for (i = 0; i < from->NumMem; i++) {
      Connect(net, from->MemTab[i], g2, delay, RandBound);
    }
  }
  else if (GetIdType(net, g1) == UNIT && GetIdType(net, g2) == UNIT) {
    w = (RectRand() * 2.0 - 1.0) / 2.0 * RandBound;
    AddConnection(net, g2, g1, delay, w);
  }
}


long 
LinkUnit(Net *net, long stream, long pos, long unit) {
  Unit *u;

  if (GetIdType(net, unit) != UNIT) return -1;
  if (GetIdType(net, stream) != STREAM) return -1;
  u = GETPTR(net, unit, Unit);
  u->link = stream;
  u->pos = pos;

  return 0;
}


void 
ProtectGroup(Net *net, long group, GroupType Protection) {
  if (GetIdType(net, group) != GROUP)
    ErrorExit(SYSTEM_ERR_EXIT, "Only groups can take a protection.");

  GETPTR(net, group, Group)->type = Protection;
}


/* --- Storage Routines --- */

static void 
SaveUnit(FILE *fp, Unit *u) {
  WriteString(u->Name, fp);

  WriteLong(u->id, fp);
  WriteLong(u->type, fp);
  WriteLong(u->backtype, fp);
  WriteFloat(u->backward_prune_thresh, fp);
  WriteLong(u->link, fp);
  WriteLong(u->pos, fp);
  WriteLong(u->NumParents, fp);
}


static void 
SaveGroup(FILE *fp, Group *g) {
  WriteString(g->Name, fp);
  WriteLong(g->id, fp);
  WriteLong(g->type, fp);
  WriteLong(g->NumParents, fp);
  WriteLong(g->NumMem, fp);
  WriteLongs(g->MemTab, g->NumMem, fp);
}


static void 
SaveStream(FILE *fp, Stream *str) {
  long i;

  WriteString(str->Name, fp);
  WriteLong(str->id, fp);
  WriteString(str->ext, fp);
  WriteString(str->path, fp);
  WriteLong(str->size, fp);
  WriteLong(str->format, fp);
  WriteLong(str->type, fp);

  WriteFloats(str->a, str->size, fp);
  WriteFloats(str->b, str->size, fp);

  WriteLong(str->NumParents, fp);

  for (i = 0; i < str->size; i++) {
    WriteString(str->CompName[i], fp);
  }

  WriteString(str->Filter, fp);
}


static void 
SaveConnection(FILE *fp, Connection *c, long long_format) {
  char x; /* Save some fields as char to save space */
  unsigned short y; /* Save some fields as short to save space */

  WriteFloat(c->w, fp);
  WriteFloat(c->plast, fp);

  if (long_format) {
    WriteLong(c->to, fp);
    WriteLong(c->from, fp);
  }
  else {
    y = (unsigned short)c->to;   WriteShort(y, fp);
    y = (unsigned short)c->from; WriteShort(y, fp);
  }

  x = c->delay; fwrite(&x, sizeof(char), 1, fp);
}


void 
SaveNet(char *fn, Net *net) {
  FILE *fp;
  char *ext;
  char *base;
  char *compression = NULL, command[128];

  /* get the leftmost file extension */
  STRDUP(base, fn);
  ext = base + strlen(base);
  while (*ext != '.' && ext > base) ext--;

  if (*ext == '.' && (strcmp(ext, ".gz") == 0)) {
    compression = "gzip -f";
    *ext = '\0';
  }
  if (*ext == '.' && (strcmp(ext, ".Z") == 0)) {
    compression = "compress -f";
    *ext = '\0';
  }

  nFOPEN(fp, base, "w");
  WriteNet(net, fp);
  nFCLOSE(fp);

  if (compression) {
    strcpy(command, compression);
    strcat(command, " ");
    strcat(command, base);

    system(command);
  }

  FREE(base);
}


void 
WriteNet(Net *net, FILE *fp) {
  long i, magic = RTDNN_MAGIC;

  WriteLong(magic, fp);
  WriteString(net->Name, fp);
  WriteFloat(net->Version, fp);

  WriteLong(net->NumUnits, fp);
  WriteLong(net->NumInput, fp);
  WriteLong(net->NumTanhyp, fp);
  WriteLong(net->NumArctan, fp);
  WriteLong(net->NumLinear, fp);
  WriteLong(net->NumOutput, fp);
  WriteLong(net->NumGroups, fp);
  WriteLong(net->NumConnections, fp);
  WriteLong(net->NumStreams, fp);
  WriteLong(net->MaxDelay, fp);
  WriteLong(net->NumId, fp);

  for (i = 0; i < net->NumUnits; i++) SaveUnit(fp, net->UTab[i]);
  for (i = 0; i < net->NumGroups; i++) SaveGroup(fp, net->GrTab[i]);

  if (net->NumId >= 65536) /* Long format */
    for (i = 0; i < net->NumConnections; i++) 
      SaveConnection(fp, net->CTab[i], 1);
  else 
    for (i = 0; i < net->NumConnections; i++) 
      SaveConnection(fp, net->CTab[i], 0);

  for (i = 0; i < net->NumStreams; i++) SaveStream(fp, net->StrTab[i]);

  SaveLogBook(fp, net->logbook);
}


static Unit *
LoadUnit(Net *net, FILE *fp) {
  Unit *u;

  CALLOC(u, 1, Unit);

  u->Name = ReadString(fp);

  u->id = ReadLong(fp);
  u->type = ReadLong(fp);
  u->backtype = ReadLong(fp);
  u->backward_prune_thresh = ReadFloat(fp);
  u->link = ReadLong(fp);
  u->pos = ReadLong(fp);

  if (net->Version != 0.5) {
    u->NumParents = ReadLong(fp);
  }

  return u;
}


static Group *
LoadGroup(FILE *fp) {
  long n;
  Group *g;

  CALLOC(g, 1, Group);

  g->Name = ReadString(fp);
  g->id = ReadLong(fp);
  g->type = ReadLong(fp);
  g->NumParents = ReadLong(fp);

  g->NumMem = ReadLong(fp);
  if (g->NumMem == 0) n = 1; else n = g->NumMem;
  CALLOC(g->MemTab, n, long)
  ReadLongs(g->MemTab, g->NumMem, fp);

  return g;
}


static Stream *
LoadStream(FILE *fp) {
  Stream *str;
  long i;

  CALLOC(str, 1, Stream);

  str->Name = ReadString(fp);
  str->id = ReadLong(fp);
  str->ext = ReadString(fp);
  str->path = ReadString(fp);
  str->size = ReadLong(fp);
  str->format = ReadLong(fp);
  str->type = ReadLong(fp);

  CALLOC(str->a, str->size, float)
  CALLOC(str->b, str->size, float)

  ReadFloats(str->a, str->size, fp);
  ReadFloats(str->b, str->size, fp);

  str->NumParents = ReadLong(fp);

  CALLOC(str->CompName, str->size, char *)
  for (i = 0; i < str->size; i++) str->CompName[i] = ReadString(fp);

  str->Filter = ReadString(fp);

  return str;
}


static Connection *
LoadConnection(Net *net, FILE *fp, long long_format) {
  Connection *c;
  char x; 
  unsigned short y;

  CALLOC(c, 1, Connection);

  if (net->Version == 0.5) ReadLong(fp); /* read past the id entry */

  c->w = ReadFloat(fp);
  c->plast = ReadFloat(fp);

  if (long_format) {
    c->to = ReadLong(fp);
    c->from = ReadLong(fp);
  }
  else {
    y = ReadShort(fp); c->to =   (long)y;
    y = ReadShort(fp); c->from = (long)y;
  }

  fread(&x, sizeof(char), 1, fp);  c->delay =  x;

  return c;
}


/* Load a RTDNN def file by file name
 * This function automatically extracts .Z and .gz compressed files */ 
Net *
LoadNet(char *fn) {
  FILE *fp;
  Net *net;
  char *ext;
  char command[256];
  char *compression = NULL;

  /* get the leftmost file extension */
  ext = fn + strlen(fn);
  while (*ext != '.' && ext > fn) ext--;

  if (*ext == '.' && (strcmp(ext, ".gz") == 0)) compression = "gunzip -c";
  if (*ext == '.' && (strcmp(ext, ".Z") == 0)) compression = "uncompress -c";

  if (compression) {
    /* open compressed files */

    strcpy(command, compression);
    strcat(command, " ");
    strcat(command, fn);

    fp = popen(command, "r");
    if (!fp) ErrorExit(SYSTEM_ERR_EXIT, "Failed reading network file %s", fn);
  }
  else {
    /* open normal files */
    nFOPEN(fp, fn, "r");
  }

  net = ReadNet(fp);

  if (compression) {
    pclose(fp);
  }
  else {
    nFCLOSE(fp);
  }

  if (!net) ErrorExit(SYSTEM_ERR_EXIT, "Failed reading network file %s", fn);

  return net;
}


/* Load a RTDNN def file from a stream */
Net *
ReadNet(FILE *fp) {
  long i, n, magic;
  Net *net;

  magic =  ReadLong(fp);
  if (magic != RTDNN_MAGIC) {
    EmitWarning("Magic number does not match for network definition ");
    return NULL;
  }
  CALLOC(net, 1, Net)
  net->Name = ReadString(fp);

  net->Version = ReadFloat(fp);

  if (fabs(net->Version - 0.5) < 0.01) {
    /* Read an old version 0.5 network */
    EmitWarning("Old NICO-version (%.2f), converting to %.2f",
		net->Version, RTDNN_VERSION);
    net->Version = 0.5;
  }
  else if (floor(net->Version) - floor(RTDNN_VERSION) != 0) {
    EmitWarning("Can not read this NICO-version (%2.2f)", net->Version);
    return NULL;
  }

  net->NumUnits = ReadLong(fp);
  net->NumInput = ReadLong(fp);
  net->NumTanhyp = ReadLong(fp);
  net->NumArctan = ReadLong(fp);
  net->NumLinear = ReadLong(fp);
  net->NumOutput = ReadLong(fp);
  net->NumGroups = ReadLong(fp);
  net->NumConnections = ReadLong(fp);
  net->NumStreams = ReadLong(fp);
  net->MaxDelay = ReadLong(fp);
  net->NumId = ReadLong(fp);

  net->NumSortedCons = 0;

  if (net->NumId == 0) net->AllocatedId = 1;
  else net->AllocatedId = net->NumId;
  CALLOC(net->IdTab, net->AllocatedId, IdEntry)

  if (net->Version == 0.5) {
    for (i = 0; i < net->NumId; i++) {
      net->IdTab[i].type = DELETED;
    }
  }

  if (net->NumUnits == 0) n = 1; else n = net->NumUnits;
  CALLOC(net->UTab, n, Unit *)
  for (i = 0; i < net->NumUnits; i++) {
    net->UTab[i] = LoadUnit(net, fp);
    net->IdTab[net->UTab[i]->id].ptr  = (void *)net->UTab[i];
    net->IdTab[net->UTab[i]->id].Name = net->UTab[i]->Name;
    net->IdTab[net->UTab[i]->id].type = UNIT;
    if (net->UTab[i]->type == BIAS) net->biasunit = net->UTab[i];
  }

  if (net->NumGroups == 0) n = 1; else n = net->NumGroups;
  CALLOC(net->GrTab, n, Group *)
  for (i = 0; i < net->NumGroups; i++) {
    net->GrTab[i] = LoadGroup(fp);
    net->IdTab[net->GrTab[i]->id].ptr  = (void *)net->GrTab[i];
    net->IdTab[net->GrTab[i]->id].Name = net->GrTab[i]->Name;
    net->IdTab[net->GrTab[i]->id].type = GROUP;
  }
  net->rootgroup = net->GrTab[0];

  if (net->NumConnections == 0) n = 1; else n = net->NumConnections;
  CALLOC(net->CTab, n, Connection *)
  net->NumAllocatedCons = n;

  if (net->NumId >= 65536) { /* Long format */
    for (i = 0; i < net->NumConnections; i++) {
      net->CTab[i] = LoadConnection(net, fp, 1);
      net->CTab[i]->index = i;
    }
  }
  else {
    for (i = 0; i < net->NumConnections; i++) {
      net->CTab[i] = LoadConnection(net, fp, 0);
      net->CTab[i]->index = i;
    }
  }
    
  if (net->Version == 0.5) {
    /* This is not really necessary, but kept to fix old nets 
       that didn't have their connections sorted */ 
    SortConnections(net); 
  }

  if (net->NumStreams == 0) n = 1; else n = net->NumStreams;
  CALLOC(net->StrTab, n, Stream *);
  for (i = 0; i < net->NumStreams; i++) {
    net->StrTab[i] = LoadStream(fp);
    net->IdTab[net->StrTab[i]->id].ptr  = (void *)net->StrTab[i];
    net->IdTab[net->StrTab[i]->id].Name = net->StrTab[i]->Name;
    net->IdTab[net->StrTab[i]->id].type = STREAM;
  }


  if (net->Version == 0.5) {
    /* Fix the numParents entry in units */
    for (i = 0; i < net->NumGroups; i++) {
      Group *g;
      long j;
      
      g = net->GrTab[i];
      for (j = 0; j < g->NumMem; j++) {
	if (GetIdType(net, g->MemTab[j]) == UNIT) {
	  GETPTR(net, g->MemTab[j], Unit)->NumParents++;
	}
      }
    }
  }

  if (net->Version == 0.5) {
    ReHash(net); 
  }

  net->logbook = LoadLogBook(fp);

  net->Version = RTDNN_VERSION;
  return net;
}


/* --- Functions to handle marking of objects --- */

/* unmarks all objects and connections */
void 
UnMarkAll(Net *net, IdentifierType type) {
  long i;

  for (i = 0; i < net->NumId; i++) {
    if (type == UNSPEC || net->IdTab[i].type == type) {
      UnMark(net, i, 0xffffffff);
    }
  }

  if (type == CONNECTION || type == UNSPEC) {
    for (i = 0; i < net->NumConnections; i++) {
      net->CTab[i]->mark = 0;
    }
  }
}


void 
Mark(Net *net, long object, long val) {
  Unit       *u;
  Group      *g;
  Stream     *s;

  switch (GetIdType(net, object)) {
    case CONNECTION: 
      ErrorExit(SYSTEM_ERR_EXIT, "Can't unmark a connection from here");
    break;
    case UNIT: 
      u = GETPTR(net, object, Unit);
      u->mark |= val; 
    break;
    case GROUP: 
      g = GETPTR(net, object, Group);
      g->mark |= val; 
    break;
    case STREAM: 
      s = GETPTR(net, object, Stream);
      s->mark |= val; 
    break;
    default:
    break;
  }
}


void 
UnMark(Net *net, long object, long val) {
  Unit       *u;
  Group      *g;
  Stream     *s;
  long inverse = val ^ 0xffffffff;

  switch (GetIdType(net, object)) {
    case CONNECTION: 
      ErrorExit(SYSTEM_ERR_EXIT, "Can't unmark a connection from here");
    break;
    case UNIT: 
      u = GETPTR(net, object, Unit);
      u->mark &= inverse; 
    break;
    case GROUP: 
      g = GETPTR(net, object, Group);
      g->mark &= inverse; 
    break;
    case STREAM: 
      s = GETPTR(net, object, Stream);
      s->mark &= inverse; 
    break;
    default:
    break;
  }
}


void 
RecursiveMark(Net *net, long object, long val) {
  long i;
  Group *g;

  switch (GetIdType(net, object)) {
    case UNIT: case STREAM: Mark(net, object, val); break;
    case GROUP: 
      g = GETPTR(net, object, Group);
      g->mark |= val; 
      for (i = 0; i < g->NumMem; i++) RecursiveMark(net, g->MemTab[i], val);
    break;
    default:
    break;
  }
}


void 
RecursiveUnMark(Net *net, long object, long val) {
  long i;
  Group *g;

  switch (GetIdType(net, object)) {
    case UNIT: case STREAM: UnMark(net, object, val); break;
    case GROUP: 
      UnMark(net, object, val);
      g = GETPTR(net, object, Group);
      for (i = 0; i < g->NumMem; i++) RecursiveUnMark(net, g->MemTab[i], val);
    break;
    default:
    break;
  }
}


void 
MarkConnections(Net *net, long from, long to, long val) {
  long i;
  Unit *f, *t;
  Connection *c;

  UnMarkAll(net, UNIT);   
  RecursiveMark(net, from, 1);
  RecursiveMark(net, to, 2);
  for (i = 0; i < net->NumConnections; i++) {
    c = net->CTab[i];
    f = GETPTR(net, c->from, Unit);
    t = GETPTR(net, c->to, Unit);
    if ((f->mark & 1) && (t->mark & 2)) {
      c->mark |= val;
    }
  }            
}


void 
UnMarkConnections(Net *net, long from, long to, long val) {
  long i;
  Unit *f, *t;
  long inverse = val ^ 0xffffffff;
  Connection *c;

  UnMarkAll(net, UNIT);   
  RecursiveMark(net, from, 1);
  RecursiveMark(net, to, 2);
  for (i = 0; i < net->NumConnections; i++) {
    c = net->CTab[i];
    f = GETPTR(net, c->from, Unit);
    t = GETPTR(net, c->to, Unit);
    if ((f->mark & 1) && (t->mark & 2)) {
      c->mark &= inverse;
    }
  }            
}


void 
SetAllConnectionMarks(Net *net, long val) {
  long i;

  for (i = 0; i < net->NumConnections; i++) {
    net->CTab[i]->mark = val;
  }
}

/* --------------------------------------------------------------------------*/
/*           END OF FILE :  RTDNN.c                                          */
/* --------------------------------------------------------------------------*/
