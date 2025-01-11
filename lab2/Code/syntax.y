%locations
%define parse.error verbose
%{
#include "tree.h"
typedef unsigned char uint8_t;
int yylex();
int yyerror(const char* msg, ...);

uint8_t right = 1;

int ignore_line = 0;
int syserr = 0;
int myerr = 0;
Node* root;
%}

%union {
    Node* node;
}

%right <node> ASSIGNOP
%left <node> OR
%left <node> AND
%left <node> RELOP
%left <node> PLUS MINUS
%left <node> STAR DIV
%right <node> NOT
%left <node> LP RP LB RB DOT

%token <node> INT //OCT HEX
%token <node> FLOAT
%token <node> ID
%token <node> STRUCT RETURN IF ELSE WHILE
%token <node> TYPE
%token <node> SEMI COMMA
%token <node> LC RC

%type <node> Program ExtDefList ExtDef Specifier ExtDecList FunDec CompSt VarDec StructSpecifier OptTag DefList Tag VarList ParamDec StmtList Exp Stmt Def DecList Dec Args

%%
Program : ExtDefList {$$ = newnewNode("Program", 1, Nter, @1.first_line); addchild($$, $1); root = $$;}
    ;
ExtDefList :            {$$ = newnewNode("ExtDefList", 1, Null, -1);} 
    | ExtDef ExtDefList {$$ = newnewNode("ExtDefList", 2, Nter, @1.first_line); addchild($$, $1); addchild($$, $2);} 
    ;
ExtDef : Specifier ExtDecList SEMI  {$$ = newnewNode("ExtDef", 1, Nter, @1.first_line); addchild($$, $1); addchild($$, $2); addchild($$, $3);}
    | Specifier SEMI                {$$ = newnewNode("ExtDef", 2, Nter, @1.first_line); addchild($$, $1); addchild($$, $2);}
    | Specifier FunDec CompSt       {$$ = newnewNode("ExtDef", 3, Nter, @1.first_line); addchild($$, $1); addchild($$, $2); addchild($$, $3);}
    | Specifier FunDec SEMI         {$$ = newnewNode("ExtDef", 4, Nter, @1.first_line); addchild($$, $1); addchild($$, $2); addchild($$, $3);}
    /* | error SEMI                    {yyerror("Missing type or Wrong type", @1.first_line); yyerrok;} */
    /* | Specifier error               {yyerror("Possibly missing \";\" at this or last line", @2.first_line); yyerrok;} */
    ;
ExtDecList : VarDec             {$$ = newnewNode("ExtDecList", 1, Nter, @1.first_line); addchild($$, $1);}
    | VarDec COMMA ExtDecList   {$$ = newnewNode("ExtDecList", 2, Nter, @1.first_line); addchild($$, $1); addchild($$, $2); addchild($$, $3);}
    /* | VarDec error ExtDecList   {yyerror("Missing \",\"", @2.first_line); yyerrok;} */
    /* | VarDec error              {yyerror("Possibly missing \";\" at this or last line", @2.first_line); yyerrok;} */
    ;

// Specifiers
Specifier : TYPE        {$$ = newnewNode("Specifier", 1, Nter, @1.first_line); addchild($$, $1);}
    | StructSpecifier   {$$ = newnewNode("Specifier", 2, Nter, @1.first_line); addchild($$, $1);}
    ;
StructSpecifier : STRUCT OptTag LC DefList RC   {$$ = newnewNode("StructSpecifier",1, Nter, @1.first_line); addchild($$, $1); addchild($$, $2); addchild($$, $3);addchild($$, $4); addchild($$, $5); }
    | STRUCT Tag                                {$$ = newnewNode("StructSpecifier",2, Nter, @1.first_line); addchild($$, $1); addchild($$, $2);}
    /* | STRUCT OptTag LC error RC                 {yyerror("Wrong struct definition", @2.first_line); yyerrok;} */
    ;
OptTag :    {$$ = newnewNode("OptTag", 1, Null, -1);}
    | ID    {$$ = newnewNode("OptTag", 2, Nter, @1.first_line); addchild($$, $1);}
    ;
Tag : ID    {$$ = newnewNode("Tag", 1, Nter, @1.first_line); addchild($$, $1);}
    ;

// Declarators
VarDec : ID                 {$$ = newnewNode("VarDec", 1, Nter, @1.first_line); addchild($$, $1);} 
    | VarDec LB INT RB      {$$ = newnewNode("VarDec", 2, Nter, @1.first_line); addchild($$, $1); addchild($$, $2); addchild($$, $3);addchild($$, $4);}
    /* | VarDec LB error RB    {yyerror("Missing \"]\"", @3.first_line); yyerrok;} */
    /* | VarDec LB error       {yyerror("Missing \"]\"", @3.first_line); yyerrok;} */
    ;
