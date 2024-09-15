%option noinput
%option nounput
%option yylineno
%option noyywrap

%{
    #include <stdio.h>
    #include "../src/tree.hpp"
    #include "sysy.tab.hh"

    void comment();
    int column = 0;
%}

D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			([Ee][+-]?{D}+)
P           ([Pp][+-]?{D}+)
FS			(f|F|l|L)
COMMENT1            "/*"[^*]*"*"+([^*/][^*]*"*"+)*"/"
COMMENT2            "//".*

%%
{COMMENT1}|{COMMENT2}   { }

"int"			{  yylval.tN =new treeNode("INT",0,yylineno);  		return(INT); }
"break"			{  yylval.tN =new treeNode("BREAK",0,yylineno); 		return(BREAK); }
"continue"		{  yylval.tN =new treeNode("CONTINUE",0,yylineno); 	return(CONTINUE); }
"do"			{  yylval.tN =new treeNode("DO",0,yylineno);			return(DO); }
"else"			{  yylval.tN =new treeNode("ELSE",0,yylineno);		return(ELSE); }
"if"			{  yylval.tN =new treeNode("IF",0,yylineno);			return(IF); }
"return"		{  yylval.tN =new treeNode("RETURN",0,yylineno);		return(RETURN); }
"void"			{  yylval.tN =new treeNode("VOID",0,yylineno);		return(VOID); }
"while"			{  yylval.tN =new treeNode("WHILE",0,yylineno);		return(WHILE); }

{L}({L}|{D})* 	{ yylval.tN =new treeNode(yytext,0,yylineno);	return(IDENTIFIER);/*IDENTIFIER*/ }

0[xX]{H}+	{  yylval.tN =new treeNode(yytext,0,yylineno);	return(CONSTANT_INT); /*16进制*/}
0[0-7]*	{  yylval.tN =new treeNode(yytext,0,yylineno);	return(CONSTANT_INT); /*8进制*/}
[1-9]{D}*		{  yylval.tN =new treeNode(yytext,0,yylineno);	return(CONSTANT_INT); /*10进制整数*/}
"0"				{  yylval.tN =new treeNode(yytext,0,yylineno);	return(CONSTANT_INT); /*10进制整数*/}


"&&"	{  yylval.tN =new treeNode("AND_OP",0,yylineno);			return(AND_OP); }
"||"	{  yylval.tN =new treeNode("OR_OP",0,yylineno);			return(OR_OP); }
"<="	{  yylval.tN =new treeNode("LE_OP",0,yylineno);			return(LE_OP); }
">="	{  yylval.tN =new treeNode("GE_OP",0,yylineno);			return(GE_OP); }
"=="	{  yylval.tN =new treeNode("EQ_OP",0,yylineno);			return(EQ_OP); }
"!="	{  yylval.tN =new treeNode("NE_OP",0,yylineno);			return(NE_OP); }
";"		{  yylval.tN =new treeNode(";",0,yylineno);	return(';'); }
","		{  yylval.tN =new treeNode(",",0,yylineno);	return(','); }
":"		{  yylval.tN =new treeNode(":",0,yylineno);	return(':'); }
"="		{  yylval.tN =new treeNode("=",0,yylineno);	return('='); }
"["		{  yylval.tN =new treeNode("[",0,yylineno);	return('['); }
"]"		{  yylval.tN =new treeNode("]",0,yylineno);	return(']'); }
"."		{  yylval.tN =new treeNode(".",0,yylineno);	return('.'); }
"!"		{  yylval.tN =new treeNode("!",0,yylineno);	return('!'); }
"-"		{  yylval.tN =new treeNode("-",0,yylineno);	return('-'); }
"+"		{  yylval.tN =new treeNode("+",0,yylineno);	return('+'); }
"*"		{  yylval.tN =new treeNode("*",0,yylineno);	return('*'); }
"/"		{  yylval.tN =new treeNode("/",0,yylineno);	return('/'); }
"%"		{  yylval.tN =new treeNode("%",0,yylineno);	return('%'); }
"<"		{  yylval.tN =new treeNode("<",0,yylineno);	return('<'); }
">"		{  yylval.tN =new treeNode(">",0,yylineno);	return('>'); }
"{"		{  yylval.tN =new treeNode("{",0,yylineno);	return('{'); }
"}"		{  yylval.tN =new treeNode("}",0,yylineno);	return('}'); }
"("		{  yylval.tN =new treeNode("(",0,yylineno);	return('('); }
")"		{  yylval.tN =new treeNode(")",0,yylineno);	return(')'); }

[\t\v\n\f ]		{ }
.			{printf("Unknown token!\n");return(Unknown);}
%%