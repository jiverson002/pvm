#ifndef VM_H
#define VM_H 1

#include "types.h"

static struct {
  struct {
    byte nzvc;
    word a;
    word x;
    word pc;
    word sp;
    byte in_spec;
    word op_spec;
    word oprnd;
  } cpu;

  byte mem[(1 << 15) + 1]; /* allocate one-extra byte to allow for loads to
                              always occur as words */
} vm;

#define NZVC    (vm.cpu.nzvc)
#define A       (vm.cpu.a)
#define X       (vm.cpu.x)
#define PC      (vm.cpu.pc)
#define SP      (vm.cpu.sp)
#define IN_SPEC (vm.cpu.in_spec)
#define OP_SPEC (vm.cpu.op_spec)
#define OPRND   (vm.cpu.oprnd)
#define MEM     (vm.mem)

#endif /* VM_H */
