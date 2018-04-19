#ifndef V9_H
#define V9_H 1

#define vm_init vm_init_v9
#define vm_stbi vm_stbi_v9
#define vm_step vm_step_v9

#ifdef __cplusplus
extern "C" {
#endif


int vm_init(void);
int vm_stbi(unsigned, char);
int vm_step(void);

#ifdef __cplusplus
}
#endif

#endif /* V9_H */
