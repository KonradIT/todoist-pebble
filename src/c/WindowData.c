#include <pebble.h>
#include "WindowData.h"
#include "Main.h"

WindowData* createWindowData(Config* config)
{
    WindowData* wd = (WindowData*)malloc(sizeof(WindowData));
    wd->projects = 0;
    wd->items = 0;
    wd->config = config;
    wd->currentPage = 1;
    wd->currentScrollable = 0;
    wd->scrolledNumber = 0;
    return wd;
}

void setProjects(WindowData* wd, ProjectStruct* projects)
{
    wd->projects = projects;
}

int getLengthOfCurrentPage()
{
    WindowData* wd = (WindowData*)window_get_user_data(window);
    if (wd->currentPage == 1)
        return wd->projects->length;
    else
        return wd->items->length;
}

void* getCurrentList()
{
    WindowData* wd = (WindowData*)window_get_user_data(window);
    if (wd->currentPage == 1)
        return wd->projects;
    else
        return wd->items;
    
}

void destroyWindowData(WindowData* wd)
{
    free(wd->projects);
    free(wd->items);
    free(wd->config);
    free(wd);
}
