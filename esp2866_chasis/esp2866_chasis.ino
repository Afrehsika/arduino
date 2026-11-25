#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// =====================================================
// Wi-Fi access point
// =====================================================
const char* ssid = "Robot_Car";
const char* password = "RobotCar123";

ESP8266WebServer server(80);

// =====================================================
// L298N pins — existing wiring preserved
// Left motor:  IN1=D1, IN2=D2, ENA=D7
// Right motor: IN3=D5, IN4=D6, ENB=D0
//
// Note: D0 (GPIO16) does not provide normal analogWrite()
// PWM on ESP8266. This version keeps your wiring and uses
// full-speed enable/disable. A speed slider needs a PWM-capable
// ENB pin or a different motor-control arrangement.
// =====================================================
#define IN1 D1
#define IN2 D2
#define ENA D7
#define IN3 D5
#define IN4 D6
#define ENB D0

// Stop the car if fresh movement commands stop arriving.
const unsigned long COMMAND_TIMEOUT_MS = 700;
unsigned long lastMovementCommand = 0;
bool isMoving = false;

// =====================================================
// Motor control
// =====================================================
void stopCar() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  digitalWrite(ENA, LOW);
  digitalWrite(ENB, LOW);
  isMoving = false;
}

void setMotorEnable(bool enabled) {
  digitalWrite(ENA, enabled ? HIGH : LOW);
  digitalWrite(ENB, enabled ? HIGH : LOW);
}

void markMovement() {
  lastMovementCommand = millis();
  isMoving = true;
  setMotorEnable(true);
}

void forward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  markMovement();
}

void backward() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  markMovement();
}

void left() {
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  markMovement();
}

void right() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
  markMovement();
}

