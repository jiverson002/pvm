#include "pep9.h"

/******************************************************************************/
/* Machine types */
/******************************************************************************/
#include <stdint.h>

typedef uint8_t  byte;
typedef uint16_t word;
typedef int8_t   sbyte;
typedef int16_t  sword;

/******************************************************************************/
/* System vectors */
/******************************************************************************/
#define dot_burn  (0xffff)
#define osRAM     (dot_burn - 11)
#define wordTemp  (dot_burn - 9)
#define charIn    (dot_burn - 7)
#define charOut   (dot_burn - 5)
#define loader    (dot_burn - 3)
#define trap      (dot_burn - 1)
#define SP_INIT   (0xfb8f)

/******************************************************************************/
/* Virtual machine structure */
/******************************************************************************/
static struct {
  struct {
    byte nzvc;
    word a;
    word x;
    word pc;
    word sp;
    byte ir;
    word opspec;
  } cpu;

  byte mem[(1 << 16) + 1]; /* allocate one-extra byte to allow for loads to
                              always occur as words */
} vm;

#define NZVC   (vm.cpu.nzvc)
#define A      (vm.cpu.a)
#define X      (vm.cpu.x)
#define PC     (vm.cpu.pc)
#define SP     (vm.cpu.sp)
#define IR     (vm.cpu.ir)
#define OpSpec (vm.cpu.opspec)
#define Mem    (vm.mem)

/******************************************************************************/
/* ISA implementation */
/******************************************************************************/
#include <stdio.h>

#if __STDC_VERSION__ < 199901L
  #define inline /* define to nothing if pre-C99 */
#endif

static inline int is_nonunary(byte in_spec) {
  return !((in_spec < 0x12) || (0x27 == (in_spec | 0x01)));
}

static inline byte ldb(word idx) {
  return Mem[idx];
}

static inline word ldw(word idx) {
  return (word)((Mem[idx] << 8) | Mem[idx + 1]);
}

static inline void stb(word idx, byte b) {
  Mem[idx] = b;
}

static inline void stw(word idx, word w) {
  Mem[idx + 0] = (byte)(w >> 8);
  Mem[idx + 1] = (byte)(w & 0x00ff);
}

static inline word add(word a, word b) {
  byte v;
  word c;

  c = a;
  a += b;

  /* Inspect the signs of the numbers and the sum. If you add numbers with
   * different signs, you cannot have an overflow. If you add two numbers with
   * the same sign and the result is not the same sign, then you have signed
   * overflow. */
  v = !((c & 0x8000) ^ (b & 0x8000)) && ((c & 0x8000) ^ (a & 0x8000));

  NZVC = 0;                   /* clear all bits */
  NZVC |= (a >= 0x8000) << 3; /* N */
  NZVC |= (a == 0x0000) << 2; /* Z */
  NZVC |= v << 1;             /* V */
  NZVC |= a < c;              /* C */ /* TODO is this correct */

  return a;
}

static inline word *get_reg(byte ir) {
  return ir <= 0x11
    ? (0x00 == (ir & 0x01) ? &A : &X)
    : (0x00 == (ir & 0x08) ? &A : &X);
}

static inline word get_addr(byte ir, word opspec) {
  switch (ir & 0x07) {
    case 0x01:  /* direct */
      return opspec;
    case 0x02:  /* indirect */
      return ldw(opspec);
    case 0x03:  /* stack-relative */
      return SP + opspec;
    case 0x04:  /* stack-relative deferred */
      return ldw(SP + opspec);
    case 0x05:  /* indexed */
      return opspec + X;
    case 0x06:  /* stack-indexed */
      return SP + opspec + X;
    case 0x07:  /* stack-deferred indexed */
      return ldw(SP + opspec) + X;
    default:    /* error */
      return 0xffff;
  }
}

static inline word get_oprnd(byte ir, word opspec) {
  return ir <= 0x25
    ? (0x00 == (ir & 0x01) ? opspec : ldw(opspec + X))
    : (0x00 == (ir & 0x07) ? opspec : ldw(get_addr(ir, opspec)));
}

