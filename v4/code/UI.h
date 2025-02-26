// Main UI
const char WEB_UI[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Relay Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <style>
    :root {
      --primary-color: #0288d1;
      --success-color: #4caf50;
      --danger-color: #f44336;
      --background-color: #121212;
      --card-background: #1e1e1e;
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

    .relay-container {
      position: relative;
      width: 100%;
      max-width: 600px;
      margin: 0 auto;
      padding: clamp(1rem, 3vw, 2rem);
      background: var(--card-background);
      border-radius: 15px;
      box-shadow: 0 4px 15px rgba(0, 0, 0, 0.3);
    }

    .relay-section {
      margin-bottom: 1.5rem;
      padding: 1rem;
      background: #2a2a2a;
      border-radius: 10px;
      box-shadow: 0 2px 10px rgba(0, 0, 0, 0.2);
    }

    .relay-section:last-child {
      margin-bottom: 0;
    }

    .relay-btn {
      width: 100%;
      padding: 12px;
      border-radius: 8px;
      border: none;
      background: var(--danger-color);
      color: var(--text-primary);
      font-size: clamp(14px, 3vw, 16px);
      cursor: pointer;
      transition: all 0.3s ease;
      box-shadow: 0 4px 10px rgba(0, 0, 0, 0.3);
      display: flex;
      align-items: center;
      justify-content: center;
      margin: 0.5rem 0;
    }

    .relay-btn.on {
      background: var(--success-color);
    }

    .relay-btn:hover {
      transform: scale(1.02);
    }

    .relay-btn:active {
      transform: scale(0.98);
    }

    .status-section {
      margin-top: 0.5rem;
      padding: 0.5rem;
      text-align: center;
    }

    .status-label {
      font-size: clamp(0.8rem, 2vw, 0.9rem);
      color: var(--text-secondary);
      margin-bottom: 0.25rem;
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

    .setup-section {
      margin-top: 1rem;
      padding: 1rem;
    }

    .setup-btn {
      background-color: #ff5722;
      color: white;
      border: none;
      cursor: pointer;
      padding: 12px;
      font-size: 1.1em;
      border-radius: 5px;
      transition: transform 0.1s ease, background-color 0.3s ease;
    }

    .setup-btn:hover {
      background-color: #e64a19;
      transform: scale(1.05);
    }

    .setup-btn:active {
      transform: scale(0.95);
    }

    /* Sensor Configuration Styles */
    .sensor-section {
      margin-top: 1.5rem;
      padding: 1rem;
      background: #2a2a2a;
      border-radius: 10px;
      box-shadow: 0 2px 10px rgba(0, 0, 0, 0.2);
    }

    .sensor-container {
      margin-top: 1.5rem;
      padding: 1rem;
      background: var(--card-background);
      border-radius: 15px;
      box-shadow: 0 4px 15px rgba(0, 0, 0, 0.3);
    }

    .sensor-card {
      background: #2a2a2a;
      border-radius: 10px;
      padding: 1rem;
      margin-bottom: 1rem;
      box-shadow: 0 2px 10px rgba(0, 0, 0, 0.2);
    }

    .sensor-value {
      font-size: 1.2rem;
      color: var(--primary-color);
      margin: 0.5rem 0;
    }

    .sensor-field {
      display: flex;
      gap: 10px;
      margin-bottom: 1rem;
      align-items: center;
    }

    .sensor-field select,
    .sensor-field input {
      padding: 8px;
      border-radius: 4px;
      border: 1px solid #444;
      background: #333;
      color: var(--text-primary);
    }

    .sensor-field select {
      flex: 2;
    }

    .sensor-field input {
      flex: 1;
      width: 80px;
    }

    .add-sensor-btn,
    .remove-sensor-btn {
      padding: 5px 10px;
      border-radius: 4px;
      border: none;
      cursor: pointer;
      transition: all 0.2s ease;
    }

    .add-sensor-btn {
      background: var(--primary-color);
      color: white;
    }

    .remove-sensor-btn {
      background: var(--danger-color);
      color: white;
    }

    .sensor-readings {
      margin-top: 1rem;
      padding: 1rem;
      background: #2a2a2a;
      border-radius: 10px;
    }

    .sensor-reading {
      display: flex;
      justify-content: space-between;
      margin-bottom: 0.5rem;
      padding: 0.5rem;
      background: #333;
      border-radius: 4px;
    }

    .sensor-value {
      font-weight: bold;
      color: var(--primary-color);
    }

    /* Responsive and touch device adjustments */
    @media screen and (max-height: 500px) and (orientation: landscape) {
      .container { padding: 0.5rem; }
      h1 { font-size: clamp(1.2rem, 4vw, 1.5rem); padding: 0.5rem; }
      .relay-container { padding: 1rem; }
      footer { padding: 0.5rem; margin-top: 1rem; }
    }

    @media (hover: none) {
      .relay-btn:hover { transform: none; }
    }

    @media screen and (min-width: 1200px) {
      .relay-container { max-width: 700px; padding: 2.5rem; }
    }
  </style>
</head>
<body>
  <div id="connectionStatus" class="connection-status connection-offline">Offline</div>
  <h1>Relay Control</h1>
  
  <div class="container">
    <div class="relay-container" id="relayContainer">
      <h2>Relays</h2>
      <div id="relayButtons"></div>
    </div>
    
    <div class="sensor-container" id="sensorContainer">
      <h2>Sensors</h2>
      <div id="sensorReadings"></div>
    </div>
  </div>

  <footer>
    <p>ESP8266 Relay Controller</p>
  </footer>

  <script>
    let socket = null;
    const relayStates = [];

    function connectWebSocket() {
      // Connect to WebSocket on port 81 (ESP8266 WebSocket default port)
      socket = new WebSocket('ws://' + window.location.hostname + ':81');
      
      socket.onopen = () => {
        document.getElementById('connectionStatus').className = 'connection-status connection-online';
        document.getElementById('connectionStatus').textContent = 'Online';
        console.log('WebSocket connected');
      };
      
      socket.onclose = () => {
        document.getElementById('connectionStatus').className = 'connection-status connection-offline';
        document.getElementById('connectionStatus').textContent = 'Offline';
        console.log('WebSocket disconnected, retrying...');
        setTimeout(connectWebSocket, 2000);
      };

      socket.onerror = (error) => {
        console.error('WebSocket error:', error);
      };
      
      socket.onmessage = (event) => {
        try {
          const data = JSON.parse(event.data);
          console.log('Received:', data);
          
          if (data.type === 'relay') {
            updateRelayState(data.index, data.state);
          } else if (data.type === 'sensor') {
            updateSensorReading(data);
          }
        } catch (e) {
          console.error('Error processing message:', e);
        }
      };
    }

    function updateRelayState(index, state) {
      relayStates[index] = state;
      const button = document.getElementById(`relay-${index}`);
      if (button) {
        button.className = `relay-btn ${state ? 'on' : ''}`;
        button.textContent = `Relay ${index + 1}: ${state ? 'ON' : 'OFF'}`;
      }
    }

    function toggleRelay(index) {
      if (!socket || socket.readyState !== WebSocket.OPEN) {
        alert('WebSocket not connected. Please check your connection.');
        return;
      }

      const message = {
        type: 'relay',
        index: index,
        state: !relayStates[index]
      };

      socket.send(JSON.stringify(message));
    }

    function createRelayButtons() {
      const container = document.getElementById('relayButtons');
      container.innerHTML = '';

      for (let i = 0; i < 4; i++) {
        const button = document.createElement('button');
        button.id = `relay-${i}`;
        button.className = 'relay-btn';
        button.textContent = `Relay ${i + 1}: OFF`;
        button.onclick = () => toggleRelay(i);
        container.appendChild(button);
        relayStates[i] = false;
      }
    }

    function updateSensorReading(data) {
      const container = document.getElementById('sensorReadings');
      let sensorCard = document.getElementById(`sensor-${data.index}`);
      
      if (!sensorCard) {
        sensorCard = document.createElement('div');
        sensorCard.id = `sensor-${data.index}`;
        sensorCard.className = 'sensor-card';
        container.appendChild(sensorCard);
      }

      let html = `<h3>Sensor ${data.index + 1}</h3>`;
      
      if (data.temperature !== undefined) {
        html += `<div class="sensor-value">Temperature: ${data.temperature.toFixed(1)}°C</div>`;
      }
      if (data.humidity !== undefined) {
        html += `<div class="sensor-value">Humidity: ${data.humidity.toFixed(1)}%</div>`;
      }
      if (data.moisture !== undefined) {
        html += `<div class="sensor-value">Moisture: ${data.moisture.toFixed(1)}%</div>`;
      }
      if (data.light !== undefined) {
        html += `<div class="sensor-value">Light: ${data.light.toFixed(1)}</div>`;
      }
      if (data.analog !== undefined) {
        html += `<div class="sensor-value">Analog: ${data.analog.toFixed(1)}</div>`;
      }

      sensorCard.innerHTML = html;
    }

    window.onload = () => {
      createRelayButtons();
      connectWebSocket();
    };
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
        .relay-pin-field {
            margin-bottom: 10px;
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
                <input type="number" name="relayCount" id="relayCount" placeholder="Number of Relays (0-4)" min="0" max="4" value="0" onchange="updateRelayPins()">
                <div id="relayPins"></div>
            </div>
        </div>

        <div class="section">
            <h2>Sensors</h2>
            <div id="sensorFields" class="relay-sensor-fields">
                <select id="sensorType" onchange="addSensorField()">
                    <option value="">Add Sensor</option>
                    <option value="1">DHT11/22 (Temp/Humid)</option>
                    <option value="2">Soil Moisture</option>
                    <option value="3">Water Level</option>
                    <option value="4">LDR</option>
                    <option value="5">LM35</option>
                </select>
                <div id="sensorConfigs"></div>
            </div>
        </div>

        <input type="submit" value="Save Configuration" class="submit-btn">
    </form>

    <script>
        let sensorCounts = { dht: 0, soil: 0, water: 0, ldr: 0, lm35: 0 };

        function addSensorField() {
            const type = document.getElementById('sensorType').value;
            if (!type) return;

            const container = document.getElementById('sensorConfigs');
            const index = Object.values(sensorCounts).reduce((a,b) => a+b);

            let html = `<div class="sensor-config">
                <input type="hidden" name="sensor${index}_type" value="${type}">
                <label>Sensor ${index + 1}:</label>`;

            if (type === "1") {
                html += `<select name="sensor${index}_dhtType">
                    <option value="11">DHT11</option>
                    <option value="22">DHT22</option>
                </select>`;
            }

            html += `<input type="number" name="sensor${index}_pin" placeholder="GPIO Pin" required>
                <button type="button" onclick="this.parentElement.remove()">Remove</button>
            </div>`;

            container.insertAdjacentHTML('beforeend', html);
        }

        function updateRelayPins() {
            const relayCount = document.getElementById('relayCount').value;
            const relayPinsContainer = document.getElementById('relayPins');
            relayPinsContainer.innerHTML = '';

            for (let i = 0; i < relayCount; i++) {
                const pinField = document.createElement('div');
                pinField.className = 'relay-pin-field';
                pinField.innerHTML = `
                    <label>Relay ${i + 1} Pin:</label>
                    <input type="number" name="relayPin${i}" placeholder="GPIO Pin" required>
                `;
                relayPinsContainer.appendChild(pinField);
            }
        }
    </script>
</body>
</html>
)rawliteral";
