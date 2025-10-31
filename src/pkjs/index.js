const clientID = '2a11ee6e18fe46ad89f9dcbd5507b76b';
const secret = 'cc88177b38774df2acfedd668be53824';
const apiUrl = 'https://api.todoist.com/sync/v9/sync';
const modernWatches = [
    "basalt", // Time/Time Steel
    "chalk", // Round
    "diorite"]; // Pebble 2

// For Aplite (Pebble / Pebble Steel) its needed:
const maxForLowMemDevices = 20;

var code;
var selectedProjectID;

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

var xhrRequest = function (url, type, callback, token = null) {
  const xhr = new XMLHttpRequest();
  xhr.onload = function () {
    if (this.status === 200) {
      callback(this.responseText);
    } else {
      sendErrorString("Error:Status " + this.status);
    }
  };
  xhr.open(type, url);
  if (token) {
    xhr.setRequestHeader('Authorization', 'Bearer ' + token);
  }
  xhr.send();
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

function getItems(responseText)
{    
    try {
        // responseText contains a JSON object with item info
        let json = JSON.parse(responseText);
        json = json.items;
    
        // My time steel crashes when there are a lot of tasks...
        if (!modernWatches.includes(getWatchVersion()) && json.length > maxForLowMemDevices) {
            json = json.slice(maxForLowMemDevices);
        }

        const isToday = selectedProjectID === 0 ? 1 : 0;
    
        //sort the list based on the item order property, if today, sort by date
        if (isToday)
        {
             json.sort((a, b) => {
                 const d1 = parseTodoistDate(a.due);
                 const d2 = parseTodoistDate(b.due);
                 return d1 - d2;
            });   
        }
        else
        {
            // Sort items considering parent-child relationships
            json.sort((a, b) => {
                // Get parent items
                const aParent = json.find(item => item.id === a.parent_id);
                const bParent = json.find(item => item.id === b.parent_id);
                
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
            
        if (json[0] && !json[0].hasOwnProperty("id"))
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
    
        for(var i=0;i<json.length;i++)
        {
            
            if (isToday)
            {
                var today = new Date();
                today.setHours(0);
                today.setMinutes(0);
                today.setSeconds(0);
                //considered "Today" if due date is in the current day or less (overdue)
                today = addDays(today, 1);
                if (json[i].due === null)
                    continue;
                var d = parseTodoistDate(json[i].due);
                if (d >= today)
                {
                    continue;
                }
            }
            else
            {
                //only proccess items that are in the selected project ID
                if (json[i].project_id != selectedProjectID)
                {
                    continue;
                }
            }            
            
            //items added via outlook have an ID tag in their content and some really weird syntax. The below is to fix this and show it as a normal item
            json[i].content = removeOutlookGarbage(json[i].content);
            
            itemNames = itemNames + json[i].content.replace("|", "") + " |";
            itemIDs = itemIDs  + json[i].id + "|";
            if (parseTodoistDateString(json[i].due) == "")
                itemDates = itemDates + "|";
            else
                itemDates = itemDates + parseTodoistDateString(json[i].due) + "|";
                itemIndentation = itemIndentation + getIndentLevel(json[i], json) + "|";
            if (json[i].due === null)
            {
                itemDueDates = itemDueDates + "|"; 
            }
            else
            {
                var d = parseTodoistDate(json[i].due);
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

function getToken(responseText) 
{
    // responseText contains a JSON object with token info
    var json = JSON.parse(responseText);
    if (responseText == "\"LOGIN_ERROR\"")
    {
        sendErrorMessage(1);
        openConfig();
        return;
    }
    // Conditions
    var token = json.access_token;
    localStorage.setItem("todoistMiniTokenV7", token);
    getProjectsFromToken();
}

function getProjects(responseText)
{
    try
    {
        // responseText contains a JSON object with project data
        var json = JSON.parse(responseText);
        json = json.projects;
    
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
                                       openConfig();   
                                  }
                                  if (code == 2)
                                  {
                                       getProjectsFromToken(); 
                                  }
                                  if (code == 3)
                                  {
                                       processTodoistDataWithGoogle();
                                  }
                                  if (code == 4)
                                  {
                                       processTodoistData();  
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

function processTodoistData() 
{
    var url = "https://todoist.com/oauth/access_token?client_id=" + encodeURIComponent(clientID) + "&client_secret=" + encodeURIComponent(secret) + "&code=" + encodeURIComponent(code);
    //localStorage.removeItem("todoistEmail");
    //localStorage.removeItem("todoistPassword");
    //note that xhr request is ASYNCHRONOUS everything after it in this function will get executed
    //before it is even finished the next path of execution HAS to be in the callback function
    xhrRequest(url, 'POST', getToken);
    
    //getProjectsFromToken();
}

function processTodoistDataWithGoogle()
{
    var url = "https://todoist.com/API/loginWithGoogle?email=" +
    encodeURIComponent(localStorage.getItem("googleEmail")) + "&oauth2_token=" + encodeURIComponent(localStorage.getItem("googleToken"));
    //note that xhr request is ASYNCHRONOUS everything after it in this function will get executed
    //before it is even finished the next path of execution HAS to be in the callback function
    xhrRequest(url, 'GET', getToken);
}

function getProjectsFromToken()
{
    const token = localStorage.getItem("todoistMiniTokenV7");
    const params = "sync_token=*&resource_types=[\"projects\"]";
    xhrRequest(apiUrl + "?" + params, 'GET', getProjects, token);
}

function getItemsForSelectedProject(projectID)
{
    selectedProjectID = projectID;
    const token = localStorage.getItem("todoistMiniTokenV7");
    const params = "sync_token=*&resource_types=[\"items\"]";
    xhrRequest(apiUrl + "?" + params, 'GET', getItems, token);
}

function getItemsForToday()
{
    selectedProjectID = 0; // seems to indicate today's project??
    const token = localStorage.getItem("todoistMiniTokenV7");
    const params = "sync_token=*&resource_types=[\"items\"]";
    xhrRequest(apiUrl + "?" + params, 'GET', getItems, token);
}

function addNewItem(itemText, projectID)
{
    // Pebble dictation always ends phrases with a dot, we don't want to send the dot to todoist.
    if (itemText.endsWith(".")) {
        itemText = itemText.slice(0, -1);
    }
    const commandsjson = [{
        "type": "item_add",
        "temp_id": createUUID(),
        "uuid": createUUID(),
        "args": {
            "content": itemText,
            "project_id": projectID
        }
    }];
    const token = localStorage.getItem("todoistMiniTokenV7");
    const params = "commands=" + encodeURIComponent(JSON.stringify(commandsjson));
    xhrRequest(apiUrl + "?" + params, 'GET', addItem, token);
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
    
    const token = localStorage.getItem("todoistMiniTokenV7");
    const params = "commands=" + encodeURIComponent(JSON.stringify(commandsjson));
    xhrRequest(apiUrl + "?" + params, 'GET', markItem, token);
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
    
    const token = localStorage.getItem("todoistMiniTokenV7");
    const params = "commands=" + encodeURIComponent(JSON.stringify(commandsjson));
    xhrRequest(apiUrl + "?" + params, 'GET', markRecurringItem, token);
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
    
    const token = localStorage.getItem("todoistMiniTokenV7");
    const params = "commands=" + encodeURIComponent(JSON.stringify(commandsjson));
    xhrRequest(apiUrl + "?" + params, 'GET', uncompleteItem, token);
}


// Listen for when the watchface is opened
Pebble.addEventListener('ready', startup);

function startup()
{
    //enables timeline by default if it has never been set.
    if (localStorage.getItem("timelineEnabled") === null)
        localStorage.setItem("timelineEnabled", "true");

    if (localStorage.getItem("todoistMiniTokenV7") === null)
    {
        sendWaitingMessageAndPerformAction(1);
    }
    else
    {
        sendWaitingMessageAndPerformAction(2);
    }
}

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    if(e.payload.SELECTED_PROJECT)
    {
        if (e.payload.SELECTED_PROJECT == "0")
            getItemsForToday();
        else
            getItemsForSelectedProject(e.payload.SELECTED_PROJECT);
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

//sets the configuration options from the config page that the user has just saved.
function setConfig(loginData)
{
    try
    {
        localStorage.setItem("ConfigData", JSON.stringify(loginData));
        var configString = loginData.scrollSpeed + '|' + loginData.backgroundColor + '|' + loginData.foregroundColor + '|' + loginData.altBackgroundColor + '|' + loginData.altForegroundColor + '|' + loginData.highlightBackgroundColor + '|' + loginData.highlightForegroundColor + '|' + loginData.timelineEnabled + '|';
        localStorage.setItem("timelineEnabled", loginData.timelineEnabled);
        var dictionary = 
        {
            "CONFIG": configString
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

function openConfig(e) {
    const baseUrl = "https://perogy.github.io/PebbleProject/indexNew.html";
    
    if (localStorage.getItem("ConfigData") === null) {
        Pebble.openURL(baseUrl);
    } else {
        const configData = JSON.parse(localStorage.getItem("ConfigData"));
        const params = new URLSearchParams({
            scrollSpeed: configData.scrollSpeed,
            backgroundColor: configData.backgroundColor,
            foregroundColor: configData.foregroundColor,
            altBackgroundColor: configData.altBackgroundColor,
            altForegroundColor: configData.altForegroundColor,
            highlightBackgroundColor: configData.highlightBackgroundColor,
            highlightForegroundColor: configData.highlightForegroundColor,
            timelineEnabled: configData.timelineEnabled
        });
        
        Pebble.openURL(`${baseUrl}#${params.toString()}`);
    }
}

function closeConfig(e) {
    try
    {
        //if they pressed back on the settings screen (no save or login), just run the startup function
        if (typeof(e.response) == "undefined")
        {
            startup();
            return;
        }
        var loginData = JSON.parse(decodeURIComponent(e.response));
        
        
        if (loginData.type == "configData")
        {
            setConfig(loginData);
            return;
        }
        
        if (loginData.googleToken)
        {
            //check whether google or normal login and then run appropriate code
            localStorage.setItem("googleToken", loginData.token);
            localStorage.setItem("googleEmail", loginData.email);
            sendWaitingMessageAndPerformAction(3);
            
        }
        else
        {
            code = loginData.code;
            sendWaitingMessageAndPerformAction(4);
        }
    }
    catch (err)
    {
        sendErrorString(err.message);
    }

}
// Listen for a configuration request
Pebble.addEventListener('showConfiguration', openConfig);

//Listen for configuration window closing
Pebble.addEventListener('webviewclosed', closeConfig);
