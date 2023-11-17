#include <WiFi.h>
#include <WebServer.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

const char* ssid = "TP-Link_0246";
const char* password = "13280222";

TinyGPSPlus gps;
HardwareSerial SerialGPS(1);

WebServer server(80);

const char* googleMapsApiKey = "AIzaSyB6jDWLc-kmcY5B7waRqvAI2blziUhfw-A"; // Reemplaza con tu Google Maps API Key

void setup() {
  Serial.begin(115200);
  SerialGPS.begin(9600, SERIAL_8N1, 16, 17);

  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi..");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Conectado a WiFi, dirección IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", getWebPage());
  });

  server.on("/location", HTTP_GET, []() {
    server.send(200, "application/json", createJsonResponse());
  });

  server.begin();
}

void loop() {
  server.handleClient();
  displayGPSData();
}

String createJsonResponse() {
  String json = "{";
  if (gps.location.isValid()) {
    json += "\"latitude\": " + String(gps.location.lat(), 6) + ",";
    json += "\"longitude\": " + String(gps.location.lng(), 6);
  } else {
    json += "\"error\": \"No GPS signal\"";
  }
  json += "}";
  return json;
}

String getWebPage() {
  String html = "<html><head>";
  html += "<script src='https://maps.googleapis.com/maps/api/js?key=";
  html += googleMapsApiKey;
  html += "&callback=initMap' async defer></script>";
  html += "<style>";
  html += "html, body { height: 100%; margin: 0; padding: 0; }";
  html += "#map { height: 70%; width: 100%; }";
  html += "#coords { margin-top: 10px; }";
  html += "</style>";
  html += "<script type='text/javascript'>";
  html += "var map; var marker;";
  html += "function initMap() {";
  html += "map = new google.maps.Map(document.getElementById('map'), {";
  html += "center: {lat: -34.397, lng: 150.644}, zoom: 8});";
  html += "marker = new google.maps.Marker({";
  html += "position: map.getCenter(), map: map});";
  html += "updateLocation();";
  html += "setInterval(updateLocation, 5000);";
  html += "}";
  html += "function updateLocation() {";
  html += "var xhr = new XMLHttpRequest();";
  html += "xhr.onreadystatechange = function() {";
  html += "if (this.readyState == 4 && this.status == 200) {";
  html += "var data = JSON.parse(this.responseText);";
  html += "if(data.latitude && data.longitude) {";
  html += "var newPos = new google.maps.LatLng(data.latitude, data.longitude);";
  html += "map.setCenter(newPos);";
  html += "marker.setPosition(newPos);";
  html += "document.getElementById('coords').innerText = 'Latitud: ' + data.latitude + ', Longitud: ' + data.longitude;";
  html += "}";
  html += "}";
  html += "};";
  html += "xhr.open('GET', '/location', true);";
  html += "xhr.send();";
  html += "}";
  html += "</script>";
  html += "</head>";
  html += "<body>";
  html += "<div id='map'></div>";
  html += "<button onclick='updateLocation()'>Actualizar Posición</button>";
  html += "<div id='coords'></div>";
  html += "</body></html>";
  return html;
}

void displayGPSData() {
  while (SerialGPS.available() > 0) {
    if (gps.encode(SerialGPS.read())) {
      if (gps.location.isValid()) {
        Serial.print("Latitud: ");
        Serial.println(gps.location.lat(), 6);
        Serial.print("Longitud: ");
        Serial.println(gps.location.lng(), 6);
      } else {
        Serial.println("Buscando señal del GPS...");
      }
    }
  }
}