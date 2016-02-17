%option yylineno
%pointer

%{
    #include <cstdlib>
    #include <cstring>
    #include "ast.hpp"
    #include "parser.hpp"

     void yyerror(const char *); 
%}

/** WRITE ME:
 *  Put any definitions you need here. Add your own regular expressions
 *  below for ALL tokens defined in the manual. You can choose not to use
 *  definitions here.
 */


/** The example below is very simple, but it shows almost everything you will
 *  need to do for this first part. The syntax is as follows:
 *
 *  regular_expression      {  c code  }
 *
 * The lexer will try to match each regular expression. When it does it will
 * execute the corresponding C code.
*/

%%

"boolean"			{return BOOLEAN;}
"char"				{return CHAR;}
"integer"			{return INTEGER;}
"string"			{return STRING;}
"intptr"			{return INTPTR;}
"charptr"			{return CHARPTR;}
"if"  				{return IF;}
"else"				{return ELSE;}
"while"				{return WHILE;}
"var"				{return VAR;}
"procedure"			{return PROCEDURE;}
"return"			{return RETURN;}
"null"				{return NULLX;}
"true"				{return TRUE;}
"false"				{return FALSE;}
"&&"				{return AND;}
"=="				{return EQ;}
">="				{return GE;}
"<="				{return LE;}
"!="				{return NEQ;}
"||"				{return OR;}
[\"].*[\"]			{yylval = stl.dup(yytext); return STRINGV;}
[\'].[\']			{return CHARV;}
[\/\=\>\<\-\!\+\*\&\^\;\:\,\|\{\}\(\)\[\]]  	{return *yytext; }
[0-7][0-7]* 				{return NUMBER;}
"0X"[0-9A-F][0-9A-F]* 		{return NUMBER;}
[0-9][0-9]*         		{return NUMBER; }
[0-1][0-1]*"b"				{return NUMBER; }
[a-zA-Z][0-9a-zA-Z_]*		{return ID;}
[ \t\n]				; /* skip whitespace */
"(%" 						{
								char current = yyinput();
								char last = ' ';
								while(true){
									last = current;
									current = yyinput();
									if(last == '%' && current == ')'){
										break;
									}
									if(current == EOF)
										yyerror((char *) "comment not ended");
								}
							}


.                   yyerror((char *) "invalid character");

%%

/** You shall not pass!
 *  You should not  have to do or edit anything past this.
 */

int yywrap(void) {
    return 1;
}
