#pragma once

enum{ID, END, BREAK, CHAR, DOUBLE, ELSE, FOR, IF, INT, RETURN, STRUCT, VOID, WHILE, 
    CT_INT, CT_REAL, CT_CHAR, CT_STRING,
    COMMA, SEMICOLON, LPAR, RPAR, LBRACKET, RBRACKET, LACC, RACC,
    ADD, SUB, MUL, DIV, DOT,
    AND, OR, NOT, NOTEQ, ASSIGN, EQUAL, LESS, LESSEQ, GREATER, GREATEREQ}; // tokens codes

typedef struct _Token{
    int code; // code (name)
    union{
        char *text; // used for ID, CT_STRING (dynamically allocated)
        long int i; // used for CT_INT, CT_CHAR
        double r; // used for CT_REAL
    };
    int line; // the input file line
    struct _Token *next; // link to the next token
}Token;

int getNextToken();

void tempTestPrint();

extern char *pCrtCh;

extern Token *tokens;
