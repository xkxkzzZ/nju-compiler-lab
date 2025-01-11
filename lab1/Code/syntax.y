%locations

%{
#include <stdio.h>
#include <stdlib.h>

#define yDEBUG 0


#include "lex.yy.c"

#define YYSTYPE Node*

// extern int yylineno;
// extern int yylex();
void yyerror(const char *s);


int count = 0;
int errorflag = 0;
int lexerror[404]; // lexerror[i] = 1 表示第i行有词法错误
int parerror[404]; // parerror[i] = 1 表示第i行有语法错误


Node* root = NULL;

Node* newNode(char* name);
Node* newnewNode(char* name, char* moreinfo);

void addChild(Node* parent, Node* child);
void printTree(Node* node, int depth);

char* lineinfo(int n);
char* errorinfo(int n);

void findTreeError(Node* node);
void printLexerror();
void printParerror();

%}

%token INT
%token FLOAT
%token TYPE STRUCT RETURN IF ELSE WHILE ID 
%token SEMI COMMA ASSIGNOP RELOP AND OR PLUS MINUS STAR DIV DOT NOT LP RP LB RB LC RC

/* %type <node> Program ExtDefList ExtDef ExtDecList Specifier StructSpecifier OptTag Tag VarDec FunDec VarList ParamDec CompSt StmtList Stmt DefList Def DecList Dec Exp Args */

%right ASSIGNOP
%left OR
%left AND 
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT
%left LP RP LB RB DOT


%%

/* High-level Definitions */

Program: ExtDefList   { $$ = newnewNode("Program", lineinfo(@$.first_line)); addChild($$, $1); root = $$;  /*printTree(root, 0);*/ }
       /* | error {  $$ = newnewNode("error", errorinfo(@$.first_line)); yyerror("Program"); } */
       ;
ExtDefList: ExtDef ExtDefList { $$ = newnewNode("ExtDefList", lineinfo(@$.first_line)); addChild($$, $1); addChild($$, $2); }
          | /* empty */    { $$ = NULL; } 
          ;
ExtDef: Specifier ExtDecList SEMI   { $$ = newnewNode("ExtDef", lineinfo(@$.first_line)); $3 = newNode("SEMI"); addChild($$, $1); addChild($$, $2); addChild($$, $3); }
      | Specifier SEMI           { $$ = newnewNode("ExtDef", lineinfo(@$.first_line));  $2 = newNode("SEMI"); addChild($$, $1); addChild($$, $2); }
      | Specifier FunDec CompSt     { $$ = newnewNode("ExtDef", lineinfo(@$.first_line)); addChild($$, $1); addChild($$, $2); addChild($$, $3); }
      | error SEMI{  $$ = newnewNode("error", errorinfo(@$.first_line)); yyerror("ExtDef"); }
      | error {  $$ = newnewNode("error", errorinfo(@$.first_line)); yyerror("ExtDef"); }
      ;
ExtDecList: VarDec  { $$ = newnewNode("ExtDecList", lineinfo(@$.first_line)); addChild($$, $1); }
          | VarDec COMMA ExtDecList     { $$ = newnewNode("ExtDecList", lineinfo(@$.first_line));  $2 = newNode("COMMA");  addChild($$, $1); addChild($$, $2);  addChild($$, $3); }
          | error {  $$ = newnewNode("error", errorinfo(@$.first_line)); yyerror("ExtDecList"); }
          ;

/* Specifiers */

Specifier: TYPE     { $$ = newnewNode("Specifier", lineinfo(@$.first_line)); addChild($$, $1); }
         | StructSpecifier  { $$ = newnewNode("Specifier", lineinfo(@$.first_line)); addChild($$, $1);}
         | error {  $$ = newnewNode("error", errorinfo(@$.first_line)); yyerror("Specifier"); }
         ;
StructSpecifier: STRUCT OptTag LC DefList RC    { $$ = newnewNode("StructSpecifier", lineinfo(@$.first_line)); $1 = newNode("STRUCT"); $3 = newNode("LC"); $5 = newNode("RC"); addChild($$, $1); addChild($$, $2); addChild($$, $3); addChild($$, $4); addChild($$, $5); }
               | STRUCT Tag      { $$ = newnewNode("StructSpecifier", lineinfo(@$.first_line));  $1 = newNode("STRUCT"); addChild($$, $1); addChild($$, $2); }
               /* | error {  $$ = newnewNode("error", errorinfo(@$.first_line)); yyerror("StructSpecifier"); } */
               ;
