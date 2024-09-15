%{
#include <stdio.h>
#include <fstream>
#include <string>
#include "../src/tree.hpp"
using namespace std;

extern char *yytext;
extern int column;
extern treeNode* root;
extern int yylineno;

extern int yylex(void);
void yyerror(const char* s);
%}

%union{
    class treeNode* tN;
}

%token <tN> IDENTIFIER CONSTANT_INT
%token <tN> LE_OP GE_OP EQ_OP NE_OP
%token <tN> AND_OP OR_OP
%token <tN> TYPE_NAME

%token <tN> INT VOID Unknown

%token <tN> IF ELSE WHILE DO CONTINUE BREAK RETURN

%token <tN> ';' ',' ':' '=' '[' ']' '.' '!' '-' '+' '*' '/' '%' '<' '>' '{' '}' '(' ')'

%type <tN> CompUnit CompUnitMore Decl VarDecl multiVar VarDef AfterVarDef IdenList InitVal InitList 
%type <tN> FuncDef AfterFuncDef FuncFParams FuncList FuncFParam IntConstFunc 
%type <tN> Block enterBlock BlockList BlockItem Stmt Exp Cond LVal ExpList PrimaryExp UnaryExp UnaryOp 
%type <tN> FuncRParams ExpFunList MulExp MulMore MulOp AddExp AddOp AddMore 
%type <tN> RelExp EqExp LAndExp LAndMore LOrExp LOrMore


%%
CompUnit: 	 Decl CompUnitMore{root = new treeNode("CompUnit", 2, $1, $2);}
			|FuncDef CompUnitMore{root = new treeNode("CompUnit", 2, $1, $2);}
			|{root = NULL;}
			;
CompUnitMore:Decl CompUnitMore{$$ = new treeNode("CompUnitMore", 2, $1, $2);}
			|FuncDef CompUnitMore{$$ = new treeNode("CompUnitMore", 2, $1, $2);}
			| {$$ = NULL;}
			;

Decl:	   	VarDecl{$$ = new treeNode("Decl", 1, $1);}
			;


VarDecl: 	INT VarDef multiVar ';'{$$ = new treeNode("VarDecl", 3, $1, $2, $3);}
			;

multiVar: 	',' VarDef multiVar {$$ = new treeNode("multiVar", 2, $2, $3);}
			| {$$ = NULL;}
			;

VarDef: 	IDENTIFIER IdenList AfterVarDef{$$ = new treeNode("VarDef", 3, $1, $2, $3);}
			;

AfterVarDef:'=' InitVal{$$ = new treeNode("AfterVarDef", 1, $2);}
			| {$$ = NULL;}
			;

IdenList: 	'[' CONSTANT_INT ']' IdenList{$$ = new treeNode("array", 2, $2, $4);}
			| {$$ = NULL;}
			;

InitVal:    Exp {$$ = new treeNode("InitVal", 1, $1);}
			| '{' InitVal InitList '}'{$$ = new treeNode("initArrayList", 2, $2, $3);}
			|'{' '}' {$$ = new treeNode("InitVal", 0);}
			;

InitList:	',' InitVal InitList{$$ = new treeNode("ArrayList", 2, $2, $3);}
			| {$$ = NULL;}
			;

FuncDef:    INT IDENTIFIER '(' FuncFParams')' Block{$$ = new treeNode("FuncDef", 4, $1, $2, $4, $6);}
			|VOID IDENTIFIER '(' FuncFParams')' Block{$$ = new treeNode("FuncDef", 4, $1, $2, $4, $6);}

FuncFParams:FuncFParam FuncList {$$ = new treeNode("FuncParams", 2, $1, $2);}
			| {$$ = NULL;}
			;

FuncList:	',' FuncFParam FuncList{$$ = new treeNode("FuncParams", 2, $2, $3);}
			| {$$ = NULL;}
			;

FuncFParam:	INT IDENTIFIER '[' ']' IntConstFunc{$$ = new treeNode("FuncFParamArray", 3, $1, $2, $5);}
			| INT IDENTIFIER{$$ = new treeNode("FuncFParam",2, $1, $2);}
			;

IntConstFunc:'[' CONSTANT_INT ']' IntConstFunc{$$ = new treeNode("FuncFParamArrayList", 2, $2, $4);}
			| {$$ = NULL;}
			;

Block:      '{' enterBlock '}'{$$ = new treeNode("enterBlock", 1, $2);}
			| '{' '}' {$$ = new treeNode("enterBlock", 0);}
			;

enterBlock:  BlockItem BlockList{$$ = new treeNode("BlockList", 2, $1, $2);}
			;

BlockList:	BlockItem BlockList{$$ = new treeNode("BlockList", 2, $1, $2);}
			| {$$ = NULL;}
			;

BlockItem:  Decl {$$ = new treeNode("BlockItem", 1, $1);}
			| Stmt	{$$ = new treeNode("BlockItem", 1, $1);}
			;

