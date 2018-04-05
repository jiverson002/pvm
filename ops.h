#ifndef OPS_H
#define OPS_H 1

#include "os.h"
#include "types.h"
#include "vm.h"

#define LDW(idx) (((word)MEM[(idx)] << 8) | (word)MEM[(idx) + 1])
#define STW(idx, w) (MEM[idx + 0] = (w) >> 8, MEM[idx + 1] = (w) & 0x00ff)

static inline byte get_reg(byte in_spec) {
  if (0x06 <= in_spec && in_spec <= 0x11) {
    return in_spec & 0x01;
  } else {
    assert(in_spec >= 0x60);
    return (in_spec & 0x08) >> 3;
  }
}

static inline word get_addr(byte in_spec, word op_spec) {
  word op_addr;

  switch (in_spec & 0x03) {
    case 0x01:  /* direct */
      op_addr = op_spec;
      break;
    case 0x02:  /* indirect */
      op_addr = LDW(op_spec);
      break;
    case 0x03:  /* stack-relative */
      op_addr = SP + op_spec;
      break;
    case 0x04:  /* stack-relative deferred */
      op_addr = LDW(SP + op_spec);
      break;
    case 0x05:  /* indexed */
      op_addr = op_spec + X;
      break;
    case 0x06:  /* stack-indexed */
      op_addr = SP + op_spec + X;
      break;
    case 0x07:  /* stack-deferred indexed */
      op_addr = LDW(SP + op_spec) + X;
      break;
      break;
    default:    /* error */
      assert(0);
      break;
  }

  return op_addr;
}

static inline word get_oprnd(byte in_spec, word op_spec) {
  word oprnd, op_addr;

  if (in_spec <= 0x25) {
    switch (in_spec & 0x01) {
      case 0x00:  /* immediate */
        oprnd = op_spec;
        break;
      case 0x01:  /* indexed */
        op_addr = op_spec + X;
        oprnd   = LDW(op_spec + X);
        break;
    }
  } else {
    switch (in_spec & 0x03) {
      case 0x00:  /* immediate */
        oprnd = op_spec;
        break;
      case 0x01:  /* direct */
      case 0x02:  /* indirect */
      case 0x03:  /* stack-relative */
      case 0x04:  /* stack-relative deferred */
      case 0x05:  /* indexed */
      case 0x06:  /* stack-indexed */
      case 0x07:  /* stack-deferred indexed */
        op_addr = get_addr(in_spec, op_spec);
        oprnd   = LDW(op_addr);
        break;
    }
  }

  return oprnd;
}

static void RET(byte in_spec, word op_spec) {
  PC = LDW(SP);
  SP += 2;

  (void)in_spec;
  (void)op_spec;
}

static void MOVSPA(byte in_spec, word op_spec) {
  A = SP;

  (void)in_spec;
  (void)op_spec;
}

static void MOVFLGA(byte in_spec, word op_spec) {
  A &= 0xfff0; /* clear A<12..15> */
  A |= NZVC;

  (void)in_spec;
  (void)op_spec;
}

static void MOVAFLG(byte in_spec, word op_spec) {
  NZVC = A & 0x000f;

  (void)in_spec;
  (void)op_spec;
}

static void NOTr(byte in_spec, word op_spec) {
  word w;

  switch (get_reg(in_spec)) {
    case 0x00:  /* accumulator */
      w = A = ~A;
      break;
    case 0x01:  /* index */
      w = X = ~X;
      break;
  }

  NZVC &= 0x03;               /* clear all but NZ */
  NZVC |= (w & 0x8000) >> 4;  /* N */
  NZVC |= (w == 0) << 2;      /* Z */

  (void)in_spec;
  (void)op_spec;
}

static void NEGr(byte in_spec, word op_spec) {
  word w;
  sword s;

  switch (get_reg(in_spec)) {
    case 0x00:  /* accumulator */
      s = A;
      w = A = -A;
      break;
    case 0x01:  /* index */
      s = X;
      w = X = -X;
      break;
  }

  NZVC &= 0x01;               /* clear all but NZV */
  NZVC |= (w & 0x8000) >> 4;  /* N */
  NZVC |= (w == 0) << 2;      /* Z */
  NZVC |= (s == -32768) << 1; /* V */

  (void)in_spec;
  (void)op_spec;
}

static void BR(byte in_spec, word op_spec) {
  PC = get_oprnd(in_spec, op_spec);
}

static void BRLE(byte in_spec, word op_spec) {
  if (NZVC & 0x0c) {
    PC = get_oprnd(in_spec, op_spec);
  }
}

static void BRLT(byte in_spec, word op_spec) {
  if (NZVC & 0x08) {
    PC = get_oprnd(in_spec, op_spec);
  }
}

static void BREQ(byte in_spec, word op_spec) {
  if (NZVC & 0x04) {
    PC = get_oprnd(in_spec, op_spec);
  }
}

static void BRNE(byte in_spec, word op_spec) {
  if (!(NZVC & 0x04)) {
    PC = get_oprnd(in_spec, op_spec);
  }
}

static void BRGE(byte in_spec, word op_spec) {
  if (!(NZVC & 0x0c)) {
    PC = get_oprnd(in_spec, op_spec);
  }
}

