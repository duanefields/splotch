/*****************************************************************************
Chat: a chat type interface for Splotch
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


