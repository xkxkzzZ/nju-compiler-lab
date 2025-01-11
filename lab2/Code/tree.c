#include "tree.h"

Node* newNode(char* _name, MyType _type, char* text)
{
    Node* res = (Node*)malloc(sizeof(Node));
    res->name = _name;
    res->type = _type;
    res->no = -1;
    res->syn = res->inh = NULL;
    res->parent = res->child = res->next = NULL;
    res->instruct = 0;
    switch (_type) {
    case Int: {
        if (*res->name == 'O')
            sscanf(text, "%ou", &res->val.type_int);
        else if (*res->name == 'H')
            sscanf(text, "%xu", &res->val.type_int);
        else
            res->val.type_int = (unsigned)atoi(text);
        break;
    }
    case Float:
        res->val.type_float = (float)atof(text);
        break;
    case Id:
    case TYpe:
    case Relop:
    case Ter:
        strcpy(res->val.type_str, text);
        break;
    default:
        printf("Wrong Type: %s\n", res->name);
        break;
    }
    return res;
}
Node* newnewNode(char* _name, int _no, MyType _type, int line)
{
    Node* res = (Node*)malloc(sizeof(Node));
    res->name = _name;
    res->type = _type;
    res->no = _no;
    res->syn = res->inh = NULL;
    res->parent = res->child = res->next = NULL;
    res->instruct = 0;
    res->line = line;
    return res;
}

void addchild(Node* parent, Node* child)
{
    if (!parent->child) {
        parent->child = child;
    } else {
        Node* p = parent->child;
        while (p->next) {
            p = p->next;
        }
        p->next = child;
    }
    child->parent = parent;
}

void outPut(Node* node, int dep)
{
    if (node->type != Null) {
        for (int i = 0; i < dep; i++) {
            printf("  ");
        }
    }
    Node* p = node;
    switch (p->type) {
    case Nter:
        printf("%s (%d)\n", p->name, p->line);
        break;
    case Relop:
    case Ter:
        printf("%s\n", p->name);
        break;
    case TYpe:
        printf("TYPE: %s\n", p->val.type_str);
        break;
    case Id:
        printf("ID: %s\n", p->val.type_str);
        break;
    case Float:
        printf("FLOAT: %f\n", p->val.type_float);
        break;
    case Int:
        printf("INT: %u\n", p->val.type_int);
        break;
    case Null:
        break;
    default:
        printf("Wrong Type: %s\n", p->name);
        break;
    }
    if (p->child) {
        outPut((Node*)p->child, dep + 1);
    }
    if (p->next) {
        outPut((Node*)p->next, dep);
    }
}

Node* get_child(Node* node, int index)
{
    Node* p = node->child;
    while (index-- && p) {
        p = p->next;
    }
    if (!p) {
        printf("Index of child out of range!\n");
        exit(-1);
    }
    return p;
}
