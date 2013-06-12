/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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
     T = 258,
     F = 259,
     EFST = 260,
     EALL = 261,
     XPATH = 262,
     NOT = 263,
     AND = 264,
     OR = 265,
     IMPLIES = 266,
     SINCE = 267,
     ALWAYS = 268,
     BEFORE = 269,
     WITHIN = 270,
     DURING = 271,
     REPMAX = 272,
     REPSINCE = 273,
     REPLIM = 274,
     POEVNAME = 275,
     POPARAM = 276,
     DENYC = 277,
     DENYD = 278,
     LIMIT = 279,
     DAY = 280,
     HOUR = 281,
     MINUTE = 282,
     SECOND = 283,
     TIMESTEP = 284,
     NUMBER = 285,
     IDENT = 286,
     ERROR = 287
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 2068 of yacc.c  */
#line 17 "past_parser.y"

  int num;
  char *text;
  event_t event_p;



/* Line 2068 of yacc.c  */
#line 90 "past_parser.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


