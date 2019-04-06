OBJS = main.o ogles2_module.o warp3dnova_module.o

%.o : %.c
	gcc -o $@ -c $< -Wall -O3 -ggdb

glsnoop: $(OBJS)
	gcc -o $@ $(OBJS) -logles2

clean:
	delete $(OBJS)
