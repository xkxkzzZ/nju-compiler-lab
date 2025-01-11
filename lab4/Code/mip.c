#include "mip.h"

extern unsigned varCount, tmpCount;
extern InterCodes* head;
int* varOffset;
int Regs;

void MIPS32(const char* fname)
{
    int n = varCount + tmpCount - 1;
    varOffset = (int*)malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++) {
        varOffset[i] = -1;
    }
    Regs = -1;

    InterCodes* p = head->next;
    FILE* f = fopen(fname, "w");
    fprintf(f, ".data\n_prompt: .asciiz \"Enter an integer:\"\n_ret: .asciiz \"\\n\"\n.globl main\n.text\n");
    fprintf(f, "read:\n  li $v0, 4\n  la $a0, _prompt\n  syscall\n  li $v0, 5\n  syscall\n  jr $ra\n\n");
    fprintf(f, "write:\n  li $v0, 1\n  syscall\n  li $v0, 4\n  la $a0, _ret\n  syscall\n  move $v0, $0\n  jr $ra\n");
    while (p) {
        if (!p->isDelete)
            switch (p->code.kind) {
            case FUNCTION:
                mfunction(f, p);
                break;
            case LABEL:
                mlabel(f, p);
                break;
            case GOTO:
                mgoto(f, p);
                break;
            case ASSIGN:
                massign(f, p);
                break;
            case ADD:
                madd(f, p);
                break;
            case SUB:
                msub(f, p);
                break;
            case MUL:
                mmul(f, p);
                break;
            case DIV:
                mdiv(f, p);
                break;
            case IF:
                mif(f, p);
                break;
            case RETURN:
                mreturn(f, p);
                break;
            case ARG:
                marg(f, p);
                break;
            case CALL:
                mcall(f, p);
                break;
            case PARAM: 
                p = mparam(f, p);
                break;
            case READ:
                mread(f, p);
                break;
            case WRITE:
                mwrite(f, p);
                break;
            default:
                break;
            }
        p = p->next;
    }
    fclose(f);
}

int calc(Operand* op, int fpoff)
{
    if (op->kind == VARIABLE || op->kind == TEMP || op->kind == PARAMETER) {
        int offset = 0;
        if (op->kind == TEMP) {
            offset = varCount - 1;
        }
        if (varOffset[op->u.var_no + offset] == -1) {
            varOffset[op->u.var_no + offset] = fpoff;
            return 4;
        } else
            return 0;
    } else
        return 0;
}

void mfunction(FILE* f, InterCodes* p)
{
    if (!strcmp("main", p->code.u.single.op->u.func_name))
        fprintf(f, "\nmain:\n");
    else
        fprintf(f, "\n_%s:\n", printOperand(p->code.u.single.op));
    fprintf(f, "  addi $sp, $sp, -4\n  sw $fp, 0($sp)\n  move $fp, $sp\n");
    int fpoff = 4;
    p = p->next;
    while (p && p->code.kind != FUNCTION) {
        if (!p->isDelete)
            switch (p->code.kind) {
            case ASSIGN: {
                fpoff += calc(p->code.u.assign.left, fpoff);
                fpoff += calc(p->code.u.assign.right, fpoff);
            } break;
            case ADD:
            case SUB:
            case MUL:
            case DIV: {
                fpoff += calc(p->code.u.binop.res, fpoff);
                fpoff += calc(p->code.u.binop.op1, fpoff);
                fpoff += calc(p->code.u.binop.op2, fpoff);
            } break;
            case IF: {
                fpoff += calc(p->code.u.cond.op1, fpoff);
                fpoff += calc(p->code.u.cond.op2, fpoff);
            } break;
            case DEC:
                if (varOffset[p->code.u.dec.op->u.var_no] == -1) {
                    varOffset[p->code.u.dec.op->u.var_no] = fpoff + p->code.u.dec.size - 4;
                    fpoff += p->code.u.dec.size;
                }
                break;
            case CALL: {
                fpoff += calc(p->code.u.sinop.res, fpoff);
            } break;
            case RETURN:
            case ARG:
            case PARAM:
            case READ:
            case WRITE:
                fpoff += calc(p->code.u.single.op, fpoff);
                break;
            }
        p = p->next;
    }
    fpoff -= 4;
    fprintf(f, "  addi $sp, $sp, -%d\n", fpoff);
}

