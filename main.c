#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "ops.h"
#include "types.h"
#include "vm.h"

#define OK(err) if (err) { printf("notok@%d\n", __LINE__); goto notok; }

static inline bool is_nonunary(byte in_spec) {
  return !(((in_spec) < 0x12) || (((in_spec) | 0x01) == 0x27));
}

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

    case 'z':
      return 0;

    default:
      assert(false);
  }
}

static inline int read(char const * const filename) {
  int ret;
  FILE *file;
  char *buf;
  long len, num;

  /* FIXME make this a compile time assertion */
  assert(sizeof(uint8_t) == sizeof(char));

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
    MEM[j] = a2x(buf[i+0]) * 16 + a2x(buf[i+1]);
  }

  free(buf);

  return 0;

notok:
  return -1;
}

int main(int argc, char **argv) {
  int ret;

  ret = read(argv[1]);
  OK(ret);

  SP = SP_INIT;

  for (;;) {
    // fetch instruction specifier
    IN_SPEC = MEM[PC];
    //printf("%.4X  %.2X", PC, IN_SPEC);

    // increment pc
    PC++;

    // decode
    // NOOP

    // if non-unary
    if (is_nonunary(IN_SPEC)) {
      // fetch operand specifier
      OP_SPEC = LDW(PC);
      //printf(" %.4X", OP_SPEC);

      // increment pc
      PC += 2;
    }
    //printf("\n");

    // execute
    if (0x00 == IN_SPEC) {
      break;
    }
    ops[IN_SPEC](IN_SPEC, OP_SPEC);
  }

  //for (int i = 0; i < 32; i++) {
  //  printf("%.2X ", MEM[i]);
  //}
  //printf("\n");

  return EXIT_SUCCESS;

notok:
  return EXIT_FAILURE;

  (void)argc;
}
