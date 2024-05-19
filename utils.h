#pragma once

#include "lexer.h"

void tkerr(const Token *tk,const char *fmt,...);

void err(const char *fmt,...);

#define SAFEALLOC(var,Type) if((var=(Type*)malloc(sizeof(Type)))==NULL)err("not enough memory");