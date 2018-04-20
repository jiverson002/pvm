OPTIMIZE := -O0 -g
STANDARD := -ansi
WARNING  := -Wall -Wextra -Wpedantic -Wshadow -Wpointer-arith -Wcast-align \
            -Wwrite-strings -Wmissing-prototypes -Wmissing-declarations \
            -Wredundant-decls -Wnested-externs -Winline -Wno-long-long \
            -Wuninitialized -Wconversion -Wstrict-prototypes
CFLAGS   := $(OPTIMIZE) $(STANDARD) $(WARNING)

pepvm.exe: main.c pep9.c pep9.h Makefile
	gcc $(CFLAGS) -o $@ main.c pep9.c
