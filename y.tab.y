%{
#include <stdio.h>
#include <stdlib.h>

int yylex(void);
void yyerror(char const *);
int yyparse(void);

void
yyerror(char const *str) {
  printf("error: %s\n", str);
}

int
main() {
  yyparse();

  return EXIT_SUCCESS;
}
%}

%token STOP RET RETTR MOVSPA MOVFLGA MOVAFLG NOTr NEGr ASLr ASRr ROLr RORr BR
%token BRLE BRLT BREQ BRNE BRGE BRGT BRV BRC CALL NOPn NOP DECI DECO HEXO STRO
%token ADDSP SUBSP ADDr SUBr ANDr ORr CPWr CPBr LDWr LDBr STWr STBr BYTE 
%%
/* */
prog
  : /* empty */
  | instruction prog
  ;

/* */
instruction
  : unary
  | non_unary
  ;

unary
  : STOP    { printf("STOP\n"); }
  | RET     { printf("RET\n"); }
  | RETTR   { printf("RETTR\n"); }
  | MOVSPA  { printf("MOVSPA\n"); }
  | MOVFLGA { printf("SMOVFLGA\n"); }
  | MOVAFLG { printf("MOVAFLG\n"); }
  | NOTr    { printf("NOTr\n"); }
  | NEGr    { printf("NEGr\n"); }
  | ASLr    { printf("ASLr\n"); }
  | ASRr    { printf("ASRr\n"); }
  | ROLr    { printf("ROLr\n"); }
  | RORr    { printf("RORr\n"); }
  | NOPn    { printf("NOPn\n"); }
  ;

non_unary
  : BR      { printf("BR\n"); }
  | BRLE    { printf("BRLE\n"); }
  | BRLT    { printf("BRLT\n"); }
  | BREQ    { printf("BREQ\n"); }
  | BRNE    { printf("BRNE\n"); }
  | BRGE    { printf("BRGE\n"); }
  | BRGT    { printf("BRGT\n"); }
  | BRV     { printf("BRV\n"); }
  | BRC     { printf("BRC\n"); }
  | CALL    { printf("CALL\n"); }
  | NOP     { printf("NOP\n"); }
  | DECI    { printf("DECI\n"); }
  | DECO    { printf("DECO\n"); }
  | HEXO    { printf("HEXO\n"); }
  | STRO    { printf("STRO\n"); }
  | ADDSP   { printf("ADDSP\n"); }
  | SUBSP   { printf("SUBSP\n"); }
  | ADDr    { printf("ADDr\n"); }
  | SUBr    { printf("SUBr\n"); }
  | ANDr    { printf("ANDr\n"); }
  | ORr     { printf("ORr\n"); }
  | CPWr    { printf("CPWr\n"); }
  | CPBr    { printf("CPBr\n"); }
  | LDWr    { printf("LDWr\n"); }
  | LDBr    { printf("LDBr\n"); }
  | STWr    { printf("STWr\n"); }
  | STBr    { printf("STBr\n"); }
  | BYTE    { printf("BYTE\n"); }
  ;
