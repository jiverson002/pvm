#include <stdio.h>
#include <stdlib.h>

#include "v9.h"

#define OK(err) if (err) { printf("notok@%d\n", __LINE__); goto notok; }

static byte a2x[256] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, /*   0 -  15 */
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, /*  16 -  31 */
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, /*  32 -  47 */
  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 0, 0, 0, 0, 0, 0, /*  48 -  63 */
  0, 10, 11, 12, 13, 14, 15,  0,  0,  0, 0, 0, 0, 0, 0, 0, /*  64 -  79 */
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, /*  80 -  95 */
  0, 10, 11, 12, 13, 14, 15,  0,  0,  0, 0, 0, 0, 0, 0, 0, /*  96 - 111 */
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, /* 112 - 127 */
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, /* 128 - 143 */
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, /* 144 - 159 */
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, /* 160 - 165 */
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, /* 176 - 191 */
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, /* 192 - 207 */
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, /* 208 - 223 */
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, /* 224 - 239 */
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0  /* 240 - 255 */
};

static int read(char const * const filename) {
  int ret, i, j;
  FILE *file;
  char *buf=NULL;
  long len;
  unsigned long num;

  file = fopen(filename, "rb");
  OK(NULL == file);

  ret = fseek(file, 0L, SEEK_END);
  OK(ret);

  len = ftell(file);
  OK(-1 == len);
  num = (unsigned long)len;

  ret = fseek(file, 0L, SEEK_SET);
  OK(ret);

  buf = malloc((num + 1) * sizeof(*buf));
  OK(NULL == buf);

  num = fread(buf, 1, num, file);
  OK(num != (unsigned long)len);

  ret = fclose(file);
  OK(ret);

  for (i=0,j=0; i < len; i+=3,j++) {
    Mem[j] = a2x[(int)buf[i+0]] * 16 + a2x[(int)buf[i+1]];
  }

  free(buf);

  return 0;

notok:
  free(buf);

  return -1;
}

static int vonNeumann() {
  A  = 0x0000;
  X  = 0x0000;
  PC = 0x0000;
  SP = SP_INIT;
  InSpec = 0x00;
  OpSpec = 0x0000;

  for (;;) {
    /* fetch instruction specifier */
    InSpec = Mem[PC];
    /*fprintf(stderr, "%.2X", InSpec);*/

    /* increment pc */
    PC++;

    /* decode */
    /* NOOP */

    /* if non-unary */
    if (is_nonunary(InSpec)) {
      /* fetch operand specifier */
      OpSpec = LDW(PC);
      /*fprintf(stderr, " %.4X", OpSpec);*/

      /* increment pc */
      PC += 2;
    }
    /*else {
      fprintf(stderr, "     ");
    }*/

    /* execute */
    if (/* STOP */0x00 == InSpec) {
      /*fprintf(stderr, "\n");*/
      break;
    }
    ops[InSpec](InSpec, OpSpec);

    /*fprintf(stderr, " %.4X %.4X %.4X %.4X %.1X\n", A, X, PC, SP, NZVC);*/
  }

  return 0;
}

/* TODO
 * [ ] add -d flag for debugging mode which will print machine code
 * [ ] add the ability to pass batch i/o on command line
 */
int main(int argc, char **argv) {
  int ret;

  if (argc < 2) {
    fprintf(stderr, "usage: pepvm file [- batch i/o]\n");
    return EXIT_FAILURE;
  }

  ret = read(argv[1]);
  OK(ret);

  ret = vonNeumann();
  OK(ret);

  return EXIT_SUCCESS;

notok:
  return EXIT_FAILURE;
}
