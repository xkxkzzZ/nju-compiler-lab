#include "inter.h"

extern InterCodes* head;
extern InterCodes* tail;

void iProgram(Node* node)
{
    head = (InterCodes*)malloc(sizeof(InterCodes));
    head->prev = head->next = NULL;
    tail = head;
    iExtDefList(node->child);
}

void iExtDefList(Node* node)
{
    if (node->no == 2) // ExtDef ExtDefList
    {
        iExtDef(node->child);
        iExtDefList(getChild(node, 1));
    }
}

void iExtDef(Node* node)
{
    switch (node->no) {
    case 1: // Specifier ExtDecList SEMI
        break;
    case 2: // Specifier SEMI
        break;
    case 3: // Specifier FunDec CompSt
        iFunDec(getChild(node, 1));
        iCompSt(getChild(node, 2));
        break;
    default:
        break;
    }
}

void iFunDec(Node* node)
{
    Operand* func = getOperandwithName(FUNCT, node->child->val.type_str);
    createSingle(FUNCTION, func);
    switch (node->no) {
    case 1: // ID LP VarList RP
        iVarList(getChild(node, 2), 1);
        break;
    case 2: // ID LP RP
        break;
    default:
        break;
    }
}

void iCompSt(Node* node)
{
    iDefList(getChild(node, 1));
    iStmtList(getChild(node, 2));
}

void iVarDec(Node* node, int isFunc, int isStruct)
{
    switch (node->no) {
    case 1: // ID
    {
        if (isFunc) {
            TableList* res = search(node->child->val.type_str);
            res->op->kind = PARAMETER;
            createSingle(PARAM, res->op);
        } else if (isStruct) {
            TableList* res = search(node->child->val.type_str);
            createDec(res->op, res->size);
        }
    } break;
    case 2: // VarDec LB INT RB
    {
        if (node->child->no != 1)  cannotTranslate();
        TableList* res = search(node->child->child->val.type_str);
        createDec(res->op, res->size);
    } break;
    default:
        break;
    }
}

void iDefList(Node* node)
{
    if (node->no == 2) // Def DefList
    {
        iDef(node->child);
        iDefList(getChild(node, 1));
    }
}

void iVarList(Node* node, int isFunc)
{
    iParamDec(node->child, isFunc);
    switch (node->no) {
    case 1: // ParamDec COMMA VarList
    {
        iVarList(getChild(node, 2), isFunc);
    } break;
    case 2: // ParamDec
        break;
    default:
        break;
    }
}

void iParamDec(Node* node, int isFunc)
{
    iVarDec(getChild(node, 1), isFunc, 0);
}

void iStmtList(Node* node)
{
    if (node->no == 2) {
        iStmt(node->child);
        iStmtList(getChild(node, 1));
    }
}

void iDef(Node* node)
{
    iDecList(getChild(node, 1), node->child->no==2);
}

void iDecList(Node* node, int isStruct)
{
    iDec(node->child, isStruct);
    switch (node->no) {
    case 1: // Dec
        break;
    case 2: // Dec COMMA DecList
        iDecList(getChild(node, 2), isStruct);
        break;
    default:
        break;
    }
}

void iArgs(Node* node, Operand* arg_list[], int index)
{
    if (node->child->syn->type->kind == ARRAY) {
        cannotTranslate();
    }
    switch (node->no) {
    case 1: // Exp COMMA Args
    {
        iArgs(getChild(node, 2), arg_list, index - 1);
        Operand* t1 = newTemp(0);
        if (node->child->syn->type->kind != INT)
            t1->type = ADDRESS;
        iExp(node->child, t1);
        arg_list[index] = t1;
    } break;
    case 2: // Exp
    {
        Operand* t1 = newTemp(0);
        if (node->child->syn->type->kind != INT)
            t1->type = ADDRESS;
        iExp(node->child, t1);
        arg_list[index] = t1;
    } break;
    default:
        break;
    }
}

