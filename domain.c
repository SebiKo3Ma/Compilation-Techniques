#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "lexer.h"
#include "utils.h"
#include "syntax.h"
#include "domain.h"

int crtDepth;

void initSymbols(Symbols *symbols)
{
    symbols->begin=NULL;
    symbols->end=NULL;
    symbols->after=NULL;
}

Symbol *addSymbol(Symbols *symbols,const char *name,int cls)
{
    Symbol *s;
    if(symbols->end==symbols->after){ // create more room
        int count=symbols->after-symbols->begin;
        int n=count*2; // double the room
        if(n==0)n=1; // needed for the initial case
        symbols->begin=(Symbol**)realloc(symbols->begin, n*sizeof(Symbol*));
        if(symbols->begin==NULL)err("not enough memory");
        symbols->end=symbols->begin+count;
        symbols->after=symbols->begin+n;
    }
    SAFEALLOC(s,Symbol)
    *symbols->end++=s;
    s->name=name;
    s->cls=cls;
    s->depth=crtDepth;
    return s;
}

// Symbol *findSymbol(Symbols *symbols,const char *name){
//     Symbol *s = *symbols->end - 1;
//     while(s >= *symbols->begin){
//         if(strcmp(s->name, name) == 0)
//             return s;
//         s--;
//     }
//     return 0;
// }


// void deleteSymbolsAfter(Symbols *symbols, Symbol*s){
//     Symbol *temp = *symbols->end - 1;
//     Symbol *toDelete;
//     while(temp != s){
//         toDelete = temp;
//         temp--;
//         symbols->end--;
//         free(toDelete);
//     }
// }


Symbol *findSymbol(Symbols *symbols, const char *name)
	{
		int n = symbols->end - symbols->begin;
		int i;
		
		for (i = n - 1; i >= 0; i--)
		{
			if (!strcmp(symbols->begin[i]->name, name))
				return symbols->begin[i];
		}
		
		return NULL;
	}

void deleteSymbolsAfter(Symbols *symbols, Symbol *start)
	{
		int pos, i;
		int n = symbols->end - symbols->begin;

		for (i = n - 1; i >= 0; i--)
		{
			if (!strcmp(symbols->begin[i]->name, start->name))
			{
				pos = i;
				break;
			}
		}

		printf("[debug]: deleteSymbolsAfter() = %d, n = %d\n", pos, n);

		for (i = pos + 1; i < n; i++)
		{
			//free(symbols->begin[i]);
			//symbols->begin[i] = NULL;
		}
	}
