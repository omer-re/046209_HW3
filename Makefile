# Makefile for the ttftps program
CC = gcc
CFLAGS = -g -Wall
CCLINK = $(CC)
OBJS = ttftps.o server.o
RM = rm -f
# Creating the  executable
ttftps: $(OBJS)
	$(CCLINK) -o ttftps $(OBJS)
# Creating the object files
‏‏server.o: server.c server.h
ttftps.o: ttftps.c server.h
# Cleaning old files before new make
clean:
	$(RM) $(TARGET) *.o *~ "#"* core.*

