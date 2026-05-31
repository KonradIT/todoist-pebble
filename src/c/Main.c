#include <pebble.h>
#include <locale.h>
#include "Main.h"
#include "CustomFunctions.h"
#include "Items.h"
#include "Projects.h"
#include "CheckboxList.h"
#include "ErrorWindow.h"
#include "WindowData.h"
#include "MainWindowClicks.h"
#include "Messaging.h"

Window* window;
MenuLayer* myMenuLayer;


//Adds item to the project as obtained through the dictation API
void addItem(char* itemText)
{
    WindowData* wd = (WindowData*)window_get_user_data(window);
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    if (!iter) {
      // Error creating outbound message
      return;
    }
    strcat(itemText, "|");
    strcat(itemText,wd->projects->projectIDs[wd->selectedProjectIndex]);
    dict_write_cstring(iter, ADD_NEW_ITEM, itemText);
    dict_write_end(iter);
  
    app_message_outbox_send();
}

//sends the project ID at specified index to the phone to get itemlist
void sendProjectIDToPhone(int index)
{
    WindowData* wd = (WindowData*)window_get_user_data(window);
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    if (!iter) {
      // Error creating outbound message
      return;
    }
  
    dict_write_cstring(iter, SELECTED_PROJECT, wd->projects->projectIDs[index]);
    dict_write_end(iter);
  
    app_message_outbox_send();
}

//sends the item ID at specified index to the phone to set an item as completed
void sendItemIDToPhone(int index)
{
    WindowData* wd = (WindowData*)window_get_user_data(window);
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    if (!iter) {
      // Error creating outbound message
      return;
    }
  
    if (wd->items->checked[index])
        dict_write_cstring(iter, SELECTED_ITEM_UNCOMPLETE, wd->items->itemIDs[index]);
    else
        dict_write_cstring(iter, SELECTED_ITEM, wd->items->itemIDs[index]);
    
    dict_write_end(iter);
  
    app_message_outbox_send();
}

//draws a checkbox in the given cell layer of the menu
void drawCheckbox(GContext *ctx, Layer *cell_layer, int index)
{
    WindowData* wd = (WindowData*)window_get_user_data(window);
    
    //do not draw checkbox at all if it is the "add new" item
    if (strcmp(wd->items->itemIDs[index],"0") == 0)
        return;
    GRect cellBounds = layer_get_bounds(cell_layer);
    GPoint g = GPoint(cellBounds.size.w*.80 + CIRCLE_RADIUS, cellBounds.size.h/2.0);
    #ifdef PBL_COLOR
        graphics_context_set_antialiased(ctx, true);
    #endif
    graphics_draw_circle(ctx, g, CIRCLE_RADIUS);
    
    if (wd->items->checked[index] == 1)
    {
        GPoint g = GPoint(cellBounds.size.w*.80 + CIRCLE_RADIUS, cellBounds.size.h/2.0);
        //needed for aplite
        graphics_context_set_fill_color(ctx, wd->config->highlightForegroundColor);
        #ifdef PBL_SDK_3
            MenuIndex currentIndex = menu_layer_get_selected_index(myMenuLayer);
            int currentRow = currentIndex.row;
            if (index == currentRow)
            {
                graphics_context_set_fill_color(ctx, wd->config->highlightForegroundColor);
            }
            else
            {
                graphics_context_set_fill_color(ctx, wd->config->foregroundColor);
            }
        #endif
        graphics_fill_circle(ctx, g, CIRCLE_RADIUS/2.0);
    }
}

//draws a dot inside the checkbox to mark as "checked"
void checkCheckbox(GContext *ctx, Layer *cell_layer, int index)
{
    WindowData* wd = (WindowData*)window_get_user_data(window);
    if (wd->items->checked[index] == 1)
    {   
        GRect cellBounds = layer_get_bounds(cell_layer);
        GPoint g = GPoint(cellBounds.size.w*.80 + CIRCLE_RADIUS, cellBounds.size.h/2.0);
        graphics_fill_circle(ctx, g, CIRCLE_RADIUS/2.0);
    }
}

