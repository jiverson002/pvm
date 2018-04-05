pepvm.exe: main.c ops.h os.h types.h vm.h Makefile
	gcc -O0 -Wall -Wextra -std=c99 -o $@ main.c
