#include "qaq.h"
InterCodes* head;
InterCodes* tail;
extern TableList* hashTable[HASHSIZE + 1];
unsigned varCount = 1, tmpCount = 1, labelCount = 1;

Operand* getOperandwithValue(int kind, long long int value)
{
    Operand* op = (Operand*)malloc(sizeof(Operand));
    op->kind = kind;
    op->u.value = value;
    return op;
}
Operand* getOperandwithNo(int kind, int no)
{
    Operand* op = (Operand*)malloc(sizeof(Operand));
    op->kind = kind;
    op->u.var_no = no;
    return op;
}
Operand* getOperandwithName(int kind, char* name)
{
    Operand* op = (Operand*)malloc(sizeof(Operand));
    op->kind = kind;
    op->u.func_name = name;
    return op;
}

Operand* newConst(long long int val)
{
    Operand* op = getOperandwithValue(CONSTANT, val);
    return op;
}

Operand* newTemp(int addr)
{
    Operand* op = getOperandwithNo(TEMP, tmpCount);
    op->type = addr ? ADDRESS : VAL;
    ++tmpCount;
    return op;
}

Operand* newLabel()
{
    Operand* op = getOperandwithNo(LAB, labelCount);
    ++labelCount;
    return op;
}
InterCodes* makeinter(int kind)
{
    InterCodes* p = (InterCodes*)malloc(sizeof(InterCodes));
    p->code.kind = kind;
    p->next = NULL;
    p->prev = tail;
    tail->next = p;
    tail = tail->next;
    return p;
}
void createAssign(unsigned type, Operand* left, Operand* right)
{
    InterCodes* p = makeinter(ASSIGN);
    p->code.type = type;
    p->code.u.assign.left = left;
    p->code.u.assign.right = right;
}

void createBinop(unsigned kind, unsigned type, Operand* res, Operand* op1, Operand* op2)
{
    InterCodes* p = makeinter(kind);
    p->code.type = type;
    p->code.u.binop.res = res;
    p->code.u.binop.op1 = op1;
    p->code.u.binop.op2 = op2;
}

void createSinop(unsigned kind, Operand* res, Operand* op)
{
    InterCodes* p = makeinter(kind);
    p->code.u.sinop.res = res;
    p->code.u.sinop.op = op;
}

void createSingle(unsigned kind, Operand* op)
{
    InterCodes* p = makeinter(kind);
    p->code.u.single.op = op;
}

void createCond(Operand* op1, Operand* op2, Operand* target, char* re)
{
    InterCodes* p = makeinter(IF);
    p->code.u.cond.op1 = op1;
    p->code.u.cond.op2 = op2;
    p->code.u.cond.target = target;
    strcpy(p->code.u.cond.relop, re);
}

void createDec(Operand* op, unsigned size)
{
    InterCodes* p = makeinter(DEC);
    p->code.u.dec.op = op;
    p->code.u.dec.size = size;
}
unsigned countSize(Type* type)
{
    if (type->kind == ARRAY) {
        return type->u.array.size * countSize(type->u.array.elem);
    } else if (type->kind == STRUCTVAR) {
        FieldList* field = type->u.structvar;
        unsigned size = 0;
        while (field) {
            size += countSize(field->type);
            field = field->tail;
        }
        return size;
    } else if (type->kind == INT || type->kind == FLOAT)
        return 4;
    else {
        printf("Wrong Type!\n");
        exit(-1);
    }
}

void setVariable()
{
    for (int i = 0; i < HASHSIZE + 1; ++i) {
        if (!hashTable[i])
            continue;
        else {
            TableList* p = hashTable[i];
            while (p) {
                if (p->type->kind != WRONGFUNC && p->type->kind != FUNC && p->type->kind != STRUCTURE) {
                    Operand* op = (Operand*)malloc(sizeof(Operand));
                    op->kind = VARIABLE;
                    if (p->type->kind == ARRAY || p->type->kind == STRUCTVAR) {
                        op->type = ADDRESS;
                        p->size = countSize(p->type);
                    } else {
                        op->type = VAL;
                    }
                    op->u.var_no = varCount;
                    p->op = op;
                    ++varCount;
                }
                p = p->next;
            }
        }
    }
}

char* printOperand(Operand* op)
{
    char* res = (char*)malloc(40);
    switch (op->kind) {
    case CONSTANT:
        sprintf(res, "#%lld", op->u.value);
        break;
    case VARIABLE:
        sprintf(res, "v%d", op->u.var_no);
        break;
    case LAB:
        sprintf(res, "label%d", op->u.var_no);
        break;
    case FUNCT:
        sprintf(res, "%s", op->u.func_name);
        break;

    case TEMP:
        sprintf(res, "t%d", op->u.var_no);
        break;
    case PARAMETER:
        sprintf(res, "v%d", op->u.var_no);
        break;

    default:
        break;
    }
    return res;
}

