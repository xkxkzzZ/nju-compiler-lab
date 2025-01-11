#include "senatic.h"
#include "same.h"
TableList* hashTable[HASHSIZE + 1];

void Program(Node* node)
{
    if (!strcmp("ExtDefList", node->child->name)) {
        ExtDefList(node->child);
    }
}

void ExtDefList(Node* node)
{
    if (node->no == 2) // ExtDef ExtDefList
    {
        ExtDef(node->child);
        ExtDefList(get_child(node, 1));
    }
}

void ExtDef(Node* node)
{
    // printf("ExtDef\n");
    Type* type = (Type*)malloc(sizeof(Type));
    FieldList* attr = newFieldList(NULL, type, NULL);
    switch (node->no) {
    case 1: // Specifier ExtDecList SEMI
        node->child->inh = attr;
        Specifier(node->child);
        get_child(node, 1)->inh = node->child->syn;
        ExtDecList(get_child(node, 1));
        break;
    case 2: // Specifier SEMI
        node->child->inh = attr;
        Specifier(node->child);
        break;
    case 3: // Specifier FunDec CompSt
        type->kind = FUNC;
        node->child->inh = attr;
        get_child(node, 1)->inh = attr;
        Specifier(node->child);
        if (node->child->syn->type->kind == STRUCTURE) {
            Type* type = (Type*)malloc(sizeof(Type));
            type->kind = STRUCTVAR;
            type->u.structvar = node->child->syn->type->u.structure;
            node->child->inh->type->u.func.ret = type;
        } else {
            node->child->inh->type->u.func.ret = node->child->syn->type;
        }
        node->child->inh->type->u.func.argc = 0;
        node->child->inh->type->u.func.args = NULL;
        FunDec(get_child(node, 1));
        get_child(node, 2)->inh = node->child->syn;
        CompSt(get_child(node, 2));
        break;
    default:
        break;
    }
}

void Specifier(Node* node)
{
    // printf("Specifier\n");
    Type* type = (Type*)malloc(sizeof(Type));
    switch (node->no) {
    case 1: // TYPE
        if (!strcmp("int", node->child->val.type_str)) {
            type->kind = eINT;
        } else {
            type->kind = eFLOAT;
        }
        node->syn = newFieldList(NULL, type, NULL);
        break;
    case 2: // StructSpecifier
    {
        type->kind = STRUCTURE;
        type->u.structure = NULL;
        node->child->inh = newFieldList(NULL, type, NULL);
        StructSpecifier(node->child);
        node->syn = node->child->syn;
    } break;
    default:
        break;
    }
}

char msg[1024];

void VarDec(Node* node)
{
    // printf("VarDec\n");
    TableList* res;
    //
    switch (node->no) {
    case 1: // ID
        res = search(node->child->val.type_str);
        //
        if (!res) {
            TableList* item = newTableList(node->child->val.type_str, node->inh->type, NULL, NULL);
            switch (node->inh->type->kind) {
            case eINT:
            case eFLOAT:
            case ARRAY:
                node->syn = newFieldList(node->child->val.type_str, node->inh->type, NULL);
                break;
            case STRUCTVAR:
            case STRUCTURE: {
                if (!node->inh->type->u.structure) {
                    sprintf(msg, "Undefined structure \"%s\"", node->inh->name);
                    print_error(17, node->line, msg);
                    node->syn = NULL;
                    return;
                }
                Type* type = (Type*)malloc(sizeof(Type));
                type->kind = STRUCTVAR;
                type->u.structvar = node->inh->type->u.structure;
                item->type = type;
                node->syn = newFieldList(node->child->val.type_str, type, NULL);
            } break;
            default:
                printf("name: %s, type: %d\n", node->child->val.type_str, node->inh->type->kind);
                exit(-1);
                break;
            }
            insert(item);
        } else {
            if (node->instruct) {
                sprintf(msg, "Redefined field \"%s\"", node->child->val.type_str);
                print_error(15, node->line, msg);
            } else {
                sprintf(msg, "Redefined variable \"%s\"", node->child->val.type_str);
                print_error(3, node->line, msg);
            }
        }
        break;
    case 2: // VarDec LB INT RB
    {
        if (node->instruct)
            node->child->instruct = 1;
        Type* type = (Type*)malloc(sizeof(Type));
        type->kind = ARRAY;
        if (node->inh->type->kind == STRUCTURE) {
            Type* p = (Type*)malloc(sizeof(Type));
            p->kind = STRUCTVAR;
            p->u.structvar = node->inh->type->u.structure;
            type->u.array.elem = p;
        } else {
            type->u.array.elem = node->inh->type;
        }
        node->child->inh = newFieldList(NULL, type, NULL);
        VarDec(node->child);
        node->syn = node->child->syn;
        break;
    }
    default:
        break;
    }
}

