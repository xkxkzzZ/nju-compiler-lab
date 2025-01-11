#include "tree.h"

#ifndef SENATIC_H
#define SENATIC_H
void Program(Node* node);
void ExtDefList(Node* node);
void ExtDef(Node* node);
void Specifier(Node* node);
void ExtDecList(Node* node);
void FunDec(Node* node);
void CompSt(Node* node);
void VarDec(Node* node);
void StructSpecifier(Node* node);
void DefList(Node* node);
void Def(Node* node);
void DecList(Node* node);
void Dec(Node* node);
void OptTag(Node* node);
void Tag(Node* node);
void VarList(Node* node);
void ParamDec(Node* node);
void StmtList(Node* node);
void Stmt(Node* node);
void Exp(Node* node);
void Args(Node* node);

void print_error(int type, int line, char* msg);

#endif