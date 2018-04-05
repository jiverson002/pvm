#include <stdio.h>
#include <stdlib.h>

#include "ops.h"
#include "types.h"
#include "vm.h"

#define OK(err) if (err) { printf("notok@%d\n", __LINE__); goto notok; }

static inline byte a2x(byte a) {
  switch (a) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      return a - 48;

    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
      a -= 32; /* fall-through */

    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
      return a - 65 + 10;

    default:
      return 0;
  }
}

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
    Mem[j] = a2x(buf[i+0]) * 16 + a2x(buf[i+1]);
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
    if (0x00 == InSpec) {
      break;
    }
    ops[InSpec](InSpec, OpSpec);
  }

  return EXIT_SUCCESS;

notok:
  return EXIT_FAILURE;

  (void)argc;
}