void draw_header_callback(GContext *ctx, Layer *cell_layer, uint16_t section_index, void *callback_context)
{
    //menu_cell_basic_header_draw(ctx, cell_layer, "Projects");
	WindowData* wd = (WindowData*)window_get_user_data(window);
    if (wd->currentPage == 1)
    {
        WindowData* wd = (WindowData*)window_get_user_data(window);
        GRect cellBounds = layer_get_bounds(cell_layer);
        graphics_context_set_text_color(ctx, wd->config->altForegroundColor);
        
        #ifdef PBL_ROUND
            cellBounds.origin.x = cellBounds.origin.x + 12;
            cellBounds.size.w = cellBounds.size.w - 24;
            graphics_draw_text(ctx, "Projects", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), cellBounds, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
        #else
            cellBounds.origin.x = cellBounds.origin.x + 8;
            graphics_draw_text(ctx, "Projects", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), cellBounds, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
        #endif
        graphics_context_set_stroke_color(ctx, wd->config->altForegroundColor);
        //graphics_draw_line(ctx,GPoint(cellBounds.origin.x,cellBounds.origin.y+cellBounds.size.h-1), GPoint(cellBounds.origin.x+cellBounds.size.w, cellBounds.origin.y+cellBounds.size.h-1));
    }
    else
    {
        WindowData* wd = (WindowData*)window_get_user_data(window);
        GRect cellBounds = layer_get_bounds(cell_layer);
        graphics_context_set_text_color(ctx, wd->config->altForegroundColor);
        
        #ifdef PBL_ROUND
            cellBounds.origin.x = cellBounds.origin.x + 12;
            cellBounds.size.w = cellBounds.size.w - 24;
            graphics_draw_text(ctx, wd->projects->projects[wd->selectedProjectIndex], fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), cellBounds, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
        #else
            cellBounds.origin.x = cellBounds.origin.x + 8;
            graphics_draw_text(ctx, wd->projects->projects[wd->selectedProjectIndex], fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), cellBounds, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
        #endif
        graphics_context_set_stroke_color(ctx, wd->config->altForegroundColor);
        //graphics_draw_line(ctx,GPoint(cellBounds.origin.x,cellBounds.origin.y+cellBounds.size.h-1), GPoint(cellBounds.origin.x+cellBounds.size.w, cellBounds.origin.y+cellBounds.size.h-1));
    }
    
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return 25;
}

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return 1;
}

