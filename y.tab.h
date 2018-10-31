/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     STOP = 258,
     RET = 259,
     RETTR = 260,
     MOVSPA = 261,
     MOVFLGA = 262,
     MOVAFLG = 263,
     NOTr = 264,
     NEGr = 265,
     ASLr = 266,
     ASRr = 267,
     ROLr = 268,
     RORr = 269,
     BR = 270,
     BRLE = 271,
     BRLT = 272,
     BREQ = 273,
     BRNE = 274,
     BRGE = 275,
     BRGT = 276,
     BRV = 277,
     BRC = 278,
     CALL = 279,
     NOPn = 280,
     NOP = 281,
     DECI = 282,
     DECO = 283,
     HEXO = 284,
     STRO = 285,
     ADDSP = 286,
     SUBSP = 287,
     ADDr = 288,
     SUBr = 289,
     ANDr = 290,
     ORr = 291,
     CPWr = 292,
     CPBr = 293,
     LDWr = 294,
     LDBr = 295,
     STWr = 296,
     STBr = 297,
     BYTE = 298
   };
#endif
/* Tokens.  */
#define STOP 258
#define RET 259
#define RETTR 260
#define MOVSPA 261
#define MOVFLGA 262
#define MOVAFLG 263
#define NOTr 264
#define NEGr 265
#define ASLr 266
#define ASRr 267
#define ROLr 268
#define RORr 269
#define BR 270
#define BRLE 271
#define BRLT 272
#define BREQ 273
#define BRNE 274
#define BRGE 275
#define BRGT 276
#define BRV 277
#define BRC 278
#define CALL 279
#define NOPn 280
#define NOP 281
#define DECI 282
#define DECO 283
#define HEXO 284
#define STRO 285
#define ADDSP 286
#define SUBSP 287
#define ADDr 288
#define SUBr 289
#define ANDr 290
#define ORr 291
#define CPWr 292
#define CPBr 293
#define LDWr 294
#define LDBr 295
#define STWr 296
#define STBr 297
#define BYTE 298




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

