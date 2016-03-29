#CC=gcc
#CFLAGS=-I.
#CFLAGS = -g -Wall -std=gnu99 -Wpointer-arith
#DEPS = cvector.h cmap.h
#OBJ = cvector.o vectortest.o cmap.o

#%.o: %.c $(DEPS)
#	$(CC) -c -o $@ $< $(CFLAGS)

#vector-test: $(OBJ)
#	$(CC) -o $@ $^ $(CFLAGS)

CC = gcc
CFLAGS = -g -Wall -std=c99
LDFLAGS =
PURIFY = purify
PFLAGS=  -demangle-program=/usr/pubsw/bin/c++filt -linker=/usr/bin/ld -best-effort

VECTOR_SRCS = cvector.c
VECTOR_HDRS = $(VECTOR_SRCS:.c=.h)

CMAP_SRCS = cmap.c
CMAP_HDRS = $(CMAP_SRCS:.c=.h)

VECTOR_TEST_SRCS = vectortest.c $(VECTOR_SRCS)
VECTOR_TEST_OBJS = $(VECTOR_TEST_SRCS:.c=.o)

CMAP_TEST_SRCS = cmaptest.c $(CMAP_SRCS)
CMAP_TEST_OBJS = $(CMAP_TEST_SRCS:.c=.o)

SPELLCHECK_SRCS = spelltest.c $(CMAP_SRCS)
SPELLCHECK_OBJS = $(SPELLCHECK_SRCS:.c=.o)

SRCS = $(SPELLCHECK_SRCS) $(VECTOR_SRCS) $(CMAP_SRCS) vectortest.c cmaptest.c
HDRS = $(VECTOR_HDRS) $(CMAP_HDRS)

EXECUTABLES = spellcheck vector-test cmap-test

default: $(EXECUTABLES)

vector-test : Makefile.dependencies $(VECTOR_TEST_OBJS)
	$(CC) -o $@ $(VECTOR_TEST_OBJS) $(LDFLAGS)

cmap-test : Makefile.dependencies $(CMAP_TEST_OBJS)
	$(CC) -o $@ $(CMAP_TEST_OBJS) $(LDFLAGS)

spellcheck : Makefile.dependencies $(SPELLCHECK_OBJS)
	$(CC) -o $@ $(SPELLCHECK_OBJS) $(LDFLAGS)

# The dependencies below make use of make's default rules,
# under which a .o automatically depends on its .c and
# the action taken uses the $(CC) and $(CFLAGS) variables.
# These lines describe a few extra dependencies involved.

Makefile.dependencies:: $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) -MM $(SRCS) > Makefile.dependencies

-include Makefile.dependencies

clean:
	\rm -fr a.out $(EXECUTABLES) *.o core Makefile.dependencies
