#ifndef PVM_H
#define PVM_H 1

struct prog {
  unsigned len;
  void *mem;
};

struct os {
  unsigned len;
  unsigned burn_addr;
  void *mem;
};

struct vm {
  int (*burn)(void const *, unsigned, unsigned);
  int (*init)(void);
  int (*load)(void const *, unsigned);
  int (*step)(void);
};

#include "pep9.h"

#endif /* VM_H */