void draw_row_callback_round(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *callback_context)
{
    WindowData* wd = (WindowData*)window_get_user_data(window);
    #ifdef PBL_COLOR
        menu_layer_set_normal_colors(myMenuLayer, wd->config->altBackgroundColor, wd->config->altForegroundColor);
    #endif
        
    if (!wd)
        return;
	
    if (wd->currentPage == 1)
    {
        if (wd->projects)
        {
            for(int i=0;i<(wd->projects->length);i++)
            {
                //remember this is the row it is CURRENTLY drawing not the SELECTED row, it will draw all rows on an update by calling the function a bunch of different times
                if (cell_index->row == i)
                {
                    GRect cellBounds = layer_get_bounds(cell_layer);
                    
                    MenuIndex currentIndex = menu_layer_get_selected_index(myMenuLayer);
                    int currentRow = currentIndex.row;
                    
                    
                    //set background color (alt color)
                    #ifdef PBL_COLOR
                        graphics_context_set_text_color(ctx, wd->config->altForegroundColor);
                        graphics_context_set_fill_color(ctx, wd->config->altBackgroundColor);
                        graphics_context_set_stroke_color(ctx, wd->config->altForegroundColor);
                    #endif
                    
                    //fill background
                    graphics_fill_rect(ctx, cellBounds, 0, GCornerNone);
                    
                    //set standard location of item box
                    cellBounds.origin.x = cellBounds.origin.x + 12;
                    cellBounds.size.w = cellBounds.size.w - 24;
                    
                    cellBounds.origin.y = cellBounds.origin.y + 3;
                    cellBounds.size.h = cellBounds.size.h - 6;
                    
                    //indent item box if needed
                    cellBounds.origin.x = cellBounds.origin.x + ((atoi(wd->projects->indentation[i])-1) * 7);
                    cellBounds.size.w = cellBounds.size.w - ((atoi(wd->projects->indentation[i])-1) * 7);
                    
                    
                    if (cell_index->row == currentRow)
                    {
                        //have to reverse background/foreground color on aplite as it seems to auto invert
                        //and basalt does not (you set the colors in "normal colors" function).
                        #ifdef PBL_COLOR
                            graphics_context_set_text_color(ctx, wd->config->highlightForegroundColor);
                            graphics_context_set_fill_color(ctx, wd->config->highlightBackgroundColor);
                            graphics_context_set_stroke_color(ctx, wd->config->highlightForegroundColor);
                        #endif
                    }
                    else
                    {
                        #ifdef PBL_COLOR
                            graphics_context_set_text_color(ctx, wd->config->foregroundColor);
                            graphics_context_set_fill_color(ctx, wd->config->backgroundColor);
                            graphics_context_set_stroke_color(ctx, wd->config->foregroundColor);
                        #endif
                    }
                    //fill rounded rect (Item space)
                    graphics_fill_rect(ctx, cellBounds, 8, GCornersAll);
                    //graphics_draw_round_rect(ctx, cellBounds, 5);
                    
                    
                    //if selected
                    if (cell_index->row == currentRow)
                    {
                        //increase cellbounds size so we can tell if the text will "overflow". if you dont do this the content size will stop at the end of the bounds
                        cellBounds.size.w = cellBounds.size.w*2.0;
                        GSize textSize = graphics_text_layout_get_content_size(wd->projects->projects[i], fonts_get_system_font(FONT_KEY_GOTHIC_18), cellBounds, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter);
                        cellBounds.size.w = cellBounds.size.w/2.0;
                        //snprintf(asdf, 100, "%d", textSize.w);
                        //displayErrorMessage(asdf);
                        if (textSize.w > cellBounds.size.w)
                        {
                            wd->currentScrollable = 1;
                        }
                        else
                        {
                            wd->currentScrollable = 0; 
                        }
                    }
                    cellBounds.origin.x = cellBounds.origin.x + 3;
                    graphics_draw_text(ctx, wd->projects->projects[i], fonts_get_system_font(FONT_KEY_GOTHIC_18), cellBounds, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
                }
            }
        }
    }
    
    if (wd->currentPage == 2)
    {
        if (wd->items && (wd->items->length != 0))
        {
            for(int i=0;i<(wd->items->length);i++)
            {
                if (cell_index->row == i)
                {
                    GRect cellBounds = layer_get_bounds(cell_layer);
                    
                    //text takes up 80% of the layer size due to the checkboxes
                    
                    //GRect newBounds = GRect(cellBounds.origin.x, cellBounds.origin.y, cellBounds.size.w*.80, cellBounds.size.h);
                    //layer_set_bounds(cell_layer, newBounds);
                    
                    MenuIndex currentIndex = menu_layer_get_selected_index(myMenuLayer);
                    int currentRow = currentIndex.row;
                    
                    //set background color (alt color)
                    #ifdef PBL_COLOR
                        graphics_context_set_text_color(ctx, wd->config->altForegroundColor);
                        graphics_context_set_fill_color(ctx, wd->config->altBackgroundColor);
                        graphics_context_set_stroke_color(ctx, wd->config->altForegroundColor);
                    #endif
                    
                    //fill background
                    graphics_fill_rect(ctx, cellBounds, 0, GCornerNone);
                    
                    //set standard location of item box
                    cellBounds.origin.x = cellBounds.origin.x + 12;
                    cellBounds.size.w = cellBounds.size.w - 24;
                    cellBounds.origin.y = cellBounds.origin.y + 3;
                    cellBounds.size.h = cellBounds.size.h - 6;
                    
                    //indent item box if needed
                    cellBounds.origin.x = cellBounds.origin.x + ((atoi(wd->items->indentation[i])-1) * 7);
                    cellBounds.size.w = cellBounds.size.w - ((atoi(wd->items->indentation[i])-1) * 7);
                    
                    GRect textBounds = GRect(cellBounds.origin.x, cellBounds.origin.y, cellBounds.size.w*.80, 30);
                    
                    
                    if (cell_index->row == currentRow)
                    {
                        //have to reverse background/foreground color on aplite as it seems to auto invert
                        //and basalt does not (you set the colors in "normal colors" function).
                        #ifdef PBL_COLOR
                            graphics_context_set_text_color(ctx, wd->config->highlightForegroundColor);
                            graphics_context_set_fill_color(ctx, wd->config->highlightBackgroundColor);
                            graphics_context_set_stroke_color(ctx, wd->config->highlightForegroundColor);
                        #endif
                    }
                    else
                    {
                        #ifdef PBL_COLOR
                            graphics_context_set_text_color(ctx, wd->config->foregroundColor);
                            graphics_context_set_fill_color(ctx, wd->config->backgroundColor);
                            graphics_context_set_stroke_color(ctx, wd->config->foregroundColor);
                        #endif
                    }
                    //fill rounded rect (Item space)
                    graphics_fill_rect(ctx, cellBounds, 8, GCornersAll);

                    //if selected
                    if (cell_index->row == currentRow)
                    {
                        GSize textSize = graphics_text_layout_get_content_size(wd->items->items[i], fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(cellBounds.origin.x, cellBounds.origin.y, cellBounds.size.w*2, 30), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft);
                        //snprintf(asdf, 100, "%d", textSize.w);
                        //displayErrorMessage(asdf);
                        if (textSize.w >= (textBounds.size.w))
                        {
                            wd->currentScrollable = 1;
                        }
                        else
                        {
                            wd->currentScrollable = 0; 
                        }
                    }
                    textBounds.origin.x = textBounds.origin.x + 3;
                    
                    graphics_draw_text(ctx, wd->items->items[i], fonts_get_system_font(FONT_KEY_GOTHIC_18), textBounds, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
                    
                    //if it has a date draw the date
                    if (strcmp(wd->items->itemDueDates[cell_index->row], "") != 0)
                    {
                        textBounds.origin.y = textBounds.origin.y+20;
                        //draw a seperator line if there is a date
                        //graphics_draw_line(ctx, GPoint(textBounds.origin.x-3, textBounds.origin.y+2), GPoint(textBounds.size.w, textBounds.origin.y+2));
                        graphics_draw_text(ctx, wd->items->itemDueDates[i], fonts_get_system_font(FONT_KEY_GOTHIC_18), textBounds, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
                    }
                    drawCheckbox(ctx, cell_layer, i);
                    //checkCheckbox(ctx, cell_layer, i);
                }
            }
        }
    }
}

void draw_row_callback_modern(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *callback_context)
{
    WindowData* wd = (WindowData*)window_get_user_data(window);
    #ifdef PBL_COLOR
        menu_layer_set_normal_colors(myMenuLayer, wd->config->altBackgroundColor, wd->config->altForegroundColor);
    #endif
    
    /*graphics_context_set_fill_color(ctx, GColorWhite);
    GPoint* snowflakePoint = (GPoint*)malloc(sizeof(GPoint));
    GPoint->x = 5;
    GPoint->y = 5;
    graphics_context_set_fill_color(ctx, wd->config->altBackgroundColor);
    graphics_draw_pixel(ctx, snowflakePoint);*/
        
    if (!wd)
        return;
	
    if (wd->currentPage == 1)
    {
        if (wd->projects)
        {
            for(int i=0;i<(wd->projects->length);i++)
            {
                //remember this is the row it is CURRENTLY drawing not the SELECTED row, it will draw all rows on an update by calling the function a bunch of different times
                if (cell_index->row == i)
                {
                    GRect cellBounds = layer_get_bounds(cell_layer);
                    
                    MenuIndex currentIndex = menu_layer_get_selected_index(myMenuLayer);
                    int currentRow = currentIndex.row;
                    
                    
                    //set background color (alt color)
                    #ifdef PBL_COLOR
                        graphics_context_set_text_color(ctx, wd->config->altForegroundColor);
                        graphics_context_set_fill_color(ctx, wd->config->altBackgroundColor);
                        graphics_context_set_stroke_color(ctx, wd->config->altForegroundColor);
                    #endif
                    
                    //fill background
                    graphics_fill_rect(ctx, cellBounds, 0, GCornerNone);
                    
                    //set standard location of item box
                    cellBounds.origin.x = cellBounds.origin.x + 5;
                    cellBounds.size.w = cellBounds.size.w - 10;
                    
                    cellBounds.origin.y = cellBounds.origin.y + 3;
                    cellBounds.size.h = cellBounds.size.h - 6;
                    
                    //indent item box if needed
                    cellBounds.origin.x = cellBounds.origin.x + ((atoi(wd->projects->indentation[i])-1) * 7);
                    cellBounds.size.w = cellBounds.size.w - ((atoi(wd->projects->indentation[i])-1) * 7);
                    
                    
                    if (cell_index->row == currentRow)
                    {
                        //have to reverse background/foreground color on aplite as it seems to auto invert
                        //and basalt does not (you set the colors in "normal colors" function).
                        #ifdef PBL_COLOR
                            graphics_context_set_text_color(ctx, wd->config->highlightForegroundColor);
                            graphics_context_set_fill_color(ctx, wd->config->highlightBackgroundColor);
                            graphics_context_set_stroke_color(ctx, wd->config->highlightForegroundColor);
                        #endif
                    }
                    else
                    {
                        #ifdef PBL_COLOR
                            graphics_context_set_text_color(ctx, wd->config->foregroundColor);
                            graphics_context_set_fill_color(ctx, wd->config->backgroundColor);
                            graphics_context_set_stroke_color(ctx, wd->config->foregroundColor);
                        #endif
                    }
                    //fill rounded rect (Item space)
                    graphics_fill_rect(ctx, cellBounds, 5, GCornersAll);
                    //graphics_draw_round_rect(ctx, cellBounds, 5);
                    
                    
                    //if selected
                    if (cell_index->row == currentRow)
                    {
                        //increase cellbounds size so we can tell if the text will "overflow". if you dont do this the content size will stop at the end of the bounds
                        cellBounds.size.w = cellBounds.size.w*2.0;
                        GSize textSize = graphics_text_layout_get_content_size(wd->projects->projects[i], fonts_get_system_font(FONT_KEY_GOTHIC_18), cellBounds, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft);
                        cellBounds.size.w = cellBounds.size.w/2.0;
                        //snprintf(asdf, 100, "%d", textSize.w);
                        //displayErrorMessage(asdf);
                        if (textSize.w > cellBounds.size.w)
                        {
                            wd->currentScrollable = 1;
                        }
                        else
                        {
                            wd->currentScrollable = 0; 
                        }
                    }
                    cellBounds.origin.x = cellBounds.origin.x + 3;
                    graphics_draw_text(ctx, wd->projects->projects[i], fonts_get_system_font(FONT_KEY_GOTHIC_18), cellBounds, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
                }
            }
        }
    }
    
    if (wd->currentPage == 2)
    {
        if (wd->items && (wd->items->length != 0))
        {
            for(int i=0;i<(wd->items->length);i++)
            {
                if (cell_index->row == i)
                {
                    GRect cellBounds = layer_get_bounds(cell_layer);
                    
                    //text takes up 80% of the layer size due to the checkboxes
                    
                    //GRect newBounds = GRect(cellBounds.origin.x, cellBounds.origin.y, cellBounds.size.w*.80, cellBounds.size.h);
                    //layer_set_bounds(cell_layer, newBounds);
                    
                    MenuIndex currentIndex = menu_layer_get_selected_index(myMenuLayer);
                    int currentRow = currentIndex.row;
                    
                    //set background color (alt color)
                    #ifdef PBL_COLOR
                        graphics_context_set_text_color(ctx, wd->config->altForegroundColor);
                        graphics_context_set_fill_color(ctx, wd->config->altBackgroundColor);
                        graphics_context_set_stroke_color(ctx, wd->config->altForegroundColor);
                    #endif
                    
                    //fill background
                    graphics_fill_rect(ctx, cellBounds, 0, GCornerNone);
                    
                    //set standard location of item box
                    cellBounds.origin.x = cellBounds.origin.x + 5;
                    cellBounds.size.w = cellBounds.size.w - 10;
                    cellBounds.origin.y = cellBounds.origin.y + 3;
                    cellBounds.size.h = cellBounds.size.h - 6;
                    
                    //indent item box if needed
                    cellBounds.origin.x = cellBounds.origin.x + ((atoi(wd->items->indentation[i])-1) * 7);
                    cellBounds.size.w = cellBounds.size.w - ((atoi(wd->items->indentation[i])-1) * 7);
                    
                    GRect textBounds = GRect(cellBounds.origin.x, cellBounds.origin.y, cellBounds.size.w*.80, 30);
                    
                    
                    if (cell_index->row == currentRow)
                    {
                        //have to reverse background/foreground color on aplite as it seems to auto invert
                        //and basalt does not (you set the colors in "normal colors" function).
                        #ifdef PBL_COLOR
                            graphics_context_set_text_color(ctx, wd->config->highlightForegroundColor);
                            graphics_context_set_fill_color(ctx, wd->config->highlightBackgroundColor);
                            graphics_context_set_stroke_color(ctx, wd->config->highlightForegroundColor);
                        #endif
                    }
                    else
                    {
                        #ifdef PBL_COLOR
                            graphics_context_set_text_color(ctx, wd->config->foregroundColor);
                            graphics_context_set_fill_color(ctx, wd->config->backgroundColor);
                            graphics_context_set_stroke_color(ctx, wd->config->foregroundColor);
                        #endif
                    }
                    //fill rounded rect (Item space)
                    graphics_fill_rect(ctx, cellBounds, 5, GCornersAll);

                    //if selected
                    if (cell_index->row == currentRow)
                    {
                        GSize textSize = graphics_text_layout_get_content_size(wd->items->items[i], fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(cellBounds.origin.x, cellBounds.origin.y, cellBounds.size.w*2, 30), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft);
                        //snprintf(asdf, 100, "%d", textSize.w);
                        //displayErrorMessage(asdf);
                        if (textSize.w >= (textBounds.size.w))
                        {
                            wd->currentScrollable = 1;
                        }
                        else
                        {
                            wd->currentScrollable = 0; 
                        }
                    }
                    textBounds.origin.x = textBounds.origin.x + 3;
                    
                    graphics_draw_text(ctx, wd->items->items[i], fonts_get_system_font(FONT_KEY_GOTHIC_18), textBounds, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
                    
                    //if it has a date draw the date
                    if (strcmp(wd->items->itemDueDates[cell_index->row], "") != 0)
                    {
                        textBounds.origin.y = textBounds.origin.y+20;
                        //draw a seperator line if there is a date
                        //graphics_draw_line(ctx, GPoint(textBounds.origin.x-3, textBounds.origin.y+2), GPoint(textBounds.size.w, textBounds.origin.y+2));
                        graphics_draw_text(ctx, wd->items->itemDueDates[i], fonts_get_system_font(FONT_KEY_GOTHIC_18), textBounds, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
                    }
                    drawCheckbox(ctx, cell_layer, i);
                    //checkCheckbox(ctx, cell_layer, i);
                }
            }
        }
    }
}

//this is executed when row is changed.
void draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *callback_context)
{
    WindowData* wd = (WindowData*)window_get_user_data(window);
    if (!wd)
        return;
	
    if (wd->currentPage == 1)
    {
        if (wd->projects)
        {
            for(int i=0;i<(wd->projects->length);i++)
            {
                //remember this is the row it is CURRENTLY drawing not the SELECTED row, it will draw all rows on an update by calling the function a bunch of different times
                if (cell_index->row == i)
                {
                    GRect cellBounds = layer_get_bounds(cell_layer);
                    
                    MenuIndex currentIndex = menu_layer_get_selected_index(myMenuLayer);
                    int currentRow = currentIndex.row;
                    
                    if (cell_index->row == currentRow)
                    {
                        //have to reverse background/foreground color on aplite as it seems to auto invert
                        //and basalt does not (you set the colors in "normal colors" function).
                        #ifdef PBL_PLATFORM_BASALT
                            graphics_context_set_text_color(ctx, wd->config->highlightForegroundColor);
                            graphics_context_set_fill_color(ctx, wd->config->highlightBackgroundColor);
                            graphics_context_set_stroke_color(ctx, wd->config->highlightForegroundColor);
                        #else
                            graphics_context_set_text_color(ctx, wd->config->highlightForegroundColor);
                            graphics_context_set_fill_color(ctx, wd->config->highlightBackgroundColor);
                            graphics_context_set_stroke_color(ctx, wd->config->highlightForegroundColor);
                        #endif
                    }
                    else
                    {
                        if(i%2 == 0)
                        {
                            graphics_context_set_text_color(ctx, wd->config->foregroundColor);
                            graphics_context_set_fill_color(ctx, wd->config->backgroundColor);
                            graphics_context_set_stroke_color(ctx, wd->config->foregroundColor);
                        }  
                        else
                        {
                            graphics_context_set_text_color(ctx, wd->config->altForegroundColor);
                            graphics_context_set_fill_color(ctx, wd->config->altBackgroundColor);
                            graphics_context_set_stroke_color(ctx, wd->config->altForegroundColor);
                        }
                    }
                    //graphics_fill_rect(ctx, cellBounds, 0, GCornerNone);
                    
                    //set standard location of item box
                    cellBounds.origin.x = cellBounds.origin.x + 5;
                    cellBounds.size.w = cellBounds.size.w - 10;
                    
                    cellBounds.origin.y = cellBounds.origin.y + 3;
                    cellBounds.size.h = cellBounds.size.h - 6;
                    
                    //indent item box if needed
                    cellBounds.origin.x = cellBounds.origin.x + ((atoi(wd->projects->indentation[i])-1) * 7);
                    cellBounds.size.w = cellBounds.size.w - ((atoi(wd->projects->indentation[i])-1) * 7);
                    
                    //if selected
                    if (cell_index->row == currentRow)
                    {
                        //increase cellbounds size so we can tell if the text will "overflow". if you dont do this the content size will stop at the end of the bounds
                        cellBounds.size.w = cellBounds.size.w*2.0;
                        GSize textSize = graphics_text_layout_get_content_size(wd->projects->projects[i], fonts_get_system_font(FONT_KEY_GOTHIC_18), cellBounds, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft);
                        cellBounds.size.w = cellBounds.size.w/2.0;
                        //snprintf(asdf, 100, "%d", textSize.w);
                        //displayErrorMessage(asdf);
                        if (textSize.w > cellBounds.size.w)
                        {
                            wd->currentScrollable = 1;
                        }
                        else
                        {
                            wd->currentScrollable = 0; 
                        }
                    }
                    cellBounds.origin.x = cellBounds.origin.x + 3;
                    graphics_draw_text(ctx, wd->projects->projects[i], fonts_get_system_font(FONT_KEY_GOTHIC_18), cellBounds, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
                }
            }
        }
    }
    if (wd->currentPage == 2)
    {
        if (wd->items && (wd->items->length != 0))
        {
            for(int i=0;i<(wd->items->length);i++)
            {
                if (cell_index->row == i)
                {
                    GRect cellBounds = layer_get_bounds(cell_layer);
                    
                   
                    //GRect newBounds = GRect(cellBounds.origin.x, cellBounds.origin.y, cellBounds.size.w*.80, cellBounds.size.h);
                    //layer_set_bounds(cell_layer, newBounds);
                    
                    MenuIndex currentIndex = menu_layer_get_selected_index(myMenuLayer);
                    int currentRow = currentIndex.row;
                    
                    if (cell_index->row == currentRow)
                    {
                        //have to reverse background/foreground color on aplite as it seems to auto invert
                        //and basalt does not (you set the colors in "normal colors" function).
                        #ifdef PBL_PLATFORM_BASALT
                            graphics_context_set_text_color(ctx, wd->config->highlightForegroundColor);
                            graphics_context_set_fill_color(ctx, wd->config->highlightBackgroundColor);
                            graphics_context_set_stroke_color(ctx, wd->config->highlightForegroundColor);
                        #else
                            graphics_context_set_text_color(ctx, wd->config->highlightForegroundColor);
                            graphics_context_set_fill_color(ctx, wd->config->highlightBackgroundColor);
                            graphics_context_set_stroke_color(ctx, wd->config->highlightForegroundColor);
                        #endif
                    }
                    else
                    {
                        if(i%2 == 0)
                        {
                            graphics_context_set_text_color(ctx, wd->config->foregroundColor);
                            graphics_context_set_fill_color(ctx, wd->config->backgroundColor);
                            graphics_context_set_stroke_color(ctx, wd->config->foregroundColor);
                        }  
                        else
                        {
                            graphics_context_set_text_color(ctx, wd->config->altForegroundColor);
                            graphics_context_set_fill_color(ctx, wd->config->altBackgroundColor);
                            graphics_context_set_stroke_color(ctx, wd->config->altForegroundColor);
                        }
                    }
                    //graphics_fill_rect(ctx, cellBounds, 0, GCornerNone);
                    
                    //set standard location of item box
                    cellBounds.origin.x = cellBounds.origin.x + 5;
                    cellBounds.size.w = cellBounds.size.w - 10;
                    
                    cellBounds.origin.y = cellBounds.origin.y + 3;
                    cellBounds.size.h = cellBounds.size.h - 6;
                    
                    //indent item box if needed
                    cellBounds.origin.x = cellBounds.origin.x + ((atoi(wd->items->indentation[i])-1) * 7);
                    cellBounds.size.w = cellBounds.size.w - ((atoi(wd->items->indentation[i])-1) * 7);
                    
                    //text takes up 80% of the layer size due to the checkboxes
                    GRect textBounds = GRect(cellBounds.origin.x, cellBounds.origin.y, cellBounds.size.w*.80, 30);

                    //if selected
                    if (cell_index->row == currentRow)
                    {
                        GSize textSize = graphics_text_layout_get_content_size(wd->items->items[i], fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(cellBounds.origin.x, cellBounds.origin.y, cellBounds.size.w*2, 30), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft);
                        //snprintf(asdf, 100, "%d", textSize.w);
                        //displayErrorMessage(asdf);
                        if (textSize.w >= (textBounds.size.w))
                        {
                            wd->currentScrollable = 1;
                        }
                        else
                        {
                            wd->currentScrollable = 0; 
                        }
                    }
                    textBounds.origin.x = textBounds.origin.x + 3;
                    graphics_draw_text(ctx, wd->items->items[i], fonts_get_system_font(FONT_KEY_GOTHIC_18), textBounds, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
                    
                    //if it has a date draw the date
                    if (strcmp(wd->items->itemDueDates[cell_index->row], "") != 0)
                    {
                        textBounds.origin.y = textBounds.origin.y+20;
                        //draw a seperator line if there is a date
                        //graphics_draw_line(ctx, GPoint(textBounds.origin.x-3, textBounds.origin.y+2), GPoint(textBounds.size.w, textBounds.origin.y+2));
                        graphics_draw_text(ctx, wd->items->itemDueDates[i], fonts_get_system_font(FONT_KEY_GOTHIC_18), textBounds, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
                    }
                    drawCheckbox(ctx, cell_layer, i);
                    //checkCheckbox(ctx, cell_layer, i);
                }
            }
        }
    }
}

//sets height of cells
uint16_t cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context)
{
    WindowData* wd = (WindowData*)window_get_user_data(window);
    if (!wd)
        return 30;
    if (wd->currentPage == 1)
    {
        return 30;
    }
    else
    {
        if (strcmp(wd->items->itemDueDates[cell_index->row], "") == 0)
        {
            return 30;
        }
        else
        {
            return 50;    
        }
    }
    
}

void draw_separator_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context)
{
    GRect cellBounds = layer_get_bounds(cell_layer);
    graphics_draw_line(ctx, GPoint(cellBounds.origin.x + 10, cellBounds.origin.y), GPoint(cellBounds.size.w - 10, cellBounds.origin.y));
}

//sets number of rows
uint16_t num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *callback_context)
{
    WindowData* wd = (WindowData*)window_get_user_data(window);
    if(!wd)
        return 0;
    if (wd->currentPage == 1)
    {
        if (wd->projects)
        {
            return wd->projects->length;
        }
        else
        {
            return 0;
        }
    }
    if (wd->currentPage == 2)
    {
        if (wd->items)
        {
            return wd->items->length;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

void timerTick(void* data)
{
    WindowData* wd = (WindowData*)window_get_user_data(window);
    MenuIndex currentIndex = menu_layer_get_selected_index(myMenuLayer);
    int currentRow = currentIndex.row;
    if (wd->currentPage == 1 && wd->currentScrollable)
    {
        scrollTextByOneLetter(wd->projects->projects[currentRow]);
        if (wd->scrolledNumber == (int)strlen(wd->projects->projects[currentRow]) - 1)
            wd->scrolledNumber = 0;
        else
            wd->scrolledNumber++;
        layer_mark_dirty(menu_layer_get_layer(myMenuLayer));
    }
    if (wd->currentPage == 2 && wd->currentScrollable)
    {
        scrollTextByOneLetter(wd->items->items[currentRow]);
        if (wd->scrolledNumber == (int)strlen(wd->items->items[currentRow]) - 1)
            wd->scrolledNumber = 0;
        else
            wd->scrolledNumber++;
        layer_mark_dirty(menu_layer_get_layer(myMenuLayer));
    }
    wd->textScrollTimer = app_timer_register(wd->config->scrollSpeed, timerTick, NULL);
}

void scrollTextBackToStart()
{
    MenuIndex currentIndex = menu_layer_get_selected_index(myMenuLayer);
    WindowData* wd = (WindowData*)window_get_user_data(window);
    while (wd->scrolledNumber > 0)
    {
        if (wd->currentPage == 1)
        {
            scrollTextByOneLetter(wd->projects->projects[currentIndex.row]);
            if (wd->scrolledNumber == (int)strlen(wd->projects->projects[currentIndex.row]) - 1)
                wd->scrolledNumber = 0;
            else
                wd->scrolledNumber++;
        }
        if (wd->currentPage == 2)
        {
            scrollTextByOneLetter(wd->items->items[currentIndex.row]);
            if (wd->scrolledNumber == (int)strlen(wd->items->items[currentIndex.row]) - 1)
                wd->scrolledNumber = 0;
            else
                wd->scrolledNumber++;
        }
    }
}

//custom button click handling as we need to change the function of the back button
void config_provider(Window *window) 
{
    window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
    window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
    window_single_click_subscribe(BUTTON_ID_SELECT, menu_select_callback);
}

void window_load(Window *window)
{
    //int screenHeight = PBL_DISPLAY_HEIGHT;
    
    //status bar shows up by default on aplite, have to subtract it from our total space
    #ifndef PBL_COLOR
        //screenHeight = screenHeight - 16;
    #endif
    if (!myMenuLayer)
    {
        #ifdef PBL_ROUND
            //myMenuLayer = menu_layer_create(GRect(0, 0, 180, 180));
        #else
            //myMenuLayer = menu_layer_create(GRect(0, 0, 144, screenHeight));
        #endif
        
        myMenuLayer = menu_layer_create(GRect(0, 0, PBL_DISPLAY_WIDTH, PBL_DISPLAY_HEIGHT));
    }
    //removed this as we need a custom back button function
    //menu_layer_set_click_config_onto_window(myMenuLayer, window);
    
    //remove that ugly shadow
    scroll_layer_set_shadow_hidden(menu_layer_get_scroll_layer(myMenuLayer), 1);
    

    Config* config = (Config*)malloc(sizeof(Config));
    loadPersistentConfig(config);
    WindowData* wd = createWindowData(config);
    window_set_user_data(window, wd);
    wd->currentPage = 1;
    window_set_background_color(window, config->backgroundColor);
    #ifdef PBL_SDK_3
        menu_layer_set_normal_colors(myMenuLayer, config->backgroundColor, config->foregroundColor);
    #endif
    
    
    
    
    #ifdef PBL_ROUND
        MenuLayerCallbacks callbacks = 
        {
            .draw_row = (MenuLayerDrawRowCallback) draw_row_callback_round,
            .draw_header = (MenuLayerDrawHeaderCallback) draw_header_callback,
            .get_num_sections = (MenuLayerGetNumberOfSectionsCallback)menu_get_num_sections_callback,
            .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) num_rows_callback,
            .get_cell_height = (MenuLayerGetCellHeightCallback) cell_height_callback,
            .get_header_height = (MenuLayerGetHeaderHeightCallback) menu_get_header_height_callback
        };
        menu_layer_set_callbacks(myMenuLayer, NULL, callbacks);
    #elif PBL_RECT && PBL_COLOR
        MenuLayerCallbacks callbacks = 
        {
            .draw_row = (MenuLayerDrawRowCallback) draw_row_callback_modern,
            .draw_header = (MenuLayerDrawHeaderCallback) draw_header_callback,
            .get_num_sections = (MenuLayerGetNumberOfSectionsCallback)menu_get_num_sections_callback,
            .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) num_rows_callback,
            .get_cell_height = (MenuLayerGetCellHeightCallback) cell_height_callback,
            .get_header_height = (MenuLayerGetHeaderHeightCallback) menu_get_header_height_callback
        };
        menu_layer_set_callbacks(myMenuLayer, NULL, callbacks);
    #else
        MenuLayerCallbacks callbacks = 
        {
            .draw_row = (MenuLayerDrawRowCallback) draw_row_callback,
            .draw_header = (MenuLayerDrawHeaderCallback) draw_header_callback,
            .get_num_sections = (MenuLayerGetNumberOfSectionsCallback)menu_get_num_sections_callback,
            .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) num_rows_callback,
            .get_cell_height = (MenuLayerGetCellHeightCallback) cell_height_callback,
            .get_header_height = (MenuLayerGetHeaderHeightCallback) menu_get_header_height_callback
        };
        menu_layer_set_callbacks(myMenuLayer, NULL, callbacks);
    #endif
    

    window_set_click_config_provider(window, (ClickConfigProvider) config_provider);
    
    layer_add_child(window_get_root_layer(window), menu_layer_get_layer(myMenuLayer));
        
}
 
void window_unload(Window *window)
{
    menu_layer_destroy(myMenuLayer);
}

void init()
{
    //sets locale to support displaying of accented characters
    setlocale(LC_ALL,"");
    //register networking callbacks
    app_message_register_inbox_received(inbox_received_callback);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback);
    
    #ifdef PBL_PLATFORM_APLITE
        app_message_open(2500, 2500);
    #else
        app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
    #endif
    
    window = window_create();
    window_set_window_handlers(window, (WindowHandlers)
    {
        .load = window_load,
        .unload = window_unload,
    });

    window_stack_push(window, true);

}
 
void deinit()
{
    WindowData* wd = (WindowData*)window_get_user_data(window);
    if (wd->config)
        savePersistentConfig(wd->config);
    window_destroy(window);
}

int main(void)
{
    init();
    app_event_loop();
    deinit();
}
