#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "lexer.h"
#include "utils.h"
#include "syntax.h"

bool readFile(char *infile){
    long length;
    FILE *f = fopen(infile, "rb");
    if (f){
        fseek (f, 0, SEEK_END);
        length = ftell (f);
        fseek (f, 0, SEEK_SET);
        pCrtCh = malloc (length);
        if (pCrtCh)
        {
            fread (pCrtCh, 1, length, f);
        }
        fclose (f);
        return true;
    }
    return false;
}

// int crtDepth;

// enum{TB_INT,TB_DOUBLE,TB_CHAR,TB_STRUCT,TB_VOID};

// typedef struct{
//     int typeBase; // TB_*
//     Symbol *s; // struct definition for TB_STRUCT
//     int nElements; // >0 array of given size, 0=array without size, <0 non array
// }Type;

// enum{CLS_VAR,CLS_FUNC,CLS_EXTFUNC,CLS_STRUCT};
// enum{MEM_GLOBAL,MEM_ARG,MEM_LOCAL};

// typedef struct _Symbol{
//     const char *name; // a reference to the name stored in a token
//     int cls; // CLS_*
//     int mem; // MEM_*
//     Type type;
//     int depth; // 0-global, 1-in function, 2... - nested blocks in function
//     union{
//     Symbols args; // used only of functions
//     Symbols members; // used only for structs
//     };
// }Symbol;
// Symbols symbols;

// struct _Symbol;
// typedef struct _Symbol Symbol;
// typedef struct{
//     Symbol **begin; // the beginning of the symbols, or NULL
//     Symbol **end; // the position after the last symbol
//     Symbol **after; // the position after the allocated space
// }Symbols;

// void initSymbols(Symbols *symbols)
// {
//     symbols->begin=NULL;
//     symbols->end=NULL;
//     symbols->after=NULL;
// }

// Symbol *addSymbol(Symbols *symbols,const char *name,int cls)
// {
//     Symbol *s;
//     if(symbols->end==symbols->after){ // create more room
//         int count=symbols->after-symbols->begin;
//         int n=count*2; // double the room
//         if(n==0)n=1; // needed for the initial case
//         symbols->begin=(Symbol**)realloc(symbols->begin, n*sizeof(Symbol*));
//         if(symbols->begin==NULL)err("not enough memory");
//         symbols->end=symbols->begin+count;
//         symbols->after=symbols->begin+n;
//     }
//     SAFEALLOC(s,Symbol)
//     *symbols->end++=s;
//     s->name=name;
//     s->cls=cls;
//     s->depth=crtDepth;
//     return s;
// }

// Symbol *findSymbol(Symbols *symbols,const char *name){
//     int i = 0;
//     Symbol *s;
//     do{
//         s = symbols->end - i;
//         if(s->name == *name)
//         return s;
//         i++;
//     } while(s != symbols->begin);
//     return 0;
// }

// Symbol *deleteSymbolsAfter(Symbols *symbols, Symbol*s){
//     ;
// }

int main(int argc, char *argv[]){
    if(argc != 2){
        printf("Invalid usage!\n");
        exit(1);
    }
    if(!readFile(argv[1])){
        printf("File %s not found!\n", argv[1]);
        return 1;
    }
    while(getNextToken() != END){} // lexical analysis
    //tempTestPrint(); // test print token list
    crtTk = tokens;
    unit();
}