OptTag: ID    { $$ = newnewNode("OptTag", lineinfo(@$.first_line));  addChild($$, $1); }
      | /* empty */   { $$ = NULL; }
      ;
Tag: ID    { $$ = newnewNode("Tag", lineinfo(@$.first_line)); addChild($$, $1); }
    | error {  $$ = newnewNode("error", errorinfo(@$.first_line)); yyerror("Tag"); }
    ;

/* Declarators */

VarDec: ID  { $$ = newnewNode("VarDec", lineinfo(@$.first_line));  addChild($$, $1); }
      | VarDec LB INT RB    { $$ = newnewNode("VarDec", lineinfo(@$.first_line)); $2 = newNode("LB");  $4 = newNode("RB");  addChild($$, $1); addChild($$, $2); addChild($$, $3); addChild($$, $4);}
      /* | error {  $$ = newnewNode("error", errorinfo(@$.first_line)); yyerror("VarDec"); } */
      ;
FunDec: ID LP VarList RP    { $$ = newnewNode("FunDec", lineinfo(@$.first_line));  $2 = newNode("LP"); $4 = newNode("RP"); addChild($$, $1); addChild($$, $2); addChild($$, $3); addChild($$, $4);}
       | ID LP RP   {$$ = newnewNode("FunDec", lineinfo(@$.first_line));  $2 = newNode("LP"); $3 = newNode("RP"); addChild($$, $1); addChild($$, $2); addChild($$, $3); }
       | error {  $$ = newnewNode("error", errorinfo(@$.first_line)); yyerror("FunDec"); }
       ;
VarList: ParamDec COMMA VarList     { $$ = newnewNode("VarList", lineinfo(@$.first_line));  $2 = newNode("COMMA"); addChild($$, $1); addChild($$, $2);  addChild($$, $3);}
         | ParamDec     { $$ = newnewNode("VarList", lineinfo(@$.first_line)); addChild($$, $1); }
         | error {  $$ = newnewNode("error", errorinfo(@$.first_line)); yyerror("VarList"); }
         ;
ParamDec: Specifier VarDec  { $$ = newnewNode("ParamDec", lineinfo(@$.first_line)); addChild($$, $1); addChild($$, $2); }
        | error {  $$ = newnewNode("error", errorinfo(@$.first_line)); yyerror("ParamDec"); }
        ;
/* Statements */


CompSt: LC DefList StmtList RC  { $$ = newnewNode("CompSt", lineinfo(@$.first_line));  $1 = newNode("LC"); $4 = newNode("RC"); addChild($$, $1); addChild($$, $2); addChild($$, $3); addChild($$, $4); }
      | LC error RC {  $$ = newnewNode("error", errorinfo(@$.first_line)); yyerror("CompSt"); $1 = newNode("LC"); $3 = newNode("RC"); addChild($$, $1); addChild($$, $3); }
      ;
StmtList: Stmt StmtList     { $$ = newnewNode("StmtList", lineinfo(@$.first_line)); addChild($$, $1); addChild($$, $2);  }
         | /* empty */   { $$ = NULL; }
         /* | error DefList {  $$ = newnewNode("error", errorinfo(@$.first_line)); yyerror("StmtList"); addChild($$, $2); } */
         ;
Stmt: Exp SEMI  { $$ = newnewNode("Stmt", lineinfo(@$.first_line)); $2 = newNode("SEMI"); addChild($$, $1); addChild($$, $2); }
    | CompSt    { $$ = newnewNode("Stmt", lineinfo(@$.first_line)); addChild($$, $1); }
    | RETURN Exp SEMI       { $$ = newnewNode("Stmt", lineinfo(@$.first_line)); $1 = newNode("RETURN"); $3 = newNode("SEMI"); addChild($$, $1); addChild($$, $2); addChild($$, $3);}
    | IF LP Exp RP Stmt     { $$ = newnewNode("Stmt", lineinfo(@$.first_line)); $1 = newNode("IF"); $2 = newNode("LP");  $4 = newNode("RP"); addChild($$, $1); addChild($$, $2); addChild($$, $3); addChild($$, $4); addChild($$, $5);}
    | IF LP Exp RP Stmt ELSE Stmt   { $$ = newnewNode("Stmt", lineinfo(@$.first_line)); $1 = newNode("IF"); $2 = newNode("LP");  $4 = newNode("RP"); $6 = newNode("ELSE"); addChild($$, $1); addChild($$, $2); addChild($$, $3); addChild($$, $4); addChild($$, $5); addChild($$, $6); addChild($$, $7); }
    | WHILE LP Exp RP Stmt  { $$ = newnewNode("Stmt", lineinfo(@$.first_line));  $1 = newNode("WHILE"); $2 = newNode("LP");  $4 = newNode("RP"); addChild($$, $1); addChild($$, $2); addChild($$, $3); addChild($$, $4); addChild($$, $5); }
    | error SEMI { $$ = newnewNode("error", errorinfo(@$.first_line)); yyerror("Stmt");  $2 = newNode("SEMI"); addChild($$, $2); }
    ;

