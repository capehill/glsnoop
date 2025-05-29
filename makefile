ifneq ($(shell uname), AmigaOS)
	AMIGADATE = $(shell date +"%-d.%-m.%Y")
else
	AMIGADATE = $(shell date LFORMAT "%-d.%-m.%Y")
endif

CC = ppc-amigaos-gcc
STRIP = ppc-amigaos-strip

NAME = glSnoop
SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)

CFLAGS = -Wall -Wextra -Wconversion -O3 -gstabs -D__AMIGA_DATE__=\"$(AMIGADATE)\"

# Dependencies
%.d : %.c
	$(CC) -MM -MP -MT $(@:.d=.o) -o $@ $< $(CFLAGS)

%.o : %.c
	$(CC) -o $@ -c $< $(CFLAGS)

$(NAME): $(OBJS) makefile
	$(CC) -o $@ $(OBJS) -lauto

clean:
	$(RM) $(OBJS) $(DEPS)

strip:
	$(STRIP) $(NAME)

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif
