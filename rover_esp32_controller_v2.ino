
#include <WiFi.h>
#include <WebServer.h>

const char* WIFI_SSID = "Rao sahab";
const char* WIFI_PASS = "Vivek@2006";
const char* CAM_IP    = "10.149.174.21";   

WebServer server(80);
int distanceCm = 999;
String rxLine = "";
unsigned long lastCheck = 0;

const char PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Mars Rover</title>
<style>
body{font-family:sans-serif;background:#2b1a14;color:#ffd9b3;text-align:center;margin:0;padding:10px}
img{width:100%;max-width:480px;border:2px solid #c1440e;border-radius:8px}
button{width:80px;height:80px;margin:4px;font-size:22px;border-radius:12px;border:0;background:#c1440e;color:#fff}
button:active{background:#ff7b3d}
.row{display:flex;justify-content:center}
#d{font-size:22px;margin:8px}
</style></head><body>
<h2>MARS ROVER CONTROL</h2>
<img id="cam">
<div id="d">Obstacle: -- cm</div>
<div class="row"><button id="F">&#9650;</button></div>
<div class="row"><button id="L">&#9664;</button><button id="S">&#9632;</button><button id="R">&#9654;</button></div>
<div class="row"><button id="B">&#9660;</button></div>
<div>Speed <input type="range" min="1" max="9" value="6" id="sp"></div>
<div><button onclick="send('A')" style="width:120px;height:50px">AUTO</button>
<button onclick="send('M')" style="width:120px;height:50px">MANUAL</button></div>
<script>
document.getElementById('cam').src='http://CAMIP:81/stream';
let timer=null;
function send(c){fetch('/cmd?c='+c).catch(()=>{});}
['F','B','L','R','S'].forEach(k=>{
  const b=document.getElementById(k);
  const start=e=>{e.preventDefault();send(k);if(k!='S'){clearInterval(timer);timer=setInterval(()=>send(k),250);}};
  const end=e=>{e.preventDefault();clearInterval(timer);send('S');};
  b.addEventListener('pointerdown',start);
  b.addEventListener('pointerup',end);
  b.addEventListener('pointerleave',end);
});
document.getElementById('sp').oninput=e=>send(e.target.value);
setInterval(()=>fetch('/dist').then(r=>r.text()).then(t=>{
  document.getElementById('d').innerText='Obstacle: '+t+' cm';}).catch(()=>{}),500);
</script></body></html>
)rawliteral";

void handleRoot() {
  String p = FPSTR(PAGE);
  p.replace("CAMIP", CAM_IP);
  server.send(200, "text/html", p);
}

void handleCmd() {
  if (server.hasArg("c")) Serial2.print(server.arg("c")[0]);
  server.send(200, "text/plain", "ok");
}

void handleDist() { server.send(200, "text/plain", String(distanceCm)); }

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);  

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);          
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) { delay(400); Serial.print("."); }
  Serial.print("\nOpen: http://"); Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/cmd", handleCmd);
  server.on("/dist", handleDist);
  server.begin();
}

void loop() {

  if (millis() - lastCheck > 5000) {
    lastCheck = millis();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi lost, reconnecting...");
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_PASS);
    } else {
      Serial.print("WiFi OK, signal "); Serial.print(WiFi.RSSI());
      Serial.print(" dBm, IP "); Serial.println(WiFi.localIP());
    }
  }

  server.handleClient();

  while (Serial2.available()) {            
    char c = Serial2.read();
    if (c == '\n') {
      if (rxLine.startsWith("D:")) distanceCm = rxLine.substring(2).toInt();
      rxLine = "";
    } else if (c != '\r') rxLine += c;
  }
}
