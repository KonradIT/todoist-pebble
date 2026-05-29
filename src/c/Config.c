#include <pebble.h>
#include "Config.h"
#include "CustomFunctions.h"

void initDefaults(Config* config)
{
    
    config->scrollSpeed = 150;
    #ifdef PBL_COLOR
        config->backgroundColor = GColorLightGray;
        config->foregroundColor = GColorBlack;
        config->altBackgroundColor = GColorDarkGray;
        config->altForegroundColor = GColorWhite;
        config->highlightBackgroundColor = GColorRed;
        config->highlightForegroundColor = GColorWhite;
    #else
        config->backgroundColor = GColorWhite;
        config->foregroundColor = GColorBlack;
        config->altBackgroundColor = GColorWhite;
        config->altForegroundColor = GColorBlack;
        config->highlightBackgroundColor = GColorBlack;
        config->highlightForegroundColor = GColorWhite;
    #endif
}

void setConfig(char* configString, Config* config)
{
    int* len = (int*)malloc(sizeof(int));
    char** configArray = splitString(configString, '|', len);
        
    //converts the number from 1-10 to its millisecond value, 10 is fastest 1 is slowest,
    //therefore 10 is 50 milliseconds, 1 is 500 milliseconds
    config->scrollSpeed = (1.0/atoi(configArray[0]))*500;

    #ifdef PBL_COLOR
        config->backgroundColor = GColorFromHEX(HexStringToUInt(configArray[1]));
        config->foregroundColor = GColorFromHEX(HexStringToUInt(configArray[2]));
        config->altBackgroundColor = GColorFromHEX(HexStringToUInt(configArray[3]));
        config->altForegroundColor = GColorFromHEX(HexStringToUInt(configArray[4]));
        config->highlightBackgroundColor = GColorFromHEX(HexStringToUInt(configArray[5]));
        config->highlightForegroundColor = GColorFromHEX(HexStringToUInt(configArray[6]));
    #else
        config->backgroundColor = GColorWhite;
        config->foregroundColor = GColorBlack;
        config->altBackgroundColor = GColorWhite;
        config->altForegroundColor = GColorBlack;
        config->highlightBackgroundColor = GColorBlack;
        config->highlightForegroundColor = GColorWhite;
    #endif
}

void savePersistentConfig(Config* config)
{
    persist_write_data(CONFIG_KEY, config, sizeof(*config));
}

void loadPersistentConfig(Config* config)
{
    if (persist_exists(CONFIG_KEY))
    {
        persist_read_data(CONFIG_KEY, config, sizeof(*config));
    }
    else
    {
        initDefaults(config);
    }
}

//gets whether the startup message for a new version has been shown
bool loadMessageShown()
{
    //persist_delete(MESSAGE_KEY);
    if (persist_exists(MESSAGE_KEY112)) 
        return 1;
    else
        return 0;
}
//sets the startup message for a new version to shown
void saveMessageShown()
{
    persist_write_bool(MESSAGE_KEY112, 1);
}
