Splotch, by Duane Fields, dkf8117@tamsun.tamu.edu
-------------------------------------------------

Release: Version 1.0 Beta

7/17/1992


INTRO
~~~~~
This is a beta/pre-release of "splotch" an interactive conversing robot.
It is mainly intended for the C programmer who has an interest in programs
like Racter and Eliza.  This program contains a function that can be
easily used by other packages.  Most of the code is mine, most of the
ideas are mine, but I have borrowed some ideas from various Eliza type
programs, and expanded on them.  The source code is VERY VERY UGLY.  I am
not a professional programmer by any stretch of the word.  It compiles
fine on a sun4, other than that, we'll have to see.

The name "splotch" came from its original conception as a "pet" to hang
around on IRC (The Internet Relay Chats).  I though of calling him "spot"
but then decided he wasn't REALLY a pet, more ill-defined.  Therefore he
was an ill-defined spot, in other words, a splotch.  Silly I know, sue me.

The key to the program is one subroutine that when giving a question
string will return a hopefully appropriate response.  This makes it easy
to port the robot to any text-based application.  I have included a simple
chat program that utilizes this function.

Splotch is basically a key-word/word-phrase matching program.  It uses two
main dictionary's "main.dict" and "syn.dict" and a directory of wordlists.
The main.dict file contains all the key templates and their responses


FEATURES
~~~~~~~~
My splotch program has several interesting features that make it an
interesting start (IMHO).  

- Easy to expand.  Dictionary is simplistic enough that adding more words
  that he can recognize is really easy, while logical operators and the
  ability to assign priority to different phrases makes it powerful.

- Synonym file lets slang, misspellings, abbreviations, etc. all match up
  with entries in the dictionary file

- Is able to extract meaningful parts of the phrases it encounters and use
  them back in its response. Example, to "I like cheese" it could replies
  "Why do you like cheese? I hate cheese."

- Can vary its responses with random inclusions from word files

- Does not repeat a reply template until it has used up all the old replies