void ExtDecList(Node* node)
{
    // printf("ExtDecList\n");
    if (node->inh->type->kind == STRUCTURE && !node->inh->type->u.structure) {
        sprintf(msg, "Undefined structure \"%s\"", node->inh->name);
        print_error(17, node->line, msg);
        return;
    }
    node->child->inh = node->inh;
    VarDec(node->child);
    node->syn = node->child->syn;
    switch (node->no) {
    case 1: // VarDec
        break;
    case 2: // VarDec COMMA ExtDecList
    {
        get_child(node, 2)->inh = node->inh;
        ExtDecList(get_child(node, 2));
        FieldList* p = node->syn;
        while (p->tail) {
            p = p->tail;
        }
        p->tail = get_child(node, 2)->syn;
    } break;
    default:
        break;
    }
}

void Tag(Node* node)
{
    // printf("Tag\n");
    //
    TableList* res = search(node->child->val.type_str);
    if (!res) {
        node->inh->name = node->child->val.type_str;
    } else {
        node->inh->name = res->name;
        node->inh->type = res->type;
    }
}

void DecList(Node* node)
{
    // printf("DecList\n");
    //
    if (node->instruct)
        node->child->instruct = 1;
    if (node->inh->type->kind == STRUCTURE && !node->inh->type->u.structure) {
        sprintf(msg, "Undefined structure \"%s\"", node->inh->name);
        print_error(17, node->line, msg);
        return;
    }
    node->child->inh = node->inh;
    Dec(node->child);
    node->syn = node->child->syn;
    switch (node->no) {
    case 1: // Dec
        break;
    case 2: // Dec COMMA DecList
    {
        if (node->instruct)
            get_child(node, 2)->instruct = 1;
        get_child(node, 2)->inh = node->inh;
        DecList(get_child(node, 2));
        FieldList* p = node->syn;
        while (p->tail) {
            p = p->tail;
        }
        p->tail = get_child(node, 2)->syn;
    } break;
    default:
        break;
    }
}

void VarList(Node* node)
{
    // printf("VarList\n");
    ParamDec(node->child);
    if (node->child->syn)
        node->inh->type->u.func.argc++;
    else {
        node->inh->type->kind = WRONGFUNC;
        return;
    }
    node->syn = node->child->syn;
    switch (node->no) {
    case 1: // ParamDec COMMA VarList
    {
        get_child(node, 2)->inh = node->inh;
        VarList(get_child(node, 2));
        FieldList* p = node->syn;
        while (p->tail) {
            p = p->tail;
        }
        p->tail = get_child(node, 2)->syn;
    } break;
    case 2: // ParamDec
        break;
    default:
        break;
    }
}

void ParamDec(Node* node)
{
    // printf("ParamDec\n");
    Specifier(node->child);
    get_child(node, 1)->inh = node->child->syn;
    VarDec(get_child(node, 1));
    node->syn = get_child(node, 1)->syn;
    if (get_child(node, 1)->syn) {
        node->syn = (FieldList*)malloc(sizeof(FieldList));
        node->syn->name = get_child(node, 1)->syn->name;
        node->syn->type = get_child(node, 1)->syn->type;
        node->syn->tail = NULL;
    } else {
        node->syn = NULL;
    }
}

void Dec(Node* node)
{
    // printf("Dec\n");
    //
    if (node->instruct)
        node->child->instruct = 1;
    node->child->inh = node->inh;
    VarDec(node->child);
    node->syn = node->child->syn;
    switch (node->no) {
    case 1: // VarDec
        break;
    case 2: // VarDec ASSIGNOP Exp
        if (node->instruct) {
            sprintf(msg, "Try to init field \"%s\" in structure", node->child->syn->name);
            print_error(15, node->child->line, msg);
            return;
        }
        Exp(get_child(node, 2));
        if (node->child->syn && get_child(node, 2)->syn) {
            if (node->child->syn->type->kind != get_child(node, 2)->syn->type->kind) {
                print_error(5, node->child->line, "Type mismatched for assignment");
            }
        }
        break;
    default:
        break;
    }
}