static inline void STOP(byte ir, word opspec) {
  (void)ir;
  (void)opspec;
}

static void RET(byte ir, word opspec) {
  PC = ldw(SP);
  SP += 2;

  (void)ir;
  (void)opspec;
}

static void MOVSPA(byte ir, word opspec) {
  A = SP;

  (void)ir;
  (void)opspec;
}

static void MOVFLGA(byte ir, word opspec) {
  A &= 0xff00; /* clear A<8..15> */
  A |= NZVC;

  (void)ir;
  (void)opspec;
}

static void MOVAFLG(byte ir, word opspec) {
  NZVC = A & 0x000f;

  (void)ir;
  (void)opspec;
}

static void NOTr(byte ir, word opspec) {
  word *r = get_reg(ir);

  *r = ~(*r);

  NZVC &= 0x03;                 /* clear all but VC */
  NZVC |= (*r >= 0x8000) << 3;  /* N */
  NZVC |= (*r == 0x0000) << 2;  /* Z */

  (void)opspec;
}

static void NEGr(byte ir, word opspec) {
  word *r = get_reg(ir);

  *r = -(*r);

  NZVC &= 0x01;                 /* clear all but C */
  NZVC |= (*r >= 0x8000) << 3;  /* N */
  NZVC |= (*r == 0x0000) << 2;  /* Z */
  NZVC |= (*r == -(*r));        /* V */

  (void)opspec;
}

static void ASLr(byte ir, word opspec) {
  byte v, c;
  word *r = get_reg(ir);

  /* check if r<0..1> is 01 or 10 */
  v = ((*r < 0x8000) && (*r >= 0x4000)) || ((*r >= 0x8000) && (*r < 0xc000));
  /* check if r<0> is 1 */
  c = *r >= 0x8000;

  *r <<= 1;

  NZVC = 0;                     /* clear all bits */
  NZVC |= (*r >= 0x8000) << 3;  /* N */
  NZVC |= (*r == 0x0000) << 2;  /* Z */
  NZVC |= v << 1;               /* Z */
  NZVC |= c;                    /* C */

  (void)opspec;
}

static void ASRr(byte ir, word opspec) {
  byte c;
  word *r = get_reg(ir);

  /* check if r<15> is 1  */
  c = *r & 0x0001;

  *r = (*r & 0x8000) | (*r >> 1);

  NZVC &= 0x02;                 /* clear all but V */
  NZVC |= (*r >= 0x8000) << 3;  /* N */
  NZVC |= (*r == 0x0000) << 2;  /* Z */
  NZVC |= c;                    /* C */

  (void)opspec;
}

static void ROLr(byte ir, word opspec) {
  byte c;
  word *r = get_reg(ir);

  /* check if r<0> is 1  */
  c = *r >= 0x8000;

  *r = (word)((*r << 1) | (NZVC & 0x0001));

  NZVC &= 0x0e;                 /* clear all but NZV */
  NZVC |= c;                    /* C */

  (void)opspec;
}

static void RORr(byte ir, word opspec) {
  byte c;
  word *r = get_reg(ir);

  /* check if r<15> is 1  */
  c = *r & 0x0001;

  *r = (word)(((NZVC & 0x0001) << 15) | (*r >> 1));

  NZVC &= 0x0e;                 /* clear all but NZV */
  NZVC |= c;                    /* C */

  (void)opspec;
}

static void BR(byte ir, word opspec) {
  PC = get_oprnd(ir, opspec);
}

static void BRLE(byte ir, word opspec) {
  PC = NZVC & 0x0c ? get_oprnd(ir, opspec) : PC;
}

static void BRLT(byte ir, word opspec) {
  PC = NZVC & 0x08 ? get_oprnd(ir, opspec) : PC;
}

static void BREQ(byte ir, word opspec) {
  PC = NZVC & 0x04 ? get_oprnd(ir, opspec) : PC;
}

static void BRNE(byte ir, word opspec) {
  PC = !(NZVC & 0x04) ? get_oprnd(ir, opspec) : PC;
}

