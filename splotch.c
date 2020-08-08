/*****************************************************************************
Splotch: an eliza type chatter bot
Copyright (C) 1992-2000  Duane K. Fields (duane@deepmagic.com)

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


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define tolow(A) (isupper(A)?(tolower(A)):(A))

#define NAME      "Splotch"    /* name of robot */
#define HISTORY   4            /* number of slots in old key queue */
#define TEMPLSIZ  2000         /* maximum number of templates */
#define DICTFILE  "main.dict"  /* name of dictionary file */
#define DEBUG 0                /* debug flag */
#define VERBOSE 0              /* verbose errors */
#define BLANK 040

struct template {
  long    toffset;           /* start of responses in file */
  char    tplate[400];       /* the template itself */
  int     priority;          /* how important key is 1=worst, 9=most */
  int     talts;             /* number of alternate replies (>0) */
  int     tnext;             /* next reply (1 <= tnext <= talts) */
}         templ[TEMPLSIZ];

char   response[400];        /* response to be returned */
char   my_nick[20];
char   words[400];           /* a template has been matched, this is % */
FILE   *dfile;               /* file pointer to main dictionary file */
int    maxtempl;             /* templ[maxtempl] is last entry */
int    oldkeywd[HISTORY];    /* queue of indices of most recent keys */

char *strcasestr();
char *phrasefind();
char *lower();

/***************************************************************************
  ask(person, question) takes two null terminated strings, one is the 
  question and the other is the person who asked the question.  The function 
  returns the int value which represents the template used (0 or greater).
  if no match was found, a -1 is returned.  ask always sets the global 
  variable "response" to contain an appropriate response, or if no match was
  found, an reply from the last entry in the dictionary. (a default reply)
 ***************************************************************************/

ask(person, question)
     char    person[20];            /* person who asked the question */
     char    question[400];         /* input line from user */

{
  int     i,j;
                                     /* expand also strlowers() */
  expand(question);                  /* swap word according to syn.dict file */

  if (DEBUG)
   fprintf(stderr, "<<expanded>>\n");

  i = trytempl(question);            /* look for a matching template */

  if (DEBUG)
    fprintf(stderr, "<<trytempl done, i=%i>>\n",i);


  if (i >=0)   {                     /* found a match, index=i */
    if (templ[i].priority == 6)  {    /* key words 6 */
      for (j=0; j < HISTORY-1; j++)  /* update history que */
        oldkeywd[j] = oldkeywd[j+1];
      oldkeywd[HISTORY-1]=i;
      }
    usetempl(i);                      /* build response */
    return(i);
  }              
  else {                              /* no match found */

    if ((random() % 100)+1)         /* HISTORY NOT IMPEMENTED! */
       usetempl(maxtempl);          /* use a neutral response */
    else {                            /* use an old key */ 
      i = (random() % HISTORY);
      i = oldkeywd[i];
      if (DEBUG)
	fprintf(stderr, "<<History used, i=%i>>\n",i);


      if (i < 0)                      /* no history yet */
	usetempl(maxtempl);
      else usetempl(i);
    }
  }

return(-1);
}



/***************************************************************************
  init() sets up the template, zeros variables. Must be called by main prg
 ***************************************************************************/

init()
{
  int    i;
  
  /* initialization section, zero array, open files */
  
  for (i=0; i < HISTORY; i++)
    oldkeywd[i] = -1;                /* no previous keywords */

  dfile = fopen (DICTFILE, "r");
  if (dfile == NULL) {
    fprintf(stderr, "ERROR: unable to read %s\n",DICTFILE);
    exit(1);
  }
  
  if (DEBUG)
    fprintf(stderr, "<<using dict file %s>>>\n", DICTFILE);

  srandom(getpid());   /* randomize seed */
  buildtempl();    /* read the templates, make 1 entry per template */

  if (DEBUG)
    fprintf(stderr, "<<templates built, seed chosen>>\n");
}





/**************************************************************************
  buildtempl() reads the MAINDICT file and fills in the template table.
  Each entry in the template table refers to a single template and all of 
  its replies.
 ***************************************************************************/

