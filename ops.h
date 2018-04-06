#ifndef OPS_H
#define OPS_H 1

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "os.h"
#include "types.h"
#include "vm.h"

#define LDW(idx) (((word)Mem[(idx)] << 8) | (word)Mem[(idx) + 1])
#define STW(idx, w) (Mem[idx + 0] = (w) >> 8, Mem[idx + 1] = (w) & 0x00ff)

static inline bool is_nonunary(byte in_spec) {
  return !(((in_spec) < 0x12) || (((in_spec) | 0x01) == 0x27));
}

static inline byte get_reg(byte inspec) {
  if (0x06 <= inspec && inspec <= 0x11) {
    return inspec & 0x01;
  } else {
    assert(inspec >= 0x60);
    return (inspec & 0x08) >> 3;
  }
}

static inline word get_addr(byte inspec, word opspec) {
  word op_addr;

  switch (inspec & 0x07) {
    case 0x01:  /* direct */
      op_addr = opspec;
      break;
    case 0x02:  /* indirect */
      op_addr = LDW(opspec);
      break;
    case 0x03:  /* stack-relative */
      op_addr = SP + opspec;
      break;
    case 0x04:  /* stack-relative deferred */
      op_addr = LDW(SP + opspec);
      break;
    case 0x05:  /* indexed */
      op_addr = opspec + X;
      break;
    case 0x06:  /* stack-indexed */
      op_addr = SP + opspec + X;
      break;
    case 0x07:  /* stack-deferred indexed */
      op_addr = LDW(SP + opspec) + X;
      break;
  }

  return op_addr;
}

static inline word get_oprnd(byte inspec, word opspec) {
  word oprnd, op_addr;

  switch (inspec & 0x01) {
    case 0x00:  /* immediate */
      oprnd = opspec;
      break;
    default:
      if (inspec <= 0x25) { /* indexed */
        op_addr = opspec + X;
      } else { /* direct, indirect, stack-relative, stack-relative deferred,
                  indexed, stack-indexed, stack-deferred indexed */
        op_addr = get_addr(inspec, opspec);
      }
      oprnd = LDW(op_addr);
      break;
  }

  return oprnd;
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
  word w;

  switch (get_reg(inspec)) {
    case 0x00:  /* accumulator */
      w = A = ~A;
      break;
    case 0x01:  /* index */
      w = X = ~X;
      break;
  }

  NZVC &= 0x03;               /* clear all but VC */
  NZVC |= (w & 0x8000) >> 12; /* N */
  NZVC |= (w == 0) << 2;      /* Z */

  (void)inspec;
  (void)opspec;
}

static void NEGr(byte inspec, word opspec) {
  word w;
  sword s;

  switch (get_reg(inspec)) {
    case 0x00:  /* accumulator */
      s = A;
      w = A = -A;
      break;
    case 0x01:  /* index */
      s = X;
      w = X = -X;
      break;
  }

  NZVC &= 0x01;               /* clear all but C */
  NZVC |= (w & 0x8000) >> 12; /* N */
  NZVC |= (w == 0) << 2;      /* Z */
  NZVC |= (s == -32768) << 1; /* V */

  (void)inspec;
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
  word oprnd = get_oprnd(inspec, opspec);

  switch (get_reg(inspec)) {
    case 0x00:  /* accumulator */
      A = oprnd;
      break;
    case 0x01:  /* index */
      X = oprnd;
      break;
  }

  NZVC &= 0x03;                   /* clear all but VC */
  NZVC |= (oprnd & 0x8000) >> 12; /* N */
  NZVC |= (oprnd == 0) << 2;      /* Z */
}

static void LDBr(byte inspec, word opspec) {
  byte b;
  word oprnd, op_addr;

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

  switch (get_reg(inspec)) {
    case 0x00:  /* accumulator */
      A = (A & 0xff00) | oprnd;
      break;
    case 0x01:  /* index */
      X = (X & 0xff00) | oprnd;
      break;
  }

  NZVC &= 0x03;               /* clear all but VC */
                              /* N is 0 by definition of Pep/9 */
  NZVC |= (oprnd == 0) << 2;  /* Z */
}

static void STWr(byte inspec, word opspec) {
  word op_addr = get_addr(inspec, opspec);

  switch (get_reg(inspec)) {
    case 0x00:  /* accumulator */
      STW(op_addr, A);
      break;
    case 0x01:  /* index */
      STW(op_addr, X);
      break;
  }
}