void mlabel(FILE* f, InterCodes* p)
{
    fprintf(f, "%s:\n", printOperand(p->code.u.single.op));
}

void mgoto(FILE* f, InterCodes* p)
{
    fprintf(f, "  j %s\n", printOperand(p->code.u.single.op));
}

void massign(FILE* f, InterCodes* p)
{
    Operand* l = p->code.u.assign.left;
    Operand* r = p->code.u.assign.right;
    int loffset = 0;
    if (l->kind == TEMP)
        loffset = varCount - 1;
    switch (p->code.type) {
    case NORMAL: {
        Regs = varOffset[l->u.var_no + loffset];
        if (r->kind == CONSTANT) {
            fprintf(f, "  li $t0, %lld\n", r->u.value);
        } else {
            int roffset = 0;
            if (r->kind == TEMP)
                roffset = varCount - 1;
            fprintf(f, "  lw $t1, -%d($fp)\n", varOffset[r->u.var_no + roffset]);
            fprintf(f, "  move $t0, $t1\n");
        }
    } break;
    case GETADDR: {
        Regs = varOffset[l->u.var_no + loffset];
        int roffset = 0;
        if (r->kind == TEMP) {
            roffset = varCount - 1;
        }
        fprintf(f, "  addi $t0, $fp, -%d\n", varOffset[r->u.var_no + roffset]);
    } break;
    case GETVAL: {
        Regs = varOffset[l->u.var_no + loffset];
        int roffset = 0;
        if (r->kind == TEMP)
            roffset = varCount - 1;
        fprintf(f, "  lw $t1, -%d($fp)\n", varOffset[r->u.var_no + roffset]);
        fprintf(f, "  lw $t0, 0($t1)\n");
    } break;
    case SETVAL: {
        fprintf(f, "  lw $t0, -%d($fp)\n", varOffset[l->u.var_no + loffset]);
        int roffset = 0;
        if (r->kind == TEMP)
            roffset = varCount - 1;
        fprintf(f, "  lw $t1, -%d($fp)\n", varOffset[r->u.var_no + roffset]);
        fprintf(f, "  sw $t1, 0($t0)\n");
    } break;
    case COPY: {
        fprintf(f, "  lw $t0, -%d($fp)\n", varOffset[l->u.var_no + loffset]);
        int roffset = 0;
        if (r->kind == TEMP)
            roffset = varCount - 1;
        fprintf(f, "  lw $t1, -%d($fp)\n", varOffset[r->u.var_no + roffset]);
        fprintf(f, "  lw $t1, 0($t1)\n");
        fprintf(f, "  sw $t1, 0($t0)\n");
    } break;
    default:
        break;
    }
    if (Regs != -1) {
        fprintf(f, "  sw $t0, -%d($fp)\n", Regs);
        Regs = -1;
    }
}

