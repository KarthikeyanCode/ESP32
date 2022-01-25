//global code to run default during script execution
var gateway, websocket;
window.addEventListener("load", on_load);

//on_load function for the webpage to setup things
function on_load(event)
{
    gateway = `ws://${window.location.hostname}/ws`;
    websocket_init();
    button_init();
}

//initialize button
function button_init()
{
    document.getElementById("ledtoggle_button").addEventListener("click", button_event);
}

//button event handler
function button_event(event)
{
    notify("sending the message through socket..");
    websocket.send("toggle led");//send the message through socket
}

//on webpage notification function
function notify(notif_string)
{
    //init and create nodes
    var main_div = document.getElementById("main_div");
    var notif_timer = 3000;//timeout in milliseconds
    var notif_node = document.createElement("p");
    var notif_node_text = document.createTextNode(notif_string);//create text node with given message
    
    //append the nodes
    notif_node.appendChild(notif_node_text);//append the text node to notif_node
    main_div.appendChild(notif_node);//append notif_node to main_div
    
    //remove the node after a timeout 
    setTimeout(() => {
        main_div.removeChild(main_div.lastChild);
    }, notif_timer);
}

//websocket functions
//##################################################################

//websocket init function
function websocket_init()
{
    notify("setting up websocket connection...");
    
    //websocket class instantiation
    websocket = new WebSocket(gateway);

    //setting up handler functions
    websocket.onopen = on_open;
    websocket.onclose = on_close;
    websocket.onmessage = on_message;
}

//websocket handler functions
//---------------------------------------------------------------
function on_open(event)
{
    notify("websocket connection opened");
}

function on_close(event)
{
    var timeout = 4000;
    notify("websocket connection closed");
    setTimeout(() => {
        websocket_init();
    }, timeout);
}

function on_message(event)
{
    var state;
    if(event.data.indexOf("high") != -1)
    {
        state = "high";
    }
    else if(event.data.indexOf("low") != -1)
    {
        state = "low";
    }
    else
    {
        //exception
    }
    document.getElementById("led_state").innerHTML = state;
}

//---------------------------------------------------------------
//##################################################################

