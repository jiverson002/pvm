#ifndef OPS_H
#define OPS_H 1

#include <assert.h>
#include <stdio.h>

#include "os.h"
#include "types.h"
#include "vm.h"

#if __STDC_VERSION__ < 199901L
  #define inline /* define to nothing if pre-C99 */
#endif

#define LDW(idx)    (((word)Mem[(idx)] << 8) | (word)Mem[(idx) + 1])
#define STW(idx, w) (Mem[(idx) + 0] = (w) >> 8, Mem[(idx) + 1] = (w) & 0x00ff)

static inline int is_nonunary(byte in_spec) {
  return !(((in_spec) < 0x12) || (((in_spec) | 0x01) == 0x27));
}

static inline word *get_reg(byte inspec) {
  if (inspec <= 0x11) {
    if (0x00 == (inspec & 0x01)) {      /* accumulator */
      return &A;
    }
    return &X;                          /* index */
  } else if (0x00 == (inspec & 0x08)) { /* accumulator */
    return &A;
  }
  return &X;                            /* index */
}

static inline word get_addr(byte inspec, word opspec) {
  switch (inspec & 0x07) {
    case 0x01:  /* direct */
      return opspec;
    case 0x02:  /* indirect */
      return LDW(opspec);
    case 0x03:  /* stack-relative */
      return SP + opspec;
    case 0x04:  /* stack-relative deferred */
      return LDW(SP + opspec);
    case 0x05:  /* indexed */
      return opspec + X;
    case 0x06:  /* stack-indexed */
      return SP + opspec + X;
    case 0x07:  /* stack-deferred indexed */
      return LDW(SP + opspec) + X;
  }
  return 0xffff; /* error */
}

static inline word get_oprnd(byte inspec, word opspec) {
  if (0x00 == (inspec & 0x01)) {  /* immediate */
    return opspec;
  } else if (inspec <= 0x25) {    /* indexed */
    return LDW(opspec + X);
  } else {                        /* direct, indirect, stack-relative,
                                     stack-relative deferred, indexed,
                                     stack-indexed, stack-deferred indexed */
    return LDW(get_addr(inspec, opspec));
  }
}

static void RET(byte inspec, word opspec) {
  PC = LDW(SP);
  SP += 2;

  (void)inspec;
  (void)opspec;
}

static void MOVSPA(byte inspec, word opspec) {
  A = SP;

  (void)inspec;
  (void)opspec;
}

static void MOVFLGA(byte inspec, word opspec) {
  A &= 0xfff0; /* clear A<12..15> */
  A |= NZVC;

  (void)inspec;
  (void)opspec;
}

static void MOVAFLG(byte inspec, word opspec) {
  NZVC = A & 0x000f;

  (void)inspec;
  (void)opspec;
}

static void NOTr(byte inspec, word opspec) {
  word *r = get_reg(inspec);

  *r = ~(*r);

  NZVC &= 0x03;                 /* clear all but VC */
  NZVC |= (*r & 0x8000) >> 12;  /* N */
  NZVC |= (*r == 0) << 2;       /* Z */

  (void)opspec;
}

static void NEGr(byte inspec, word opspec) {
  word *r = get_reg(inspec);
  sword s = *r;

  *r = -(*r);

  NZVC &= 0x01;                 /* clear all but C */
  NZVC |= (*r & 0x8000) >> 12;  /* N */
  NZVC |= (*r == 0) << 2;       /* Z */
  NZVC |= (s == -32768) << 1;   /* V */

  (void)opspec;
}

static void BR(byte inspec, word opspec) {
  PC = get_oprnd(inspec, opspec);
}

static void BRLE(byte inspec, word opspec) {
  if (NZVC & 0x0c) {
    PC = get_oprnd(inspec, opspec);
  }
}

static void BRLT(byte inspec, word opspec) {
  if (NZVC & 0x08) {
    PC = get_oprnd(inspec, opspec);
  }
}

static void BREQ(byte inspec, word opspec) {
  if (NZVC & 0x04) {
    PC = get_oprnd(inspec, opspec);
  }
}

static void BRNE(byte inspec, word opspec) {
  if (!(NZVC & 0x04)) {
    PC = get_oprnd(inspec, opspec);
  }
}

static void BRGE(byte inspec, word opspec) {
  if (!(NZVC & 0x0c)) {
    PC = get_oprnd(inspec, opspec);
  }
}

static void BRGT(byte inspec, word opspec) {
  if (!(NZVC & 0x08)) {
    PC = get_oprnd(inspec, opspec);
  }
}

static void BRV(byte inspec, word opspec) {
  if (NZVC & 0x02) {
    PC = get_oprnd(inspec, opspec);
  }
}

static void BRC(byte inspec, word opspec) {
  if (NZVC & 0x01) {
    PC = get_oprnd(inspec, opspec);
  }
}

static void CALL(byte inspec, word opspec) {
  SP -= 2;
  STW(SP, PC);
  PC = get_oprnd(inspec, opspec);
}

