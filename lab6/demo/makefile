all : tsh tshtest launch copy

launch : launch.c tshlib.c
	$(cc) $(EXTRA) $(INCS) $(FLAGS) -o launch launch.c tshlib.c -L$(OBJS) -lsng

copy: tsh tshtest launch
	cp tsh ../bin
	cp tshtest ../bin
	cp launch ../bin