buildtempl()
{
  char    line[400];
  char    temp[400];
  int     i;
 
  
  i=0;   /* first template starts at zero */
  
  /* loop, one pass per template (including all replies) */
  while (!feof(dfile)) {
    fgets(line, sizeof(line), dfile);
    
    while ((line[0] == '#') || (isspace(line[0])) && (!feof(dfile)))
      fgets(line, sizeof(line), dfile);
    
    if (feof(dfile))
      break;
    
    /* read in template */
    strcpy(templ[i].tplate, line);

    /* read priority */
    fgets(line, sizeof(line), dfile);

    if (atoi(line) == 0)  /* no entry */
      templ[i].priority = 9;                /* default priority */
    else
      templ[i].priority = atoi(line);
    
    /* set number of alternate replies to 0 */
    templ[i].talts = 0;
    
    /* count number or responses, start with asterisk */
    if (line[0] != '*')
      fgets(line, sizeof(line), dfile);

    /* where to find first response withing dictionary */
    templ[i].toffset = ftell(dfile) - strlen(line);

    while ((line[0] == '*') && !feof(dfile)) {
      templ[i].talts++;
      fgets(line, sizeof(line), dfile);
    }
    
    /* pick a random starting point for the responses */
    templ[i].tnext = 1 + (random() % (templ[i].talts));
    if (templ[i].tnext > templ[i].talts)
      templ[i].tnext = 1;

    strcpy(temp, templ[i].tplate);
    temp[strlen(temp)-1]='\0';

    if (VERBOSE)
      fprintf(stderr, "<<Template[%i]=:%s:>>\n",i,temp);
    

    i++;   /* next template */

    if (i >= TEMPLSIZ) {
      fprintf(stderr, "ERROR: template array too small\n");
      exit(1);
    }
    
  }
  
  /* all templates have been read and stored */
  maxtempl = i -1;
}

 


/**************************************************************************
 usetempl() a template has been sucessfully matched.  generate output 
**************************************************************************/
usetempl(i)
     int     i;
{
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

  fseek(dfile, templ[i].toffset, 0);	/* seek the template */
  fgets(text, sizeof(text), dfile);
       
  if (templ[i].tnext > templ[i].talts)
    templ[i].tnext = 1;
  n = templ[i].tnext;
  templ[i].tnext++;
  
  /* skip to the proper alternative */
  for (k = 1; k < n; k++)
    fgets(text, sizeof(text), dfile);
  
  /* make reply using template */
  strcpy(response, text+1);
  if (response[strlen(response)-1] == '\n')
    response[strlen(response)-1]='\0';

  if (words[0] != '\0') {

    p=strstr(words, ",");          /* chop off trailing cluases */
    if (p != NULL)
       p[0]='\0';
    
    p=strstr(words, ". ");
    if (p != NULL)
      p[0]='\0';

    p=strstr(words, "...");
    if (p != NULL)
      p[0]='\0';

    p=strstr(words, "?");
    if (p != NULL)
      p[0]='\0';


    p=strstr(words, ";");                      
    if (p != NULL)
       p[0]='\0';

    p=strstr(words, "!");
    if (p != NULL)
       p[0]='\0';
    
    p=strstr(words, ":");
    if (p != NULL)
      p[0]='\0';

    sprintf(text, " %s", lower(my_nick));
    sprintf(text2,"than %s", lower(my_nick));
    pp=strstr(words, text2);
    p=strstr(words, text);                        
    if ((p != NULL) && (pp == NULL))
       p[0]='\0';

    p=strstr(words, " please");
    if(p != NULL)
      p[0]='\0';


    if (ispunct(words[strlen(words)-1]))
      words[strlen(words)-1]='\0';

    fix();                           /* fix grammer */
    
    swap (response, "%", words);
  }

  swap(response, lower(my_nick), "I");          /* should use NAME */
  swap(response, "%", " ");
  for (i=0; words[i] != '\0'; i++)
    words[i]=words[i] & 0177;

/* fix problems with grammer */

  gswap("i are", "I am");
  gswap("me have", "I have");
  gswap("me don't", "I don't");
  gswap("you am", "you are");
  gswap("me am", "I am");
  gswap("i", "I");
  gswap("did not be", "weren't");

  for (i=0; words[i] != '\0'; i++)
    words[i]=words[i] & 0177;


  /* do file insertians */

  while (strstr(response, "@") != NULL) {
    p1=0;
    p2=0;
    strcpy (filename, "\0");
    strcpy (new, "\0");
    strcpy (add, "\0");
    
    while (response[p1] != '@')
      new[p1]=response[p1++];   

    p1++;
    new[p1-1]='\0';
    while (! (response[p1] == '.') && response[p1] != '\0')
      filename[p2++]=response[p1++];
    filename[p2++]=response[p1++];
    filename[p2++]=response[p1++];
    filename[p2]='\0';
    if (! grline(filename, add)) {              /* problem reading file */
      strcpy (response, "hmmm");                /* error msg printed by */
      return(0);                                /* grline()             */
    }
    
    if (response[p1] != '\0')
      sprintf(text, "%s%s%s", new, add, &response[p1]);
    else sprintf(text, "%s%s",new,add);
    strcpy(response, text); 
  } 
}





 
  
