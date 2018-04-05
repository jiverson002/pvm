#ifndef OPS_H
#define OPS_H 1

#include "os.h"
#include "types.h"
#include "vm.h"

#define LDW(idx) (((word)MEM[(idx)] << 8) | (word)MEM[(idx) + 1])

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

static inline void BR(byte in_spec, word op_spec) {
  word oprnd = get_oprnd(in_spec, op_spec);
  PC = oprnd;
}

static inline void LDWr(byte in_spec, word op_spec) {
  word oprnd = get_oprnd(in_spec, op_spec);

  switch (get_reg(in_spec)) {
    case 0x00:  /* accumulator */
      A = oprnd;
      break;
    case 0x01:  /* index */
      X = oprnd;
      break;
  }

  NZVC = (NZVC & 0x03) | ((oprnd & 0x80) >> 4) | ((oprnd == 0) << 2);
}

static inline void LDBr(byte in_spec, word op_spec) {
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

  NZVC = (NZVC & 0x03) | ((oprnd == 0) << 2);
}

static inline void STWr(byte in_spec, word op_spec) {
  word op_addr = get_addr(in_spec, op_spec);

  switch (get_reg(in_spec)) {
    case 0x00:  /* accumulator */
      MEM[op_addr + 0] = A >> 8;
      MEM[op_addr + 1] = A & 0x00ff;
      break;
    case 0x01:  /* index */
      MEM[op_addr + 0] = X >> 8;
      MEM[op_addr + 1] = X & 0x00ff;
      break;
  }
}

static inline void STBr(byte in_spec, word op_spec) {
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

static inline void DECO(byte in_spec, word op_spec) {
  word oprnd  = get_oprnd(in_spec, op_spec);
  sword value = 0 | oprnd;
  printf("%d", value);
}

static inline void ADDSP(byte in_spec, word op_spec) {
  word oprnd = get_oprnd(in_spec, op_spec);
  SP += oprnd;
}

static inline void SUBSP(byte in_spec, word op_spec) {
  word oprnd = get_oprnd(in_spec, op_spec);
  SP -= oprnd;
}

static void (*ops[256])(byte, word) = {
  /* STOP */    NULL, 
  /* RET */     NULL, 
  /* RETTR */   NULL, 
  /* MOVSPA */  NULL, 
  /* MOVFLGA */ NULL, 
  /* MOVAFLG */ NULL, 
  /* NOTr */    NULL, NULL,
  /* NEGr */    NULL, NULL,
  /* ASLr */    NULL, NULL,
  /* ASRr */    NULL, NULL,
  /* ROLr */    NULL, NULL,
  /* RORr */    NULL, NULL,
  /* BR */      BR,   BR,
  /* BRLE */    NULL, NULL,
  /* BRLT */    NULL, NULL,
  /* BREQ */    NULL, NULL,
  /* BRNE */    NULL, NULL,
  /* BRGE */    NULL, NULL,
  /* BRGT */    NULL, NULL,
  /* BRV */     NULL, NULL,
  /* BRC */     NULL, NULL,
  /* CALL */    NULL, NULL,
  /* NOPn */    NULL, NULL,
  /* NOP */     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  /* DECI */    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  /* DECO */    DECO, DECO, DECO, DECO, DECO, DECO, DECO, DECO,
  /* HEXO */    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  /* STRO */    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  /* ADDSP */   ADDSP, ADDSP, ADDSP, ADDSP, ADDSP, ADDSP, ADDSP,
  /* SUBSP */   SUBSP, SUBSP, SUBSP, SUBSP, SUBSP, SUBSP, SUBSP,
  /* ADDr */    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
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