Stmt:       LVal '=' Exp ';'	{$$ = new treeNode("AssignStmt", 2, $1, $3);}
			| Exp ';'{$$ = new treeNode("Stmt", 1, $1);}
			| ';' {$$ = new treeNode("Stmt", 0);}
			| Block{$$ = new treeNode("Stmt", 1, $1);}
			| IF '(' Cond ')' Stmt ELSE Stmt{$$ = new treeNode("IfStmt", 4, $3, $5, $6, $7);}
			| IF '(' Cond ')' Stmt{$$ = new treeNode("IfStmt", 2, $3, $5);}
			| WHILE '(' Cond ')' Stmt{$$ = new treeNode("WhileStmt", 2, $3, $5);}
			| RETURN Exp ';'{$$ = new treeNode("ReturnStmt", 2, $1, $2);}
			| RETURN ';'{$$ = new treeNode("ReturnStmt", 1, $1);}
			;

Exp:        AddExp{$$ = new treeNode("Exp", 1, $1);};

Cond:       LOrExp{$$ = new treeNode("Cond", 1, $1);};

LVal:       IDENTIFIER ExpList{$$ = new treeNode("LVal", 2, $1, $2);};

ExpList:	'[' Exp ']' ExpList{$$ = new treeNode("LValArray", 2, $2, $4);}
			| {$$ = NULL;}
			;

PrimaryExp: '(' Exp ')' {$$ = new treeNode("PrimaryExp", 1, $2);}
			| LVal {$$ = new treeNode("PrimaryLVal", 1, $1);}
			| CONSTANT_INT {$$ = new treeNode("ConstInt", 1, $1);}
			;

UnaryExp:   PrimaryExp {$$ = new treeNode("UnaryExp", 1, $1);}
			| IDENTIFIER '(' FuncRParams ')' {$$ = new treeNode("FuncCall", 2, $1, $3);}
			| IDENTIFIER '('  ')' {$$ = new treeNode("FuncCall", 1, $1);}
			| UnaryOp UnaryExp{$$ = new treeNode("UnaryExp", 2, $1, $2);}
			;

UnaryOp:    '+' {$$ = new treeNode("UnaryOp", 1, $1);}
			| '-' {$$ = new treeNode("UnaryOp", 1, $1);}
			| '!' {$$ = new treeNode("UnaryOp", 1, $1);}
			;

FuncRParams:Exp ExpFunList{$$ = new treeNode("FuncRParams", 2, $1, $2);}
			;

ExpFunList: ',' Exp ExpFunList{$$ = new treeNode("ExpFunList", 2, $2, $3);}
			| {$$ = NULL;}
			;

MulExp:     UnaryExp MulMore{$$ = new treeNode("MulExp", 2, $1, $2);}
			;

MulMore:	MulOp UnaryExp MulMore{$$ = new treeNode("MulMore", 3, $1, $2, $3);}
			| {$$ = NULL;}
			;

MulOp:	  	'*'{$$ = new treeNode("MulOp", 1, $1);}
			| '/'{$$ = new treeNode("MulOp", 1, $1);}
			| '%'{$$ = new treeNode("MulOp", 1, $1);}
			;

AddExp:     MulExp AddMore{$$ = new treeNode("AddExp", 2, $1, $2);}
			;

AddOp:	  	'+'{$$ = new treeNode("AddOp", 1, $1);}
			| '-'{$$ = new treeNode("AddOp", 1, $1);}
			;

AddMore:	AddOp MulExp AddMore{$$ = new treeNode("AddMore", 3, $1, $2, $3);}
			| {$$ = NULL;}
			;

RelExp:     AddExp {$$ = new treeNode("RelExp", 1, $1);}
			| AddExp '<' AddExp{$$ = new treeNode("RelExp", 3, $1, $2, $3);}
			| AddExp '>' AddExp{$$ = new treeNode("RelExp", 3, $1, $2, $3);}
			| AddExp LE_OP AddExp{$$ = new treeNode("RelExp", 3, $1, $2, $3);}
			| AddExp GE_OP AddExp{$$ = new treeNode("RelExp", 3, $1, $2, $3);}
			;

EqExp:      RelExp {$$ = new treeNode("EqExp", 1, $1);}
			| EqExp EQ_OP RelExp{$$ = new treeNode("EqExp", 3, $1, $2, $3);}
			| EqExp NE_OP RelExp{$$ = new treeNode("EqExp", 3, $1, $2, $3);}
			;

LAndExp:    EqExp LAndMore{$$ = new treeNode("LAndExp", 2, $1, $2);}

LAndMore:	AND_OP EqExp LAndMore{$$ = new treeNode("LAndMore", 3, $1, $2, $3);}
			|{$$ = NULL;}
			;

LOrExp:     LAndExp LOrMore{$$ = new treeNode("LOrExp", 2, $1, $2);}

LOrMore:	OR_OP LAndExp LOrMore{$$ = new treeNode("LOrMore", 3, $1, $2, $3);}
			|{$$ = NULL;}
			;


%%

void yyerror(char const *s)
{
    fflush(stdout);
	printf("\n%*s\n%*s\n", column, "^", column, s);
}

