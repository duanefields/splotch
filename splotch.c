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

#include "splotch.h"
#include <unistd.h>
/***************************************************************************
  splotch_ask(person, question) takes two null terminated strings, one is the 
  question and the other is the person who asked the question.  The function 
  returns the int value which represents the template used (0 or greater).
  if no match was found, a -1 is returned.  ask always sets the global 
  variable "response" to contain an appropriate response, or if no match was
  found, an reply from the last entry in the dictionary. (a default reply)
 ***************************************************************************/

int splotch_ask(splotch_t *me, char *person, char *question)
     /* person who asked the question */
     /* input line from user */
{
  int     i,j;
                                     /* expand also strlowers() */
  _sp_expand(question);              /* swap word according to syn.dict file */

  if (DEBUG)
   fprintf(stderr, "<<expanded>>\n");

  i = _sp_trytempl(me, question);        /* look for a matching template */

  if (DEBUG)    fprintf(stderr, "<<trytempl done, i=%i>>\n",i);


  if (i >=0)   {                     /* found a match, index=i */
    if (me->templ[i].priority == 6)  {    /* key words 6 */
      for (j=0; j < SPLOTCH_HISTORY-1; j++)  /* update history que */
        me->oldkeywd[j] = me->oldkeywd[j+1];
      me->oldkeywd[SPLOTCH_HISTORY-1]=i;
      }
    _sp_usetempl(me,i);                   /* build response */
    return(i);
  }              
  else {                              /* no match found */

    if ((random() % 100)+1)           /* SPLOTCH_HISTORY NOT IMPEMENTED! */
       _sp_usetempl(me, me->maxtempl);    /* use a neutral response */
    else {                            /* use an old key */ 
      i = (random() % SPLOTCH_HISTORY);
      i = me->oldkeywd[i];
      if (DEBUG)
	fprintf(stderr, "<<History used, i=%i>>\n",i);


      if (i < 0)                      /* no history yet */
	_sp_usetempl(me, me->maxtempl);
      else _sp_usetempl(me, i);
    }
  }

return(-1);
}



/***************************************************************************
  splotch_init() sets up the template, zeros variables. 
  Must be called by main prg
  Returns the new splotch objest. (To destroy, simply free the object)
 ***************************************************************************/

splotch_t *splotch_init(void)
{
  int    i;
  splotch_t *me;

  me = calloc(1,sizeof(splotch_t));
  if (me==NULL) fprintf(stderr,"Failed to malloc!");

  /* initialization section, zero array, open files */
  
  for (i=0; i < SPLOTCH_HISTORY; i++)
    me->oldkeywd[i] = -1;                /* no previous keywords */

  me->dfile = fopen (SPLOTCH_PATH SPLOTCH_DICTFILE, "r");
  if (me->dfile == NULL) {
    fprintf(stderr, "ERROR: unable to read %s\n",SPLOTCH_DICTFILE);
    exit(1);
  }
  
  if (DEBUG)
    fprintf(stderr, "<<using dict file %s>>>\n", SPLOTCH_DICTFILE);

  srandom(getpid());   /* randomize seed */
  _sp_buildtempl(me);    /* read the templates, make 1 entry per template */

  if (DEBUG)
    fprintf(stderr, "<<templates built, seed chosen>>\n");

  strcpy(me->my_nick, "splotch");

  return me;
}





/**************************************************************************
  buildtempl() reads the MAINDICT file and fills in the template table.
  Each entry in the template table refers to a single template and all of 
  its replies.
 ***************************************************************************/