FILES
~~~~~
Makefile   the Makefile
chat.c     the chat client
main.dict  the main dictionary
splotch.c  the splotch routines
syn.dict   the synonym dictionary
words/*    the wordfiles


MAIN.DICT
~~~~~~~~~
main.dict has the following format:

- each entry separated by one blank line
- each entry consists of a key template on line 1 (up to 400 characters)
                      a priority from 1-9 (9=highest) on line 2
                      responses, one to a line, starting with "*"'s

the key templates consist of words or word phrases separated by ":"'s
special character flags can prefix any word group. These codes are:

'&' logical AND (The key will be true ONLY if the marked phrase is present)
'!' logical NOT (The key will be true ONLY if the marked phrase is NOT there)
'+' NAME flag (The key will be true ONLY if the robots name is mentioned)
No special character is treated as a logical OR

example: bye:goodbye:see you later:!say:!tell:+

would match for "bye" "goodbye" or "see you later" provided that the name
of the robot (maintained within the program) was also included (the + code)
The template would NOT match for "tell joe goodbye, robot" because tell is
a prefixed with a '!'.

Templates can also contain a special "wild card" character, the '%'.
a '%' stand for any group of words up to a punctuation mark, and the words
that the '%' stands for can then be used in your responses.
This is best illustrated by an example:

example: Do you have any % for me?

would match "Do you have any cheese for me?" or "Do you have any old fruit
for me?"  And then you could have responses like "No, I have no % for you"
which would return "No, I have no cheese for you"

The program chops off anything after punctuation in the '%' match:

example: "Do you have any %"
         "Do you have any cheese, splotch"

would chop off the ", splotch" and respond with "Yes, I have some cheese"
or whatever.

response may also contain references to more data files with the '@' char.
the '@' must prefaces a filename that ends in '.w' and names a file of
responses or words located in the "words" subdirectory of the program from
the directory in which it was run. This is a good way to build even more
randomness into the responses. For instance:

example: *My favorite color is @color.w. What's yours?

would pull a random line out of the file "words/color.w" and insert it in
the response string.  The format for the word file is just one response per
line (no '*' is needed).  The first line of a word file is the number
of entries contained in the word file.  (This makes for fast lookups) If
the number is listed as 0, the program will count all the responses and
insert a correct count.  Note that responses in word files can contain
recursive references to other word files!  There are lots of word file
included, some are used, some are leftovers from earlier versions.  I need
a better naming convention for them, so if someone thinks of one let me
know.  I think this feature has lots of potential.  Should be able to
create completely random sentences by inserting verbs, nouns, etc.


SYN.DICT
~~~~~~~~
The syn.dict file is an easy way to make all incoming questions "normal".
By normal I mean contractions expanded, slang and misspellings corrected,
abbreviations expanded, etc.  This way your main.dict file does not have
to take every possible variation into account.

The format is "NEW WORD or PHRASE:OLD WORD:OLD WORD: ..."
              a ":" should separate each word.
NOTE: you MUST NOT have any garbage lines or comments at the end
Lines starting with a "#", space, or CR are comments, and are thus ignored

example: they will:they'll:theyll

this will replace every occurrence of "they'll" or "theyll" with "they will"

note that the program is smart enough to match only words and phrases, not
substrings.  For example "dog" would match only "dog" not "doghouse" or
"dogcatcher"


PROGRAM OVERVIEW
~~~~~~~~~~~~~~~~
After getting a question string, the program expands the question based on
the syn.dict file.  It then checks all of the known templates from main.dict
for one that matches the input.  Once a match is found, the program then
records its priority number (1-9, 9=highest) and continues searching
the rest of the dictionary.  When it is done, the matching template is
used and a response is chosen.  The program starts off by randomly picking
the first response, then incrementing to the next response when the
template is found again.  This keeps it from repeating itself until
absolutely necessary. Responses are then expanded out with input from the
"words" files if necessary, and the final response string is returned.
The priority allows you have much better control over your templates, so
you could have "Do you %" and "Do you have", assigning the longer key a
higher priority than the more general one.


PROGRAM OPERATION
~~~~~~~~~~~~~~~~~
The splotch.c program reads in the data, parses the questions, and
formulates a response.  init() first read in all the templates from the
main.dict file into a template structure.  A template structure contains a
long which represents the offset location of the first response within the
dictionary, the template itself, the priority of the template, the number
of replys available to the template, and the next reply that should be
used by the program (an int).

After reading in the dictionary file with the init() functions, the
ask(person, question) function can be called.  It will then return the
number of the template it responded to (or -1) and will cause the global
variable "response" to be set to contain the response. Note that if it
does not find a valid template match it will use one of responses from the
last entry in the dictionary.

See program comments for more info.


USING SPLOTCH
~~~~~~~~~~~~~
To use splotch in a program you must first make a call to init() so that
it will read in the dictionary data.  Then you can use the ask function.
Provided is a routine called chat that acts as a stand-alone interface for
a one on one conversation with splotch.  Notice the problem that when
going one-on-one the human participant tends to make assumptions as to
subject and topic, which confuses poor splotch.  Hopefully someone can
figure out a way of remembering the current thread of discussion.  The
splotch routines really shine when it able to interact with multiple users
and provide comments on the conversations.  I have written a program
(basically a hacked up sagebot) interface for IRC that works really
nicely.  I hesitate to release that because then every tom, dick, and
harry on IRC would be running my bot.  I mainly want INTERESTED
PROGRAMMERS who can help contribute to the project to play with it.  Mail
me if you would like more information.


BETA TESTER NOTES:
~~~~~~~~~~~~~~~~~~
Please let me know the inappropriate responses you get and even a possible
dictionary fix to correct them.  Of course there is no way to get ALL
responses correct, but the dictionary is easy enough to add to that it
should not be a problem.  If anyone wants to make suggestions or
improvement to the code feel free! The reason I am releasing this (at
least in a limited capacity) is so that it can improve


FUTURE IDEAS:
~~~~~~~~~~~~~
-the ability to know that certain templates dictate the current "topic"

-the ability to recognize pronouns, and questions as pertaining to the
 previous subject

-more complex responses with more detailed parts (noun, verb, etc)

-bigger dictionarys! always expanding...

-the ability to process multiple sentences on one input and even output
 multiple sentences.

-the ability to tell random storys