/*****************************/
/* gets random line from file*/
/*****************************/ 
int grline(infile,s1)
char infile[25], s1[256];
  
{
  int i,r,x;
  FILE *fp;
  char fname[25];
  
  sprintf(fname, "words/%s", infile);
  
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
    fixfile(fname);
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
expand (s)
char *s;
{
   char *old;
   char *new;
   char line[255];
   FILE *fp;

   strcat(line, "#");
   strlower(s);
   fp=fopen("syn.dict","r");
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
           
           strlower(line);
           
           new = strtok (line, ":\n");
           old = strtok (NULL, ":\n");
           while (old != NULL)
             {
               swap(s,old,new);        
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
 
swap(s, old, new)
char *s, *old, *new;
{
   char *n;
   char *n0;
   char *i;
   char *s0;
   char *new0;
  
   i=NULL;
   
   n=(char *)malloc(255);   
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

strlower(s)
char *s;
{
   register int i;
   
   for (i=0; s[i]; ++i)
       s[i] = tolower (s[i]);
     
}
 



char *lower(s)
char  *s;
{
  int i;
  char tmp[400];
  
  for (i=0; s[i]; ++i)
    tmp[i] = tolower(s[i]);
  tmp[i]='\0';
  return(tmp);
    
}



/***************************************************************************
fixfile will count the non-blank lines in a file and put the count as the
first line of the file.  Uses a tmp file called "tmp"
***************************************************************************/

fixfile(fname)
char fname[50];

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
  tmpfp=fopen("tmp","w");
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
      sprintf(tline, "%\n",count);
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
  tmpfp=fopen("tmp", "r");
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
char *strcasestr (s1, s2)
char *s1, *s2;
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

trytempl(question)
     char question[];
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
  words[0] = '\0';                    /* the % words */
  firstime=1;
  for (i=0; i <= maxtempl; i++) {     /* loop through all templates */
    done=0;
    strcpy(t, templ[i].tplate);
    firstime=1;
    while (done == 0) {
      if (firstime) {
	key = strtok(t, ":\n");
	firstime=0;
      }
      else
	 key=strtok(key[strlen(key)], ":\n");
      if (key == NULL) {
	done=1;
	break;
      }
      switch (key[0]) {
      case '!': if (phrasefind(question, key+1) != NULL) {
	           found=0;
		   done=1;
		 }
	        break;
      case '&': if (phrasefind(question, key+1) == NULL) {
	           found=0;
		   done=1;
		 }
	        break;
      case '+': if (phrasefind(question, my_nick) == NULL) {  /* name */
                   found=0;
                   done=1;
                 }
                break;
      default:  if (!strstr(key, " %")) {    /* regular template, no % */
	           if (phrasefind(question, key))
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
		    p1=phrasefind(question, key1);
		    if (p1 != NULL)
		      if (!ispunct(p1[strlen(p1)-1])) {
			p2=phrasefind(p1+1, key2);
			if (p1 != NULL && p2 != NULL) {     /* both keys found */
			  found=1;
			  strcpy(words, p1+strlen(key1)+1);
			  words[p2-p1-strlen(key1)-2]='\0'; /* -2 for spaces */
			}
		      }
		    else ;
		    
		  }
		  else {		    /* xxxx % */
		    p1=phrasefind(question, key1);
		    if ((p1 != NULL)&& p1[strlen(key1)] != '\0') 
		      {
		      /* if (!ispunct(p1[strlen(p1)-1])) { */
			found=1;
			strcpy(words, p1+strlen(key1)+1);
		      } 
		  }
		}
      }
    }
   
    if (found && done)
      {
        if (templ[i].priority >score) {
  	  winner=i; 
	  score=templ[i].priority;
	  strcpy(winwords, words);
/*	  printf ("the new template is %s\n",templ[i].tplate); */
	}
	  
	if (templ[i].priority == 9) {
	  strcpy(words, winwords);
	  return(winner);
	}
      }
    found=0;
    done=0;
  }
  strcpy(words, winwords);
  return(winner);
}








/**************************************************************************
 phrasefind()
**************************************************************************/
char *phrasefind(string, searchstring)
char *string;
char *searchstring;
{

        while (!isalnum(*string) && (*string != '\0')) string++;
        while (isalnum(*string))
        {
                if (!strncasecmp(string, searchstring, strlen(searchstring)) &&
                        !isalnum(*(string + strlen(searchstring))))
                        return string;
                while (isalnum(*string)) string++;
                while ((*string != '\0') && !isalnum(*string)) string++;
        }
        return NULL;
}

/**
 * Apparently this is in string.h now (dkf)
 **/
int strncasecmp (a, b, n)
char *a;
char *b;
int  *n;
{

        for (; (*a != '\0') && (*b != '\0') &&
                (tolow (*a) == tolow (*b)) && (n  > 0); a++, b++, n--);
        if (n == 0)
                return 0;
        return (tolow (*a) - tolow (*b));
}
**/





fix()
{
  int i;
  
  gswap("that you", "that I");
  gswap("my", "your");
  gswap("you", "me");
  gswap("your", "my");
  gswap("me", "you");
  gswap("mine", "yours");
  gswap("am", "are");
  gswap("yours", "mine");
  gswap("yourself", "myself");
  gswap("myself", "yourself");
  gswap("are", "am");
  gswap("i", "you");

  for (i=0; words[i] != '\0'; i++)
    words[i] = (words[i] & 0177);              /* readjust parity */
 
}



/*
scan the array "words" for the string OLD.  if it occurs,
replace it by NEW.  also set the parity bits on in the
replacement text to  mark them as already modified
*/
gswap(old, new)
	char    old[], new[];
{     
	int     i, nlen, olen, flag, base, delim;
	olen = 0;
	while (old[olen] != 0)
		olen++;
	nlen = 0;
	while (new[nlen] != 0)
		nlen++;

	for (base = 0; words[base] != 0; base++) {
		flag = 1;
		for (i = 0; i < olen; i++)
			if (old[i] != words[base + i]) {
				flag = 0;
				break;
			}
		delim = words[base + olen];
		if (flag == 1 && (base == 0 || words[base - 1] == BLANK)
		    && (delim == BLANK || delim == '\n' || delim == 0)) {
			shift(base, nlen - olen);
			for (i = 0; i < nlen; i++)
				words[base + i] = new[i] + 128;
		}
	}
}



shift(base, delta)
	int     base, delta;
{
	int     i, k;
	if (delta == 0)
		return;
	if (delta > 0) {
		k = base;
		while (words[k] != 0)
			k++;
		for (i = k; i >= base; i--)
			words[i + delta] = words[i];
	} else	/* delta <0 */
		for (i = 0; i == 0 || words[base + i - 1] != 0; i++)
			words[base + i] = words[base + i - delta];
}