// =====================================================
// Browser controller
// =====================================================
const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover">
<meta name="theme-color" content="#0b1220">
<title>Robot Control</title>
<style>
  :root {
    color-scheme: dark;
    --bg: #0b1220;
    --panel: #131e30;
    --panel2: #19273c;
    --line: #2a3a52;
    --text: #edf4ff;
    --muted: #93a4bd;
    --blue: #3b82f6;
    --green: #34d399;
    --red: #ef4444;
  }
  * { box-sizing: border-box; }
  body {
    margin: 0; min-height: 100vh; padding: 20px;
    font-family: system-ui, -apple-system, Segoe UI, Roboto, Arial, sans-serif;
    color: var(--text);
    background: radial-gradient(ellipse at top, #172842 0%, var(--bg) 58%);
  }
  .app { width: min(100%, 920px); margin: 0 auto; }
  header {
    display: flex; align-items: center; justify-content: space-between;
    gap: 14px; margin-bottom: 20px;
  }
  .brand { display:flex; align-items:center; gap:12px; }
  .logo {
    width: 46px; height:46px; display:grid; place-items:center;
    border-radius: 14px; background:#1d3b65; font-size:24px;
    border:1px solid #31527d;
  }
  h1 { margin:0; font-size: clamp(22px, 4vw, 30px); letter-spacing:-.5px; }
  .subtitle { margin:4px 0 0; color:var(--muted); font-size:13px; }
  .badge {
    display:flex; align-items:center; gap:8px; white-space:nowrap;
    border:1px solid var(--line); background:#101b2b;
    border-radius:999px; padding:9px 12px; color:var(--muted); font-size:12px;
  }
  .dot { width:8px;height:8px;border-radius:50%;background:var(--green); box-shadow:0 0 12px #34d39970; }
  .layout { display:grid; grid-template-columns:1.05fr .95fr; gap:16px; }
  .card {
    border:1px solid var(--line); border-radius:22px;
    background:linear-gradient(160deg, #172439, #101a2a);
    padding:20px; box-shadow:0 16px 40px #00000020;
  }
  .card-head { display:flex; align-items:center; justify-content:space-between; gap:10px; margin-bottom:18px; }
  h2 { font-size:15px; margin:0; letter-spacing:.2px; }
  .eyebrow { color:var(--muted); font-size:11px; text-transform:uppercase; letter-spacing:1.2px; }
  .drive-grid {
    display:grid; grid-template-columns:repeat(3, minmax(0, 1fr));
    gap:10px; width:min(100%, 330px); margin:10px auto 16px;
  }
  button {
    font:inherit; border:0; color:var(--text); cursor:pointer;
    -webkit-tap-highlight-color:transparent; user-select:none; touch-action:none;
  }
  .drive {
    aspect-ratio:1/1; min-width:0; border-radius:20px;
    background:linear-gradient(145deg,#263d5d,#1a2a41);
    border:1px solid #3a5272; font-size:clamp(25px, 6vw, 34px);
    box-shadow:inset 0 1px 0 #ffffff10, 0 7px 15px #00000020;
    transition:transform .08s, background .08s;
  }
  .drive:active, .drive.active { transform:scale(.96); background:var(--blue); }
  .empty { visibility:hidden; }
  .stop {
    aspect-ratio:1/1; border-radius:20px; background:linear-gradient(145deg,#f04444,#b91c1c);
    font-size:14px; font-weight:900; letter-spacing:1px;
    box-shadow:0 7px 18px #ef444430; border:1px solid #ff777760;
  }
  .stop:active { transform:scale(.96); background:#ff5555; }
  .hint { text-align:center; color:var(--muted); font-size:12px; margin:0; line-height:1.6; }
  .state {
    display:flex; align-items:center; gap:10px; padding:12px 14px;
    border-radius:14px; background:#0c1727; border:1px solid var(--line);
    margin-bottom:16px; min-height:50px;
  }
  .state-icon { font-size:18px; }
  #stateText { font-size:13px; font-weight:650; }
  #stateSub { font-size:11px; color:var(--muted); margin-top:3px; }
  .telemetry { display:grid; grid-template-columns:1fr 1fr; gap:10px; }
  .metric { background:#0c1727; border:1px solid var(--line); border-radius:14px; padding:13px; }
  .metric-label { color:var(--muted); font-size:11px; margin-bottom:7px; }
  .metric-value { font-size:15px; font-weight:700; }
  .camera-placeholder {
    margin-top:12px; min-height:105px; display:grid; place-items:center; text-align:center;
    padding:16px; border:1px dashed #405571; border-radius:16px; color:var(--muted);
    background:repeating-linear-gradient(135deg,#101b2b,#101b2b 10px,#121f31 10px,#121f31 20px);
    font-size:12px; line-height:1.6;
  }
  .camera-placeholder strong { display:block; color:var(--text); font-size:13px; margin-bottom:4px; }
  .footer { text-align:center; color:#6f819a; font-size:11px; margin:18px 0 0; }
  @media (max-width:680px) {
    body { padding:15px; }
    header { align-items:flex-start; }
    .badge { padding:8px 10px; font-size:11px; }
    .layout { grid-template-columns:1fr; }
    .card { padding:17px; }
    .drive-grid { width:min(100%, 310px); gap:9px; }
  }
</style>
</head>
<body>
<div class="app">
  <header>
    <div class="brand">
      <div class="logo">🤖</div>
      <div>
        <h1>Robot Control</h1>
        <p class="subtitle">Wi-Fi rover · Control station</p>
      </div>
    </div>
    <div class="badge"><span class="dot" id="connectionDot"></span><span id="connectionText">Connecting</span></div>
  </header>

  <main class="layout">
    <section class="card">
      <div class="card-head">
        <h2>Movement</h2>
        <span class="eyebrow">Press & hold</span>
      </div>
      <div class="drive-grid">
        <button class="empty" tabindex="-1" aria-hidden="true"></button>
        <button class="drive" data-command="forward" aria-label="Move forward">▲</button>
        <button class="empty" tabindex="-1" aria-hidden="true"></button>
        <button class="drive" data-command="left" aria-label="Turn left">◀</button>
        <button class="stop" id="stopButton" aria-label="Emergency stop">STOP</button>
        <button class="drive" data-command="right" aria-label="Turn right">▶</button>
        <button class="empty" tabindex="-1" aria-hidden="true"></button>
        <button class="drive" data-command="backward" aria-label="Move backward">▼</button>
        <button class="empty" tabindex="-1" aria-hidden="true"></button>
      </div>
      <p class="hint">Hold a direction to move. Release to stop.<br>Keyboard: ↑ ↓ ← → · Space = stop</p>
    </section>

    <section class="card">
      <div class="card-head">
        <h2>Robot status</h2>
        <span class="eyebrow">Live link</span>
      </div>
      <div class="state">
        <span class="state-icon" id="stateIcon">◉</span>
        <div>
          <div id="stateText">Ready to connect</div>
          <div id="stateSub">Join the Robot_Car Wi-Fi network</div>
        </div>
      </div>
      <div class="telemetry">
        <div class="metric"><div class="metric-label">CURRENT COMMAND</div><div class="metric-value" id="commandValue">STOP</div></div>
        <div class="metric"><div class="metric-label">CONTROL MODE</div><div class="metric-value" id="modeValue">Touch / keyboard</div></div>
      </div>
      <div class="camera-placeholder">
        <div><strong>📷 Camera expansion slot</strong>OV7670 video panel can be added here later.<br>Camera feed is not active yet.</div>
      </div>
    </section>
  </main>
  <p class="footer">ESP8266 control interface · If the connection drops, the robot's command timeout stops the motors.</p>
</div>

<script>
  const stateText = document.getElementById('stateText');
  const stateSub = document.getElementById('stateSub');
  const connectionText = document.getElementById('connectionText');
  const connectionDot = document.getElementById('connectionDot');
  const commandValue = document.getElementById('commandValue');
  const stateIcon = document.getElementById('stateIcon');

  let activeCommand = null;
  let commandTimer = null;
  let requestInProgress = false;
  let pendingCommand = null;
  let online = false;

  function showConnection(ok) {
    online = ok;
    connectionText.textContent = ok ? 'Robot online' : 'Connection lost';
    connectionDot.style.background = ok ? '#34d399' : '#ef4444';
    connectionDot.style.boxShadow = ok ? '0 0 12px #34d39970' : '0 0 12px #ef444470';
    if (!ok) {
      stateText.textContent = 'Connection unavailable';
      stateSub.textContent = 'Check that you are connected to Robot_Car';
    } else if (!activeCommand) {
      stateText.textContent = 'Connected and ready';
      stateSub.textContent = 'Hold a direction to drive';
    }
  }

  async function sendCommand(command) {
    pendingCommand = command;
    if (requestInProgress) return;
    requestInProgress = true;
    try {
      while (pendingCommand !== null) {
        const next = pendingCommand;
        pendingCommand = null;
        const response = await fetch('/' + next, {
          method: 'GET',
          cache: 'no-store',
          signal: AbortSignal.timeout(900)
        });
        if (!response.ok) throw new Error('HTTP ' + response.status);
        showConnection(true);
      }
    } catch (error) {
      showConnection(false);
    } finally {
      requestInProgress = false;
    }
  }

  function startMove(event, command) {
    event.preventDefault();
    if (activeCommand === command) return;
    activeCommand = command;
    if (event.currentTarget.setPointerCapture) {
      try { event.currentTarget.setPointerCapture(event.pointerId); } catch (_) {}
    }
    document.querySelectorAll('.drive').forEach(b => b.classList.toggle('active', b.dataset.command === command));
    commandValue.textContent = command.toUpperCase();
    stateIcon.textContent = '↗';
    stateText.textContent = 'Moving ' + command;
    stateSub.textContent = 'Release the control to stop';
    sendCommand(command);
    clearInterval(commandTimer);
    commandTimer = setInterval(() => {
      if (activeCommand === command) sendCommand(command);
    }, 200);
  }

  function releaseMove() {
    if (!activeCommand) return;
    activeCommand = null;
    clearInterval(commandTimer);
    commandTimer = null;
    document.querySelectorAll('.drive').forEach(b => b.classList.remove('active'));
    commandValue.textContent = 'STOP';
    stateIcon.textContent = '■';
    stateText.textContent = 'Stopping';
    stateSub.textContent = 'Motor stop command sent';
    sendCommand('stop');
  }

  function emergencyStop() {
    activeCommand = null;
    clearInterval(commandTimer);
    commandTimer = null;
    document.querySelectorAll('.drive').forEach(b => b.classList.remove('active'));
    commandValue.textContent = 'STOP';
    stateIcon.textContent = '■';
    stateText.textContent = 'Emergency stop';
    stateSub.textContent = 'Motor stop command sent';
    sendCommand('stop');
  }

  document.querySelectorAll('.drive').forEach(button => {
    button.addEventListener('pointerdown', event => startMove(event, button.dataset.command));
    button.addEventListener('pointerup', releaseMove);
    button.addEventListener('pointercancel', releaseMove);
    button.addEventListener('lostpointercapture', releaseMove);
    button.addEventListener('contextmenu', event => event.preventDefault());
  });
  document.getElementById('stopButton').addEventListener('click', emergencyStop);

  const keyCommands = {
    ArrowUp: 'forward', ArrowDown: 'backward',
    ArrowLeft: 'left', ArrowRight: 'right'
  };
  document.addEventListener('keydown', event => {
    if (event.code === 'Space') {
      event.preventDefault();
      emergencyStop();
      return;
    }
    const command = keyCommands[event.key];
    if (command && !event.repeat) {
      event.preventDefault();
      const button = document.querySelector('[data-command="' + command + '"]');
      if (button) startMove({preventDefault(){}, currentTarget:button, pointerId:0}, command);
    }
  });
  document.addEventListener('keyup', event => {
    if (keyCommands[event.key]) {
      event.preventDefault();
      releaseMove();
    }
  });

  document.addEventListener('visibilitychange', () => {
    if (document.hidden) emergencyStop();
  });
  window.addEventListener('blur', emergencyStop);
  window.addEventListener('pagehide', emergencyStop);

  async function checkConnection() {
    try {
      const response = await fetch('/health', {cache:'no-store', signal:AbortSignal.timeout(1000)});
      showConnection(response.ok);
    } catch (_) {
      showConnection(false);
    }
  }
  setInterval(checkConnection, 2500);
  checkConnection();
</script>
</body>
</html>
)rawliteral";

// =====================================================
// Setup
// =====================================================
void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  stopCar();

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  Serial.println();
  Serial.println(F("Robot car controller started."));
  Serial.print(F("Wi-Fi name: "));
  Serial.println(ssid);
  Serial.print(F("IP address: "));
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html", webpage);
  });

  server.on("/health", HTTP_GET, []() {
    server.send(200, "text/plain", "OK");
  });

  server.on("/forward", HTTP_GET, []() {
    forward();
    server.send(200, "text/plain", "Forward");
  });

  server.on("/backward", HTTP_GET, []() {
    backward();
    server.send(200, "text/plain", "Backward");
  });

  server.on("/left", HTTP_GET, []() {
    left();
    server.send(200, "text/plain", "Left");
  });

  server.on("/right", HTTP_GET, []() {
    right();
    server.send(200, "text/plain", "Right");
  });

  server.on("/stop", HTTP_GET, []() {
    stopCar();
    server.send(200, "text/plain", "Stopped");
  });

  server.onNotFound([]() {
    stopCar();
    server.send(404, "text/plain", "Not found");
  });

  server.begin();
  Serial.println(F("Web server ready."));
}

void loop() {
  server.handleClient();

  // Safety timeout: if movement refreshes stop arriving, disable motors.
  if (isMoving && millis() - lastMovementCommand > COMMAND_TIMEOUT_MS) {
    stopCar();
  }
}
