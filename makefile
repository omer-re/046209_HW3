# Makefile for ttftps program - re-writing makefile
CC = gcc
CFLAGS = -g -Wall
CCLINK = $(CC)
OBJS = ttftps.o server.o
RM = rm -f
TARGET = ttftps

# Creat the executable
ttftps: $(OBJS)
	$(CCLINK) -o ttftps $(OBJS)

# creating the object files
server.o: server.c server.h
ttftps.o: ttftps.c server.h

# Cleaning old files
clean:
	$(RM) $(TARGET) *.o *~ "#"* core.*