void StructSpecifier(Node* node)
{
    // printf("StructSpecifier\n");
    switch (node->no) {
    case 1: // STRUCT OptTag LC DefList RC
    {
        Type* type = (Type*)malloc(sizeof(Type));
        type->kind = STRUCTURE;
        FieldList* field = (FieldList*)malloc(sizeof(FieldList));
        field->tail = NULL;
        field->type = type;
        get_child(node, 1)->inh = node->inh;
        get_child(node, 3)->inh = node->inh;
        get_child(node, 3)->instruct = 1;
        OptTag(get_child(node, 1));
        DefList(get_child(node, 3));
        if (!node->inh->type->u.structure) {
            node->inh->type->u.structure = get_child(node, 3)->syn;
        } else {
            FieldList* p = node->inh->type->u.structure;
            if (p->type->kind == STRUCTURE) {
                p->type->u.structure = field;
            } else {
                p->tail = field;
            }
        }
        node->syn = node->inh;
    } break;
    case 2: // STRUCT Tag
        node->inh->type->u.structure = NULL;
        get_child(node, 1)->inh = node->inh;
        Tag(get_child(node, 1));
        node->syn = node->inh;
        break;
    default:
        break;
    }
}

void OptTag(Node* node)
{
    // printf("OptTag\n");

    if (node->no == 2) // ID
    {
        TableList* res = search(node->child->val.type_str);
        if (!res) {
            TableList* item = newTableList(node->child->val.type_str, node->inh->type, NULL, NULL);
            item->type = node->inh->type;
            node->inh->name = node->child->val.type_str;
            insert(item);
        } else {
            sprintf(msg, "Duplicated name \"%s\"", node->child->val.type_str);
            print_error(16, node->line, msg);
        }
    }
}

void StmtList(Node* node)
{
    // printf("StmtList\n");
    if (node->no == 2) {
        node->child->inh = node->inh;
        get_child(node, 1)->inh = node->inh;
        Stmt(node->child);
        StmtList(get_child(node, 1));
    }
}

void Stmt(Node* node)
{
    // printf("Stmt\n");
    switch (node->no) {
    case 1: // Exp SEMI
        Exp(node->child);
        break;

    case 4: // IF LP Exp RP Stmt
        get_child(node, 4)->inh = node->inh;
        Exp(get_child(node, 2));
        if (get_child(node, 2)->syn) {
            if (get_child(node, 2)->syn->type->kind != eINT) {
                print_error(7, get_child(node, 2)->line, "Type mismatched for operands");
            }
        }
        Stmt(get_child(node, 4));
        break;
    case 5: // IF LP Exp RP Stmt ELSE Stmt
        get_child(node, 4)->inh = node->inh;
        get_child(node, 6)->inh = node->inh;
        Exp(get_child(node, 2));
        if (get_child(node, 2)->syn) {
            if (get_child(node, 2)->syn->type->kind != eINT) {
                print_error(7, get_child(node, 2)->line, "Type mismatched for operands");
            }
        }
        Stmt(get_child(node, 4));
        Stmt(get_child(node, 6));
        break;
    case 2: // CompSt
        node->child->inh = node->inh;
        CompSt(node->child);
        break;
    case 3: // RETURN Exp SEMI
        Exp(get_child(node, 1));
        if (get_child(node, 1)->syn) {
            if (node->inh->type->kind != get_child(node, 1)->syn->type->kind) {
                if (node->inh->type->kind == STRUCTURE && get_child(node, 1)->syn->type->kind == STRUCTVAR) {
                    int res = same_struct(node->inh->type->u.structure, get_child(node, 1)->syn->type->u.structvar);
                    if (res)
                        break;
                }
                print_error(8, get_child(node, 1)->line, "Type mismatched for return");
            }
        }
        break;

    case 6: // WHILE LP Exp RP Stmt
        get_child(node, 4)->inh = node->inh;
        Exp(get_child(node, 2));
        if (get_child(node, 2)->syn) {
            if (get_child(node, 2)->syn->type->kind != eINT) {
                print_error(7, get_child(node, 2)->line, "Type mismatched for operands");
            }
        }
        Stmt(get_child(node, 4));
        break;
    default:
        break;
    }
}

void Def(Node* node)
{
    // printf("Def\n");
    // Specifier DecList SEMI

    if (node->instruct) {
        node->child->instruct = 1;
        get_child(node, 1)->instruct = 1;
    }
    node->child->inh = node->inh;
    Specifier(node->child);
    get_child(node, 1)->inh = node->child->syn;
    DecList(get_child(node, 1));
    node->syn = get_child(node, 1)->syn;
}

