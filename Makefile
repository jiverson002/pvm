pepvm.exe: main.c ops.h os.h types.h vm.h Makefile
	gcc -O0 -Wall -Wextra -ansi -pedantic -o $@ main.c