void madd(FILE* f, InterCodes* p)
{
    Operand* res = p->code.u.binop.res;
    Operand* op1 = p->code.u.binop.op1;
    Operand* op2 = p->code.u.binop.op2;
    int resoffset = 0, op1offset = 0, op2offset = 0;
    if (res->kind == TEMP)
        resoffset = varCount - 1;
    if (op1->kind == TEMP)
        op1offset = varCount - 1;
    if (op2->kind == TEMP)
        op2offset = varCount - 1;
    Regs = varOffset[res->u.var_no + resoffset];
    if (p->code.type == NORMAL) {
        if (op1->kind != CONSTANT && op2->kind == CONSTANT) {
            fprintf(f, "  lw $t1, -%d($fp)\n", varOffset[op1->u.var_no + op1offset]);
            fprintf(f, "  addi $t0, $t1, %lld\n", op2->u.value);
        } else if (op1->kind == CONSTANT && op2->kind != CONSTANT) {
            fprintf(f, "  lw $t2, -%d($fp)\n", varOffset[op2->u.var_no + op2offset]);
            fprintf(f, "  addi $t0, $t2, %lld\n", op1->u.value);
        } else {
            fprintf(f, "  lw $t1, -%d($fp)\n", varOffset[op1->u.var_no + op1offset]);
            fprintf(f, "  lw $t2, -%d($fp)\n", varOffset[op2->u.var_no + op2offset]);
            fprintf(f, "  add $t0, $t1, $t2\n");
        }
    } else if (p->code.type == GETADDR) {
        if (op2->kind == CONSTANT) {
            int addroffset = op2->u.value - varOffset[op1->u.var_no + op1offset];
            fprintf(f, "  addi $t0, $fp, %d\n", addroffset);
        } else {
            fprintf(f, "  addi $t0, $fp, -%d\n", varOffset[op1->u.var_no + op1offset]);
            fprintf(f, "  lw $t2, -%d($fp)\n", varOffset[op2->u.var_no + op2offset]);
            fprintf(f, "  add $t0, $t0, $t2\n");
        }
    }
    if (Regs != -1) {
        fprintf(f, "  sw $t0, -%d($fp)\n", Regs);
        Regs = -1;
    }
}

void msub(FILE* f, InterCodes* p)
{
    Operand* res = p->code.u.binop.res;
    Operand* op1 = p->code.u.binop.op1;
    Operand* op2 = p->code.u.binop.op2;
    int resoffset = 0, op1offset = 0, op2offset = 0;
    if (res->kind == TEMP)
        resoffset = varCount - 1;
    if (op1->kind == TEMP)
        op1offset = varCount - 1;
    if (op2->kind == TEMP)
        op2offset = varCount - 1;
    Regs = varOffset[res->u.var_no + resoffset];
    if (op1->kind != CONSTANT && op2->kind == CONSTANT) {
        fprintf(f, "  lw $t1, -%d($fp)\n", varOffset[op1->u.var_no + op1offset]);
        fprintf(f, "  addi $t0, $t1, %lld\n", -op2->u.value);
    } else if (op1->kind == CONSTANT && op2->kind != CONSTANT) {
        fprintf(f, "  li $t1, %lld\n", op1->u.value);
        fprintf(f, "  lw $t2, -%d($fp)\n", varOffset[op2->u.var_no + op2offset]);
        fprintf(f, "  sub $t0, $t1, $t2\n");
    } else {
        fprintf(f, "  lw $t1, -%d($fp)\n", varOffset[op1->u.var_no + op1offset]);
        fprintf(f, "  lw $t2, -%d($fp)\n", varOffset[op2->u.var_no + op2offset]);
        fprintf(f, "  sub $t0, $t1, $t2\n");
    }
    if (Regs != -1) {
        fprintf(f, "  sw $t0, -%d($fp)\n", Regs);
        Regs = -1;
    }
}

void mmul(FILE* f, InterCodes* p)
{
    Operand* res = p->code.u.binop.res;
    Operand* op1 = p->code.u.binop.op1;
    Operand* op2 = p->code.u.binop.op2;
    int resoffset = 0, op1offset = 0, op2offset = 0;
    if (res->kind == TEMP)
        resoffset = varCount - 1;
    if (op1->kind == TEMP)
        op1offset = varCount - 1;
    if (op2->kind == TEMP)
        op2offset = varCount - 1;
    Regs = varOffset[res->u.var_no + resoffset];
    if (op1->kind != CONSTANT && op2->kind == CONSTANT) {
        fprintf(f, "  lw $t1, -%d($fp)\n", varOffset[op1->u.var_no + op1offset]);
        fprintf(f, "  li $t2, %lld\n", op2->u.value);
        fprintf(f, "  mul $t0, $t1, $t2\n");
    } else if (op1->kind == CONSTANT && op2->kind != CONSTANT) {
        fprintf(f, "  li $t1, %lld\n", op1->u.value);
        fprintf(f, "  lw $t2, -%d($fp)\n", varOffset[op2->u.var_no + op2offset]);
        fprintf(f, "  mul $t0, $t1, $t2\n");
    } else {
        fprintf(f, "  lw $t1, -%d($fp)\n", varOffset[op1->u.var_no + op1offset]);
        fprintf(f, "  lw $t2, -%d($fp)\n", varOffset[op2->u.var_no + op2offset]);
        fprintf(f, "  mul $t0, $t1, $t2\n");
    }
    if (Regs != -1) {
        fprintf(f, "  sw $t0, -%d($fp)\n", Regs);
        Regs = -1;
    }
}