void Exp(Node* node)
{
    // printf("Exp\n");
    TableList* res;
    switch (node->no) {
    case 11: // NOT Exp
        Exp(get_child(node, 1));
        if (get_child(node, 1)->syn) {
            if (get_child(node, 1)->syn->type->kind == eINT) {
                node->syn = get_child(node, 1)->syn;
            } else {
                print_error(7, node->line, "Type mismatched for operands");
            }
        }
        break;

    case 14: // Exp LB Exp RB
    {
        Exp(node->child);
        FieldList* attr = node->child->syn;
        if (!attr)
            return;
        if (attr->type->kind != ARRAY) {
            sprintf(msg, "\"%s\" is not a array", attr->name);
            print_error(10, node->child->line, msg);
        } else {
            FieldList* p = (FieldList*)malloc(sizeof(FieldList));
            p->tail = NULL;
            p->name = attr->name;
            p->type = attr->type->u.array.elem;
            node->syn = p;
        }
        Exp(get_child(node, 2));
        attr = get_child(node, 2)->syn;
        if (!attr)
            return;
        if (attr->type->kind != eINT) {
            sprintf(msg, "Index of array \"%s\" is not an integer", node->child->syn->name);
            print_error(12, get_child(node, 2)->line, msg);
        }
    } break;

    case 12: // ID LP Args RP
        res = search(node->child->val.type_str);
        Args(get_child(node, 2));
        if (!get_child(node, 2)->syn) {
            node->syn = NULL;
            return;
        }
        if (res) {
            if (res->type->kind != FUNC) {
                sprintf(msg, "\"%s\" is not a function", res->name);
                print_error(11, node->line, msg);
            } else {
                int tmp = same_func(res->type->u.func.args, get_child(node, 2)->syn);
                if (tmp) {
                    node->syn = newFieldList(NULL, res->type->u.func.ret, NULL);
                } else {
                    sprintf(msg, "Function \"%s\" is not applicable for arguments", res->name);
                    print_error(9, get_child(node, 2)->line, msg);
                }
            }
        } else {
            sprintf(msg, "Undefined function \"%s\"", node->child->val.type_str);
            print_error(2, node->line, msg);
            return;
        }
        break;
    case 13: // ID LP RP
        res = search(node->child->val.type_str);
        if (res) {
            if (res->type->kind != FUNC) {
                sprintf(msg, "\"%s\" is not a function", res->name);
                print_error(11, node->line, msg);
            } else if (res->type->u.func.argc != 0) {
                sprintf(msg, "Function \"%s\" is not applicable for arguments", res->name);
                print_error(9, node->line, msg);
            } else {
                node->syn = newFieldList(NULL, res->type->u.func.ret, NULL);
            }
        } else {
            sprintf(msg, "Undefined function \"%s\"", node->child->val.type_str);
            print_error(2, node->line, msg);
        }
        break;
    case 15: // Exp DOT ID
    {
        Exp(node->child);
        FieldList* attr = node->child->syn;
        if (!attr)
            return;
        if (attr->type->kind != STRUCTVAR) {
            print_error(13, node->child->line, "Illegal use of \".\"");
        } else {
            int flag = 0;
            for (FieldList* p = attr->type->u.structvar; p; p = p->tail) {
                if (!strcmp(get_child(node, 2)->val.type_str, p->name)) {
                    flag = 1;
                    node->syn = p;
                    break;
                }
            }
            if (!flag) {
                sprintf(msg, "Non-existent field \"%s\"", get_child(node, 2)->val.type_str);
                print_error(14, node->child->line, msg);
            }
        }
    } break;
    case 16: // ID
        res = search(node->child->val.type_str);
        if (res) {
            node->syn = newFieldList(res->name, res->type, NULL);
        } else {
            sprintf(msg, "Undefined Variable \"%s\"", node->child->val.type_str);
            print_error(1, node->line, msg);
        }
        break;

    case 1: // Exp ASSIGNOP Exp
    {
        int id = node->child->no;
        if (id < 14 || id > 16) {
            print_error(6, node->child->line, "The left-hand side of an assignment must be a variable");
            Exp(get_child(node, 2));
            return;
        } else {
            Exp(node->child);
        }
        Exp(get_child(node, 2));
        if (node->child->syn && get_child(node, 2)->syn) {
            if (node->child->syn->type->kind != get_child(node, 2)->syn->type->kind) {
                print_error(5, node->child->line, "Type mismatched for assignment");
            } else if (node->child->syn->type->kind == STRUCTVAR) {
                FieldList* p = node->child->syn->type->u.structvar;
                FieldList* q = get_child(node, 2)->syn->type->u.structvar;
                int res = same_struct(p, q);
                if (!res) {
                    print_error(5, node->child->line, "Type mismatched for assignment");
                } else {
                    node->syn = node->child->syn;
                }
            } else if (node->child->syn->type->kind == ARRAY) {
                Type* p = node->child->syn->type;
                Type* q = get_child(node, 2)->syn->type;
                int res = same_array(p, q);
                if (!res) {
                    print_error(5, node->child->line, "Type mismatched for assignment");
                } else {
                    node->syn = node->child->syn;
                }
            } else {
                node->syn = node->child->syn;
            }
        } else {
            print_error(5, node->child->line, "Type mismatched for assignment");
        }
    } break;
    case 2: // Exp AND Exp
        Exp(node->child);
        Exp(get_child(node, 2));
        if (node->child->syn && get_child(node, 2)->syn) {
            if (node->child->syn->type->kind != eINT || get_child(node, 2)->syn->type->kind != eINT) {
                print_error(7, node->child->line, "Type mismatched for operands");
            } else {
                Type* type = (Type*)malloc(sizeof(Type));
                type->kind = eINT;
                node->syn = newFieldList(NULL, type, NULL);
            }
        }
        break;
    case 3: // Exp OR Exp
        Exp(node->child);
        Exp(get_child(node, 2));
        if (node->child->syn && get_child(node, 2)->syn) {
            if (node->child->syn->type->kind != eINT || get_child(node, 2)->syn->type->kind != eINT) {
                print_error(7, node->child->line, "Type mismatched for operands");
            } else {
                Type* type = (Type*)malloc(sizeof(Type));
                type->kind = eINT;
                node->syn = newFieldList(NULL, type, NULL);
            }
        }
        break;

    case 17: // INT
    {
        Type* type = (Type*)malloc(sizeof(Type));
        type->kind = eINT;
        node->syn = newFieldList(NULL, type, NULL);
    } break;
    case 18: // FLOAT
    {
        Type* type = (Type*)malloc(sizeof(Type));
        type->kind = eFLOAT;
        node->syn = newFieldList(NULL, type, NULL);
    } break;

    case 6: // Exp MINUS Exp
        Exp(node->child);
        Exp(get_child(node, 2));
        if (node->child->syn && get_child(node, 2)->syn) {
            if (node->child->syn->type->kind == eINT && get_child(node, 2)->syn->type->kind == eINT || node->child->syn->type->kind == eFLOAT && get_child(node, 2)->syn->type->kind == eFLOAT) {
                Type* type = (Type*)malloc(sizeof(Type));
                type->kind = node->child->syn->type->kind;
                node->syn = newFieldList(NULL, type, NULL);
            } else {
                print_error(7, node->child->line, "Type mismatched for operands");
            }
        } else {
            print_error(7, node->child->line, "Type mismatched for operands");
        }
        break;
    case 7: // Exp STAR Exp
        Exp(node->child);
        Exp(get_child(node, 2));
        if (node->child->syn && get_child(node, 2)->syn) {
            if (node->child->syn->type->kind == eINT && get_child(node, 2)->syn->type->kind == eINT || node->child->syn->type->kind == eFLOAT && get_child(node, 2)->syn->type->kind == eFLOAT) {
                Type* type = (Type*)malloc(sizeof(Type));
                type->kind = node->child->syn->type->kind;
                node->syn = newFieldList(NULL, type, NULL);
            } else {
                print_error(7, node->child->line, "Type mismatched for operands");
            }
        } else {
            print_error(7, node->child->line, "Type mismatched for operands");
        }
        break;
    case 8: // Exp DIV Exp
        Exp(node->child);
        Exp(get_child(node, 2));
        if (node->child->syn && get_child(node, 2)->syn) {
            if (node->child->syn->type->kind == eINT && get_child(node, 2)->syn->type->kind == eINT || node->child->syn->type->kind == eFLOAT && get_child(node, 2)->syn->type->kind == eFLOAT) {
                Type* type = (Type*)malloc(sizeof(Type));
                type->kind = node->child->syn->type->kind;
                node->syn = newFieldList(NULL, type, NULL);
            } else {
                print_error(7, node->child->line, "Type mismatched for operands");
            }
        } else {
            print_error(7, node->child->line, "Type mismatched for operands");
        }
        break;
    case 4: // Exp RELOP Exp
        Exp(node->child);
        Exp(get_child(node, 2));
        if (node->child->syn && get_child(node, 2)->syn) {
            if (node->child->syn->type->kind == eINT && get_child(node, 2)->syn->type->kind == eINT || node->child->syn->type->kind == eFLOAT && get_child(node, 2)->syn->type->kind == eFLOAT) {
                Type* type = (Type*)malloc(sizeof(Type));
                type->kind = eINT;
                node->syn = newFieldList(NULL, type, NULL);
            } else {
                print_error(7, node->child->line, "Type mismatched for operands");
            }
        }
        break;
    case 5: // Exp PLUS Exp
        Exp(node->child);
        Exp(get_child(node, 2));
        if (node->child->syn && get_child(node, 2)->syn) {
            if (node->child->syn->type->kind == eINT && get_child(node, 2)->syn->type->kind == eINT || node->child->syn->type->kind == eFLOAT && get_child(node, 2)->syn->type->kind == eFLOAT) {
                Type* type = (Type*)malloc(sizeof(Type));
                type->kind = node->child->syn->type->kind;
                node->syn = newFieldList(NULL, type, NULL);
            } else {
                print_error(7, node->child->line, "Type mismatched for operands");
            }
        } else {
            print_error(7, node->child->line, "Type mismatched for operands");
        }
        break;

    case 9: // LP Exp RP
        Exp(get_child(node, 1));
        node->syn = get_child(node, 1)->syn;
        break;
    case 10: // MINUS Exp
        Exp(get_child(node, 1));
        if (get_child(node, 1)->syn) {
            if (get_child(node, 1)->syn->type->kind == eINT || get_child(node, 1)->syn->type->kind == eFLOAT) {
                node->syn = get_child(node, 1)->syn;
            } else {
                print_error(7, node->line, "Type mismatched for operands");
            }
        }
        break;

    default:
        break;
    }
}

