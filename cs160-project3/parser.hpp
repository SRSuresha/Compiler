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
     MAIN = 258,
     BOOLEAN = 259,
     CHAR = 260,
     INTEGER = 261,
     STRING = 262,
     INTPTR = 263,
     CHARPTR = 264,
     IF = 265,
     ELSE = 266,
     WHILE = 267,
     VAR = 268,
     PROCEDURE = 269,
     RETURN = 270,
     ID = 271,
     NULLX = 272,
     NUMBER = 273,
     AND = 274,
     OR = 275,
     STRINGV = 276,
     CHARV = 277,
     TRUE = 278,
     FALSE = 279,
     EQ = 280,
     GE = 281,
     LE = 282,
     NEQ = 283,
     Dummy = 284
   };
#endif
/* Tokens.  */
#define MAIN 258
#define BOOLEAN 259
#define CHAR 260
#define INTEGER 261
#define STRING 262
#define INTPTR 263
#define CHARPTR 264
#define IF 265
#define ELSE 266
#define WHILE 267
#define VAR 268
#define PROCEDURE 269
#define RETURN 270
#define ID 271
#define NULLX 272
#define NUMBER 273
#define AND 274
#define OR 275
#define STRINGV 276
#define CHARV 277
#define TRUE 278
#define FALSE 279
#define EQ 280
#define GE 281
#define LE 282
#define NEQ 283
#define Dummy 284




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