static void BRGE(byte ir, word opspec) {
  PC = !(NZVC & 0x0c) ? get_oprnd(ir, opspec) : PC;
}

static void BRGT(byte ir, word opspec) {
  PC = !(NZVC & 0x08) ? get_oprnd(ir, opspec) : PC;
}

static void BRV(byte ir, word opspec) {
  PC = NZVC & 0x02 ? get_oprnd(ir, opspec) : PC;
}

static void BRC(byte ir, word opspec) {
  PC = NZVC & 0x01 ? get_oprnd(ir, opspec) : PC;
}

static void CALL(byte ir, word opspec) {
  SP -= 2;
  stw(SP, PC);
  PC = get_oprnd(ir, opspec);
}

static void LDWr(byte ir, word opspec) {
  word *r = get_reg(ir);

  *r = get_oprnd(ir, opspec);

  NZVC &= 0x03;                 /* clear all but VC */
  NZVC |= (*r >= 0x8000) << 3;  /* N */
  NZVC |= (*r == 0x0000) << 2;  /* Z */
}

static void LDBr(byte ir, word opspec) {
  byte oprnd;
  word *r = get_reg(ir);

  switch (ir | 0x08) {
    case 0xd8:  /* immediate */
      oprnd = (byte)get_oprnd(ir, opspec);
      break;
    default:    /* direct, indirect, stack-relative, stack-relative deferred,
                   indexed, stack-indexed, stack-deferred indexed */
      if (charIn == get_addr(ir, opspec)) {
        scanf("%c", &oprnd);
      } else {
        /* shift b/c two bytes were loaded and the high one is the one at the
         * desired address. */
        oprnd = (byte)(get_oprnd(ir, opspec) >> 8);
      }
      break;
  }

  *r = (*r & 0xff00) | oprnd;

  NZVC &= 0x03;                 /* clear all but VC */
                                /* N is 0 by definition of Pep/9 */
  NZVC |= (*r == 0x0000) << 2;  /* Z */
}

static void STWr(byte ir, word opspec) {
  stw(get_addr(ir, opspec), *get_reg(ir));
}

static void STBr(byte ir, word opspec) {
  byte b = *get_reg(ir) & 0x00ff;
  word op_addr = get_addr(ir, opspec);

  if (charOut == op_addr) {
    printf("%c", b);
  }
  else {
    stb(op_addr, b);
  }
}

static void DECI(byte ir, word opspec) {
  int i;
  word w;

  scanf("%d", &i);
  w = (word)i;

  stw(get_addr(ir, opspec), w);

  NZVC &= 0x01;                 /* clear all but C */
  NZVC |= (w >= 0x8000) << 3;   /* N */
  NZVC |= (w == 0x0000) << 2;   /* Z */
  NZVC |= (i != (sword)w) << 1; /* V */
}

static void DECO(byte ir, word opspec) {
  printf("%d", (sword)get_oprnd(ir, opspec));
}

static void HEXO(byte ir, word opspec) {
  printf("%.4X", get_oprnd(ir, opspec));
}

static void STRO(byte ir, word opspec) {
  word op_addr = get_addr(ir, opspec);
  while ('\0' != ldb(op_addr)) {
    printf("%c", ldb(op_addr++));
  }
}

static void ADDSP(byte ir, word opspec) {
  /* FIXME need to modify NZVC bits */
  SP += get_oprnd(ir, opspec);
}

static void SUBSP(byte ir, word opspec) {
  /* FIXME need to modify NZVC bits */
  SP -= get_oprnd(ir, opspec);
}

static void ADDr(byte ir, word opspec) {
  word *r = get_reg(ir);

  *r = add(*r, get_oprnd(ir, opspec));
}

static void SUBr(byte ir, word opspec) {
  word *r = get_reg(ir);

  *r = add(*r, -get_oprnd(ir, opspec));
}

static void ANDr(byte ir, word opspec) {
  word *r = get_reg(ir);

  *r &= get_oprnd(ir, opspec);;

  NZVC &= 0x03;                 /* clear all but VC */
  NZVC |= (*r >= 0x8000) << 3;  /* N */
  NZVC |= (*r == 0x0000) << 2;  /* Z */
}

