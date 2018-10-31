H [0-7A-F]
W [ \n]
OpSpec {W}{H}{H}{W}{H}{H}
%option noyywrap
%{
#include <stdio.h>

#include "y.tab.h"
%}
%%
00                { return STOP; }
01                { return RET; }
02                { return RETTR; }
03                { return MOVSPA; }
04                { return MOVFLGA; }
05                { return MOVAFLG; }
0[67]             { return NOTr; }
0[89]             { return NEGr; }
0[AB]             { return ASLr; }
0[CD]             { return ASRr; }
0[EF]             { return ROLr; }
1[01]             { return RORr; }
1[23]{OpSpec}     { return BR; }
1[45]{OpSpec}     { return BRLE; }
1[67]{OpSpec}     { return BRLT; }
1[89]{OpSpec}     { return BREQ; }
1[AB]{OpSpec}     { return BRNE; }
1[CD]{OpSpec}     { return BRGE; }
1[EF]{OpSpec}     { return BRGT; }
2[01]{OpSpec}     { return BRV; }
2[23]{OpSpec}     { return BRC; }
2[45]{OpSpec}     { return CALL; }
2[67]{OpSpec}     { return NOPn; }
2[89A-F]{OpSpec}  { return NOP; }
3[0-7]{OpSpec}    { return DECI; }
3[89A-F]{OpSpec}  { return DECO; }
4[0-7]{OpSpec}    { return HEXO; }
4[89A-F]{OpSpec}  { return STRO; }
5[0-7]{OpSpec}    { return ADDSP; }
5[89A-F]{OpSpec}  { return SUBSP; }
6[0-7A-F]{OpSpec} { return ADDr; }
7[0-7A-F]{OpSpec} { return SUBr; }
8[0-7A-F]{OpSpec} { return ANDr; }
9[0-7A-F]{OpSpec} { return ORr; }
A[0-7A-F]{OpSpec} { return CPWr; }
B[0-7A-F]{OpSpec} { return CPBr; }
C[0-7A-F]{OpSpec} { return LDWr; }
D[0-7A-F]{OpSpec} { return LDBr; }
E[0-7A-F]{OpSpec} { return STWr; }
F[0-7A-F]{OpSpec} { return STBr; }
{H}{H}            { return BYTE; }

{W}+ { ; }
zz   { yyterminate(); }
.    { printf("INVALID '%c'\n", yytext[0]); }
%%
