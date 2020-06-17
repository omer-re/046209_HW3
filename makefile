# Makefile for the smash program
CC = gcc 
CFLAGS = -g -Wall 
CCLINK = $(CC)
RM = rm -f
TARGET = ttftps
# Creating the  executable
ttftps: server.c
	$(CCLINK) $(CFLAGS) server.c -o $(TARGET)
# Cleaning old files before new make
clean:
	$(RM) $(TARGET) *.o *~ "#"* core.* 