static void BRGT(byte in_spec, word op_spec) {
  if (!(NZVC & 0x08)) {
    PC = get_oprnd(in_spec, op_spec);
  }
}

static void BRV(byte in_spec, word op_spec) {
  if (NZVC & 0x02) {
    PC = get_oprnd(in_spec, op_spec);
  }
}

static void BRC(byte in_spec, word op_spec) {
  if (NZVC & 0x01) {
    PC = get_oprnd(in_spec, op_spec);
  }
}

static void CALL(byte in_spec, word op_spec) {
  SP -= 2;
  STW(SP, PC);
  PC = get_oprnd(in_spec, op_spec);
}

static void LDWr(byte in_spec, word op_spec) {
  word oprnd = get_oprnd(in_spec, op_spec);

  switch (get_reg(in_spec)) {
    case 0x00:  /* accumulator */
      A = oprnd;
      break;
    case 0x01:  /* index */
      X = oprnd;
      break;
  }

  NZVC &= 0x03;                   /* clear all but NZ */
  NZVC |= (oprnd & 0x8000) >> 4;  /* N */
  NZVC |= (oprnd == 0) << 2;      /* Z */
}

static void LDBr(byte in_spec, word op_spec) {
  byte b;
  word oprnd, op_addr;

  switch (in_spec | 0x08) {
    case 0xd8: /* immediate */
      oprnd = get_oprnd(in_spec, op_spec);
      break;
    case 0xd9:  /* direct */
    case 0xda:  /* indirect */
    case 0xdb:  /* stack-relative */
    case 0xdc:  /* stack-relative deferred */
    case 0xdd:  /* indexed */
    case 0xde:  /* stack-indexed */
    case 0xdf:  /* stack-deferred indexed */
      op_addr = get_addr(in_spec, op_spec);
      if (charIn == op_addr) {
        scanf("%c", &b);
        oprnd = b;
      } else {
        /* shift b/c two bytes were loaded and the high one is the one at the
         * desired address. */
        oprnd = get_oprnd(in_spec, op_spec) >> 8;
      }
      break;
  }

  switch (get_reg(in_spec)) {
    case 0x00:  /* accumulator */
      A = (A & 0xff00) | oprnd;
      break;
    case 0x01:  /* index */
      X = (X & 0xff00) | oprnd;
      break;
  }

  NZVC &= 0x03;               /* clear all but NZ */
                              /* N is 0 by definition of Pep/9 */
  NZVC |= (oprnd == 0) << 2;  /* Z */
}

static void STWr(byte in_spec, word op_spec) {
  word op_addr = get_addr(in_spec, op_spec);

  switch (get_reg(in_spec)) {
    case 0x00:  /* accumulator */
      STW(op_addr, A);
      break;
    case 0x01:  /* index */
      STW(op_addr, X);
      break;
  }
}

static void STBr(byte in_spec, word op_spec) {
  word op_addr = get_addr(in_spec, op_spec);

  if (charOut == op_addr) {
    switch (get_reg(in_spec)) {
      case 0x00:  /* accumulator */
        printf("%c", (byte)(A & 0x00ff));
        break;
      case 0x01:  /* index */
        printf("%c", (byte)(X & 0x00ff));
        break;
    }
  }
  else {
    switch (get_reg(in_spec)) {
      case 0x00:  /* accumulator */
        MEM[op_addr] = (byte)(A & 0x00ff);
        break;
      case 0x01:  /* index */
        MEM[op_addr] = (byte)(X & 0x00ff);
        break;
    }
  }
}

static void DECI(byte in_spec, word op_spec) {
  int i;
  word w;
  word op_addr = get_addr(in_spec, op_spec);

  scanf("%d", &i);
  w = i;

  STW(op_addr, w);

  NZVC &= 0x01;                 /* clear all but C */
  NZVC |= (w & 0x8000) >> 4;    /* N */
  NZVC |= (w == 0) << 2;        /* Z */
  NZVC |= (i != (sword)w) << 1; /* V */

  assert((i >= -32768 && i <= 32767) || (NZVC & 0x02));
  assert(((sword)w >= 0) || (NZVC & 0x08));
  assert(((sword)w != 0) || (NZVC & 0x04));
}

static void DECO(byte in_spec, word op_spec) {
  printf("%d", (sword)get_oprnd(in_spec, op_spec));
}

static void HEXO(byte in_spec, word op_spec) {
  printf("%.4X", get_oprnd(in_spec, op_spec));
}

static void STRO(byte in_spec, word op_spec) {
  word op_addr = get_addr(in_spec, op_spec);
  while ('\0' != MEM[op_addr]) {
    printf("%c", MEM[op_addr++]);
  }
}

static void ADDSP(byte in_spec, word op_spec) {
  /* FIXME need to modify NZVC bits */
  SP += get_oprnd(in_spec, op_spec);
}

static void SUBSP(byte in_spec, word op_spec) {
  /* FIXME need to modify NZVC bits */
  SP -= get_oprnd(in_spec, op_spec);
}

static void ADDr(byte in_spec, word op_spec) {
  word o, w, v;
  word oprnd = get_oprnd(in_spec, op_spec);

  switch (get_reg(in_spec)) {
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
  NZVC |= (w & 0x8000) >> 4;  /* N */
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
  /* SUBr */    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  /* ANDr */    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  /* ORr */     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  /* CPWr */    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
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
