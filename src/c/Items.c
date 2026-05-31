#include <pebble.h>
#include "Items.h"

ItemStruct* createEmptyItemList()
{
    ItemStruct* itemList = (ItemStruct*)malloc(sizeof(ItemStruct));
    itemList->items = 0;
    itemList->itemIDs = 0;
    itemList->itemDueDates = 0;
    itemList->indentation = 0;
    itemList->length = 0;
    itemList->checked = 0;
    return itemList;
}
ItemStruct* createItemList(char** items, char** itemIDs, char** itemDueDates, char** itemIndentation, int length)
{
    ItemStruct* itemList = (ItemStruct*)malloc(sizeof(ItemStruct));
    itemList->items = items;
    itemList->itemIDs = itemIDs;
    itemList->itemDueDates = itemDueDates;
    itemList->indentation = itemIndentation;
    itemList->checked = 0;
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
        free(is->itemDueDates[i]);
        free(is->indentation[i]);
    }
    free(is->items);
    free(is->itemIDs);
    free(is->itemDueDates);
    free(is->indentation);
    free(is->checked);
    free(is);
}

void unSerializeItemsString(ItemStruct* itemList, char* itemNamesString, char* itemIDsString, char* itemDueDatesString, char* itemIndentationString)
{
    itemList->items = splitString(itemNamesString, '|', &itemList->length);
    itemList->itemIDs = splitString(itemIDsString, '|', &itemList->length);
    itemList->itemDueDates = splitString(itemDueDatesString, '|', &itemList->length);
    itemList->indentation = splitString(itemIndentationString, '|', &itemList->length);
    itemList->checked = (bool*)malloc(sizeof(bool)*itemList->length);
    
    for (int i=0;i < itemList->length;i++)
    {
        itemList->checked[i] = 0;
    }
}
