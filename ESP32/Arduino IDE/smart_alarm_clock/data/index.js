
function alarm_set_indication(alarm_time)
{
    var alarm_set_div = document.getElementById("notification_div");
    var alarm_set_node_timer = 3000;//in milliseconds
    var set_node = document.createElement("p");
    var set_node_text = document.createTextNode("The alarm has been set for " + alarm_time);
    set_node.appendChild(set_node_text);
    alarm_set_div.appendChild(set_node);
    //alarm_set_div.childNodes[0].style.color = "white";
    setTimeout(() => {
        alarm_set_div.removeChild(alarm_set_div.lastChild);
    }, alarm_set_node_timer);
}

function xmlhttp_request()
{
    const xhr = new XMLHttpRequest();
    var alarm_time = document.getElementById("alarm_time_input").value;
    var alarm_snooze_time = document.getElementById("alarm_snooze_time_input").value;
    const json_obj = {
        "type" : "alarm",
        "time" : String(alarm_time),
        "snooze_time" : parseInt(String(alarm_snooze_time))
    };

    //"=>" function returns value by default 
    xhr.onload = () => { //xhr.onload is a function that takes not arguments "()" and executes code inside "{}"
        if(xhr.status >= 200 && xhr.status < 300)
        {
            alarm_set_indication(alarm_time);
            //POST request made 
        }
    };

    xhr.open("POST", "http://192.168.1.125/index");
    xhr.setRequestHeader("conent-type", "application/json");
    xhr.send(JSON.stringify(json_obj));
    
}