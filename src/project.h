#ifndef __PROJECT_H__
#define __PROJECT_H__

//#define D(x) x
#define D(x)

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void htoul(BYTE abBuffer[], DWORD dwValue);
void htous(BYTE abBuffer[], WORD wValue);
DWORD utohl(const BYTE abBuffer[]);
WORD utohs(const BYTE abBuffer[]);

#endif
