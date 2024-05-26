#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "types.h"
#include "utils.h"
#include "domain.h"

Type createType(int typeBase,int nElements){
    Type t;
    t.typeBase=typeBase;
    t.nElements=nElements;
    return t;
}

void cast(Type *dst,Type *src){
    if(src->nElements>-1){
        if(dst->nElements>-1){
            if(src->typeBase!=dst->typeBase)
            tkerr(crtTk,"an array cannot be converted to an array of another type");
        }
        else{
            tkerr(crtTk,"an array cannot be converted to a non-array");
        }
    }
    else{
        if(dst->nElements>-1){
        tkerr(crtTk,"a non-array cannot be converted to an array");
        }
    }
    switch(src->typeBase){
        case TB_CHAR:
        case TB_INT:
        case TB_DOUBLE:
        switch(dst->typeBase){
            case TB_CHAR:
            case TB_INT:
            case TB_DOUBLE:
            return;
        }
        case TB_STRUCT:
            if(dst->typeBase==TB_STRUCT){
                if(src->s!=dst->s)
                tkerr(crtTk,"a structure cannot be converted to another one");
                return;
            }
    }
    tkerr(crtTk,"incompatible types");
}

Symbol *addExtFunc(const char *name,Type type){
    Symbol *s=addSymbol(&symbols,name,CLS_EXTFUNC);
    s->type=type;
    initSymbols(&s->args);
    return s;
}

Symbol *addFuncArg(Symbol *func,const char *name,Type type){
    Symbol *a=addSymbol(&func->args,name,CLS_VAR);
    a->type=type;
    return a;
}

void addExtFuncs(){
    Symbol *s;

    s=addExtFunc("put_s",createType(TB_VOID,-1));
    addFuncArg(s,"s",createType(TB_CHAR,0));
}

int getPriority(Type *s){
    if(s->typeBase == 'TB_CHAR')
        return 0;
    else if(s->typeBase == 'TB_INT')
        return 1;
    else if(s->typeBase == 'TB_FLOAT')
        return 2;
}

Type getArithType(Type *s1,Type *s2){
    if(s1->typeBase == s2->typeBase)
        return *s1;
    else return (getPriority(s1) > getPriority(s2)) ? * s1 : *s2; 
}