window.addEventListener('load', getTimer);

function getTimer() {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/readings", true);
    xhr.send();
}

function reset() {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/reset", true);
    xhr.send();
}

function change_gamemode_start_stop() {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/change_gamemode_start_stop", true);
    xhr.send();
}

if (!!window.EventSource) {
    var source = new EventSource('/events');

    source.addEventListener('open', function (e) {
        console.log("Events Connected");
    }, false);

    source.addEventListener('error', function (e) {
        if (e.target.readyState != EventSource.OPEN) {
            console.log("Events Disconnected");
        }
    }, false);

    source.addEventListener('message', function (e) {
        console.log("message", e.data);
    }, false);

    source.addEventListener('timer_stopped', function (e) {
        console.log("timer_stopped", e.data);
        var time = e.data;
        document.getElementById('elapsed-time').innerHTML = time + "s";
    }, false);
}