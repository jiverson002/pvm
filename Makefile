pepvm.exe: main.c ops.h os.h types.h vm.h Makefile
	gcc -Wall -Wextra -std=c99 -o $@ main.c
