cc = gcc
FLAGS = -g
INCS = -I../include
OBJS = ../obj
EXTRA =

all : tsh tshtest launch copy

tsh : tsh.c tsh.h
	$(cc) $(INCS) $(FLAGS) $(EXTRA) -o tsh tsh.c -L$(OBJS) -lsng

tshtest : tshtest.c tshtest.h
	$(cc) $(INCS) $(FLAGS) $(EXTRA) -o tshtest tshtest.c -L$(OBJS) -lsng

launch : launch.c launch.h tshlib.c tshlib.h
	$(cc) $(INCS) $(FLAGS) $(EXTRA) -o launch launch.c tshlib.c -L$(OBJS) -lsng

copy : tsh tshtest launch
	cp tsh ../bin
	cp tshtest ../bin
	cp launch ../bin

clean :
	rm -f tsh tshtest launch