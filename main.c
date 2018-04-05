#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OK(err) if (err) { printf("notok@%d\n", __LINE__); goto notok; }

typedef uint8_t  byte;
typedef uint16_t word;

typedef union {
  byte raw[2];
  word val;
} reg_word;

typedef union {
  byte raw[1];
  byte val;
} reg_byte;

struct {
  struct {
    reg_byte nzvc;
    reg_word a;
    reg_word x;
    reg_word pc;
    reg_word sp;
    reg_byte in_spec;
    reg_word op_spec;
  } cpu;

  byte mem[1<<16];
} vm;

#define NZVC    (vm.cpu.nzvc.val)
#define A       (vm.cpu.a.val)
#define X       (vm.cpu.x.val)
#define PC      (vm.cpu.pc.val)
#define SP      (vm.cpu.sp.val)
#define IN_SPEC (vm.cpu.in_spec.val)
#define OP_SPEC (vm.cpu.op_spec.val)
#define MEM     (vm.mem)

#define IS_UNARY(in_spec) (((in_spec) < 0x12) || (((in_spec) | 0x01) == 0x27))

static byte a2x(byte a) {
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
    case 'z':
    a -= 32; /* fall-through */

    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    return a - 65 + 10;

    default:
    assert(false);
  }
}

static int load(char const * const filename) {
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

  memset(&vm, 0, sizeof(vm));

  ret = load(argv[1]);
  OK(ret);
  
  for (;;) {
    // fetch instruction specifier
    IN_SPEC = MEM[PC];
    printf("%.2X", IN_SPEC);

    // increment pc
    PC++;

    // decode
    // NOOP

    // if non-unary
    if (!IS_UNARY(IN_SPEC)) {
      // fetch operand specifier
      OP_SPEC = ((word)MEM[PC] << 8) | (word)MEM[PC+1];
      printf(" %.4X", OP_SPEC);

      // increment pc
      PC += 2;
    }
    printf("\n");

    // execute
    if (0x00 == MEM[PC]) {
      break;
    }
  }

  return EXIT_SUCCESS;

notok:
  return EXIT_FAILURE;

  (void)argc;
}