void mdiv(FILE* f, InterCodes* p)
{
    Operand* res = p->code.u.binop.res;
    Operand* op1 = p->code.u.binop.op1;
    Operand* op2 = p->code.u.binop.op2;
    int resoffset = 0, op1offset = 0, op2offset = 0;
    if (res->kind == TEMP)
        resoffset = varCount - 1;
    if (op1->kind == TEMP)
        op1offset = varCount - 1;
    if (op2->kind == TEMP)
        op2offset = varCount - 1;
    Regs = varOffset[res->u.var_no + resoffset];
    if (op1->kind != CONSTANT && op2->kind == CONSTANT) {
        fprintf(f, "  lw $t1, -%d($fp)\n", varOffset[op1->u.var_no + op1offset]);
        fprintf(f, "  li $t2, %lld\n", op2->u.value);
        fprintf(f, "  div $t1, $t2\n  mflo $t0\n");
    } else if (op1->kind == CONSTANT && op2->kind != CONSTANT) {
        fprintf(f, "  li $t1, %lld\n", op1->u.value);
        fprintf(f, "  lw $t2, -%d($fp)\n", varOffset[op2->u.var_no + op2offset]);
        fprintf(f, "  div $t1, $t2\n  mflo $t0\n");
    } else {
        fprintf(f, "  lw $t1, -%d($fp)\n", varOffset[op1->u.var_no + op1offset]);
        fprintf(f, "  lw $t2, -%d($fp)\n", varOffset[op2->u.var_no + op2offset]);
        fprintf(f, "  div $t1, $t2\n  mflo $t0\n");
    }
    if (Regs != -1) {
        fprintf(f, "  sw $t0, -%d($fp)\n", Regs);
        Regs = -1;
    }
}

void mif(FILE* f, InterCodes* p)
{
    Operand* op1 = p->code.u.cond.op1;
    Operand* op2 = p->code.u.cond.op2;
    Operand* tar = p->code.u.cond.target;
    int resoffset = 0, op1offset = 0, op2offset = 0;
    if (op1->kind == TEMP)
        op1offset = varCount - 1;
    if (op2->kind == TEMP)
        op2offset = varCount - 1;
    if (op1->kind == CONSTANT)
        fprintf(f, "  li $t1, %lld\n", op1->u.value);
    else
        fprintf(f, "  lw $t1, -%d($fp)\n", varOffset[op1->u.var_no + op1offset]);
    if (op2->kind == CONSTANT)
        fprintf(f, "  li $t2, %lld\n", op2->u.value);
    else
        fprintf(f, "  lw $t2, -%d($fp)\n", varOffset[op2->u.var_no + op2offset]);
    char* relop = p->code.u.cond.relop;
    if (!strcmp("==", relop))
        fprintf(f, "  beq $t1, $t2, %s\n", printOperand(tar));
    else if (!strcmp("!=", relop))
        fprintf(f, "  bne $t1, $t2, %s\n", printOperand(tar));
    else if (!strcmp(">", relop))
        fprintf(f, "  bgt $t1, $t2, %s\n", printOperand(tar));
    else if (!strcmp("<", relop))
        fprintf(f, "  blt $t1, $t2, %s\n", printOperand(tar));
    else if (!strcmp(">=", relop))
        fprintf(f, "  bge $t1, $t2, %s\n", printOperand(tar));
    else if (!strcmp("<=", relop))
        fprintf(f, "  ble $t1, $t2, %s\n", printOperand(tar));
}

