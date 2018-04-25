#ifndef VM_H
#define VM_H 1

struct vm {
  int (*burn)(void*, unsigned, unsigned);
  int (*init)(void);
  int (*load)(void*, unsigned);
  int (*stbi)(unsigned, unsigned char);
  int (*step)(void);
};

struct os {
  unsigned len;
  unsigned burn_addr;
  void *mem;
};

struct prog {
  unsigned len;
  void *mem;
};

#endif /* VM_H */
