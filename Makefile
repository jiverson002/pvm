.POSIX:

OPTIMIZE := -O0 -g
STANDARD := -ansi -pedantic
WARNING  := -Wall -Wextra -Wpedantic -Wshadow -Wpointer-arith -Wcast-align \
            -Wwrite-strings -Wmissing-prototypes -Wmissing-declarations \
            -Wredundant-decls -Wnested-externs -Winline -Wno-long-long \
            -Wuninitialized -Wstrict-prototypes \
					  -Wno-sign-conversion -Wno-unused-function

CFLAGS := $(OPTIMIZE) $(STANDARD) $(WARNING)

PVM_LIB := libpvm.a

$(PVM_LIB): $(PVM_LIB)(pep9.o)

clean:
	rm -f $(PVM_LIB)
