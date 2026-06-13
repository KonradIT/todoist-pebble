#ifndef _CUSTOMFUNCTIONS_H_
#define _CUSTOMFUNCTIONS_H_

#include <wchar.h>

char* getSubString(char* str, int startIndex, int endIndex);

void outputArrayContents(char** array, int arrayLength);
char** splitString(char* str, char delimiter, int* length);
wchar_t** splitStringWChar(wchar_t* str, char delimiter, int* length);
wchar_t* getSubStringWChar(wchar_t* str, int startIndex, int endIndex);
void scrollTextByOneLetter(char* text);
void loopingCopy(char* dst, char* src, int startIndex, int length);
unsigned int HexStringToUInt(char const* hexstring);

#endif