static void LDWr(byte inspec, word opspec) {
  word *r = get_reg(inspec);

  *r = get_oprnd(inspec, opspec);

  NZVC &= 0x03;                 /* clear all but VC */
  NZVC |= (*r & 0x8000) >> 12;  /* N */
  NZVC |= (*r == 0) << 2;       /* Z */
}

static void LDBr(byte inspec, word opspec) {
  byte b;
  word oprnd, op_addr;
  word *r = get_reg(inspec);

  switch (inspec | 0x08) {
    case 0xd8: /* immediate */
      oprnd = get_oprnd(inspec, opspec);
      break;
    case 0xd9:  /* direct */
    case 0xda:  /* indirect */
    case 0xdb:  /* stack-relative */
    case 0xdc:  /* stack-relative deferred */
    case 0xdd:  /* indexed */
    case 0xde:  /* stack-indexed */
    case 0xdf:  /* stack-deferred indexed */
      op_addr = get_addr(inspec, opspec);
      if (charIn == op_addr) {
        scanf("%c", &b);
        oprnd = b;
      } else {
        /* shift b/c two bytes were loaded and the high one is the one at the
         * desired address. */
        oprnd = get_oprnd(inspec, opspec) >> 8;
      }
      break;
  }

  *r = (*r & 0xff00) | oprnd;

  NZVC &= 0x03;           /* clear all but VC */
                          /* N is 0 by definition of Pep/9 */
  NZVC |= (*r == 0) << 2; /* Z */
}

static void STWr(byte inspec, word opspec) {
  STW(get_addr(inspec, opspec), *get_reg(inspec));
}

static void STBr(byte inspec, word opspec) {
  byte b = *get_reg(inspec) & 0x00ff;
  word op_addr = get_addr(inspec, opspec);

  if (charOut == op_addr) {
    printf("%c", b);
  }
  else {
    Mem[op_addr] = b;
  }
}

static void DECI(byte inspec, word opspec) {
  int i;
  word w;

  scanf("%d", &i);
  w = i;

  STW(get_addr(inspec, opspec), w);

  NZVC &= 0x01;                 /* clear all but C */
  NZVC |= (w & 0x8000) >> 12;   /* N */
  NZVC |= (w == 0) << 2;        /* Z */
  NZVC |= (i != (sword)w) << 1; /* V */

  assert((i >= -32768 && i <= 32767) || (NZVC & 0x02));
  assert(((sword)w >= 0) || (NZVC & 0x08));
  assert(((sword)w != 0) || (NZVC & 0x04));
}

static void DECO(byte inspec, word opspec) {
  printf("%d", (sword)get_oprnd(inspec, opspec));
}

static void HEXO(byte inspec, word opspec) {
  printf("%.4X", get_oprnd(inspec, opspec));
}

static void STRO(byte inspec, word opspec) {
  word op_addr = get_addr(inspec, opspec);
  while ('\0' != Mem[op_addr]) {
    printf("%c", Mem[op_addr++]);
  }
}

static void ADDSP(byte inspec, word opspec) {
  /* FIXME need to modify NZVC bits */
  SP += get_oprnd(inspec, opspec);
}

static void SUBSP(byte inspec, word opspec) {
  /* FIXME need to modify NZVC bits */
  SP -= get_oprnd(inspec, opspec);
}

static void ADDr(byte inspec, word opspec) {
  word o, v;
  word oprnd = get_oprnd(inspec, opspec);
  word *r = get_reg(inspec);

  o = *r;
  *r += oprnd;

  /* Inspect the signs of the numbers and the sum. If you add numbers with
   * different signs, you cannot have an overflow. If you add two numbers with
   * the same sign and the result is not the same sign, then you have signed
   * overflow. */
  v = !((o & 0x8000) ^ (oprnd & 0x8000)) && ((o & 0x8000) ^ (*r & 0x8000));

  NZVC = 0;                     /* clear all bits */
  NZVC |= (*r & 0x8000) >> 12;  /* N */
  NZVC |= (*r == 0) << 2;       /* Z */
  NZVC |= v << 1;               /* V */
  NZVC |= (*r < o);             /* C */
}

static void SUBr(byte inspec, word opspec) {
  word o, v;
  word oprnd = ~get_oprnd(inspec, opspec) + 1;
  word *r = get_reg(inspec);

  o = *r;
  *r += oprnd;

  /* See note in ADDr for explanation! */
  v = !((o & 0x8000) ^ (oprnd & 0x8000)) && ((o & 0x8000) ^ (*r & 0x8000));

  NZVC = 0;                     /* clear all bits */
  NZVC |= (*r & 0x8000) >> 12;  /* N */
  NZVC |= (*r == 0) << 2;       /* Z */
  NZVC |= v << 1;               /* V */
  NZVC |= (*r < o);             /* C */
}

static void ANDr(byte inspec, word opspec) {
  word *r = get_reg(inspec);

  *r &= get_oprnd(inspec, opspec);;

  NZVC &= 0x03;                 /* clear all but VC */
  NZVC |= (*r & 0x8000) >> 12;  /* N */
  NZVC |= (*r == 0) << 2;       /* Z */
}

