cc = gcc
FLAGS = -g
INCS = -I../include -I../tsh
OBJS = ../obj

all: master worker api_test

master: master.c ../tsh/tshlib.c
	$(cc) $(FLAGS) $(INCS) -o master master.c ../tsh/tshlib.c -L$(OBJS) -lsng

worker: worker.c ../tsh/tshlib.c
	$(cc) $(FLAGS) $(INCS) -o worker worker.c ../tsh/tshlib.c -L$(OBJS) -lsng

api_test: api_test.c ../tsh/tshlib.c
	$(cc) $(FLAGS) $(INCS) -o api_test api_test.c ../tsh/tshlib.c -L$(OBJS) -lsng

clean:
	rm -f master worker api_test
