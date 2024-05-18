#include <WiFi.h>
#include <Preferences.h>

#include <WebServer.h>

Preferences preferences;

String wifi_ssid;
String wifi_password;
String wifi_hostname = "thermostat-relay-1";

const int http_server_port = 80;

WebServer server(http_server_port);

// set to true to initialize or reset credentials through serial line
const int force_save_prefs = true;

const int switch_pin = GPIO_NUM_32;


bool readLineSerial(char *buf, size_t length)
{
  size_t read_sz = 0;
  bool got_input = (Serial.available() > 0);

  if (got_input)
  {
    read_sz = Serial.readBytesUntil('\n', buf, length);
    if (read_sz > 0 && buf[read_sz - 1] == '\r')
      read_sz--;
    buf[read_sz] = '\0';
  }
  return got_input;
}


String readStringSerial(const char *prompt)
{
  char buf[255];
  Serial.printf(prompt);
  while (!readLineSerial(buf, sizeof(buf)))
    ;
  return String(buf);
}


void savePreferences()
{
  wifi_ssid = readStringSerial("wifi ssid: ");
  wifi_password = readStringSerial("wifi password: ");

  preferences.begin("wifi", false); // read-write
  preferences.putString("ssid", wifi_ssid);
  preferences.putString("password", wifi_password);

  preferences.end();
}


void loadPreferences()
{
  preferences.begin("wifi", true); // read-only

  wifi_ssid = preferences.getString("ssid", "");
  wifi_password = preferences.getString("password", "");

  preferences.end();
}


void initWiFi()
{
  WiFi.mode(WIFI_STA);

  WiFi.setHostname(wifi_hostname.c_str()); // define hostname
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());

  Serial.print("Connecting to WiFi ..");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(1000);
  }

  Serial.println("wifi ip: " + WiFi.localIP());
  Serial.println("wifi mac: " + WiFi.macAddress());
  Serial.printf("wifi rssi: %d\n", WiFi.RSSI());
  Serial.printf("wifi channel: %d\n", WiFi.channel());
}


void handleIndexPage()
{
  String enabled = server.arg("enabled");
  Serial.println("enabled state " + enabled);
  String response = R"==(
      <html>
      <head></head>
      <script>
          function sendSwitch(enabled) {
              fetch("/api/v1/switch", {
                  method: 'POST',
                  mode: 'no-cors',
                  headers: {
                      'Content-Type': 'application/x-www-form-urlencoded'
                  },
                  body: new URLSearchParams({
                      'enabled': enabled
                  })
              });
          }
      </script>
      <body>
      <button onclick="sendSwitch(1)">Turn on</button>
      <button onclick="sendSwitch(0)">Turn off</button>
      </body>
      </html>
    )==";
  server.send(200, "text/html", response);
}


void setSwitch(bool enabled)
{
  digitalWrite(switch_pin, enabled ? HIGH : LOW);
}


void handleSwitchRequest()
{
  bool enabled = (server.arg("enabled") == "1");
  Serial.printf("Enabled: %d\n", enabled);
  setSwitch(enabled);
  server.send(200, "text/plain", "ok");
}


void initWebServer()
{
  server.on("/", HTTP_GET, handleIndexPage);
  server.on("/api/v1/switch", HTTP_POST, handleSwitchRequest);
  server.begin();
}


void setup()
{
  Serial.begin(115200);

  pinMode(switch_pin, OUTPUT);

  if (force_save_prefs)
    savePreferences();

  loadPreferences();
  initWiFi();
  initWebServer();
}


void loop()
{
  server.handleClient();
  delay(2);
}
