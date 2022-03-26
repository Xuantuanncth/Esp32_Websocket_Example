const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <title>ESP Web Server</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="icon" href="data:,">
    <style>
        html {
            font-family: Arial, Helvetica, sans-serif;
            text-align: center;
        }

        h1 {
            font-size: 1.8rem;
            color: white;
        }

        h2 {
            font-size: 1.5rem;
            font-weight: bold;
            color: #143642;
        }

        h3 {
            font-size: 1.5rem;
            font-weight: bold;
            color: #0b7da7;
        }

        .topnav {
            overflow: hidden;
            background-color: #143642;
        }

        body {
            margin: 0;
        }

        .content {
            padding: 30px;
            max-width: 600px;
            margin: 0 auto;
        }

        .card {
            background-color: #F8F7F9;
            ;
            box-shadow: 2px 2px 12px 1px rgba(140, 140, 140, .5);
            padding-top: 10px;
            padding-bottom: 20px;
        }

        .button {
            padding: 15px 50px;
            font-size: 24px;
            text-align: center;
            outline: none;
            color: #fff;
            background-color: #0f8b8d;
            border: none;
            border-radius: 5px;
            -webkit-touch-callout: none;
            -webkit-user-select: none;
            -khtml-user-select: none;
            -moz-user-select: none;
            -ms-user-select: none;
            user-select: none;
            -webkit-tap-highlight-color: rgba(0, 0, 0, 0);
        }

        /*.button:hover {background-color: #0f8b8d}*/
        .button:active {
            background-color: #0f8b8d;
            box-shadow: 2 2px #CDCDCD;
            transform: translateY(2px);
        }

        .state {
            font-size: 1.5rem;
            color: #8c8c8c;
            font-weight: bold;
        }

        .textInput {
            font: 1.5em sans-serif;
            color: rgb(24, 68, 53);
            font-weight: 200;
        }

        .colorInput {
            color: #0f8b8d;
            height: 40px;
            width: 120px;
        }
    </style>
    <title>ESP Web Server</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="icon" href="data:,">
</head>