void iDec(Node* node, int isStruct)
{
    iVarDec(node->child, 0, isStruct);
    switch (node->no) {
    case 1: // VarDec
        break;
    case 2: // VarDec ASSIGNOP Exp
    {
        if (node->child->no == 1) {
            TableList* res = search(node->child->child->val.type_str);
            Operand* t1 = newTemp(0);
            iExp(getChild(node, 2), t1);
            createAssign(NORMAL, res->op, t1);
        } else {
            Operand* t1 = newTemp(1);
            iExp(getChild(node, 2), t1);
            TableList* res = search(node->child->child->child->val.type_str);
            unsigned lsize = res->type->u.array.size;
            unsigned rsize = getChild(node, 2)->syn->type->u.array.size;
            unsigned n = lsize > rsize ? rsize : lsize;
            if (n > 0) {
                Operand* t2 = newTemp(1);
                createAssign(GETADDR, t2, res->op);
                createAssign(COPY, t2, t1);
                Operand* four = getOperandwithValue(CONSTANT, 4);
                for (unsigned i = 1; i < n; ++i) {
                    createBinop(ADD, NORMAL, t2, t2, four);
                    createBinop(ADD, NORMAL, t1, t1, four);
                    createAssign(COPY, t2, t1);
                }
            }
        }
    } break;
    default:
        break;
    }
}
void iStmt(Node* node)
{
    switch (node->no) {
    case 1: // Exp SEMI
        iExp(node->child, NULL);
        break;
    case 2: // CompSt
        iCompSt(node->child);
        break;
    case 3: // RETURN Exp SEMI
    {
        Operand* t1 = newTemp(0);
        iExp(getChild(node, 1), t1);
        createSingle(RETURN, t1);
    } break;
    case 4: // IF LP Exp RP Stmt
    {
        Operand* label1 = newLabel();
        Operand* label2 = newLabel();
        iCond(getChild(node, 2)->no, getChild(node, 2), label1, label2);
        createSingle(LABEL, label1);
        iStmt(getChild(node, 4));
        createSingle(LABEL, label2);
    } break;
    case 5: // IF LP Exp RP Stmt ELSE Stmt
    {
        Operand* label1 = newLabel();
        Operand* label2 = newLabel();
        Operand* label3 = newLabel();
        iCond(getChild(node, 2)->no, getChild(node, 2), label1, label2);
        createSingle(LABEL, label1);
        iStmt(getChild(node, 4));
        createSingle(GOTO, label3);
        createSingle(LABEL, label2);
        iStmt(getChild(node, 6));
        createSingle(LABEL, label3);
    } break;
    case 6: // WHILE LP Exp RP Stmt
    {
        Operand* label1 = newLabel();
        Operand* label2 = newLabel();
        Operand* label3 = newLabel();
        createSingle(LABEL, label1);
        iCond(getChild(node, 2)->no, getChild(node, 2), label2, label3);
        createSingle(LABEL, label2);
        iStmt(getChild(node, 4));
        createSingle(GOTO, label1);
        createSingle(LABEL, label3);
    } break;
    default:
        break;
    }
}