static void ORr(byte inspec, word opspec) {
  word *r = get_reg(inspec);

  *r |= get_oprnd(inspec, opspec);

  NZVC &= 0x03;                 /* clear all but VC */
  NZVC |= (*r & 0x8000) >> 12;  /* N */
  NZVC |= (*r == 0) << 2;       /* Z */
}

static void CPWr(byte inspec, word opspec) {
  word o, w, v;
  word oprnd = ~get_oprnd(inspec, opspec) + 1;
  word *r = get_reg(inspec);

  o = *r;
  w = *r + oprnd;

  /* See note in ADDr for explanation! */
  v = !((o & 0x8000) ^ (oprnd & 0x8000)) && ((o & 0x8000) ^ (w & 0x8000));

  NZVC = 0;                   /* clear all bits */
  NZVC |= (w & 0x8000) >> 12; /* N */
  NZVC |= (w == 0) << 2;      /* Z */
  NZVC |= v << 1;             /* V */
  NZVC |= (w < o);            /* C */
}

static void (*ops[256])(byte, word) = {
  /* STOP */    NULL,
  /* RET */     RET,
  /* RETTR */   NULL,
  /* MOVSPA */  MOVSPA,
  /* MOVFLGA */ MOVFLGA,
  /* MOVAFLG */ MOVAFLG,
  /* NOTr */    NOTr, NOTr,
  /* NEGr */    NEGr, NEGr,
  /* ASLr */    NULL, NULL,
  /* ASRr */    NULL, NULL,
  /* ROLr */    NULL, NULL,
  /* RORr */    NULL, NULL,
  /* BR */      BR,   BR,
  /* BRLE */    BRLE, BRLE,
  /* BRLT */    BRLT, BRLT,
  /* BREQ */    BREQ, BREQ,
  /* BRNE */    BRNE, BRNE,
  /* BRGE */    BRGE, BRGE,
  /* BRGT */    BRGT, BRGT,
  /* BRV */     BRV, BRV,
  /* BRC */     BRC, BRC,
  /* CALL */    CALL, CALL,
  /* NOPn */    NULL, NULL,
  /* NOP */     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  /* DECI */    DECI, DECI, DECI, DECI, DECI, DECI, DECI, DECI,
  /* DECO */    DECO, DECO, DECO, DECO, DECO, DECO, DECO, DECO,
  /* HEXO */    HEXO, HEXO, HEXO, HEXO, HEXO, HEXO, HEXO, HEXO,
  /* STRO */    STRO, STRO, STRO, STRO, STRO, STRO, STRO, STRO,
  /* ADDSP */   ADDSP, ADDSP, ADDSP, ADDSP, ADDSP, ADDSP, ADDSP,
  /* SUBSP */   SUBSP, SUBSP, SUBSP, SUBSP, SUBSP, SUBSP, SUBSP,
  /* ADDr */    ADDr, ADDr, ADDr, ADDr, ADDr, ADDr, ADDr, ADDr,
                ADDr, ADDr, ADDr, ADDr, ADDr, ADDr, ADDr, ADDr,
  /* SUBr */    SUBr, SUBr, SUBr, SUBr, SUBr, SUBr, SUBr, SUBr,
                SUBr, SUBr, SUBr, SUBr, SUBr, SUBr, SUBr, SUBr,
  /* ANDr */    ANDr, ANDr, ANDr, ANDr, ANDr, ANDr, ANDr, ANDr,
                ANDr, ANDr, ANDr, ANDr, ANDr, ANDr, ANDr, ANDr,
  /* ORr */     ORr, ORr, ORr, ORr, ORr, ORr, ORr, ORr,
                ORr, ORr, ORr, ORr, ORr, ORr, ORr, ORr,
  /* CPWr */    CPWr, CPWr, CPWr, CPWr, CPWr, CPWr, CPWr, CPWr,
                CPWr, CPWr, CPWr, CPWr, CPWr, CPWr, CPWr, CPWr,
  /* CPBr */    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  /* LDWr */    LDWr, LDWr, LDWr, LDWr, LDWr, LDWr, LDWr, LDWr,
                LDWr, LDWr, LDWr, LDWr, LDWr, LDWr, LDWr, LDWr,
  /* LDBr */    LDBr, LDBr, LDBr, LDBr, LDBr, LDBr, LDBr, LDBr,
                LDBr, LDBr, LDBr, LDBr, LDBr, LDBr, LDBr, LDBr,
  /* STWr */    STWr, STWr, STWr, STWr, STWr, STWr, STWr, STWr,
                STWr, STWr, STWr, STWr, STWr, STWr, STWr, STWr,
  /* STBr */    STBr, STBr, STBr, STBr, STBr, STBr, STBr, STBr,
                STBr, STBr, STBr, STBr, STBr, STBr, STBr, STBr
};

#endif /* OPS_H */