void _sp_buildtempl(splotch_t *me)
{
  char    line[400];
  char    temp[400];
  int     i;
 
  
  i=0;   /* first template starts at zero */
  
  /* loop, one pass per template (including all replies) */
  while (!feof(me->dfile)) {
    fgets(line, sizeof(line), me->dfile);
    
    while ((line[0] == '#') || ((isspace(line[0])) && (!feof(me->dfile))))
      fgets(line, sizeof(line), me->dfile);
    
    if (feof(me->dfile))
      break;
    
    /* read in template */
    strcpy(me->templ[i].tplate, line);

    /* read priority */
    fgets(line, sizeof(line), me->dfile);

    if (atoi(line) == 0)  /* no entry */
      me->templ[i].priority = 9;                /* default priority */
    else
      me->templ[i].priority = atoi(line);
    
    /* set number of alternate replies to 0 */
    me->templ[i].talts = 0;
    
    /* count number or responses, start with asterisk */
    if (line[0] != '*')
      fgets(line, sizeof(line), me->dfile);

    /* where to find first response withing dictionary */
    me->templ[i].toffset = ftell(me->dfile) - strlen(line);

    while ((line[0] == '*') && !feof(me->dfile)) {
      me->templ[i].talts++;
      fgets(line, sizeof(line), me->dfile);
    }
    
    /* pick a random starting point for the responses */
    me->templ[i].tnext = 1 + (random() % (me->templ[i].talts));
    if (me->templ[i].tnext > me->templ[i].talts)
     me-> templ[i].tnext = 1;

    strcpy(temp, me->templ[i].tplate);
    temp[strlen(temp)-1]='\0';

    if (VERBOSE)
      fprintf(stderr, "<<Template[%i]=:%s:>>\n",i,temp);
    

    i++;   /* next template */

    if (i >= SPLOTCH_TEMPLSIZ) {
      fprintf(stderr, "ERROR: template array too small\n");
      exit(1);
    }
    
  }
  
  /* all templates have been read and stored */
  me->maxtempl = i -1;
}

/**************************************************************************
 usetempl() a template has been sucessfully matched.  generate output 
**************************************************************************/
int _sp_usetempl(splotch_t *me, int i)
{

  char    buf[400];
  int     k, n, x,p1,p2;
  char    text[400];
  char    text2[400];
  char    add[400];
  char    filename[400];
  char    new[400];
  char    *p;
  char    *pp;

  if (DEBUG)
    fprintf(stderr, "<<entering usetempl>>\n");

  fseek(me->dfile, me->templ[i].toffset, 0);	/* seek the template */
  fgets(text, sizeof(text), me->dfile);
       
  if (me->templ[i].tnext > me->templ[i].talts)
    me->templ[i].tnext = 1;
  n = me->templ[i].tnext;
  me->templ[i].tnext++;

  /* skip to the proper alternative */
  for (k = 1; k < n; k++) {
    fgets(text, sizeof(text), me->dfile);
  }

  /* make reply using template */
  strcpy(me->response, text+1);
  if (me->response[strlen(me->response)-1] == '\n')
    me->response[strlen(me->response)-1]='\0';

  if (me->words[0] != '\0') {

    p=strstr(me->words, ",");          /* chop off trailing cluases */
    if (p != NULL)
       p[0]='\0';
    
    p=strstr(me->words, ". ");
    if (p != NULL)
      p[0]='\0';

    p=strstr(me->words, "...");
    if (p != NULL)
      p[0]='\0';

    p=strstr(me->words, "?");
    if (p != NULL)
      p[0]='\0';


    p=strstr(me->words, ";");                      
    if (p != NULL)
       p[0]='\0';

    p=strstr(me->words, "!");
    if (p != NULL)
       p[0]='\0';
    
    p=strstr(me->words, ":");
    if (p != NULL)
      p[0]='\0';

    sprintf(text, " %s", lower(me->my_nick,buf));
    sprintf(text2,"than %s", lower(me->my_nick,buf));
    pp=strstr(me->words, text2);
    p=strstr(me->words, text);                        
    if ((p != NULL) && (pp == NULL))
       p[0]='\0';

    p=strstr(me->words, " please");
    if(p != NULL)
      p[0]='\0';


    if (ispunct(me->words[strlen(me->words)-1]))
      me->words[strlen(me->words)-1]='\0';

    _sp_fix(me);                           /* fix grammer */
    
    _sp_swap (me->response, "%", me->words);
  }

  _sp_swap(me->response, lower(me->my_nick,buf), "I");         
  _sp_swap(me->response, "%", " ");
  for (i=0; me->words[i] != '\0'; i++)
    me->words[i]=me->words[i] & 0177;

/* fix problems with grammer */

  _sp_gswap(me, "i are", "I am");
  _sp_gswap(me, "me have", "I have");
  _sp_gswap(me, "me don't", "I don't");
  _sp_gswap(me, "you am", "you are");
  _sp_gswap(me, "me am", "I am");
  _sp_gswap(me, "i", "I");
  _sp_gswap(me, "did not be", "weren't");

  for (i=0; me->words[i] != '\0'; i++)
    me->words[i]=me->words[i] & 0177;


  /* do file insertians */

  while (strstr(me->response, "@") != NULL) {
    p1=0;
    p2=0;
    strcpy (filename, "\0");
    strcpy (new, "\0");
    strcpy (add, "\0");
    
    while (me->response[p1] != '@')
      new[p1]=me->response[p1++];   

    p1++;
    new[p1-1]='\0';
    while (! (me->response[p1] == '.') && me->response[p1] != '\0')
      filename[p2++]=me->response[p1++];
    filename[p2++]=me->response[p1++];
    filename[p2++]=me->response[p1++];
    filename[p2]='\0';
    if (! _sp_grline(filename, add)) {              /* problem reading file */
      strcpy (me->response, "hmmm");                /* error msg printed by */
      return(0);                                /* grline()             */
    }
    
    if (me->response[p1] != '\0')
      sprintf(text, "%s%s%s", new, add, &(me->response[p1]));
    else sprintf(text, "%s%s",new,add);
    strcpy(me->response, text); 
  } 
}
  
