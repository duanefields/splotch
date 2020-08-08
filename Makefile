chat:	chat.o splotch.o
	gcc chat.o splotch.o -o chat
	strip chat

chat.o: chat.c splotch.h

splotch.o: splotch.c splotch.h










