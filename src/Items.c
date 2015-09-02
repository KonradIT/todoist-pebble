#include <pebble.h>
#include "Items.h"

ItemStruct* createEmptyItemList()
{
    ItemStruct* itemList = (ItemStruct*)malloc(sizeof(ItemStruct));
    itemList->items = 0;
    itemList->itemIDs = 0;
    itemList->itemDates = 0;
    itemList->itemDueDates = 0;
    itemList->indentation = 0;
    itemList->length = 0;
    itemList->checked = (bool*)malloc(sizeof(bool));
    itemList->recurringTask = (bool*)malloc(sizeof(bool));
    return itemList;
}
ItemStruct* createItemList(char** items, char** itemIDs, char** itemDates, char** itemDueDates, char** itemIndentation, int length)
{
    ItemStruct* itemList = (ItemStruct*)malloc(sizeof(ItemStruct));
    itemList->items = items;
    itemList->itemIDs = itemIDs;
    itemList->itemDates = itemDates;
    itemList->itemDueDates = itemDueDates;
    itemList->indentation = itemIndentation;
    itemList->checked = (bool*)calloc(sizeof(bool),length);
    itemList->recurringTask = (bool*)calloc(sizeof(bool),length);
    itemList->length = length;
    return itemList;
}
void destroyItemList(ItemStruct* is)
{
    if (is == 0)
        return;
    
    for(int i=0;i < is->length;i++)
    {
        free(is->items[i]);
        free(is->itemIDs[i]);
        free(is->itemDates[i]);
        free(is->itemDueDates[i]);
        free(is->indentation[i]);
    }
    free(is->items);
    free(is->itemIDs);
    free(is->itemDates);
    free(is->itemDueDates);
    free(is->indentation);
    free(is->checked);
    free(is);
}

void unSerializeItemsString(ItemStruct* itemList, char* itemNamesString, char* itemIDsString, char* itemDatesString, char* itemDueDatesString, char* itemIndentationString)
{
    itemList->items = splitString(itemNamesString, '|', &itemList->length);
    itemList->itemIDs = splitString(itemIDsString, '|', &itemList->length);
    itemList->itemDates = splitString(itemDatesString, '|', &itemList->length);
    itemList->itemDueDates = splitString(itemDueDatesString, '|', &itemList->length);
    itemList->indentation = splitString(itemIndentationString, '|', &itemList->length);
    
    itemList->checked = (bool*)realloc(itemList->checked, sizeof(bool)*itemList->length);
    itemList->recurringTask = (bool*)realloc(itemList->recurringTask, sizeof(bool)*itemList->length);
    for (int i=0;i < itemList->length;i++)
    {
        itemList->checked[i] = 0;
    }
    
    //check if the itemdate specifies that it is a recurring task, mark it if it is
    for (int i=0;i < itemList->length;i++)
    {
        if ((strstr(stringToLower(itemList->itemDates[i]), "every") != 0) || (strstr(stringToLower(itemList->itemDates[i]), "after") != 0)
           || (strstr(stringToLower(itemList->itemDates[i]), "daily") != 0) || (strstr(stringToLower(itemList->itemDates[i]), "monthly") != 0)
           || (strstr(stringToLower(itemList->itemDates[i]), "weekly") != 0) || (strstr(stringToLower(itemList->itemDates[i]), "yearly") != 0))
        {
            itemList->recurringTask[i] = 1;
        }
        else
        {
            itemList->recurringTask[i] = 0;
        }
            
    }
}