static void ORr(byte ir, word opspec) {
  word *r = get_reg(ir);

  *r |= get_oprnd(ir, opspec);

  NZVC &= 0x03;                 /* clear all but VC */
  NZVC |= (*r >= 0x8000) << 3;  /* N */
  NZVC |= (*r == 0x0000) << 2;  /* Z */
}

static void CPWr(byte ir, word opspec) {
  (void)add(*get_reg(ir), get_oprnd(ir, opspec));
}

static void CPBr(byte ir, word opspec) {
  /* TODO is this correct? */
  (void)add((word)(*get_reg(ir) << 8), ~(get_oprnd(ir, opspec) & 0xff00) + 0x0100);
}

static void TRAP(byte ir, word opspec) {
  word temp = ldb(wordTemp);
  stb(temp - 1, IR);
  stw(temp - 3, SP);
  stw(temp - 5, PC);
  stw(temp - 7, X);
  stw(temp - 9, A);
  stw(temp - 10, NZVC);
  SP = temp - 10;
  PC = ldb(trap);

  (void)ir;
  (void)opspec;
}

static void (*ops[256])(byte, word) = {
  /* STOP */    STOP,
  /* RET */     RET,
  /* RETTR */   NULL,
  /* MOVSPA */  MOVSPA,
  /* MOVFLGA */ MOVFLGA,
  /* MOVAFLG */ MOVAFLG,
  /* NOTr */    NOTr, NOTr,
  /* NEGr */    NEGr, NEGr,
  /* ASLr */    ASLr, ASLr,
  /* ASRr */    ASRr, ASRr,
  /* ROLr */    ROLr, ROLr,
  /* RORr */    RORr, RORr,
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
  /* NOPn */    TRAP, TRAP,
  /* NOP */     TRAP, TRAP, TRAP, TRAP, TRAP, TRAP, TRAP, TRAP,
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
  /* CPBr */    CPBr, CPBr, CPBr, CPBr, CPBr, CPBr, CPBr, CPBr,
                CPBr, CPBr, CPBr, CPBr, CPBr, CPBr, CPBr, CPBr,
  /* LDWr */    LDWr, LDWr, LDWr, LDWr, LDWr, LDWr, LDWr, LDWr,
                LDWr, LDWr, LDWr, LDWr, LDWr, LDWr, LDWr, LDWr,
  /* LDBr */    LDBr, LDBr, LDBr, LDBr, LDBr, LDBr, LDBr, LDBr,
                LDBr, LDBr, LDBr, LDBr, LDBr, LDBr, LDBr, LDBr,
  /* STWr */    STWr, STWr, STWr, STWr, STWr, STWr, STWr, STWr,
                STWr, STWr, STWr, STWr, STWr, STWr, STWr, STWr,
  /* STBr */    STBr, STBr, STBr, STBr, STBr, STBr, STBr, STBr,
                STBr, STBr, STBr, STBr, STBr, STBr, STBr, STBr
};

static int init(void) {
  A  = 0x0000;
  X  = 0x0000;
  PC = 0x0000;
  SP = SP_INIT;
  IR = 0x00;
  OpSpec = 0x0000;

  return 0;
}

static int stbi(unsigned addr, char b) {
  if (addr > 0xffff)
    return -1;

  stb((word)addr, (byte)b);

  return 0;
}

static int step(void) {
  /* fetch instruction specifier */
  IR = ldb(PC);

  /* increment pc */
  PC += sizeof(byte);

  /* decode */
  /* NOOP */

  /* if non-unary */
  if (is_nonunary(IR)) {
    /* fetch operand specifier */
    OpSpec = ldw(PC);

    /* increment pc */
    PC += sizeof(word);
  }

  /* execute */
  ops[IR](IR, OpSpec);

  /* return 0 only if IR == STOP(0x0000) */
  return IR;
}

struct vm pep9 = { init, stbi, step };
