ifneq ($(shell uname), AmigaOS)
	CC 		= ppc-amigaos-gcc
	DELETE		= rm -f
else
	CC 		= gcc
	DELETE		= delete
endif

OBJS = main.o ogles2_module.o warp3dnova_module.o logger.o gui.o common.o filter.o
CFLAGS = -Wall -Wextra -O3 -gstabs

%.o : %.c makefile
	$(CC) -o $@ -c $< $(CFLAGS)

glsnoop: $(OBJS) makefile
	$(CC) -o $@ $(OBJS) -lauto

clean:
	$(DELETE) $(OBJS)