/*****************************/
/* gets random line from file*/
/*****************************/ 
int _sp_grline(char* infile, char* s1)
{
  int i,r,x;
  FILE *fp;
  char fname[250];

  if (DEBUG) fprintf(stderr,"Entering grline! \n");
  sprintf(fname, SPLOTCH_PATH "words/%s", infile);
  
  if ((fp=fopen(fname, "r")) == NULL)
    {
       fprintf(stderr, "\n ERROR! could not open :%s: \n",infile);
       return(0);
    }

  fgets (s1, 256, fp);
  while ((s1[0] == '#') || (isspace(s1[0])))
    fgets(s1, 256, fp);

  if (atoi(s1)==0)
  {
    fclose(fp);
    _sp_fixfile(fname);
    fp=fopen(fname,"r");
    fgets(s1, 256, fp);
    while ((s1[0] == '#') || (isspace(s1[0])))
      fgets(s1, 256, fp);
  }

  x=atoi(s1);
  r=(random() % x)+1;
  for (i=1; i<=r && (! feof(fp)); i++)
      fgets(s1, 256, fp);
  s1[strlen(s1)-1]='\0';
  fclose(fp);
  return(1);
  
}
 
/***************************************************************************
   expand() takes a string pointer.  Using they syn.dict file (format is
   given in the syn.dict file itself) it expands synonyms.
****************************************************************************/ 
int _sp_expand (char *s)
{
   char *old;
   char *new;
   char line[255];
   FILE *fp;

   strcat(line, "#");
   _sp_strlower(s);
   fp=fopen(SPLOTCH_PATH SPLOTCH_SYNFILE,"r");
   if (fp == NULL)
     {
        fprintf(stderr, "ERROR: Could not open the file syn.dict\n");
        return(1);
     }
   else
     {
       while(!feof(fp))
         {
           fgets(line,255,fp);
           while ((line[0] == '#') || (isspace(line[0])))
             fgets(line,255,fp);
           
           _sp_strlower(line);
           
           new = strtok (line, ":\n");
           old = strtok (NULL, ":\n");
           while (old != NULL)
             {
               _sp_swap(s,old,new);        
               old = strtok (NULL, ":\n");
             }
         }
       fclose(fp);
     }
}              

/*********************************************************************
   swap() takes a string pointer and a two words.  All occurances of 
   the first word are replaced by the second word.
 *********************************************************************/
 
void _sp_swap(char *s, char *old, char *new)
{
   char *n;
   static char m[255];
   char *n0;
   char *i;
   char *s0;
   char *new0;


  
   i=NULL;
   
   n=m;   
   n0=n;
   s0=s;
   new0=new;
   
   while ((i=strstr(s,old)) != NULL)
         {
            while (s != i)
            {
               *n=*s;
               s++;
               n++;
            }
            
            if ( (isspace(i[strlen(old)]) || ispunct(i[strlen(old)]) ||
                  i[strlen(old)] == '\0')  && 
               (s==s0 || isspace(s[-1]) || ispunct(s[-1])) )
               
               {
                  new = new0;
                  while (*new)
                        {
                           *n=*new;
                           new++;
                           n++;
                        }
                  s += strlen(old);       
               }      
            else
               {
                  *n=*s;
                  s++;
                  n++;
               }   
         } 
         

   strcpy(n,s);
   strcpy(s0,n0);
}





/*********************************************************************
   strlower() takes a string pointer and makes that string lowercase
 *********************************************************************/

void _sp_strlower(char *s)
{
   register int i;
   
   for (i=0; s[i]; ++i)
       s[i] = tolower (s[i]);
     
}
 



char *lower(char *s, char *tmp)
{
  int i;
  
  for (i=0; s[i]; ++i)
    tmp[i] = tolower(s[i]);
  tmp[i]='\0';
  return(tmp);
}