void writeInterCodes(const char* fielname, bool opt)
{
    InterCodes* p = head->next;
    FILE* f = fopen(fielname, "w");
    while (p) {
        if (!p->isDelete || !opt)
            switch (p->code.kind) {
            case READ:
                fprintf(f, "READ %s\n", printOperand(p->code.u.single.op));
                break;
            case WRITE:
                if (p->code.u.single.op->type == VAL || p->code.u.single.op->kind == CONSTANT)
                    fprintf(f, "WRITE %s\n", printOperand(p->code.u.single.op));
                else
                    fprintf(f, "WRITE *%s\n", printOperand(p->code.u.single.op));
                break;
            case ASSIGN: {
                char* l = printOperand(p->code.u.assign.left);
                char* r = printOperand(p->code.u.assign.right);
                switch (p->code.type) {
                case NORMAL:
                    fprintf(f, "%s := %s\n", l, r);
                    break;
                case GETADDR:
                    fprintf(f, "%s := &%s\n", l, r);
                    break;
                case GETVAL:
                    fprintf(f, "%s := *%s\n", l, r);
                    break;
                case SETVAL:
                    fprintf(f, "*%s := %s\n", l, r);
                    break;
                case COPY:
                    fprintf(f, "*%s := *%s\n", l, r);
                    break;
                default:
                    break;
                }
            } break;
            case ADD: {
                char* r = printOperand(p->code.u.binop.res);
                char* op1 = printOperand(p->code.u.binop.op1);
                char* op2 = printOperand(p->code.u.binop.op2);
                if (p->code.type == NORMAL)
                    fprintf(f, "%s := %s + %s\n", r, op1, op2);
                else if (p->code.type == GETADDR)
                    fprintf(f, "%s := &%s + %s\n", r, op1, op2);
                else {
                    printf("Wrong Binop:ADD type!\n");
                    exit(-1);
                }
            } break;
            case SUB: {
                char* r = printOperand(p->code.u.binop.res);
                char* op1 = printOperand(p->code.u.binop.op1);
                char* op2 = printOperand(p->code.u.binop.op2);
                fprintf(f, "%s := %s - %s\n", r, op1, op2);
            } break;
            case MUL: {
                char* r = printOperand(p->code.u.binop.res);
                char* op1 = printOperand(p->code.u.binop.op1);
                char* op2 = printOperand(p->code.u.binop.op2);
                fprintf(f, "%s := %s * %s\n", r, op1, op2);
            } break;
            case DIV: {
                char* r = printOperand(p->code.u.binop.res);
                char* op1 = printOperand(p->code.u.binop.op1);
                char* op2 = printOperand(p->code.u.binop.op2);
                fprintf(f, "%s := %s / %s\n", r, op1, op2);
            } break;

            case LABEL:
                fprintf(f, "LABEL %s :\n", printOperand(p->code.u.single.op));
                break;
            case FUNCTION:
                fprintf(f, "FUNCTION %s :\n", printOperand(p->code.u.single.op));
                break;
            case PARAM:
                fprintf(f, "PARAM %s\n", printOperand(p->code.u.single.op));
                break;

            case GOTO:
                fprintf(f, "GOTO %s\n", printOperand(p->code.u.single.op));
                break;
            case IF: {
                char* op1 = printOperand(p->code.u.cond.op1);
                char* op2 = printOperand(p->code.u.cond.op2);
                char* tar = printOperand(p->code.u.cond.target);
                fprintf(f, "IF %s %s %s GOTO %s\n", op1, p->code.u.cond.relop, op2, tar);
            } break;
            case RETURN:
                fprintf(f, "RETURN %s\n", printOperand(p->code.u.single.op));
                break;
            case DEC:
                fprintf(f, "DEC %s %u\n", printOperand(p->code.u.dec.op), p->code.u.dec.size);
                break;
            case ARG:
                fprintf(f, "ARG %s\n", printOperand(p->code.u.single.op));
                break;
            case CALL: {
                char* res = printOperand(p->code.u.sinop.res);
                char* op = printOperand(p->code.u.sinop.op);
                fprintf(f, "%s := CALL %s\n", res, op);
            } break;

            default:
                break;
            }
        p = p->next;
    }
    fclose(f);
}