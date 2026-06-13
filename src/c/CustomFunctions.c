#include <pebble.h>
#include <ctype.h>
#include <wchar.h>
#include "CustomFunctions.h"

void* iso_realloc(void* ptr, size_t size) {
  if (ptr != NULL) {
    return realloc(ptr, size);
  } else {
    return malloc(size);
  }
}

char* getSubString(char* str, int startIndex, int endIndex)
{
    char* newStr = (char*)malloc(endIndex - startIndex + 1);
    int count = 0;
    for(int i = startIndex; i < endIndex; i++)
    {
        newStr[count] = str[i];
        count++;
    }
    newStr[count] = '\0';
    return newStr;
}

wchar_t* getSubStringWChar(wchar_t* str, int startIndex, int endIndex)
{
    //had to change this to support wide chars as before it was just assuming that all chars were 1 byte
    wchar_t* newStr = (wchar_t*)malloc((endIndex - startIndex + 1)*sizeof(wchar_t));
    int count = 0;
    for(int i = startIndex; i < endIndex; i++)
    {
        newStr[count] = str[i];
        count++;
    }
    newStr[count] = '\0';
    return newStr;
}

void outputArrayContents(char** array, int arrayLength)
{
    for(int i=0; i<arrayLength; i++)
    {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "\nArray Element %d: %s\n", i, array[i]);
    }
}
//return an array of items which have been split by delimiter
char** splitString(char* str, char delimiter, int* length)
{
    char** splitArray = (char**)malloc(sizeof(char*));
    int previousDelimiterIndex = 0;
    int tokenCount = 0;
            
    for(int i=0; i<(int)strlen(str); i++)
    {
        //if we hit the delimiter or end of string execute below code
        if ((str[i] == delimiter) || (i == ((int)strlen(str)-1)))
        {
            tokenCount++;
            //reallocating to size of a token
            splitArray = (char**)iso_realloc(splitArray, sizeof(char*)*(tokenCount));
            splitArray[tokenCount-1] = (char*)malloc((i-previousDelimiterIndex) + 1);
            //memset(splitArray[tokenCount-1], '\0', (i-previousDelimiterIndex) + 1);
            //(i-previousDelimiterIndex) + 1
            char* derp = getSubString(str, previousDelimiterIndex, i);
            //splitArray[tokenCount-1] = strcpy(splitArray[tokenCount-1], derp);
            strcpy(splitArray[tokenCount-1], derp);
            free(derp);
            //outputArrayContents(splitArray, tokenCount);
            previousDelimiterIndex = i+1;
        }
    }
    *length = tokenCount;
    
    return splitArray;
}

//return an array of items which have been split by delimiter (Wide character version)
wchar_t** splitStringWChar(wchar_t* str, char delimiter, int* length)
{
    wchar_t** splitArray = (wchar_t**)malloc(sizeof(wchar_t*));
    int previousDelimiterIndex = 0;
    int tokenCount = 0;
    
    for(int i=0; i<(int)wcslen(str); i++)
    {
        //if we hit the delimiter or end of string execute below code
        if ((str[i] == delimiter) || (i == ((int)wcslen(str)-1)))
        {
            tokenCount++;
            //reallocating to size of a token
            splitArray = (wchar_t**)iso_realloc(splitArray, sizeof(wchar_t*)*(tokenCount));
            splitArray[tokenCount-1] = (wchar_t*)malloc(((i-previousDelimiterIndex) + 1) * sizeof(wchar_t));
            //memset(splitArray[tokenCount-1], '\0', (i-previousDelimiterIndex) + 1);
            //(i-previousDelimiterIndex) + 1
            wchar_t* derp = getSubStringWChar(str, previousDelimiterIndex, i);
            //splitArray[tokenCount-1] = strcpy(splitArray[tokenCount-1], derp);
            wcscpy(splitArray[tokenCount-1], derp);
            
            free(derp);
            //outputArrayContents(splitArray, tokenCount);
            previousDelimiterIndex = i+1;
        }
    }
    *length = tokenCount;
    
    return splitArray;
}

void scrollTextByOneLetter(char* text)
{
    
    int length = strlen(text);
    
    int repetitions = 1;
    //due to unicode issues, if the character ascii is greater than 127 it is a two byte character, so run this loop two times instead of one
    if (text[0] > 127)
        repetitions = 2;
    
    for (int j=0; j < repetitions; j++)
    {
        char tmp = text[0];
        for(int i=0;i < (length-1);i++)
        {
            text[i] = text[i+1];
            //APP_LOG(APP_LOG_LEVEL_DEBUG, "Current Scroll: %c\n", text[i]);
        }
        text[length-1] = tmp;
    }
        
}

//credit to KevinCooper from pebble forums
unsigned int HexStringToUInt(char const* hexstring)
{
    unsigned int result = 0;
    char const *c = hexstring;
    unsigned char thisC;
    while( (thisC = *c) != 0 )
    {
        thisC = toupper(thisC);
        result <<= 4;
        if( isdigit(thisC))
            result += thisC - '0';
        else if(isxdigit(thisC))
            result += thisC - 'A' + 10;
        else
        {
            APP_LOG(APP_LOG_LEVEL_DEBUG, "ERROR: Unrecognised hex character \"%c\"", thisC);
            return 0;
        }
        ++c;
    }
    return result;  
}

//this will copy a substring from src to dst and if it hits the end of the string it will loop
//back around and start from the beginning of string.
void loopingCopy(char* dst, char* src, int startIndex, int length)
{
    int i = 0;
    for(i=0;i < length; i++)
    {
        dst[i] = src[startIndex];
        if ((startIndex) == ((int)strlen(src) - 1))
        {
            startIndex = 0;
            continue;
        }
        startIndex++;
    }
    //dst[i + 1] = '\0';
}

