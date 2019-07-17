ifneq ($(shell uname), AmigaOS)
	CC 		= ppc-amigaos-gcc
	DELETE		= rm -f
else
	CC 		= gcc
	DELETE		= delete
endif

OBJS = main.o ogles2_module.o warp3dnova_module.o logger.o gui.o common.o filter.o timer.o profiling.o
DEPS = $(OBJS:.o=.d)

CFLAGS = -Wall -Wextra -O3 -gstabs

# Dependencies
%.d : %.c
	$(CC) -MM -MP -MT $(@:.d=.o) -o $@ $< $(CFLAGS)

%.o : %.c
	$(CC) -o $@ -c $< $(CFLAGS)

glSnoop: $(OBJS) makefile
	$(CC) -o $@ $(OBJS) -lauto

clean:
	$(DELETE) $(OBJS)

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif
