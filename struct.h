#ifndef COMMON_H
#define COMMON_H 1

struct vm {
  int (*burn)(void const *, unsigned, unsigned);
  int (*init)(void);
  int (*load)(void const *, unsigned);
  int (*step)(void);
};

#endif /* COMMON_H */