void mreturn(FILE* f, InterCodes* p)
{
    Operand* ret = p->code.u.single.op;
    int retoffset = 0;
    if (ret->kind == TEMP)
        retoffset = varCount - 1;
    if (ret->kind == CONSTANT) {
        fprintf(f, "  li $v0, %lld\n", ret->u.value);
    } else {
        fprintf(f, "  lw $v0, -%d($fp)\n", varOffset[ret->u.var_no + retoffset]);
    }
    fprintf(f, "  addi $sp, $fp, 4\n  lw $fp, 0($fp)\n");
    fprintf(f, "  jr $ra\n");
}

void marg(FILE* f, InterCodes* p)
{
    fprintf(f, "  addi $sp, $sp, -4\n");
    Operand* op = p->code.u.single.op;
    int opoffset = 0;
    if (op->kind == TEMP)
        opoffset = varCount - 1;
    if (op->kind == CONSTANT) {
        fprintf(f, "  li $t0, %lld\n", op->u.value);
    } else {
        fprintf(f, "  lw $t0, -%d($fp)\n", varOffset[op->u.var_no + opoffset]);
    }
    fprintf(f, "  sw $t0, 0($sp)\n");
}

void mcall(FILE* f, InterCodes* p)
{
    Operand* res = p->code.u.sinop.res;
    Operand* op = p->code.u.sinop.op;
    fprintf(f, "  addi $sp, $sp, -4\n  sw $ra, 0($sp)\n");
    if (!strcmp("main", op->u.func_name))
        fprintf(f, "  jal main\n");
    else
        fprintf(f, "  jal _%s\n", printOperand(op));
    fprintf(f, "  lw $ra, 0($sp)\n  addi $sp, $sp, 4\n");
    int resoffset = 0;
    if (res->kind == TEMP)
        resoffset = varCount - 1;
    fprintf(f, "  sw $v0, -%d($fp)\n", varOffset[res->u.var_no + resoffset]);
    TableList* func = search(op->u.func_name);
    int argc = func->type->u.function.argc;
    fprintf(f, "  addi $sp, $sp, %d\n", argc * 4);
}

InterCodes* mparam(FILE* f, InterCodes* p){
    TableList* func = search(p->prev->code.u.single.op->u.func_name);
    int argc = func->type->u.function.argc;
    for (int i = 0; i < argc; i++) {
        Operand* op = p->code.u.single.op;
        fprintf(f, "  lw $t0, %d($fp)\n", 8 + i * 4);
        fprintf(f, "  sw $t0, -%d($fp)\n", varOffset[op->u.var_no]);
        p = p->next;
    }
    p = p->prev;
    return p;
}

void mread(FILE* f, InterCodes* p)
{
    Operand* op = p->code.u.single.op;
    int opoffset = 0;
    if (op->kind == TEMP) {
        opoffset = varCount - 1;
    }
    fprintf(f, "  addi $sp, $sp, -4\n  sw $ra, 0($sp)\n  jal read\n  lw $ra, 0($sp)\n  addi $sp, $sp, 4\n");
    fprintf(f, "  sw $v0, -%d($fp)\n", varOffset[op->u.var_no + opoffset]);
}

void mwrite(FILE* f, InterCodes* p)
{
    Operand* op = p->code.u.single.op;
    if (op->kind == CONSTANT) {
        fprintf(f, "  li $a0, %lld\n", op->u.value);
    } else {
        int opoffset = 0;
        if (op->kind == TEMP) {
            opoffset = varCount - 1;
        }
        fprintf(f, "  lw $a0, -%d($fp)\n", varOffset[op->u.var_no + opoffset]);
    }
    fprintf(f, "  addi $sp, $sp, -4\n  sw $ra, 0($sp)\n  jal write\n  lw $ra, 0($sp)\n  addi $sp, $sp, 4\n");
}