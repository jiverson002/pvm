OPTIMIZE := -O0 -g
STANDARD := -ansi -pedantic
WARNING  := -Wall -Wextra -pedantic -Wshadow -Wpointer-arith -Wcast-align \
            -Wwrite-strings -Wmissing-prototypes -Wmissing-declarations \
            -Wredundant-decls -Wnested-externs -Winline -Wno-long-long \
            -Wuninitialized -Wconversion -Wstrict-prototypes
CFLAGS   := $(OPTIMIZE) $(STANDARD) $(WARNING)

pepvm.exe: main.c v9.h Makefile
	gcc -O0 $(CFLAGS) -o $@ main.c