FunDec : ID LP VarList RP   {$$ = newnewNode("FunDec", 1, Nter, @1.first_line); addchild($$, $1); addchild($$, $2); addchild($$, $3);addchild($$, $4);}  
    | ID LP RP              {$$ = newnewNode("FunDec", 2, Nter, @1.first_line); addchild($$, $1); addchild($$, $2); addchild($$, $3);}
    /* | ID LP error RP        {yyerror("Wrong argument(s)", @3.first_line); yyerrok;} */
    /* | error LP VarList RP   {yyerror("Wrong function name", @1.first_line); yyerrok;} */
    ;
VarList : ParamDec COMMA VarList    {$$ = newnewNode("VarList", 1, Nter, @1.first_line); addchild($$, $1); addchild($$, $2); addchild($$, $3);}
    | ParamDec                      {$$ = newnewNode("VarList", 2, Nter, @1.first_line); addchild($$, $1);}
    ;
ParamDec : Specifier VarDec {$$ = newnewNode("ParamDec", 1, Nter, @1.first_line); addchild($$, $1); addchild($$, $2);}
    ;

// Statements
CompSt : LC DefList StmtList RC {$$ = newnewNode("CompSt", 1, Nter, @1.first_line); addchild($$, $1); addchild($$, $2); addchild($$, $3);addchild($$, $4);}
    /* | LC error RC                  {yyerror("Wrong statement(s)", @1.first_line); yyerrok;} */
    ;
StmtList :          {$$ = newnewNode("StmtList", 1, Null, -1);}
    | Stmt StmtList {$$ = newnewNode ("StmtList", 2, Nter, @1.first_line); addchild($$, $1); addchild($$, $2); }
    ;
Stmt : Exp SEMI                     {$$ = newnewNode("Stmt", 1, Nter, @1.first_line);addchild($$, $1); addchild($$, $2); }   // 语句
    | CompSt                        {$$ = newnewNode("Stmt", 2, Nter, @1.first_line);addchild($$, $1);}
    | RETURN Exp SEMI               {$$ = newnewNode("Stmt", 3, Nter, @1.first_line);addchild($$, $1); addchild($$, $2); addchild($$, $3);}
    | IF LP Exp RP Stmt             {$$ = newnewNode("Stmt", 4, Nter, @1.first_line);addchild($$, $1); addchild($$, $2); addchild($$, $3); addchild($$, $4); addchild($$, $5);}
    | IF LP Exp RP Stmt ELSE Stmt   {$$ = newnewNode("Stmt", 5, Nter, @1.first_line);addchild($$, $1); addchild($$, $2); addchild($$, $3);addchild($$, $4); addchild($$, $5); addchild($$, $6);addchild($$, $7); }
    | WHILE LP Exp RP Stmt          {$$ = newnewNode("Stmt", 6, Nter, @1.first_line);addchild($$, $1); addchild($$, $2); addchild($$, $3); addchild($$, $4); addchild($$, $5);}
    | error SEMI                    {yyerror("Wrong expression or Definition after statement", @1.first_line); yyerrok;}
    | Exp error                     {if (@1.first_line != ignore_line) {yyerror("Possibly missing \";\" at this or last line", @1.first_line); yyerrok;} yyerrok;}
    /* | RETURN Exp error              {yyerror("Possibly missing \";\" at this or last line", @3.first_line); yyerrok;} */
    /* | RETURN error SEMI             {yyerror("Wrong return value", @2.first_line);} */
    ;

DefList :           {{$$ = newnewNode("DefList", 1, Null, -1);}}
    | Def DefList   {$$ = newnewNode("DefList", 2, Nter, @1.first_line);addchild($$, $1); addchild($$, $2);}
    ;
Def : Specifier DecList SEMI    {$$ = newnewNode("Def", 1, Nter, @1.first_line); addchild($$, $1); addchild($$, $2); addchild($$, $3); }
    /* | Specifier error SEMI      {yyerror("Possibly missing ID", @2.first_line);} */
    ;
DecList : Dec           {$$ = newnewNode("DecList", 1, Nter, @1.first_line);addchild($$, $1);}
    | Dec COMMA DecList {$$ = newnewNode("DecList", 2, Nter, @1.first_line); addchild($$, $1); addchild($$, $2); addchild($$, $3); }
    | Dec error DecList {yyerror("Missing \",\"", @2.first_line);}
    /* | Dec error         {yyerror("Possibly missing \";\" at this or last line", @2.first_line); yyerrok;} */
    ;
Dec : VarDec                {$$ = newnewNode("Dec", 1, Nter, @1.first_line); addchild($$, $1);}
    | VarDec ASSIGNOP Exp   {$$ = newnewNode("Dec", 2, Nter, @1.first_line); addchild($$, $1); addchild($$, $2); addchild($$, $3); } 
    /* | VarDec ASSIGNOP error   {yyerror("Wrong expression", @3.first_line); yyerrok;} */
    ;

