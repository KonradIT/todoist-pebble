const apiUrl = 'https://api.todoist.com/api/v1/sync';

// modernWatches lists the *platforms* that have a fair amount of RAM
// to support large task lists.
// https://developer.repebble.com/guides/tools-and-resources/hardware-information/
const modernWatches = [
    // These have 64k for code + heap:
    "basalt", // Time/Time Steel
    "chalk", // Round
    "diorite", // Pebble 2
    "flint", // Pebble 2 Duo
    // These have 128k for code + heap:
    "emery", // Pebble Time 2
    "gabbro", // Pebble Round 2
];


// For Aplite (Pebble / Pebble Steel) its needed:
const maxForLowMemDevices = 20;

// TODO: Persist settings on watch? This may make startup times faster.
// https://developer.repebble.com/guides/user-interfaces/app-configuration/#persisting-settings
var Clay = require('@rebble/clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig);

var markCompletedUUID;
var markCompletedItemID;

function createUUID() {
    if (typeof crypto !== 'undefined' && crypto.randomUUID) {
        return crypto.randomUUID();
    }
    
    // http://www.ietf.org/rfc/rfc4122.txt
    var s = [];
    var hexDigits = "0123456789abcdef";
    for (var i = 0; i < 36; i++) {
        s[i] = hexDigits.substr(Math.floor(Math.random() * 0x10), 1);
    }
    s[14] = "4";  // bits 12-15 of the time_hi_and_version field to 0010
    s[19] = hexDigits.substr((s[19] & 0x3) | 0x8, 1);  // bits 6-7 of the clock_seq_hi_and_reserved to 01
    s[8] = s[13] = s[18] = s[23] = "-";

    var uuid = s.join("");
    return uuid;
}

var xhrRequest = function (url, type, callback, body = null) {
  const token = getAPIToken();

  const xhr = new XMLHttpRequest();
  xhr.onload = function () {
    if (this.status === 200 || this.status === 204) {
      callback(this.responseText || '{}');
    } else {
      sendErrorString("Error:Status " + this.status);
    }
  };
  xhr.open(type, url);
  xhr.setRequestHeader('Authorization', 'Bearer ' + token);
  if (body) {
    xhr.setRequestHeader('Content-Type', 'application/json');
  }
  xhr.send(body);
};

function getWatchVersion()
{
    var platform;
    if(Pebble.getActiveWatchInfo) {
      var watchinfo= Pebble.getActiveWatchInfo();
      platform=watchinfo.platform;
      } else {
        platform="aplite";
      }
    return platform;
}

//if integer is 0 return a string 00. Used for times
function leadingZeroCheck(number)
{
    if (number < 10)
        return "0" + number;
    else
        return number;
}

function removeOutlookGarbage(str)
{
    if (startsWith(str,"[[outlook=id"))
    {
        var contentStart = str.indexOf(",") + 2;
        var contentEnd = str.indexOf("]]");
        return str.substring(contentStart, contentEnd);
    }
    else
    {
        return str;
    }
}

//defining my own startsWith as the built in one does not seem to work
//str = string to search
//strMatch = string to match
function startsWith(str, strMatch)
{
    for(var i = 0; i < str.length; i++)
    {
        if (str.substring(0, i) == strMatch)
            return true;
    }
    return false;
}

function addDays(date, days) {
    var result = new Date(date);
    result.setDate(result.getDate() + days);
    return result;
}

function parseTodoistDate(dueObject) {
    if (dueObject === null) {
        return new Date(2025, 1, 1);
    }
    // parses this: 2016-12-0T12:00:00.000000 to a Date object
    const timestamp = Date.parse(dueObject.date);
    return new Date(timestamp); 
}

function parseTodoistDateString(dueObject) {
    if (dueObject === null) {
        return "";
    }
    return dueObject.string;
}

function getIndentLevel(item, allItems) {
    let level = 1; // Start at level 1
    let currentItem = item;
    
    // Traverse up the parent hierarchy until we reach a top-level item
    while (currentItem.parent_id) {
        level++;
        // Find the parent item
        currentItem = allItems.find(i => i.id === currentItem.parent_id);
        // Break if parent not found to prevent infinite loops
        if (!currentItem) break;
    }
    
    return level;
}

function getItems(selectedProjectID, state)
{
    try {
        // Extract the current user's ID for filtering out other users' assigned tasks below.
        const currentUserID = state.user.id;

        let items = state.items;

        // My time steel crashes when there are a lot of tasks...
        if (!modernWatches.includes(getWatchVersion()) && items.length > maxForLowMemDevices) {
            items = items.slice(maxForLowMemDevices);
        }

        const isToday = selectedProjectID === 0 ? 1 : 0;

        //sort the list based on the item order property, if today, sort by date
        if (isToday)
        {
             items.sort((a, b) => {
                 const d1 = parseTodoistDate(a.due);
                 const d2 = parseTodoistDate(b.due);
                 return d1 - d2;
            });
        }
        else
        {
            // Sort items considering parent-child relationships
            items.sort((a, b) => {
                // Get parent items
                const aParent = items.find(item => item.id === a.parent_id);
                const bParent = items.find(item => item.id === b.parent_id);

                // If items have different parents, sort by parent's child_order
                if (aParent !== bParent) {
                    const aParentOrder = aParent ? parseInt(aParent.child_order) : parseInt(a.child_order);
                    const bParentOrder = bParent ? parseInt(bParent.child_order) : parseInt(b.child_order);
                    return bParentOrder - aParentOrder;
                }

                // If items have same parent (or both are top-level), sort by their child_order
                return parseInt(b.child_order) - parseInt(a.child_order);
            });
        }

        if (items[0] && !items[0].hasOwnProperty("id"))
        {
            sendErrorMessage(3);
            return;
        }

        const monthNames = ["Jan", "Feb", "Mar", "Apr", "May", "Jun",
                          "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"];


        // Conditions
        let itemNames = "";
        let itemIDs = "";
        let itemDates = "";
        let itemDueDates = "";
        let itemIndentation = "";

        const watchVersion = getWatchVersion();

        //only put "Add New" if we are on modern watches
        if (modernWatches.includes(watchVersion) && !isToday)
        {
            itemNames += "+ Add New |";
            itemIDs += "0|";
            itemDates += "|";
            itemDueDates += "|";
            itemIndentation += "1|";
        }

        for (const item of items)
        {
            // Ignore checked (completed) tasks. These will eventually stop getting synced.
            if (item.checked)
                continue;

            if (isToday)
            {
                // Ignore items assigned to someone other than the current user,
                // mimicking the official app. TODO: Make this configurable.
                if (!!item.responsible_uid && item.responsible_uid != currentUserID)
                {
                    continue;
                }

                var today = new Date();
                today.setHours(0);
                today.setMinutes(0);
                today.setSeconds(0);
                //considered "Today" if due date is in the current day or less (overdue)
                today = addDays(today, 1);
                if (item.due === null)
                    continue;
                var d = parseTodoistDate(item.due);
                if (d >= today)
                {
                    continue;
                }
            }
            else
            {
                //only proccess items that are in the selected project ID
                if (item.project_id != selectedProjectID)
                {
                    continue;
                }
            }

            //items added via outlook have an ID tag in their content and some really weird syntax. The below is to fix this and show it as a normal item
            item.content = removeOutlookGarbage(item.content);

            itemNames = itemNames + item.content.replace("|", "") + " |";
            itemIDs = itemIDs  + item.id + "|";
            if (parseTodoistDateString(item.due) == "")
                itemDates = itemDates + "|";
            else
                itemDates = itemDates + parseTodoistDateString(item.due) + "|";
                itemIndentation = itemIndentation + getIndentLevel(item, items) + "|";
            if (item.due === null)
            {
                itemDueDates = itemDueDates + "|";
            }
            else
            {
                var d = parseTodoistDate(item.due);
                //if the time is 23:59 this specifies "no time"
                if ((d.getHours() == 23) && (d.getMinutes() == 59))
                    itemDueDates = itemDueDates + monthNames[d.getMonth()] + " " + d.getDate() + "|";
                else
                    itemDueDates = itemDueDates + monthNames[d.getMonth()] + " " + d.getDate() + " " + d.getHours() + ":" + leadingZeroCheck(d.getMinutes())  + "|";
            }
        }



        var dictionary =
        {
            "ITEM_NAMES": itemNames,
            "ITEM_IDS": itemIDs,
            "ITEM_DATES": itemDates,
            "ITEM_DUE_DATES": itemDueDates,
            "ITEM_INDENTATION": itemIndentation
        };


        // Send to Pebble
        Pebble.sendAppMessage(dictionary,
                              function(e)
                              {

                              },
                              function(e)
                              {
                                  sendErrorString(e.error.message);
                              });
    }
    catch (err)
    {
        sendErrorString(err.message);
    }
}

function getProjects(state)
{
    try
    {
        json = state.projects;
    
        if (json[0])
        {
            if (!json[0].hasOwnProperty("id"))
            {
                sendErrorMessage(2);
                return;
            }
        }
        // Conditions
        var projectNames = "";
        var projectIDs = "";
        var projectIndentation = "";
        
        //put today project in (custom)
        projectNames = "Today |";
        projectIDs = "0|";
        projectIndentation = "1|";
        
        //sort the list based on the item order property
        json.sort(function(a, b) {
            return parseInt(a.item_order) - parseInt(b.item_order);
        });
        
        for(var i=0;i<json.length;i++)
        {
            projectNames = projectNames + json[i].name.replace("|", "")  + " |";
            projectIDs = projectIDs  + json[i].id + "|";
            projectIndentation = projectIndentation + getIndentLevel(json[i], json) + "|";

            // Set inbox_project in localStorage if the project has inbox_project set to true
            if (json[i].inbox_project && localStorage.getItem("inboxProjectID") === "" ) {
                localStorage.setItem("inboxProjectID", json[i].id);
            }
        }
    
        var dictionary = 
        {
            "PROJECT_NAMES": projectNames,
            "PROJECT_IDs": projectIDs,
            "PROJECT_INDENTATION": projectIndentation
        };
    
        // Send to Pebble
        Pebble.sendAppMessage(dictionary,
                              function(e) 
                              {
                                  sendErrorString(e.error.message);
                              });   
    }
    catch (err)
    {
        sendErrorString(err.message);
    }
}

function markItem(responseText)
{
    if (responseText.search("ok") > 0)
    {
        var dictionary = 
        {
            "SELECTED_ITEM": "1"
        };
        Pebble.sendAppMessage(dictionary,
                          function(e) 
                          {
                              
                          },
                          function(e) 
                          {
                              sendErrorString(e.error.message);
                          });   
    }
    else
    {
        var dictionary = 
        {
            "SELECTED_ITEM": "0"
        };
        Pebble.sendAppMessage(dictionary,
                          function(e) 
                          {
                              
                          },
                          function(e) 
                          {
                              sendErrorString(e.error.message);
                          });   
    }
    
}

function markRecurringItem(responseText)
{
    if (responseText.search("ok") > 0)
    {
        var dictionary = 
        {
            "SELECTED_ITEM": "1"
        };
        Pebble.sendAppMessage(dictionary,
                          function(e) 
                          {
                              
                          },
                          function(e) 
                          {
                              sendErrorString(e.error.message);
                          });   
    }
    else
    {
        var dictionary = 
        {
            "SELECTED_ITEM": "0"
        };
        Pebble.sendAppMessage(dictionary,
                          function(e) 
                          {
                              
                          },
                          function(e) 
                          {
                              sendErrorString(e.error.message);
                          });   
    }
    
}

function uncompleteItem(responseText)
{
    if (responseText.search("ok") > 0)
    {
        var dictionary = 
        {
            "SELECTED_ITEM_UNCOMPLETE": "1"
        };
        Pebble.sendAppMessage(dictionary,
                          function(e) 
                          {
                              
                          },
                          function(e) 
                          {
                              sendErrorString(e.error.message);
                          });   
    }
    else
    {
        var dictionary = 
        {
            "SELECTED_ITEM_UNCOMPLETE": "0"
        };
        Pebble.sendAppMessage(dictionary,
                          function(e) 
                          {
                              
                          },
                          function(e) 
                          {
                              sendErrorString(e.error.message);
                          });   
    }
    
}

function addItem(responseText)
{
    //add response error handling here
        var dictionary = 
        {
            "ADD_NEW_ITEM": "1"
        };
        Pebble.sendAppMessage(dictionary,
                          function(e) 
                          {
                              
                          },
                          function(e) 
                          {
                              sendErrorString(e.error.message);
                          });
}

//code 1 = waiting for config
//code 2 = waiting to load data
function sendWaitingMessageAndPerformAction(code)
{
    try
    {
        //when we send app message it just needs to be a 1 or 2 (config or loading) 3 = timeline loading
        var sendCode;
        if ((code == 3) || (code == 4))
            sendCode = 2;
        else if (code == 5)
            sendCode = 3;
        else
            sendCode = code;
        
     
        var dictionary = 
            {
                "WAITING": sendCode
            };
            Pebble.sendAppMessage(dictionary,
                              function(e) 
                              {
                                  if (code == 1)
                                  {
                                      Pebble.openURL(clay.generateUrl());
                                  }
                                  if (code == 2)
                                  {
                                      todoistSync(getProjects);
                                  }
                              },
                              function(e) 
                              {
                                  sendErrorString(e.error.message);
                              });
    }
    catch (err)
    {
        sendErrorString(err.message);
    }
      
}

//code 1 = Login failed
function sendErrorMessage(code)
{
    var dictionary = 
        {
            "ERROR": code
        };
        Pebble.sendAppMessage(dictionary,
                          function(e) 
                          {
                              
                          },
                          function(e) 
                          {
                              sendErrorString(e.error.message);
                          });   
}

function sendErrorString(errorMsg)
{
    var dictionary = 
        {
            "ERRORMSG": errorMsg
        };
        Pebble.sendAppMessage(dictionary,
                          function(e) 
                          {
                              
                          },
                          function(e) 
                          {
                              sendErrorString(e.error.message);
                          });   
}

function getAPIToken()
{
    const settings = JSON.parse(localStorage.getItem('clay-settings'));
    return settings.API_TOKEN;
}

// todoistSync refreshes the sync state and runs a callback with the latest state.
//
// This attempts to almost always do an incremental sync, which is much more
// efficient, and can support a higher refresh rate.
// https://developer.todoist.com/api/v1/#tag/Sync
function todoistSync(callback)
{
    // Look for existing sync state of an appropriate revision.
    let syncToken = localStorage.getItem("todoistSyncToken") || "*";
    if (localStorage.getItem("todoistSyncRev") != "rev1")
        syncToken = "*";  // last sync has a different subset; start afresh
    // TODO: Periodically (e.g. every 30d?), do a full sync to clear old state, like deleted tasks.
    let state = {};
    if (syncToken != "*")
        state = JSON.parse(localStorage.getItem("todoistSyncState") || "{}");

    // Sync data.
    //console.log("Starting Todoist sync with syncToken=" + syncToken);
    // Unfortunately, URLSearchParams is not in Pebble's JS environment.
    const params = "sync_token=" + encodeURIComponent(syncToken) + `&resource_types=["items","projects","user"]`;
    xhrRequest(apiUrl + "?" + params, "POST", function(response) {
        //console.log("Todoist sync returned " + response.length + " bytes");
        const data = JSON.parse(response);

        // Merge incremental data.
        if (data.full_sync)
            state = {};
        for (const key of ["items", "projects"]) {  // handle resource types that are lists
            if (!state[key])
                state[key] = [];
            for (const val of (data[key] || [])) {
                // Replace corresponding entry in state[key], matching on ID.
                let found = false;
                for (let i = 0; i < state[key].length; ++i) {
                    if (state[key][i].id == val.id) {
                        state[key][i] = val;
                        found = true;
                        break;
                    }
                }
                if (!found)
                    state[key].push(val);
            }
        }
        for (const key of ["user"]) {  // handle resource types that are values
            const val = data[key];
            if (!!val)
                state[key] = val;
        }

        // Save sync state.
        localStorage.setItem("todoistSyncState", JSON.stringify(state));
        localStorage.setItem("todoistSyncToken", data.sync_token);
        localStorage.setItem("todoistSyncRev", "rev1");

        // Call the final callback.
        callback(state);
    });
}

function addNewItem(itemText, projectID)
{
    // Pebble dictation always ends phrases with a dot, we don't want to send the dot to todoist.
    if (itemText.endsWith(".")) {
        itemText = itemText.slice(0, -1);
    }
    
    // Check if this is for Inbox - use Quick Add API
    if (localStorage.getItem("inboxProjectID") !== "" && projectID === localStorage.getItem("inboxProjectID")) {
        const quickAddData = {
            "text": itemText + " #Inbox",
            "auto_reminder": false,
            "meta": false
        };
        xhrRequest('https://api.todoist.com/api/v1/tasks/quick', 'POST', addItem, JSON.stringify(quickAddData));
    } else {
        // Use sync API for other projects
        const commandsjson = [{
            "type": "item_add",
            "temp_id": createUUID(),
            "uuid": createUUID(),
            "args": {
                "content": itemText,
                "project_id": projectID
            }
        }];
        const params = "commands=" + encodeURIComponent(JSON.stringify(commandsjson));
        xhrRequest(apiUrl + "?" + params, 'POST', addItem);
    }
}

function markItemAsCompleted(itemID)
{
    markCompletedUUID = createUUID();
    markCompletedItemID = itemID;
    const commandsjson = [{
        "type": "item_complete",
        "uuid": createUUID(),
        "args": {
            "id": itemID
        }
    }];
    
    const params = "commands=" + encodeURIComponent(JSON.stringify(commandsjson));
    xhrRequest(apiUrl + "?" + params, 'POST', markItem);
}

function markRecurringItemAsCompleted(itemID)
{
    markCompletedUUID = createUUID();
    markCompletedItemID = itemID;
    var commandsjson = [{
        "type": "item_update_date_complete", 
        "uuid": createUUID(), 
        "args": 
        {
            "id": itemID
        }
    }];
    
    const params = "commands=" + encodeURIComponent(JSON.stringify(commandsjson));
    xhrRequest(apiUrl + "?" + params, 'POST', markRecurringItem);
}

function markItemAsUncompleted(itemID)
{
    markCompletedUUID = createUUID();
    markCompletedItemID = itemID;
    var commandsjson = [{
        "type": "item_uncomplete", 
        "uuid": createUUID(), 
        "args": 
        {
            "ids": [itemID]
        }
    }];
    
    const params = "commands=" + encodeURIComponent(JSON.stringify(commandsjson));
    xhrRequest(apiUrl + "?" + params, 'POST', uncompleteItem);
}


// Listen for when the watchface is opened
Pebble.addEventListener('ready', startup);

function startup()
{
    //enables timeline by default if it has never been set.
    if (localStorage.getItem("timelineEnabled") === null)
        localStorage.setItem("timelineEnabled", "true");

    sendWaitingMessageAndPerformAction(2);
}

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    if(e.payload.SELECTED_PROJECT)
    {
        if (e.payload.SELECTED_PROJECT == "0")  // seems to indicate today's project?
            todoistSync(getItems.bind(null, 0));
        else
            todoistSync(getItems.bind(null, e.payload.SELECTED_PROJECT));
    }
    if(e.payload.SELECTED_ITEM)
    {
        markItemAsCompleted(e.payload.SELECTED_ITEM);
    }
    if(e.payload.SELECTED_ITEM_RECURRING)
    {
        markRecurringItemAsCompleted(e.payload.SELECTED_ITEM_RECURRING);
    }
    if(e.payload.SELECTED_ITEM_UNCOMPLETE)
    {
        markItemAsUncompleted(e.payload.SELECTED_ITEM_UNCOMPLETE);
    }
    if(e.payload.ADD_NEW_ITEM)
    {
        var array = e.payload.ADD_NEW_ITEM.split("|");
        addNewItem(array[0], array[1]);
    }
  }                     
);
