#pragma once

#include <pebble.h>
#include "Items.h"
#include "Projects.h"
#include "Config.h"

typedef struct mainWindowData
{
    ProjectStruct* projects;
    ItemStruct* items;
    Config* config;
    //the index of the currently selected project (used for header)
    int selectedProjectIndex;
    int currentPage;
    //whether the current selected item overflows the space of the screen and will have to scroll
    bool currentScrollable;
    //how many characters has this item currently scrolled
    int scrolledNumber;
    //timer that scrolls the text one character every tick
    AppTimer* textScrollTimer;
} WindowData;

WindowData* createWindowData(Config* config);
void setProjects(WindowData* wd, ProjectStruct* projects);
int getLengthOfCurrentPage();
void* getCurrentList();
void destroyWindowData(WindowData* wd);