void FunDec(Node* node)
{
    // printf("FunDec\n");
    TableList* res = search(node->child->val.type_str);
    if (!res) {
        TableList* item = newTableList(node->child->val.type_str, node->inh->type, NULL, NULL);
        insert(item);
    } else {
        sprintf(msg, "Redefined function \"%s\"", node->child->val.type_str);
        print_error(4, node->line, msg);
        node->inh->type->kind = WRONGFUNC;
    }
    switch (node->no) {
    case 1: // ID LP VarList RP
        get_child(node, 2)->inh = node->inh;
        VarList(get_child(node, 2));
        node->inh->type->u.func.args = get_child(node, 2)->syn;
        break;
    case 2: // ID LP RP
        node->inh->type->u.func.args = NULL;
        break;
    default:
        break;
    }
}

void CompSt(Node* node)
{
    // printf("CompSt\n");
    get_child(node, 1)->inh = node->inh;
    get_child(node, 2)->inh = node->inh;
    DefList(get_child(node, 1));
    StmtList(get_child(node, 2));
}

void DefList(Node* node)
{
    // printf("DefList\n");

    if (node->no == 2) // Def DefList
    {
        if (node->instruct) {
            node->child->instruct = 1;
            get_child(node, 1)->instruct = 1;
        }
        node->child->inh = node->inh;
        get_child(node, 1)->inh = node->inh;
        Def(node->child);
        node->syn = node->child->syn;
        DefList(get_child(node, 1));
        FieldList* p = node->syn;
        if (!p)
            return;
        while (p->tail) {
            p = p->tail;
        }
        p->tail = get_child(node, 1)->syn;
    }
}

void Args(Node* node)
{
    // printf("Args\n");
    Exp(node->child);
    if (node->child->syn) {
        node->syn = (FieldList*)malloc(sizeof(FieldList));
        node->syn->name = node->child->syn->name;
        node->syn->type = node->child->syn->type;
        node->syn->tail = NULL;
    } else {
        node->syn = NULL;
    }
    switch (node->no) {
    case 1: // Exp COMMA Args
        Args(get_child(node, 2));
        if (node->syn)
            node->syn->tail = get_child(node, 2)->syn;
        break;
    case 2: // Exp
        break;
    default:
        break;
    }
}

void print_error(int type, int line, char* msg)
{
    printf("Error type %d at Line %d: %s.\n", type, line, msg);
}