<body>
    <div class="topnav">
        <h1>ESP WebSocket Server</h1>
    </div>
    <div class="content">
        <div class="card">
            <h2>Output - GPIO 2</h2>
            <p class="state">state: <span id="state">%STATE%</span></p>
            <p><button id="button" class="button">Toggle</button></p>
        </div>
        <br>
        <div class="card">
            <div>
                <h3>Text Input</h3>
                <input type="text" id="text_input" placeholder="InputText" class="textInput">
                <p><button id="bt_text_input" class="button" onclick="sendtext()">Send</button></p>
            </div>
        </div>
        <br>
        <div class="card">
            <div>
                <h3>Color Input</h3>
                <input type="color" name="colorInput" id="color_input" class="colorInput">
                <p><button id="bt_color_input" class="button" onclick="sendcolor()">Save</button></p>
            </div>
        </div>
        <br>
        <div class="card">
            <div>
                <h3>Effect</h3>
                <p><button id="bt_Effect1" class="button" onclick="sendEff1()">Effect1</button></p>
                <p><button id="bt_Effect2" class="button" onclick="sendEff2()">Effect2</button></p>
                <p><button id="bt_Effect3" class="button" onclick="sendEff3()">Effect3</button></p>
                <p><button id="bt_Scroll" class="button" onclick="Scroll()">Scroll Right</button></p>
                <input id="speed_time" type="range" min="10" max="99" value="50" >
                <p><button id="bt_Speed" class="button" onclick="Speed()">Speed</button></p>
            </div>
        </div>
        <br>
        <div class="card">
            <div>
                <h3>Setting Time On</h3>
                <input type="time" name="timeOnSetting" id="timeON" class="colorInput">
                <p><button id="bt_timeON" class="button" onclick="sendTimeON()">Save</button></p>
            </div>
        </div>
        <br>
        <div class="card">
            <div>
                <h3>Setting Time OFF</h3>
                <input type="time" name="timeOFFSetting" id="timeOFF" class="colorInput">
                <p><button id="bt_timeOFF" class="button" onclick="sendTimeOFF()">Save</button></p>
            </div>
        </div>
    </div>
    <script>
        var gateway = `ws://${window.location.hostname}/ws`;
        var websocket;
        let scroll_text = 0;
        window.addEventListener('load', onLoad);
        function initWebSocket() {
            console.log('Trying to open a WebSocket connection...');
            websocket = new WebSocket(gateway);
            websocket.onopen = onOpen;
            websocket.onclose = onClose;
            websocket.onmessage = onMessage; // <-- add this line
        }
        function onOpen(event) {
            console.log('Connection opened');
        }
        function onClose(event) {
            console.log('Connection closed');
            setTimeout(initWebSocket, 2000);
        }
        function onMessage(event) {
            var state;
            console.log("=========> event: ", event.data);
            let result = event.data.split(",");
            if(result[0] == "1") {
                document.getElementById('text_input').value = result[1];
            }
            if(result[0] == "2") {
                scroll_text = result[1];
                if(result[1] == "0") {
                    document.getElementById("bt_Scroll").innerHTML = "Scroll Right";
                } else {
                    document.getElementById("bt_Scroll").innerHTML = "Scroll Left";
                }
                
            }
            if (event.data == "1") {
                state = "ON";
            }
            else {
                state = "OFF";
            }
            document.getElementById('state').innerHTML = state;
        }
        function onLoad(event) {
            initWebSocket();
            initButton();
        }
        function initButton() {
            document.getElementById('button').addEventListener('click', toggle);
        }
        function sendtext() {
            let text = document.getElementById('text_input').value;
            console.log("Send text ==========> value: ", text);
            websocket.send("1,"+text);
        }
        function sendcolor() {
            let color_input = document.getElementById("color_input").value;
            console.log(" ========> send Color");
            if (hexToRgb(color_input)) {
                let color = hexToRgb(color_input);
                console.log(color);
                 websocket.send("2,"+color.r + "," + color.g + "," + color.b);
            }
        }
        function sendEff1() {
            console.log("=========> Eff1");
            websocket.send("3");
        }
        function sendEff2() {
            console.log("=========> Eff2");
            websocket.send("4");
        }
        function sendEff3() {
            console.log("=========> Eff3");
            websocket.send("5");
        }
        function Scroll() {
            console.log("============> Scroll");
            scroll_text = scroll_text?0:1;
            scroll_text?(document.getElementById("bt_Scroll").innerHTML = "Scroll Right"):(document.getElementById("bt_Scroll").innerHTML = "Scroll Left");
            websocket.send("8,"+scroll_text);
        }
        function Speed() {
            console.log("============> Speed");
            let speed_run = document.getElementById("speed_time").value;
            websocket.send("9,"+speed_run);
        }
        function toggle() {
            websocket.send('toggle');
        }
        function sendTimeON() {
            console.log("==========> Time ON");
            let timeOn = document.getElementById("timeON").value;
            websocket.send("6,"+timeOn);
        }
        function sendTimeOFF() {
            console.log("=========> Time OFF");
            let timeOFF = document.getElementById("timeOFF").value;
            websocket.send("7,"+timeOFF);
        }
        function hexToRgb(hex) {
            // Expand shorthand form (e.g. "03F") to full form (e.g. "0033FF")
            var shorthandRegex = /^#?([a-f\d])([a-f\d])([a-f\d])$/i;
            hex = hex.replace(shorthandRegex, function (m, r, g, b) {
                return r + r + g + g + b + b;
            });

            var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
            return result ? {
                r: parseInt(result[1], 16),
                g: parseInt(result[2], 16),
                b: parseInt(result[3], 16)
            } : null;
        }
    </script>
</body>
</html>
)rawliteral";
