#include <pgmspace.h>

const char PAGE_MAIN[] PROGMEM = R"=====(

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Moisture Sensor Monitor</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
        }
        .sensor {
            margin: 20px;
            padding: 10px;
            border: 1px solid #ccc;
            display: inline-block;
        }
        .button {
            margin-top: 10px;
            padding: 10px 20px;
            background-color: #4CAF50;
            color: white;
            border: none;
            cursor: pointer;
        }
        .button.off {
            background-color: #f44336;
        }
    </style>
    <script>
        function fetchData() {
            fetch('/live')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('sensor1').innerText = data.sensor1 + '%';
                });
        }
        function toggleSensor(sensorId) {
            fetch(`/toggle?sensor=${sensorId}`, { method: 'POST' })
                .then(response => response.text())
                .then(status => {
                    const button = document.getElementById(`button-${sensorId}`);
                    if (status === "ON") {
                        button.classList.remove('off');
                        button.innerText = "Turn Off";
                    } else {
                        button.classList.add('off');
                        button.innerText = "Turn On";
                    }
                });
        }
        setInterval(fetchData, 2000);
    </script>
</head>
<body>
    <h1>Moisture Sensor Monitor</h1>
    <div class="sensor">
        <h2>Sensor 1</h2>
        <p>Moisture Level: <span id="sensor1">Loading...</span></p>
        <button id="button-sensor1" class="button" onclick="toggleSensor('sensor1')">Turn Off</button>
    </div>
    
</body>
</html>

)=====";