static void STBr(byte inspec, word opspec) {
  word op_addr = get_addr(inspec, opspec);

  if (charOut == op_addr) {
    switch (get_reg(inspec)) {
      case 0x00:  /* accumulator */
        printf("%c", (byte)(A & 0x00ff));
        break;
      case 0x01:  /* index */
        printf("%c", (byte)(X & 0x00ff));
        break;
    }
  }
  else {
    switch (get_reg(inspec)) {
      case 0x00:  /* accumulator */
        Mem[op_addr] = (byte)(A & 0x00ff);
        break;
      case 0x01:  /* index */
        Mem[op_addr] = (byte)(X & 0x00ff);
        break;
    }
  }
}

static void DECI(byte inspec, word opspec) {
  int i;
  word w;
  word op_addr = get_addr(inspec, opspec);

  scanf("%d", &i);
  w = i;

  STW(op_addr, w);

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
  word o, w, v;
  word oprnd = get_oprnd(inspec, opspec);

  switch (get_reg(inspec)) {
    case 0x00:  /* accumulator */
      o = A;
      w = A += oprnd;
      break;
    case 0x01:  /* index */
      o = X;
      w = X += oprnd;
      break;
  }

  /* Inspect the signs of the numbers and the sum. If you add numbers with
   * different signs, you cannot have an overflow. If you add two numbers with
   * the same sign and the result is not the same sign, then you have signed
   * overflow. */
  v = !((o & 0x8000) ^ (oprnd & 0x8000)) && ((o & 0x8000) ^ (w & 0x8000));

  NZVC = 0;                   /* clear all bits */
  NZVC |= (w & 0x8000) >> 12; /* N */
  NZVC |= (w == 0) << 2;      /* Z */
  NZVC |= v << 1;             /* V */
  NZVC |= (w < o);            /* C */
}

static void SUBr(byte inspec, word opspec) {
  word o, w, v;
  word oprnd = ~get_oprnd(inspec, opspec) + 1;

  switch (get_reg(inspec)) {
    case 0x00:  /* accumulator */
      o = A;
      w = A += oprnd;
      break;
    case 0x01:  /* index */
      o = X;
      w = X += oprnd;
      break;
  }

  /* Inspect the signs of the numbers and the sum. If you add numbers with
   * different signs, you cannot have an overflow. If you add two numbers with
   * the same sign and the result is not the same sign, then you have signed
   * overflow. */
  v = !((o & 0x8000) ^ (oprnd & 0x8000)) && ((o & 0x8000) ^ (w & 0x8000));

  NZVC = 0;                   /* clear all bits */
  NZVC |= (w & 0x8000) >> 12; /* N */
  NZVC |= (w == 0) << 2;      /* Z */
  NZVC |= v << 1;             /* V */
  NZVC |= (w < o);            /* C */
}

static void ANDr(byte inspec, word opspec) {
  word w;
  word oprnd = get_oprnd(inspec, opspec);

  switch (get_reg(inspec)) {
    case 0x00:  /* accumulator */
      w = A &= oprnd;
      break;
    case 0x01:  /* index */
      w = X &= oprnd;
      break;
  }

  NZVC &= 0x03;               /* clear all but VC */
  NZVC |= (w & 0x8000) >> 12; /* N */
  NZVC |= (w == 0) << 2;      /* Z */
}

static void ORr(byte inspec, word opspec) {
  word w;
  word oprnd = get_oprnd(inspec, opspec);

  switch (get_reg(inspec)) {
    case 0x00:  /* accumulator */
      w = A |= oprnd;
      break;
    case 0x01:  /* index */
      w = X |= oprnd;
      break;
  }

  NZVC &= 0x03;               /* clear all but VC */
  NZVC |= (w & 0x8000) >> 12; /* N */
  NZVC |= (w == 0) << 2;      /* Z */
}

static void CPWr(byte inspec, word opspec) {
  word o, w, v;
  word oprnd = ~get_oprnd(inspec, opspec) + 1;

  switch (get_reg(inspec)) {
    case 0x00:  /* accumulator */
      o = A;
      w = A + oprnd;
      break;
    case 0x01:  /* index */
      o = X;
      w = X + oprnd;
      break;
  }

  /* Inspect the signs of the numbers and the sum. If you add numbers with
   * different signs, you cannot have an overflow. If you add two numbers with
   * the same sign and the result is not the same sign, then you have signed
   * overflow. */
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
