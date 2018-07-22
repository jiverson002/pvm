#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "pdb.h"
#include "pep9.h"
#include "vm.h"

#define OK(err) if (err) { printf("notok@%s:%d\n", __FILE__, __LINE__); goto notok; }

static unsigned char a2x[256] = {
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

static int read_file(char const * const filename, void **mem, unsigned *mem_len) {
  int ret;
  unsigned i, j;
  FILE *file;
  unsigned char *buf = NULL;
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

  buf = calloc(num + 1, sizeof(*buf));
  OK(NULL == buf);

  num = fread(buf, 1, num, file);
  OK(num != (unsigned long)len);

  ret = fclose(file);
  OK(ret);

  for (i=0,j=0; i < num; i+=3,j++) {
    buf[j] = a2x[buf[i + 0]] * 16 + a2x[buf[i + 1]];
  }

  *mem = buf;
  *mem_len = j - 1; /* subtract 1 for the zz delimiter */

  return 0;

notok:
  free(buf);

  return -1;
}

static void usage(void) {
  fprintf(stderr, "usage: pepvm [-dgh] [-b BURN_ADDRESS] [-o OS_FILE] [FILE]\n");
  /*
   * -d diagnostic mode - printf instructions as they are executed
   * -g debugging mode - step through program like gdb
   * -h print the help message
   * -o OS_FILE provide the machine code for an os to install
   * -b BURN_ADDRESS the address where the last byte of the os should be
   *    installed
   */
}

int main(int argc, char **argv) {
  int ret, c, dbg;
  struct os os;
  struct prog prog;
  struct vm vm;
  char *os_file;

  dbg = 0;
  os_file = NULL;

  memset(&os, 0, sizeof(struct os));
  memset(&prog, 0, sizeof(struct prog));
  memset(&vm, 0, sizeof(struct vm));

  opterr = 0;
  while (-1 != (c = getopt(argc, argv, "+dhgb:o:"))) {
    switch (c) {
      case 'd':
        break;
      case 'g':
        dbg = 1;
        break;
      case 'h':
        usage();
        return 0;
      case 'b':
        os.burn_addr = (unsigned)strtoul(optarg, NULL, 16);
        break;
      case 'o':
        os_file = optarg;
        break;
      default:
        usage();
        abort();
    }
  }

  if (1 < argc - optind) { /* cannot ever specify more than one program file */
    usage();
    OK(-1);
  }

  if (!dbg && 0 == argc - optind) { /* must specify exactly one program file */
    usage();                        /* in non-debugging mode */
    OK(-1);
  }

  vm = pep9;

  if (os_file) {
    ret = read_file(os_file, &(os.mem), &(os.len));
    OK(ret);
  }

  if (1 == argc - optind) {
    ret = read_file(argv[optind], &(prog.mem), &(prog.len));
    OK(ret);
  }

  ret = pdb(&vm, &os, &prog, dbg);
  OK(ret);

  return EXIT_SUCCESS;

notok:
  return EXIT_FAILURE;
}
