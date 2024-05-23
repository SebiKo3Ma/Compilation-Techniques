#pragma once

#include "utils.h"
#include "domain.h"

typedef union{
    long int i; // int, char
    double d; // double
    const char *str; // char[]
}CtVal;

typedef struct{
    Type type; // type of the result
    int isLVal; // if it is a LVal
    int isCtVal; // if it is a constant value (int, real, char, char[])
    CtVal ctVal; // the constat value
}RetVal;

Type createType(int typeBase,int nElements);

void cast(Type *dst,Type *src);

void addExtFuncs();

Type getArithType(Type *s1,Type *s2);