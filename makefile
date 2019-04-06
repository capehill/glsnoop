OBJS = main.o ogles2_module.o

%.o : %.c
	gcc -o $@ -c $< -Wall -O0 -ggdb

glsnoop: $(OBJS)
	gcc -o $@ $(OBJS) -logles2

clean:
	delete $(OBJS)
