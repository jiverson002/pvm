.POSIX:

OPTIMIZE := -O0 -g
STANDARD := -std=c99 -pedantic
WARNING  := -Wall -Wextra -Wpedantic -Wshadow -Wpointer-arith -Wcast-align \
            -Wwrite-strings -Wmissing-prototypes -Wmissing-declarations \
            -Wredundant-decls -Wnested-externs -Winline -Wno-long-long \
            -Wuninitialized -Wstrict-prototypes \
            -Wno-sign-conversion -Wno-unused-function

CFLAGS := $(OPTIMIZE) $(STANDARD) $(WARNING)
YFLAGS := -dy

HDRS := y.tab.h
SRCS := y.tab.c lex.yy.c
OBJS := y.tab.o lex.yy.o

PVM_EXE := pvm.exe

$(PVM_EXE): $(OBJS)
	$(CC) $(OBJS) -o $@ -ll

clean:
	rm -f $(HDRS) $(SRCS) $(OBJS)
