/*****************************************************************************
Splotch: an eliza type chatter bot
Copyright (C) 1992-2000  Duane K. Fields (duane@deepmagic.com)
changes Copyright (C) 2001 Mark Rages (markrages@yahoo.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*****************************************************************************/

#ifndef SPLOTCH_H
#define SPLOTCH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Debugging flags */
#define DEBUG   0
#define VERBOSE 0

/* Configuration */
#define SPLOTCH_PATH     "./"
#define SPLOTCH_DICTFILE "main.dict"
#define SPLOTCH_SYNFILE  "syn.dict"
#define SPLOTCH_TEMPFILE "/tmp/splotch.tmp"

/* Sizes */
#define SPLOTCH_HISTORY  10
#define SPLOTCH_TEMPLSIZ 500

/* Character constants */
#define BLANK ' '

/* Template entry */
typedef struct {
  char  tplate[400];   /* the key template string */
  int   priority;      /* priority 1-9, 9=highest */
  int   talts;         /* number of alternate replies */
  long  toffset;       /* file offset of first reply */
  int   tnext;         /* next reply to use (cycles through talts) */
} templ_t;

/* Main splotch object */
typedef struct {
  FILE     *dfile;                    /* open handle to main.dict */
  templ_t   templ[SPLOTCH_TEMPLSIZ]; /* all loaded templates */
  int       maxtempl;                 /* index of last template */
  int       oldkeywd[SPLOTCH_HISTORY];/* history of matched keywords */
  char      my_nick[64];              /* bot's name */
  char      response[400];            /* output: last generated response */
  char      words[400];               /* scratch: % wildcard match */
} splotch_t;

/* Public API */
splotch_t *splotch_init(void);
int        splotch_ask(splotch_t *me, char *person, char *question);

/* Internal functions */
void  _sp_buildtempl(splotch_t *me);
int   _sp_usetempl(splotch_t *me, int i);
int   _sp_trytempl(splotch_t *me, char *question);
int   _sp_grline(char *infile, char *s1);
int   _sp_expand(char *s);
int   _sp_fixfile(char *fname);
void  _sp_fix(splotch_t *me);
void  _sp_swap(char *s, char *old, char *new);
void  _sp_gswap(splotch_t *me, char *old, char *new);
void  _sp_shift(splotch_t *me, int base, int delta);
void  _sp_strlower(char *s);
char *_sp_strcasestr(char *s1, char *s2);
char *_sp_phrasefind(char *string, char *searchstring);
char *lower(char *s, char *tmp);

#endif /* SPLOTCH_H */
