%option yylineno
%pointer

%{
    #include <cstdlib>
    #include <cstring>
    #include "ast.hpp"
    #include "parser.hpp"

     void yyerror(const char *);
%}



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
[\"].*[\"]			{yylval.u_base_charptr = strdup(yytext);  return STRINGV;}
[\'].[\']			{yylval.u_base_int=(int)(*yytext);   return CHARV;}

[\/\=\>\<\-\!\+\*\&\^\;\:\,\|\{\}\(\)\[\]]  	{return *yytext; }

0[0-7]* 				{
								yylval.u_base_int = strtol(yytext,NULL,8);
								return NUMBER;
							}

0X[0-9A-F][0-9A-F]* 		{yylval.u_base_int=strtol(yytext,NULL,16); return NUMBER;}
0x[0-9A-F][0-9A-F]* 		{yylval.u_base_int=strtol(yytext,NULL,16); return NUMBER;}
0|[1-9][0-9]*         		{yylval.u_base_int=strtol(yytext,NULL,10); return NUMBER; }
0[0-1]*b				{yylval.u_base_int=strtol(yytext,NULL,2);  return NUMBER; }

[a-zA-Z][0-9a-zA-Z_]*		{yylval.u_base_charptr = strdup(yytext); return ID;}


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

<<EOF>> 					{yyterminate();}
.                   yyerror((char *) "invalid character");


%%

/** You shall not pass!
 *  You should not  have to do or edit anything past this.
 */

int yywrap(void) {
    return 1;
}
