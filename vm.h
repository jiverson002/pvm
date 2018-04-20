#ifndef VM_H
#define VM_H 1

struct vm {
  int (*init)(void);
  int (*stbi)(unsigned, char);
  int (*step)(void);
};

#endif /* VM_H */
