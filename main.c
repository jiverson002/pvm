#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "pep9.h"
#include "vm.h"

#define OK(err) if (err) { printf("notok@%d\n", __LINE__); goto notok; }

struct vm pepvm;

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

static int read_prog(char const * const filename) {
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
    pepvm.stbi(j, a2x[buf[i + 0]] * 16 + a2x[buf[i + 1]]);
  }

  free(buf);

  return 0;

notok:
  free(buf);

  return -1;
}

static int read_os(char const * const filename, void **os, unsigned *os_len) {
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

  *os = buf;
  *os_len = j - 1; /* subtract 1 for the zz delimiter */

  return 0;

notok:
  return -1;
}

static int von_neumann() {
  int ret;
  
  ret = pepvm.init();
  OK(ret);

  while (pepvm.step());

  return 0;

notok:
  return -1;
}

int main(int argc, char **argv) {
  int ret, c;
  unsigned os_len, burn_addr = 0x0000;
  void *os;
  char *os_file = NULL;

  if (argc < 2) {
    fprintf(stderr, "usage: pepvm [-di] [-b BURN_ADDRESS] [-o OS_FILE] FILE\n");
    /*
     * -d diagnostic mode - printf instructions as they are executed
     * -i interactive debugging mode - step through program like gdb
     * -o OS_FILE provide the machine code for an os to install
     * -b BURN_ADDRESS the address where the last byte of the os should be
     *    installed
     */
    return EXIT_FAILURE;
  }

  opterr = 0;
  while (-1 != (c = getopt(argc, argv, "+dib:o:"))) {
    switch (c) {
      case 'd':
        break;
      case 'i':
        break;
      case 'b':
        burn_addr = (unsigned)strtoul(optarg, NULL, 16);
        break;
      case 'o':
        os_file = optarg;
        break;
      default:
        abort();
    }
  }

  if (1 != argc - optind) {
    fprintf(stderr, "You must provide exactly one program file\n");
    return EXIT_FAILURE;
  }

  pepvm = pep9;

  if (os_file) {
    ret = read_os(os_file, &os, &os_len);
    OK(ret);

    ret = pepvm.burn(os, os_len, burn_addr);
    OK(ret);
  }

  ret = read_prog(argv[optind]);
  OK(ret);

  ret = von_neumann();
  OK(ret);

  return EXIT_SUCCESS;

notok:
  return EXIT_FAILURE;
}
