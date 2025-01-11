#include "sem.h"
#include <stdbool.h>

typedef struct Operand_ Operand;
typedef struct InterCode InterCode;
typedef struct InterCodes InterCodes;

struct Operand_ { 		
    enum { 		//kind of operand
        VARIABLE, 		// v1
        TEMP, 			// t2
        PARAMETER,	 	// v3
        CONSTANT, 		// #4
        LAB,			// label5
        FUNCT 			// foo
    } kind;	
    // is value or address (for assign)
    enum { VAL,
        ADDRESS } type;
    // moreinfo
    union {
        int var_no;			// nomber for v,t,p,label
        long long int value; // value for c
        char* func_name;	// function name for f
    } u;
};
struct InterCode { 
    enum { 	//kind of intercode
        LABEL,	// LABEL x:
        FUNCTION,	// FUNCTION f:
        ASSIGN,		
          		// check operand is value or address in type below
        ADD,		// x := y + z
        SUB,		// x := y - z
        MUL,		// x := y * z
        DIV,		// x := y / z
        GOTO,		// GOTO x
        IF,			// IF x [relop] y GOTO z
        RETURN,		// RETURN x
        DEC,		// DEC x [size]
        ARG,		// ARG x
        CALL,		// x := CALL f
        PARAM,		// PARAM x
        READ,		// READ x
        WRITE 		// WRITE x
    } kind; 
    enum { 		// type for assign
        NORMAL,		// x := y
        GETADDR,	// x := &y
        GETVAL,		// x := *y
        SETVAL,		// *x := y
        COPY 		// *x := *y
    } type;
    union {
        struct {	// for assign
            Operand *left, *right;	// left := right
        } assign;
        struct {	// binary operand for +-*/ 
            Operand *res, *op1, *op2;	// res := op1 op op2
        } binop;
        struct {	// single operand for call
            Operand *res, *op;	// res = CALL op
        } sinop;	
        struct {	// for read,write,label,goto,return...
            Operand* op;	// READ op
        } single;
        struct {	// conditional for if
            Operand *op1, *op2, *target; // IF op1 [relop] op2 GOTO target
            char relop[4];
        } cond;
        struct {	// declearation for DEC
            Operand* op;	// DEC op size
            unsigned size;
        } dec;
    } u;
};

struct InterCodes {
    InterCode code;
    bool isDelete;
    InterCodes *prev, *next;
};



Operand* getOperandwithValue(int kind, long long int val);
Operand* getOperandwithName(int kind, char* name);
Operand* getOperandwithNo(int kind, int no);
InterCodes* makeinter(int kind);


void createAssign(unsigned type, Operand* left, Operand* right);
void createBinop(unsigned kind, unsigned type, Operand* res, Operand* op1, Operand* op2);
void createSinop(unsigned kind, Operand* res, Operand* op);
void createSingle(unsigned kind, Operand* op);
void createCond(Operand* op1, Operand* op2, Operand* target, char* re);
void createDec(Operand* op, unsigned size);


char* printOperand(Operand* op);
void writeInterCodes(const char* fielname, bool opt);


unsigned countSize(Type* type);
void setVariable();
Operand* newConst(long long int val);
Operand* newTemp(int type);
Operand* newLabel();
void cannotTranslate();
