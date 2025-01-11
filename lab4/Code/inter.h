#include "qaq.h"

void iCond(int id, Node* node, Operand* label_true, Operand* label_false);
void iExp(Node* node, Operand* place);
void iArgs(Node* node, Operand* arg_list[], int index);
void iProgram(Node* node);
void iExtDefList(Node* node);
void iExtDef(Node* node);
void iFunDec(Node* node);
void iCompSt(Node* node);
void iVarDec(Node* node, int isFunc, int isStruct);
void iDefList(Node* node);
void iVarList(Node* node, int isFunc);
void iParamDec(Node* node, int isFunc);
void iStmtList(Node* node);
void iStmt(Node* node);
void iDef(Node* node);
void iDecList(Node* node, int isStruct);
void iDec(Node* node, int isStruct);
