
var xhrRequest = function(url, type, callback) {
    var xhr = new XMLHttpRequest();
    xhr.onload = function() {
        callback(this.responseText);
    };
    xhr.open(type, url);
    xhr.send();
}

function locationSuccess(pos) {
    var url = "http://api.openweathermap.org/data/2.5/weather?lat=" +
        pos.coords.latitude + "&lon=" + pos.coords.longitude;

    xhrRequest(url, 'GET',
        function(responseText) {
            var json = JSON.parse(responseText);
            // console.log(JSON.stringify(json, null, "  "));

            var tempCelcius = json.main.temp - 273.15;
            var tempFahrenheit = Math.round(tempCelcius * 9 / 5 + 32);
            console.log("Temperature is " + tempFahrenheit);

            var conditions = json.weather[0].main;
            console.log("Conditions are " + conditions);

            var dictionary = {
                "KEY_TEMPERATURE": tempFahrenheit,
                "KEY_CONDITIONS": conditions
            };

            Pebble.sendAppMessage(dictionary,
                function(e) {
                    console.log("Weather info sent to Pebble successfully!");
                },
                function(e) {
                    console.log("Error sending weather info to Pebble!");
                }
            );
        }
    );
}

function locationError(err) {
    console.log("Error requesting location: ");
}

function getWeather() {
    // console.log("getWeather()");
    navigator.geolocation.getCurrentPosition(
        locationSuccess,
        locationError,
        {
            timeout: 15000, maximumAge: 60000
        }
    );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready',
    function(e) {
        console.log("PebbleKit JS ready!");
        getWeather();
    }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
    function(e) {
        console.log("AppMessage received!");
        getWeather();
    }
);