void iCond(int id, Node* node, Operand* label_true, Operand* label_false)
{
    switch (id) {
    case 2: // Exp AND Exp
    {
        Operand* label1 = newLabel();
        iCond(node->child->no, node->child, label1, label_false);
        createSingle(LABEL, label1);
        iCond(getChild(node, 2)->no, getChild(node, 2), label_true, label_false);
    } break;
    case 3: // Exp OR Exp
    {
        Operand* label1 = newLabel();
        iCond(node->child->no, node->child, label_true, label1);
        createSingle(LABEL, label1);
        iCond(getChild(node, 2)->no, getChild(node, 2), label_true, label_false);
    } break;
    case 4: // Exp RELOP Exp
    {
        Operand* t1 = newTemp(0);
        Operand* t2 = newTemp(0);
        iExp(node->child, t1);
        iExp(getChild(node, 2), t2);
        createCond(t1, t2, label_true, getChild(node, 1)->val.type_str);
        createSingle(GOTO, label_false);
    } break;
    case 11: // NOT Exp
        iCond(getChild(node, 1)->no, getChild(node, 1), label_false, label_true);
        break;
    default: {
        Operand* t1 = newTemp(0);
        iExp(node, t1);
        Operand* zero = getOperandwithValue(CONSTANT, 0);
        createCond(t1, zero, label_true, "!=");
        createSingle(GOTO, label_false);
    } break;
    }
}
void iExp(Node* node, Operand* place)
{
    switch (node->no) {
    case 1: // Exp ASSIGNOP Exp
    {
        if (node->child->no == 16) {
            TableList* res = search(node->child->child->val.type_str);
            Operand* t1 = newTemp(0);
            if (getChild(node, 2)->syn->type->kind != INT)
                t1->type = ADDRESS;
            iExp(getChild(node, 2), t1);
            if (res->op->type == VAL)
                createAssign(NORMAL, res->op, t1);
            else {
                unsigned lsize = res->type->u.array.size;
                unsigned rsize = getChild(node, 2)->syn->type->u.array.size;
                unsigned n = lsize > rsize ? rsize : lsize;
                if (n > 0) {
                    Operand* t2 = newTemp(1);
                    if (res->op->kind == VARIABLE) {
                        createAssign(GETADDR, t2, res->op);
                    } else {
                        createAssign(NORMAL, t2, res->op);
                    }
                    createAssign(COPY, t2, t1);
                    Operand* four = getOperandwithValue(CONSTANT, 4);
                    for (unsigned i = 1; i < n; ++i) {
                        createBinop(ADD, NORMAL, t2, t2, four);
                        createBinop(ADD, NORMAL, t1, t1, four);
                        createAssign(COPY, t2, t1);
                    }
                }
            }
            if (place) {
                createAssign(NORMAL, place, res->op);
            }
        } else if (node->child->no == 14) {
            Operand* t1 = newTemp(1);
            iExp(node->child, t1);
            Operand* t2 = newTemp(0);
            if (getChild(node, 2)->syn->type->kind != INT)
                t2->type = ADDRESS;
            iExp(getChild(node, 2), t2);
            if (t2->type == VAL)
                createAssign(SETVAL, t1, t2);
            else if (getChild(node, 2)->syn->type->kind == ARRAY) {
                cannotTranslate();
            } 
            if (place) {
                if (place->type == VAL) {
                    createAssign(GETVAL, place, t1);
                } else {
                    createAssign(NORMAL, place, t1);
                }
            }
        } else if (node->child->no == 15) {
            Operand* t1 = newTemp(1);
            iExp(node->child, t1);
            Operand* t2 = newTemp(0);
            if (getChild(node, 2)->syn->type->kind != INT)
                t2->type = ADDRESS;
            iExp(getChild(node, 2), t2);
            if (t2->type == VAL)
                createAssign(SETVAL, t1, t2);
            else {
                unsigned lsize = node->child->syn->type->u.array.size;
                unsigned rsize = getChild(node, 2)->syn->type->u.array.size;
                unsigned n = lsize > rsize ? rsize : lsize;
                if (n > 0) {
                    createAssign(COPY, t1, t2);
                    Operand* four = getOperandwithValue(CONSTANT, 4);
                    for (unsigned i = 1; i < n; ++i) {
                        createBinop(ADD, NORMAL, t1, t1, four);
                        createBinop(ADD, NORMAL, t2, t2, four);
                        createAssign(COPY, t1, t2);
                    }
                    if (n > 1 && place) {
                        Operand* con = getOperandwithValue(CONSTANT, 4 * (n - 1));
                        createBinop(SUB, NORMAL, t1, t1, con);
                    }
                }
            }
            if (place) {
                if (place->type == VAL) {
                    createAssign(GETVAL, place, t1);
                } else {
                    createAssign(NORMAL, place, t1);
                }
            }
        }
    } break;
    case 2: // Exp AND Exp
    case 3: // Exp OR Exp
    case 4: // Exp RELOP Exp
    case 11: // NOT Exp
    {
        Operand* label1 = newLabel();
        Operand* label2 = newLabel();
        if (place) {
            Operand* zero = getOperandwithValue(CONSTANT, 0);
            createAssign(NORMAL, place, zero);
        }
        iCond(node->no, node, label1, label2);
        createSingle(LABEL, label1);
        if (place) {
            Operand* one = getOperandwithValue(CONSTANT, 1);
            createAssign(NORMAL, place, one);
        }
        createSingle(LABEL, label2);
    } break;
    case 5: // Exp PLUS Exp
    {
        Operand* t1 = newTemp(0);
        Operand* t2 = newTemp(0);
        iExp(node->child, t1);
        iExp(getChild(node, 2), t2);
        if (place)
            createBinop(ADD, NORMAL, place, t1, t2);
    } break;
    case 6: // Exp MINUS Exp
    {
        Operand* t1 = newTemp(0);
        Operand* t2 = newTemp(0);
        iExp(node->child, t1);
        iExp(getChild(node, 2), t2);
        if (place)
            createBinop(SUB, NORMAL, place, t1, t2);
    } break;
    case 7: // Exp STAR Exp
    {
        Operand* t1 = newTemp(0);
        Operand* t2 = newTemp(0);
        iExp(node->child, t1);
        iExp(getChild(node, 2), t2);
        if (place)
            createBinop(MUL, NORMAL, place, t1, t2);
    } break;
    case 8: // Exp DIV Exp
    {
        Operand* t1 = newTemp(0);
        Operand* t2 = newTemp(0);
        iExp(node->child, t1);
        iExp(getChild(node, 2), t2);
        if (place)
            createBinop(DIV, NORMAL, place, t1, t2);
    } break;
    case 9: // LP Exp RP
        iExp(getChild(node, 1), place);
        break;
    case 10: // MINUS Exp
    {
        Operand* t1 = newTemp(0);
        iExp(getChild(node, 1), t1);
        if (place) {
            Operand* zero = getOperandwithValue(CONSTANT, 0);
            createBinop(SUB, NORMAL, place, zero, t1);
        }
    } break;
    case 12: // ID LP Args RP
    {
        TableList* function = search(node->child->val.type_str);
        int argCount = function->type->u.function.argc;
        Operand* arg_list[argCount];
        iArgs(getChild(node, 2), arg_list, argCount - 1);
        if (!strcmp(function->name, "write")) {
            createSingle(WRITE, arg_list[0]);
            if (place) {
                Operand* zero = getOperandwithValue(CONSTANT, 0);
                createAssign(NORMAL, place, zero);
            }
        } else {
            for (int i = 0; i < argCount; ++i) {
                createSingle(ARG, arg_list[i]);
            }
            Operand* func = getOperandwithName(FUNCT, function->name);
            if (place)
                createSinop(CALL, place, func);
            else {
                Operand* t1 = newTemp(0);
                createSinop(CALL, t1, func);
            }
        }
    } break;
    case 13: // ID LP RP
    {
        TableList* function = search(node->child->val.type_str);
        if (!strcmp(function->name, "read")) {
            if (place)
                createSingle(READ, place);
            else {
                Operand* t1 = newTemp(0);
                createSingle(READ, t1);
            }
        } else {
            Operand* func = getOperandwithName(FUNCT, function->name);
            if (place)
                createSinop(CALL, place, func);
            else {
                Operand* t1 = newTemp(0);
                createSinop(CALL, t1, func);
            }
        }
    } break;
    case 14: // Exp LB Exp RB
    {
        if (node->child->no == 16) {
            TableList* res = search(node->child->child->val.type_str);
            Operand* t1 = newTemp(0);
            iExp(getChild(node, 2), t1);
            unsigned elemSize = res->size / node->child->syn->type->u.array.size;
            Operand* esize = getOperandwithValue(CONSTANT, elemSize);
            Operand* t2 = newTemp(0);
            createBinop(MUL, NORMAL, t2, t1, esize);
            if (place) {
                if (place->type == VAL) {
                    Operand* t3 = newTemp(1);
                    if (res->op->kind == VARIABLE) {
                        createBinop(ADD, GETADDR, t3, res->op, t2);
                    } else {
                        createBinop(ADD, NORMAL, t3, res->op, t2);
                    }
                    createAssign(GETVAL, place, t3);
                } else {
                    if (res->op->kind == VARIABLE)
                        createBinop(ADD, GETADDR, place, res->op, t2);
                    else
                        createBinop(ADD, NORMAL, place, res->op, t2);
                }
            }
        } else {
            if (node->child->no == 14) {
                printf("Cannot translate: Code contains variables of multi-dimensional array type or parameters of array type.\n");
                exit(-1);
            }
            Operand* t0 = newTemp(1);
            iExp(node->child, t0);
            Operand* t1 = newTemp(0);
            iExp(getChild(node, 2), t1);
            TableList* res = search(node->child->syn->name);
            unsigned elemSize = res->size / node->child->syn->type->u.array.size;

            Operand* esize = getOperandwithValue(CONSTANT, elemSize);
            Operand* t2 = newTemp(0);
            createBinop(MUL, NORMAL, t2, t1, esize);
            if (place) {
                if (node->syn->type->kind == INT) {
                    if (place->type == VAL) {
                        Operand* t3 = newTemp(1);
                        createBinop(ADD, NORMAL, t3, t0, t2);
                        createAssign(GETVAL, place, t3);
                    } else {
                        createBinop(ADD, NORMAL, place, t0, t2);
                    }
                } else {
                    createBinop(ADD, NORMAL, place, t0, t2);
                }
            }
        }
    } break;
    case 15: // Exp DOT ID
    {
        if (!place)
            break;
        if (node->child->no == 16) {
            TableList* res = search(node->child->child->val.type_str);
            FieldList* field = res->type->u.structvar;
            unsigned offset = 0;
            while (strcmp(field->name, getChild(node, 2)->val.type_str)) {
                TableList* tmp = search(field->name);
                offset += tmp->size;
                field = field->tail;
            }
            Operand* con = getOperandwithValue(CONSTANT, offset);
            if (place) {
                if (place->type == VAL) {
                    Operand* t1 = newTemp(1);
                    if (res->op->kind == VARIABLE)
                        createBinop(ADD, GETADDR, t1, res->op, con);
                    else
                        createBinop(ADD, NORMAL, t1, res->op, con);
                    createAssign(GETVAL, place, t1);
                } else {
                    if (res->op->kind == VARIABLE)
                        createBinop(ADD, GETADDR, place, res->op, con);
                    else
                        createBinop(ADD, NORMAL, place, res->op, con);
                }
            }
        } else {
            Operand* t0 = newTemp(1);
            iExp(node->child, t0);
            FieldList* field = node->child->syn->type->u.structvar;
            unsigned offset = 0;
            while (strcmp(field->name, getChild(node, 2)->val.type_str)) {
                TableList* tmp = search(field->name);
                offset += tmp->size;
                field = field->tail;
            }

            Operand* con = getOperandwithValue(CONSTANT, offset);
            if (place) {
                if (node->syn->type->kind == INT) {
                    if (place->type == VAL) {
                        Operand* t1 = newTemp(1);
                        createBinop(ADD, NORMAL, t1, t0, con);
                        createAssign(GETVAL, place, t1);
                    } else {
                        createBinop(ADD, NORMAL, place, t0, con);
                    }
                } else {
                    createBinop(ADD, NORMAL, place, t0, con);
                }
            }
        }
    } break;
    case 16: // ID
    {
        if (place) {
            TableList* res = search(node->child->val.type_str);
            if (res->op->type == VAL) {
                if (place->type == VAL)
                    createAssign(NORMAL, place, res->op);
                else {
                    createAssign(SETVAL, place, res->op);
                }
            } else {
                if (res->op->kind == VARIABLE){
                    createAssign(GETADDR, place, res->op);
                }else{
                    createAssign(NORMAL, place, res->op);}
            }
        }
    } break;
    case 17: // INT
    {
        if (place) {

            Operand* op = getOperandwithValue(CONSTANT, node->child->val.type_int);
            createAssign(NORMAL, place, op);
        }
    } break;
    default:
        break;
    }
}

void cannotTranslate()
{
    printf("Cannot translate: Code contains variables of multi-dimensional array type or parameters of array type.\n");
    exit(-1);
}