/***************************************************************************
fixfile will count the non-blank lines in a file and put the count as the
first line of the file.  Uses a tmp file called "tmp"
***************************************************************************/

int _sp_fixfile(char *fname)
{
  int count;
  FILE *tmpfp;
  FILE *fp;
  char line[255];
  char tline[255];

  count=0;

  fp=fopen(fname, "r");
  if (fp  == NULL)
    {
      fprintf(stderr, "ERROR: Could not open %s file\n",fname);
      return(1);
    }
  fgets(line,255,fp);
  while (!feof(fp))
    {
      if (!((line[0] == '#') || (isspace(line[0]))))
	count++;
      fgets(line,255,fp);
    }
  rewind(fp);
  tmpfp=fopen(SPLOTCH_TEMPFILE,"w");
  if (tmpfp == NULL)
    {
      fprintf(stderr, "ERROR: Could not create tmp file\n");
      return(1);
    }
  fgets(line, 255, fp);
  
  while ((line[0] == '#') || (isspace(line[0])))
    {
      fputs(line,tmpfp);
      fgets(line,255,fp);
    }
  if (atoi(line)==0)
    {
      count=count-1;
      sprintf(tline, "%i\n",count);
      fputs(tline, tmpfp);
    }
  else
    {
      sprintf(tline, "%i\n",count);
      fputs(tline, tmpfp);
      fputs(line, tmpfp);
    }
  fgets(line,255,fp);
  while (!feof(fp))
    {
      fputs(line,tmpfp);
      fgets(line,255,fp);
    }
  fclose(tmpfp);
  fclose(fp);
  
  fp=fopen(fname, "w");
  tmpfp=fopen(SPLOTCH_TEMPFILE, "r");
  fgets(line,255,tmpfp);
  while (!feof(tmpfp))
    {
      fputs(line,fp);
      fgets(line,255,tmpfp);
    }
  fclose(tmpfp);
  fclose(fp);
}

/*********************************
strcasestr()
**********************************/
char *_sp_strcasestr (char *s1, char *s2)
{

        char n1[256], n2[256];
        int j;

        for (j=0;s1[j] != '\0';j++)
                n1[j] = toupper (s1[j]);
        n1[j] = '\0';
        for (j=0;s2[j] != '\0';j++)
                n2[j] = toupper (s2[j]);
        n2[j] = '\0';
       
        return (strstr (n1, n2));
}

/***************************************************************************
 trytempl() will try to match the current line to a template.  In turn, try
 all templates to see if if they find a match.
***************************************************************************/

int _sp_trytempl(splotch_t *me, char *question)
{
  int     i,tmp;
  char    t[400];
  int     found,done;
  char    *key;
  char    key1[400];                 /* first half of a % template */
  char    key2[400];                 /* second half of a % template */
  char    winwords[400];
  int     firstime;
  int     p,j;
  char    *p1,*p2;
  int     score;                     /* current highest priority */
  int     winner;                    /* current high scorer */
  
  winner=-1;
  found=0;
  done=0;
  score=0;
  me->words[0] = '\0';                    /* the % words */
  firstime=1;
  for (i=0; i <= me->maxtempl; i++) {     /* loop through all templates */
    /* for (i=180; i <= 182; i++) {     loop through all templates */
    done=0;
    strcpy(t, me->templ[i].tplate);
    firstime=1;

    if (DEBUG && VERBOSE)
      fprintf(stderr, "Checking %s Against Template %d : %s \n",question,i,t); 
    while (done == 0) {
      if (firstime) {
	key = strtok(t, ":\n");
	firstime=0;
      } 
      /*     else key=strtok(key[strlen(key)], ":\n"); */
      else key=strtok(NULL, ":\n");
      
      if (key == NULL) {
	done=1;
	break;
      }
      switch (key[0]) {
      case '!': if (_sp_phrasefind(question, key+1) != NULL) {
	found=0;
	done=1;
      }
      break;
      case '&': if (_sp_phrasefind(question, key+1) == NULL) {
	found=0;
	done=1;
      }
      break;
      case '+': if (_sp_phrasefind(question, me->my_nick) == NULL) {  /* name */
	found=0;
	done=1;
      }
      break;
      default:  if (!strstr(key, " %")) {    /* regular template, no % */
	if (_sp_phrasefind(question, key))
	  found=1;
	break;
      }
      else {     		  /* is a % in template */

	key1[0]=key[0];
	p=0;
	j=0;
	while ((key1[p++]=key[j++])!='\%');    
	key1[p-2]='\0';
	
	if ((key[p] != '\0') && (key[p] != '\n') )  { /* xxx % xxx */
	  strcpy(key2, &key[p+1]);
	  j=0;
	  p1=_sp_phrasefind(question, key1);
	  if (p1 != NULL) {
	    if (!ispunct(p1[strlen(p1)-1])) {
	      p2=_sp_phrasefind(p1+1, key2);
	      if (p1 != NULL && p2 != NULL) {     /* both keys found */
		found=1;
		strcpy(me->words, p1+strlen(key1)+1);
		me->words[p2-p1-strlen(key1)-2]='\0'; /* -2 for spaces */
	      }
	    }
	  } 
	}
	else {		    /* xxxx % */
	  p1=_sp_phrasefind(question, key1);
	  if ((p1 != NULL)&& p1[strlen(key1)] != '\0') 
	    {
	      /* if (!ispunct(p1[strlen(p1)-1])) { */
	      found=1;
	      strcpy(me->words, p1+strlen(key1)+1);
	    } 
	}
      }
      }
    }
    
    /*    if (found && done) */
    if (found)
      {
	if (DEBUG) {
	  fprintf (stderr,"priority is %d\n",me->templ[i].priority);
	  fprintf (stderr,"score is %d\n",score);
	}
        if (me->templ[i].priority >score) {
  	  winner=i; 
	  score=me->templ[i].priority;
	  strcpy(winwords, me->words);
	  if (DEBUG) 
	    fprintf (stderr,"the new template is %s\n",me->templ[i].tplate);
	}
	
	if (me->templ[i].priority == 9) {
	  strcpy(me->words, winwords);
	  return(winner);
	}
      }
    found=0;
    done=0;
  }
  strcpy(me->words, winwords);
  return(winner); 
}