/* Local Definitions */

DefList: Def DefList    {$$ = newnewNode("DefList", lineinfo(@$.first_line)); addChild($$, $1); addChild($$, $2);  }
        | /* empty */    { $$ = NULL; if(yDEBUG) printf("DefList to empty\n");}
        ;
Def: Specifier DecList SEMI     { $$ = newnewNode("Def", lineinfo(@$.first_line)); $3 = newNode("SEMI"); addChild($$, $1); addChild($$, $2); addChild($$, $3); }
    | error SEMI {  $$ = newnewNode("error", errorinfo(@$.first_line)); yyerror("Def"); }
    ;
DecList: Dec    { $$ = newnewNode("DecList", lineinfo(@$.first_line)); addChild($$, $1); }
       | Dec COMMA DecList  { $$ = newnewNode("DecList", lineinfo(@$.first_line)); $2 = newNode("COMMA"); addChild($$, $1); addChild($$, $2);addChild($$, $3); }
       | error {  $$ = newnewNode("error", errorinfo(@$.first_line)); yyerror("DecList"); }
       ;
Dec: VarDec         { $$ = newnewNode("Dec", lineinfo(@$.first_line)); addChild($$, $1); }
    | VarDec ASSIGNOP Exp   { $$ = newnewNode("Dec", lineinfo(@$.first_line)); $2 = newNode("ASSIGNOP"); addChild($$, $1); addChild($$, $2); addChild($$, $3); }
    | error {  $$ = newnewNode("error", errorinfo(@$.first_line)); yyerror("Dec"); }
    ;

/* Expressions */

Exp: Exp ASSIGNOP Exp   { $$ = newnewNode("Exp", lineinfo(@$.first_line)); $2 = newNode("ASSIGNOP"); addChild($$, $1); addChild($$, $2); addChild($$, $3); }
    | Exp AND Exp       { $$ = newnewNode("Exp", lineinfo(@$.first_line)); $2 = newNode("AND"); addChild($$, $1); addChild($$, $2); addChild($$, $3); }
    | Exp OR Exp        { $$ = newnewNode("Exp", lineinfo(@$.first_line));  $2 = newNode("OR"); addChild($$, $1); addChild($$, $2); addChild($$, $3); }
    | Exp RELOP Exp     { $$ = newnewNode("Exp", lineinfo(@$.first_line)); $2 = newNode("RELOP"); addChild($$, $1); addChild($$, $2); addChild($$, $3); }
    | Exp PLUS Exp      { $$ = newnewNode("Exp", lineinfo(@$.first_line)); $2 = newNode("PLUS"); addChild($$, $1); addChild($$, $2); addChild($$, $3); }
    | Exp MINUS Exp     { $$ = newnewNode("Exp", lineinfo(@$.first_line));  $2 = newNode("MINUS"); addChild($$, $1); addChild($$, $2); addChild($$, $3); }
    | Exp STAR Exp      { $$ = newnewNode("Exp", lineinfo(@$.first_line));  $2 = newNode("STAR"); addChild($$, $1); addChild($$, $2); addChild($$, $3); }
    | Exp DIV Exp       { $$ = newnewNode("Exp", lineinfo(@$.first_line)); $2 = newNode("DIV"); addChild($$, $1); addChild($$, $2); addChild($$, $3); }
    | LP Exp RP         { $$ = newnewNode("Exp", lineinfo(@$.first_line));  $1 = newNode("LP"); $3 = newNode("RP");addChild($$, $1); addChild($$, $2); addChild($$, $3); }
    | MINUS Exp         { $$ = newnewNode("Exp", lineinfo(@$.first_line));  $1 = newNode("MINUS");addChild($$, $1); addChild($$, $2); }
    | NOT Exp           { $$ = newnewNode("Exp", lineinfo(@$.first_line)); $1 = newNode("NOT"); addChild($$, $1); addChild($$, $2); }
    | ID LP Args RP     { $$ = newnewNode("Exp", lineinfo(@$.first_line));  $2 = newNode("LP"); $4 = newNode("RP"); addChild($$, $1); addChild($$, $2); addChild($$, $3); addChild($$, $4); }
    | ID LP RP          { $$ = newnewNode("Exp", lineinfo(@$.first_line)); $2 = newNode("LP"); $3 = newNode("RP"); addChild($$, $1); addChild($$, $2); addChild($$, $3); }
    | Exp LB Exp RB     { $$ = newnewNode("Exp", lineinfo(@$.first_line)); $2 = newNode("LB"); $4 = newNode("RB"); addChild($$, $1); addChild($$, $2); addChild($$, $3); addChild($$, $4); }
    | Exp DOT ID        { $$ = newnewNode("Exp", lineinfo(@$.first_line)); $2 = newNode("DOT");  addChild($$, $1); addChild($$, $2); addChild($$, $3); }
    | ID                { $$ = newnewNode("Exp", lineinfo(@$.first_line)); addChild($$, $1); }
    | INT               { $$ = newnewNode("Exp", lineinfo(@$.first_line));  addChild($$, $1); }
    | FLOAT             { $$ = newnewNode("Exp", lineinfo(@$.first_line));  addChild($$, $1); }
    | error {  $$ = newnewNode("error", errorinfo(@$.first_line)); yyerror("Exp"); }
    ;
