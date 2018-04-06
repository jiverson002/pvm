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
    byte inspec;
    word opspec;
  } cpu;

  byte mem[(1 << 16) + 1]; /* allocate one-extra byte to allow for loads to
                              always occur as words */
} vm;

#define NZVC   (vm.cpu.nzvc)
#define A      (vm.cpu.a)
#define X      (vm.cpu.x)
#define PC     (vm.cpu.pc)
#define SP     (vm.cpu.sp)
#define InSpec (vm.cpu.inspec)
#define OpSpec (vm.cpu.opspec)
#define Mem    (vm.mem)

#endif /* VM_H */
