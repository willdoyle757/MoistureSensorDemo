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
            padding: 20px;
            border: 1px solid #ccc;
            display: inline-block;
            background-color: #d0d3d9;
            border-radius: 25px;
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
        setInterval(fetchData, 2000);
    </script>
</head>
<body>
    <h1>Moisture Sensor Monitor</h1>
    <div class="sensor">
        <h2>Sensor 1</h2>
        <p>Moisture Level: <span id="sensor1">Loading...</span></p>
    </div>
    
</body>
</html>



)=====";