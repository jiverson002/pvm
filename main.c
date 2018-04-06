#include <stdio.h>
#include <stdlib.h>

#include "ops.h"
#include "types.h"
#include "vm.h"

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
  int ret;
  FILE *file;
  char *buf=NULL;
  long len, num;

  file = fopen(filename, "rb");
  OK(NULL == file);

  ret = fseek(file, 0L, SEEK_END);
  OK(ret);

  len = ftell(file);
  OK(-1 == len);

  ret = fseek(file, 0L, SEEK_SET);
  OK(ret);

  buf = malloc((len + 1) * sizeof(*buf));
  OK(NULL == buf);

  num = fread(buf, 1, len, file);
  OK(num != len);

  ret = fclose(file);
  OK(ret);

  for (int i=0,j=0; i < len; i+=3,j++) {
    Mem[j] = a2x[(int)buf[i+0]] * 16 + a2x[(int)buf[i+1]];
  }

  free(buf);

  return 0;

notok:
  free(buf);

  return -1;
}

int main(int argc, char **argv) {
  int ret;

  ret = read(argv[1]);
  OK(ret);

  A  = 0;
  X  = 0;
  PC = 0;
  SP = SP_INIT;
  InSpec = 0;
  OpSpec = 0;

  for (;;) {
    // fetch instruction specifier
    InSpec = Mem[PC];

    // increment pc
    PC++;

    // decode
    // NOOP

    // if non-unary
    if (is_nonunary(InSpec)) {
      // fetch operand specifier
      OpSpec = LDW(PC);

      // increment pc
      PC += 2;
    }

    // execute
    if (/* STOP */0x00 == InSpec) {
      break;
    }
    ops[InSpec](InSpec, OpSpec);
  }

  return EXIT_SUCCESS;

notok:
  return EXIT_FAILURE;

  (void)argc;
}
