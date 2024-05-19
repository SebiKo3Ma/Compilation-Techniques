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
