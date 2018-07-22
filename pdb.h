#ifndef PDB_H
#define PDB_H 1

#define PDB_MAJOR 0
#define PDB_MINOR 0
#define PDB_PATCH 0

#include "vm.h"

int pdb(struct vm *, struct os *, struct prog *, int);

#endif /* PDB_H */
