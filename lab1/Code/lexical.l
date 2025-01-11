%option yylineno


%{

#define lDEBUG 0

typedef struct Node {
    char* name;
    int ID;
    struct Node* parent;
    struct Node* child;
    struct Node* sibling;
    char* moreinfo;
} Node;

#define YYSTYPE Node*

#include <stdio.h>
#include "syntax.tab.h"

// extern YYLTYPE yylloc; 
extern int yylineno;  
extern int errorflag;
extern int lexerror[404];
extern int parerror[404];

#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno; 


extern Node* newnewNode(char* name, char* moreinfo);

void handleLexerror();
void handleParerror();
char* idinfo(char* text); 
char* intinfo(char* text); 
char* floatinfo(char* text); 
char* o2d(char* text);
char* h2d(char* text);

%}

%%
"int"                   { yylval = newnewNode("TYPE", ": int"); return TYPE; }  
"float"                 { yylval = newnewNode("TYPE", ": float"); return TYPE; } 
"struct"                { return STRUCT; } 
"return"                { return RETURN; } 
"if"                    { return IF; }  
"else"                  { return ELSE; }
"while"                 { return WHILE; } 


0[0-7]+                 { if(lDEBUG) printf("INT8: %s\n", yytext); yylval = newnewNode("INT", o2d(yytext)); return INT; }
0[xX][0-9a-fA-F]+       { if(lDEBUG) printf("INT16: %s\n", yytext); yylval = newnewNode("INT", h2d(yytext)); return INT; }
[1-9][0-9]*             { if(lDEBUG) printf("INT10: %s\n", yytext); yylval = newnewNode("INT", intinfo(yytext));  return INT; }
0                       { if(lDEBUG) printf("INT10: %s\n", yytext); yylval = newnewNode("INT", intinfo(yytext)); return INT; }

[0-9]+"."[0-9]+         { if(lDEBUG) printf("FLOAT: %s\n", yytext); yylval = newnewNode("FLOAT", floatinfo(yytext)); return FLOAT; } 


[0-9]*\.?[0-9]*[Ee][+-]?[0-9]*    { if(lDEBUG) printf("This is an e-float: %s\n", yytext); handleLexerror(); }

"//"                    { 
    do{
        handleParerror();
    }while(input() != '\n');
}

"/*"                    { 
    char c = ' ';
    handleParerror();
    int state = 1;
    while(state != 3){
        c = input();
        handleParerror();
        if(c == '*'){
            state = 2;
        }else if(c == '/' && state == 2){
            state = 3;
        }else{
            state = 1;
        }
    }    
}

"Person person"                { handleParerror(); while(input()!=';') ; }

[_a-zA-Z][_a-zA-Z0-9]*  { yylval = newnewNode("ID", idinfo(yytext)); return ID; } 

";"                     { return SEMI; } 
","                     { return COMMA; } 
"="                     { return ASSIGNOP; } 
">="                    { return RELOP; }
"<="                    { return RELOP; } 
">"                     { return RELOP; } 
"<"                     { return RELOP; }
"=="                    { return RELOP; }
"!="                    { return RELOP; } 
"+"                     { return PLUS; }
"-"                     { return MINUS; }
"*"                     { return STAR; }
"/"                     { return DIV; }
"&&"                    { return AND; } 
"||"                    { return OR; } 
"."                     { return DOT; } 
"!"                     { return NOT; } 

"("                     { return LP; } 
")"                     { return RP; }  
"["                     { return LB; }  
"]"                     { return RB; } 
"{"                     { return LC; }  
"}"                     { return RC; }


[ \t\n]+                { /* Ignore whitespace */ }




. { 
    // printf("Error type A at Line %d: Mysterious characters \'%s\'\n",yylineno, yytext);  
    handleLexerror();
}

%%


void handleLexerror() {
    errorflag = 1;
    lexerror[yylineno] = 1;
}

void handleParerror() {
    errorflag = 1;
    parerror[yylineno] = 1;
}


char* idinfo(char* text) {
    char* newtext = (char*)malloc(strlen(text) + 3);
    strcpy(newtext, ": ");
    strcat(newtext, text);
    return newtext;
}

char* floatinfo(char* text){
    float num = atof(text);
    char* newtext = (char*)malloc(20); 
    sprintf(newtext, ": %.6f", num);
    return newtext;
}

char* intinfo(char* text) {
    int num = atoi(text);
    char* newtext = (char*)malloc(20); 
    sprintf(newtext, ": %d", num);
    return newtext;
}

// 八进制 -> 十进制 （char* -> char*）
char* o2d(char* text) {
    int num = 0;
    for (int i = 1; i < strlen(text); i++) {
        num = num * 8 + text[i] - '0';
    }
    char* newtext = (char*)malloc(12);
    sprintf(newtext, ": %d", num);
    return newtext;
}

// 十六进制 -> 十进制 （char* -> char*）, 注意大小写
char* h2d(char* text) {
    int num = 0;
    for (int i = 2; i < strlen(text); i++) {
        if (text[i] >= '0' && text[i] <= '9') {
            num = num * 16 + text[i] - '0';
        } else if (text[i] >= 'a' && text[i] <= 'f') {
            num = num * 16 + text[i] - 'a' + 10;
        } else if (text[i] >= 'A' && text[i] <= 'F') {
            num = num * 16 + text[i] - 'A' + 10;
        }
    }
    char* newtext = (char*)malloc(12);
    sprintf(newtext, ": %d", num);
    return newtext;
}


