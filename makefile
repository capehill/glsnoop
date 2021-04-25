ifneq ($(shell uname), AmigaOS)
	CC 		= ppc-amigaos-gcc
	STRIP = ppc-amigaos-strip
	AMIGADATE = $(shell date +"%-d.%-m.%Y")
else
	CC 		= gcc
	STRIP = strip
	AMIGADATE = $(shell date LFORMAT "%-d.%-m.%Y")
endif

NAME = glSnoop
OBJS = main.o ogles2_module.o warp3dnova_module.o logger.o gui.o common.o filter.o timer.o profiling.o
DEPS = $(OBJS:.o=.d)

CFLAGS = -Wall -Wextra -O3 -gstabs -D__AMIGA_DATE__=\"$(AMIGADATE)\"

# Dependencies
%.d : %.c
	$(CC) -MM -MP -MT $(@:.d=.o) -o $@ $< $(CFLAGS)

%.o : %.c
	$(CC) -o $@ -c $< $(CFLAGS)

$(NAME): $(OBJS) makefile
	$(CC) -o $@ $(OBJS) -lauto

clean:
	$(RM) $(OBJS)

strip:
	$(STRIP) $(NAME)

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif
