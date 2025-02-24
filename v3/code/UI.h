// Main UI
const char WEB_UI[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Valve Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <style>
    :root {
      --primary-color: #0288d1;
      --success-color: #4caf50;
      --danger-color: #f44336;
      --background-color: #121212;
      --card-background: #1a1a1a;
      --text-primary: #ffffff;
      --text-secondary: #888888;
    }

    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }

    body {
      font-family: 'Arial', sans-serif;
      text-align: center;
      margin: 0;
      padding: 0;
      background: var(--background-color);
      color: var(--text-primary);
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      justify-content: space-between;
    }

    h1 {
      font-size: clamp(1.5rem, 5vw, 2rem);
      color: var(--primary-color);
      text-shadow: 2px 2px 5px rgba(0, 0, 0, 0.3);
      padding: 1rem;
      margin: 0;
    }

    .container {
      margin: 1.5rem;
      padding: 1rem;
      flex-grow: 1;
      display: flex;
      flex-direction: column;
      justify-content: center;
    }

    .valve-container {
      position: relative;
      width: 100%;
      max-width: 400px;
      margin: 0 auto;
      padding: clamp(1rem, 3vw, 2rem);
      background: var(--card-background);
      border-radius: 15px;
      box-shadow: 0 4px 15px rgba(0, 0, 0, 0.3);
    }

    .pipe {
      width: 100%;
      height: clamp(15px, 3vw, 20px);
      background: #333;
      margin: 10px 0;
      border-radius: 10px;
      position: relative;
      overflow: hidden;
    }

    .water-flow {
      position: absolute;
      top: 0;
      left: -100%;
      width: 200%;
      height: 100%;
      background: linear-gradient(90deg,
        transparent 0%,
        var(--primary-color) 50%,
        transparent 100%
      );
      transition: transform 0.3s ease;
      transform: translateX(0);
    }

    .flowing .water-flow {
      animation: flowWater 1s linear infinite;
    }

    @keyframes flowWater {
      from { transform: translateX(-50%); }
      to { transform: translateX(0%); }
    }

    .valve-btn {
      width: clamp(60px, 15vw, 80px);
      height: clamp(60px, 15vw, 80px);
      border-radius: 50%;
      border: none;
      background: var(--danger-color);
      color: var(--text-primary);
      font-size: clamp(12px, 3vw, 14px);
      cursor: pointer;
      transition: all 0.3s ease;
      box-shadow: 0 4px 10px rgba(0, 0, 0, 0.3);
      display: flex;
      align-items: center;
      justify-content: center;
      margin: 1rem auto;
      padding: 0;
    }

    .valve-btn.on {
      background: var(--success-color);
    }

    .valve-btn:hover {
      transform: scale(1.1);
    }

    .valve-btn:active {
      transform: scale(0.95);
    }

    .valve-icon {
      font-size: clamp(20px, 5vw, 24px);
      margin-bottom: 5px;
    }

    .status-section {
      margin-top: 1rem;
      padding: 0.5rem;
    }

    .status-label {
      font-size: clamp(0.8rem, 2vw, 0.9rem);
      color: var(--text-secondary);
      margin-bottom: 0.5rem;
    }

    .status-value {
      font-size: clamp(1rem, 2.5vw, 1.2rem);
      font-weight: bold;
    }

    .status-on {
      color: var(--success-color);
    }

    .status-off {
      color: var(--danger-color);
    }

    .connection-status {
      position: fixed;
      top: 10px;
      right: 10px;
      padding: 5px 10px;
      border-radius: 5px;
      font-size: 0.8rem;
    }

    .connection-online {
      background-color: var(--success-color);
      color: white;
    }

    .connection-offline {
      background-color: var(--danger-color);
      color: white;
    }

    footer {
      padding: 1rem;
      font-size: clamp(0.8rem, 2vw, 0.9rem);
      color: var(--text-secondary);
      background: var(--card-background);
      margin-top: 2rem;
    }

    /* Responsive and touch device adjustments */
    @media screen and (max-height: 500px) and (orientation: landscape) {
      .container { padding: 0.5rem; }
      h1 { font-size: clamp(1.2rem, 4vw, 1.5rem); padding: 0.5rem; }
      .valve-container { padding: 1rem; }
      footer { padding: 0.5rem; margin-top: 1rem; }
    }

    @media (hover: none) {
      .valve-btn:hover { transform: none; }
    }

    @media screen and (min-width: 1200px) {
      .valve-container { max-width: 500px; padding: 2.5rem; }
      .pipe { height: 25px; }
    }
  </style>
</head>
<body>
  <div id="connectionStatus" class="connection-status connection-offline">Disconnected</div>

  <header>
    <h1>Valve Control</h1>
  </header>

  <div class="container">
    <div class="valve-container">
      <div class="pipe">
        <div class="water-flow" id="waterFlow"></div>
      </div>

      <button id="toggleButton" class="valve-btn" onclick="toggleValve()">
        <div>
          <span id="buttonText">...</span>
        </div>
      </button>

      <div class="pipe">
        <div class="water-flow" id="waterFlow2"></div>
      </div>

      <div class="status-section">
        <div class="status-label">Valve Status</div>
        <div class="status-value">
          <span id="valveStatus">...</span>
        </div>
      </div>
    </div>
  </div>

  <footer>
    Irrigation Control System
  </footer>

  <script>
    const toggleButton = document.getElementById('toggleButton');
    const buttonText = document.getElementById('buttonText');
    const valveStatus = document.getElementById('valveStatus');
    const pipes = document.querySelectorAll('.pipe');
    const connectionStatus = document.getElementById('connectionStatus');

    let socket;
    let isOpen = false;

    function connectWebSocket() {
      socket = new WebSocket(`ws://${window.location.hostname}:81`);

      socket.onopen = function(e) {
        connectionStatus.textContent = 'Connected';
        connectionStatus.className = 'connection-status connection-online';
        console.log('WebSocket connection established');
      };

      socket.onmessage = function(event) {
        const status = event.data;
        updateUI(status === 'ON');
      };

      socket.onclose = function(event) {
        connectionStatus.textContent = 'Disconnected';
        connectionStatus.className = 'connection-status connection-offline';
        console.log('WebSocket connection closed');
        
        // Attempt to reconnect after 5 seconds
        setTimeout(connectWebSocket, 5000);
      };

      socket.onerror = function(error) {
        console.error('WebSocket Error:', error);
      };
    }

    function toggleValve() {
      if (!socket || socket.readyState !== WebSocket.OPEN) {
        alert('WebSocket not connected. Please check your connection.');
        return;
      }

      const command = isOpen ? 'OFF' : 'ON';
      socket.send(command);
    }

    function updateUI(open) {
      isOpen = open;
      
      valveStatus.textContent = isOpen ? 'OPEN' : 'CLOSED';
      valveStatus.className = isOpen ? 'status-value status-on' : 'status-value status-off';

      toggleButton.className = `valve-btn ${isOpen ? 'on' : 'off'}`;
      buttonText.textContent = isOpen ? 'OPEN' : 'CLOSED';

      pipes.forEach(pipe => {
        pipe.className = `pipe ${isOpen ? 'flowing' : ''}`;
      });
    }

    // Initial WebSocket connection
    connectWebSocket();
  </script>
</body>
</html>
)rawliteral";

// Web Setup Page HTML
const char SETUP_UI[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Device Configuration</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body {
            font-family: 'Arial', sans-serif;
            max-width: 500px;
            margin: 0 auto;
            padding: 20px;
            background-color: #121212;
            color: #e0e0e0;
        }
        h1, h2 {
            text-align: center;
        }
        h1 {
            color: #ffb400;
        }
        input, select {
            width: 100%;
            padding: 10px;
            margin: 10px 0;
            box-sizing: border-box;
            border: 1px solid #333;
            background-color: #1e1e1e;
            color: #e0e0e0;
            border-radius: 5px;
        }
        input:focus, select:focus {
            outline: none;
            border-color: #ffb400;
            box-shadow: 0 0 5px #ffb400;
        }
        .section {
            border: 1px solid #333;
            border-radius: 10px;
            padding: 15px;
            margin-bottom: 15px;
            background-color: #1e1e1e;
        }
        .submit-btn {
            background-color: #ff5722;
            color: white;
            border: none;
            cursor: pointer;
            padding: 12px;
            font-size: 1.1em;
            border-radius: 5px;
            transition: transform 0.1s ease, background-color 0.3s ease;
        }
        .submit-btn:hover {
            background-color: #e64a19;
            transform: scale(1.05);
        }
        .submit-btn:active {
            transform: scale(0.95);
        }
        #adafruitFields {
            margin-top: 15px;
        }
        .relay-sensor-fields {
            margin-top: 15px;
        }
    </style>
</head>
<body>
    <h1>Device Configuration</h1>
    <form action="/save-config" method="POST">
        <div class="section">
            <h2>WiFi Configuration</h2>
            <input type="text" name="wifiSSID" placeholder="WiFi SSID" required>
            <input type="password" name="wifiPassword" placeholder="WiFi Password" required>
        </div>

        <div class="section">
            <h2>Device Name</h2>
            <input type="text" name="mdnsName" placeholder="mDNS Name (e.g., esp-device)" value="esp-device">
        </div>

        <div class="section">
            <h2>Adafruit IO</h2>
            <select name="useAdafruitIO">
                <option value="false">Disable Adafruit IO</option>
                <option value="true">Enable Adafruit IO</option>
            </select>
            <div id="adafruitFields" style="display:none;">
                <input type="text" name="ioUsername" placeholder="Adafruit IO Username">
                <input type="text" name="ioKey" placeholder="Adafruit IO Key">
                <input type="text" name="relayFeedName" placeholder="Relay Feed Name" value="relay">
                <input type="text" name="ipFeedName" placeholder="IP Feed Name" value="ip">
            </div>
        </div>

        <div class="section">
            <h2>Relays</h2>
            <div id="relayFields" class="relay-sensor-fields">
                <input type="number" name="relayCount" placeholder="Number of Relays" min="0" max="8" value="0">
                <div id="relayPins"></div>
            </div>
        </div>

        <div class="section">
            <h2>Sensors</h2>
            <div id="sensorFields" class="relay-sensor-fields">
                <input type="number" name="sensorCount" placeholder="Number of Sensors" min="0" max="8" value="0">
                <div id="sensorPins"></div>
            </div>
        </div>

        <input type="submit" value="Save Configuration" class="submit-btn">
    </form>

    <script>
        document.querySelector('select[name="useAdafruitIO"]').addEventListener('change', function() {
            var adafruitFields = document.getElementById('adafruitFields');
            adafruitFields.style.display = this.value === 'true' ? 'block' : 'none';
        });

        document.querySelector('input[name="relayCount"]').addEventListener('change', function() {
            var relayPins = document.getElementById('relayPins');
            relayPins.innerHTML = '';
            for (var i = 0; i < this.value; i++) {
                relayPins.innerHTML += '<input type="number" name="relayPin' + i + '" placeholder="Relay ' + (i + 1) + ' Pin">';
            }
        });

        document.querySelector('input[name="sensorCount"]').addEventListener('change', function() {
            var sensorPins = document.getElementById('sensorPins');
            sensorPins.innerHTML = '';
            for (var i = 0; i < this.value; i++) {
                sensorPins.innerHTML += '<input type="number" name="sensorPin' + i + '" placeholder="Sensor ' + (i + 1) + ' Pin">';
            }
        });
    </script>
</body>
</html>
)rawliteral";
