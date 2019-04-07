OBJS = main.o ogles2_module.o warp3dnova_module.o logger.o
CFLAGS = -Wall -O3 -ggdb

%.o : %.c makefile
	gcc -o $@ -c $< $(CFLAGS)

glsnoop: $(OBJS) makefile
	gcc -o $@ $(OBJS)

clean:
	delete $(OBJS)

