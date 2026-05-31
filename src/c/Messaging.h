#pragma once
    
#include <pebble.h>

#define PROJECT_NAMES 0
#define PROJECT_IDs 1
#define SELECTED_PROJECT 2
#define ITEM_NAMES 3
#define ITEM_IDS 4
#define SELECTED_ITEM 5 
//informs us that we are waiting for javascript to complete something (generally means to show a "loading" window).
#define WAITING 6
//an error happened in javascript
#define ERROR 7
#define CONFIG 8
#define ERRORMSG 10
#define ITEM_DUE_DATES 13
#define PROJECT_INDENTATION 14
#define ITEM_INDENTATION 15
#define ADD_NEW_ITEM 16
#define SELECTED_ITEM_UNCOMPLETE 17
    
void addItem(char* itemText);
void sendItemIDToPhone(int index);
void sendProjectIDToPhone(int index);
void inbox_received_callback(DictionaryIterator *iterator, void *context);
void inbox_dropped_callback(AppMessageResult reason, void *context);
void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context);
void outbox_sent_callback(DictionaryIterator *iterator, void *context);