// Expressions
Exp : Exp ASSIGNOP Exp      {$$ = newnewNode("Exp", 1, Nter,  @1.first_line ); addchild($$, $1); addchild($$, $2); addchild($$, $3); }
    | Exp AND Exp           {$$ = newnewNode("Exp", 2, Nter,  @1.first_line ); addchild($$, $1); addchild($$, $2); addchild($$, $3); }
    | Exp OR Exp            {$$ = newnewNode("Exp", 3, Nter,  @1.first_line ); addchild($$, $1); addchild($$, $2); addchild($$, $3); }
    | Exp RELOP Exp         {$$ = newnewNode("Exp", 4, Nter,  @1.first_line ); addchild($$, $1); addchild($$, $2); addchild($$, $3); }
    | Exp PLUS Exp          {$$ = newnewNode("Exp", 5, Nter,  @1.first_line ); addchild($$, $1); addchild($$, $2); addchild($$, $3); }
    | Exp MINUS Exp         {$$ = newnewNode("Exp", 6, Nter,  @1.first_line ); addchild($$, $1); addchild($$, $2); addchild($$, $3); }
    | Exp STAR Exp          {$$ = newnewNode("Exp", 7, Nter,  @1.first_line ); addchild($$, $1); addchild($$, $2); addchild($$, $3); }
    | Exp DIV Exp           {$$ = newnewNode("Exp", 8, Nter,  @1.first_line ); addchild($$, $1); addchild($$, $2); addchild($$, $3); }
    | LP Exp RP             {$$ = newnewNode("Exp", 9, Nter,  @1.first_line ); addchild($$, $1); addchild($$, $2); addchild($$, $3); }
    | NOT Exp               {$$ = newnewNode("Exp", 11, Nter, @1.first_line);addchild($$, $1); addchild($$, $2);}
    | MINUS Exp %prec NOT   {$$ = newnewNode("Exp", 10, Nter, @1.first_line);addchild($$, $1); addchild($$, $2);}
    | ID LP Args RP         {$$ = newnewNode("Exp", 12, Nter, @1.first_line );addchild($$, $1); addchild($$, $2);addchild($$, $3); addchild($$, $4);}
    | ID LP RP              {$$ = newnewNode("Exp", 13, Nter, @1.first_line); addchild($$, $1); addchild($$, $2); addchild($$, $3); }
    | Exp LB Exp RB         {$$ = newnewNode("Exp", 14, Nter, @1.first_line );addchild($$, $1); addchild($$, $2);addchild($$, $3); addchild($$, $4);}
    | Exp DOT ID            {$$ = newnewNode("Exp", 15, Nter, @1.first_line ); addchild($$, $1); addchild($$, $2); addchild($$, $3); }
    | ID                    {$$ = newnewNode("Exp", 16, Nter, @1.first_line ); addchild($$, $1);}
    | INT                   {$$ = newnewNode("Exp", 17, Nter, @1.first_line ); addchild($$, $1);}
    | FLOAT                 {$$ = newnewNode("Exp", 18, Nter, @1.first_line ); addchild($$, $1);}
    /* | Exp ASSIGNOP error    {yyerror("Wrong expression", @3.first_line); yyerrok;}
    | Exp AND error         {yyerror("Wrong expression", @3.first_line); yyerrok;}
    | Exp OR error          {yyerror("Wrong expression", @3.first_line); yyerrok;}
    | Exp RELOP error       {yyerror("Wrong expression", @3.first_line); yyerrok;}
    | Exp PLUS error        {yyerror("Wrong expression", @3.first_line); yyerrok;}
    | Exp MINUS error       {yyerror("Wrong expression", @3.first_line); yyerrok;}
    | Exp STAR error        {yyerror("Wrong expression", @3.first_line); yyerrok;}
    | Exp DIV error         {yyerror("Wrong expression", @3.first_line); yyerrok;}
    | LP error RP           {yyerror("Wrong expression", @2.first_line);}
    | MINUS error           {yyerror("Wrong expression", @2.first_line); yyerrok;}
    | NOT error             {yyerror("Wrong expression", @2.first_line); yyerrok;}
    | ID LP error RP        {yyerror("Wrong argument(s)", @3.first_line); yyerrok;}
    | ID LP error SEMI      {yyerror("Missing \")\"", @3.first_line); ignore_line = @3.first_line;}
    | Exp LB error RB       {yyerror("Missing \"]\"", @3.first_line);}
    | Exp LB error SEMI     {yyerror("Missing \"]\"", @3.first_line); ignore_line = @3.first_line;} */
    ;
Args : Exp COMMA Args   {$$ = newnewNode("Args", 1, Nter, @1.first_line ); addchild($$, $1); addchild($$, $2); addchild($$, $3); }
    | Exp               {$$ = newnewNode("Args", 2, Nter, @1.first_line ); addchild($$, $1);}
    ;

%%
#include "lex.yy.c"

int yyerror(const char *msg, ...)
{
    right = 0;
    if (msg[0] == 's' && msg[1] == 'y')
    {
        printf("Error type B at Line %d: %s.", yylineno, msg);
        syserr++;
    }
    else
    {
        printf(" %s.\n", msg);
        myerr++;
    }
    return 0;
}