Args: Exp COMMA Args    { $$ = newnewNode("Args", lineinfo(@$.first_line)); $2 = newNode("COMMA"); addChild($$, $1); addChild($$, $2); addChild($$, $3); }
    | Exp      { $$ = newnewNode("Args", lineinfo(@$.first_line)); addChild($$, $1); }
    /* | error {  $$ = newnewNode("error", errorinfo(@$.first_line)); yyerror("Args"); } */
     ;




%%

void yyerror(const char* msg) {
    if(yDEBUG) printf("Error type B at Line %d: %s\n", yylineno, msg);
    errorflag = 1;
    /* printf("Error type B: %s\n", msg); */
}


Node* newNode(char* name){
    Node* node = (Node*)malloc(sizeof(Node));
    node->name = name;
    node->ID = count++;
    node->parent = NULL;
    node->child = NULL;
    node->sibling = NULL;
    node->moreinfo = "";
    /* printf("newNode: %s\n", name); */
    if(yDEBUG) printf("newNode: %s(%d)\n", name, node->ID);
    return node;
}

Node* newnewNode(char* name, char* moreinfo) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->name = name;
    node->ID = count++;
    node->parent = NULL;
    node->child = NULL;
    node->sibling = NULL;
    node->moreinfo = moreinfo;
    /* printf("newNode: %s\n", name); */
    if(yDEBUG) printf("newNode: %s(%d)\n", name, node->ID);
    return node;
}

char* lineinfo(int n) {
    char* str = (char*)malloc(10);
    sprintf(str, " (%d)", n);
    return str;
}

char* errorinfo(int n){
    char* str = (char*)malloc(10);
    sprintf(str, "%d", n);
    return str;
}

void addChild(Node* parent, Node* child) {
    if (parent == NULL || child == NULL) {
        // printf("NULL\n");
        return;
    }
    if (parent->child == NULL) {
        parent->child = child;
    } else {
        Node* sibling = parent->child;
        while (sibling->sibling != NULL) {
            sibling = sibling->sibling;
        }
        sibling->sibling = child;
    }
    child->parent = parent;
    /* printf("yylineo: %d\n", yylineno); */
    if(yDEBUG) printf("addChild: %s(%d) -> %s(%d)\n", parent->name,parent->ID,  child->name, child->ID);
}

void printTree(Node* node, int depth) {
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
    // printf("%s\n", node->name);
    // printf("%s(%d)\n", node->name, node->ID);
    printf("%s%s\n", node->name, node->moreinfo);
    Node* child = node->child;
    while (child != NULL) {
        printTree(child, depth + 1);
        child = child->sibling;
    }
}

// 在树上找到error的节点，并输出
void findTreeError(Node* node) {
    if (node == NULL) {
        return;
    }
    if (strcmp(node->name, "error") == 0) {
        int l = atoi(node->moreinfo);
        if(!lexerror[l]) parerror[l] = 1;
        /* printf("Error type B at Line %s: \n", node->moreinfo); */
        return;
    }
    Node* child = node->child;
    while (child != NULL) {
        findTreeError(child);
        child = child->sibling;
    }
}

void printLexerror() {
    for (int i = 0; i < 400; i++) {
        if (lexerror[i] == 1) {
            printf("Error type A at Line %d: \n", i);
        }
    }
}

void printParerror() {
    for (int i = 0; i < 400; i++) {
        if (parerror[i] == 1) {
            printf("Error type B at Line %d: \n", i);
        }
    }
}