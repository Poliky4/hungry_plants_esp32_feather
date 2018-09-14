#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

const char *ssid = "YourSSIDHere";
const char *password = "YourPSKHere";

bool hasWifi = false;

WebServer server(80);

void handleRoot()
{
  char temp[400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf(
    temp,
    400,
    "<html>\
      <head>\
        <meta http-equiv='refresh' content='5'/>\
        <title>ESP32 Demo</title>\
        <style>\
          body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
        </style>\
      </head>\
      <body>\
        <h1>Hello from ESP32!</h1>\
        <p>Uptime: %02d:%02d:%02d</p>\
        <img src=\"/test.svg\" />\
      </body>\
    </html>",
    hr,
    min % 60,
    sec % 60
  );
  server.send(200, "text/html", temp);
}

void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
}

void handleWifi() {
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to wifi!");
    server.send(
      200,
      "text/html", "<h1>You Connected to a new wifi successfully!</h1>,"
    );
  } else {
    Serial.println("wifi connection failed!");
    String inputSSID = server.arg("ssid");
    String inputPWD = server.arg("pwd");

    int ssidLength = inputSSID.length() + 1;
    char newSSID[ssidLength];
    inputSSID.toCharArray(newSSID, ssidLength);
    
    int pwdLength = inputPWD.length() + 1;
    char newPWD[pwdLength];
    inputPWD.toCharArray(newPWD, pwdLength);
  
    Serial.println("WIFI? " + inputSSID + " " + inputPWD);
    server.send(200, "text/html", inputSSID + " " + inputPWD);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(newSSID, newPWD);    
  }
}

void setupAP()
{
  Serial.println();
  Serial.print("Configuring access point...");
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", handleRoot);

  server.on("/wifi", handleWifi);

  server.begin();
  Serial.println("HTTP server started");
}

void setupWebserver()
{
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());

  if (MDNS.begin("esp32"))
  {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/test.svg", drawGraph);
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void setup(void)
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin();

  for (size_t i = 0; i < 3; i++)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.print("WIFI!");
      hasWifi = true;
    }
    delay(500);
  }

  if(hasWifi) {
    setupWebserver();
  } else {
    setupAP();
  }
}

void loop(void)
{
  server.handleClient();
}

void drawGraph()
{
  String out = "";
  char temp[100];
  out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";
  out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
  out += "<g stroke=\"black\">\n";
  int y = rand() % 130;
  for (int x = 10; x < 390; x += 10)
  {
    int y2 = rand() % 130;
    sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, 140 - y, x + 10, 140 - y2);
    out += temp;
    y = y2;
  }
  out += "</g>\n</svg>\n";

  server.send(200, "image/svg+xml", out);
}
