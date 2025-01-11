#include "inter.h"



void MIPS32(const char* fname);

void mfunction(FILE* f, InterCodes* p);
void mlabel(FILE* f, InterCodes* p);
void mgoto(FILE* f, InterCodes* p);
void massign(FILE* f, InterCodes* p);
void madd(FILE* f, InterCodes* p);
void msub(FILE* f, InterCodes* p);
void mmul(FILE* f, InterCodes* p);
void mdiv(FILE* f, InterCodes* p);
void mif(FILE* f, InterCodes* p);
void mreturn(FILE* f, InterCodes* p);
void marg(FILE* f, InterCodes* p);
void mcall(FILE* f, InterCodes* p);
InterCodes* mparam(FILE* f, InterCodes* p);
void mread(FILE* f, InterCodes* p);
void mwrite(FILE* f, InterCodes* p);

