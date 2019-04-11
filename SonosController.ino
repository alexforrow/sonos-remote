#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

#define WIFI_SSID "XXX"
#define WIFI_PSK  "XXX"
#define SONOS_HOST "192.168.0.24"

#define SONOS_PAUSE "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:Pause xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\"><InstanceID>0</InstanceID></u:Pause></s:Body></s:Envelope>"
#define SONOS_PLAY  "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:Play xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\"><InstanceID>0</InstanceID><Speed>1</Speed></u:Play></s:Body></s:Envelope>"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PSK;

ESP8266WebServer server(80);

const int led = 13;

void setup(void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", []() {
    server.send(200, "text/html", "<p><a href=\"/play\">play</a></p><p><a href=\"/pause\">pause</a></p>");
  });

  server.on("/play", []() {
    String response = sonosCmd("Play", SONOS_PLAY);
    server.send(200, "text/xml", response);
  });

  server.on("/pause", []() {
    String response = sonosCmd("Pause", SONOS_PAUSE);
    server.send(200, "text/xml", response);
  });

  server.begin();
  Serial.println("HTTP server started");
}

String sonosCmd(String cmd, String payload) {
  HTTPClient http;
  char url[64];
  sprintf(url, "http://%s:1400/MediaRenderer/AVTransport/Control", SONOS_HOST);
  http.begin(url);
  http.addHeader("Content-Type", "text/xml");
  http.addHeader("Soapaction", "rn:schemas-upnp-org:service:AVTransport:1#" + cmd);
  int httpCode = http.POST(payload);
  String response = http.getString();
  
  Serial.println(httpCode);
  Serial.println(response);
  
  http.end();
  return response;
}

void loop(void) {
  server.handleClient();
}
