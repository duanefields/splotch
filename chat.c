/**********************************************************
 chat 1.0, a simple client for the splotch routines

 7/17/1992 Duane Fields (dkf8117@tamsun.tamu.edu)
***********************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "splotch.c"

main() 
{
  char   question[400];
  char   tmp[400];
    
  strcpy(my_nick, NAME);
  init();
  fprintf(stdout, "Hello! Let's chat ('exit' to quit)\n\n");
  fflush(stdout);
  strcpy(question, "foo");
  while (strncmp(question, "exit", 4)) {
    printf("> ");
    gets(question);
    if (strncmp(question,"exit", 4))
	{
	  sprintf(tmp, "%s: %s",NAME, question);
	  strcpy(question, tmp);
	
	  printf("\n");
	  if (question[0] != '\0') {
	    ask("Joe", question);
	    fprintf(stdout, "%s\n", response);  
	    fflush(stdout);
	  }
	}
    else fflush(stdout);
  }
}