/**************************************************************************
 phrasefind()
**************************************************************************/
char *_sp_phrasefind(char *string, char *searchstring)
{

        while (!isalnum(*string) && (*string != '\0')) string++;
        while (isalnum(*string))
        {
                if (!strncasecmp(string, searchstring, strlen(searchstring)) &&
		    !isalnum(*(string + strlen(searchstring)))) {
		  /* fprintf(stderr, "Matched %s",string); */
		  return string; }
                while (isalnum(*string)) string++;
                while ((*string != '\0') && !isalnum(*string)) string++;
        }
        return NULL;
}




void _sp_fix(splotch_t *me)
{
  int i;

  _sp_gswap(me, "that you", "that I");
  _sp_gswap(me, "my", "your");
  _sp_gswap(me, "you", "me");
  _sp_gswap(me, "your", "my");
  _sp_gswap(me, "me", "you");
  _sp_gswap(me, "mine", "yours");
  _sp_gswap(me, "am", "are");
  _sp_gswap(me, "yours", "mine");
  _sp_gswap(me, "yourself", "myself");
  _sp_gswap(me, "myself", "yourself");
  _sp_gswap(me, "are", "am");
  _sp_gswap(me, "i", "you");

  for (i=0; me->words[i] != '\0'; i++)
    me->words[i] = (me->words[i] & 0177);         /* readjust parity */
 
}



/*
scan the array "words" for the string OLD.  if it occurs,
replace it by NEW.  also set the parity bits on in the
replacement text to  mark them as already modified
*/
void _sp_gswap(splotch_t *me, char *old, char *new)
{     
	int     i, nlen, olen, flag, base, delim;
	olen = 0;
	while (old[olen] != 0)
		olen++;
	nlen = 0;
	while (new[nlen] != 0)
		nlen++;

	for (base = 0; me->words[base] != 0; base++) {
		flag = 1;
		for (i = 0; i < olen; i++)
			if (old[i] != me->words[base + i]) {
				flag = 0;
				break;
			}
		delim = me->words[base + olen];
		if (flag == 1 && (base == 0 || me->words[base - 1] == BLANK)
		    && (delim == BLANK || delim == '\n' || delim == 0)) {
			_sp_shift(me, base, nlen - olen);
			for (i = 0; i < nlen; i++)
				me->words[base + i] = new[i] + 128;
		}
	}
}



void _sp_shift(splotch_t *me, int base, int delta)
{
	int     i, k;
	if (delta == 0)
		return;
	if (delta > 0) {
		k = base;
		while (me->words[k] != 0)
			k++;
		for (i = k; i >= base; i--)
			me->words[i + delta] = me->words[i];
	} else	/* delta <0 */
		for (i = 0; i == 0 || me->words[base + i - 1] != 0; i++)
			me->words[base + i] = me->words[base + i - delta];
}


