#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>


// ================= WIFI SETTINGS =================

const char* ssid = "Asustem";
const char* password = "12345678";


// ================= ARKESEL SETTINGS =================

String apiKey = "UXJnRnpWSmt5dkNkSVBWeHRod0k";

String phoneNumber = "233550545381";

String senderID = "ESP8266";


// ================= WEB SERVER =================

ESP8266WebServer server(80);


// ================= LED =================

// External LED connected to D1 (GPIO5)
#define LED_PIN D1

bool ledState = false;



// ================= SEND SMS USING ARKESEL =================

void sendSMS(String message)
{

  if (WiFi.status() == WL_CONNECTED)
  {

    WiFiClientSecure client;

    // Ignore SSL certificate verification
    client.setInsecure();


    HTTPClient http;


    http.begin(client, "https://sms.arkesel.com/api/v2/sms/send");


    http.addHeader("Content-Type", "application/json");

    http.addHeader("api-key", apiKey);



    String json = "{";

    json += "\"sender\":\"" + senderID + "\",";
    json += "\"message\":\"" + message + "\",";
    json += "\"recipients\":[\"" + phoneNumber + "\"]";

    json += "}";


    Serial.println("Sending SMS...");
    Serial.println(json);



    int response = http.POST(json);



    Serial.print("Arkesel Response: ");
    Serial.println(response);


    String result = http.getString();

    Serial.println(result);



    http.end();

  }

  else
  {
    Serial.println("WiFi not connected");
  }

}



// ================= WEB PAGE =================

void handleRoot()
{

  String html = "<!DOCTYPE html>";
  html += "<html>";
  html += "<head>";
  html += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  html += "<title>ESP8266 Control</title>";
  html += "</head>";

  html += "<body style='text-align:center;font-family:Arial;'>";


  html += "<h1>ESP8266 LED CONTROL</h1>";


  if (ledState)
  {
    html += "<h2>LED STATUS: ON</h2>";
  }
  else
  {
    html += "<h2>LED STATUS: OFF</h2>";
  }



  html += "<a href='/ON'>";
  html += "<button style='font-size:30px;padding:15px;'>ON</button>";
  html += "</a>";


  html += "<br><br>";


  html += "<a href='/OFF'>";
  html += "<button style='font-size:30px;padding:15px;'>OFF</button>";
  html += "</a>";



  html += "</body>";
  html += "</html>";



  server.send(200, "text/html", html);

}



// ================= LED ON =================

void ledOn()
{

  digitalWrite(LED_PIN, HIGH);

  ledState = true;


  Serial.println("LED ON");


  sendSMS("Alert: ESP8266 LED has been turned ON");


  server.sendHeader("Location", "/");

  server.send(303);

}



// ================= LED OFF =================

void ledOff()
{

  digitalWrite(LED_PIN, LOW);

  ledState = false;


  Serial.println("LED OFF");


  server.sendHeader("Location", "/");

  server.send(303);

}



// ================= SETUP =================

void setup()
{

  Serial.begin(115200);

  delay(1000);


  Serial.println();
  Serial.println("==========================");
  Serial.println("ESP8266 ARKESEL LED SERVER");
  Serial.println("==========================");



  // LED setup

  pinMode(LED_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);



  // WiFi

  WiFi.mode(WIFI_STA);


  Serial.print("Connecting to ");

  Serial.println(ssid);



  WiFi.begin(ssid, password);



  while(WiFi.status() != WL_CONNECTED)
  {

    delay(500);

    Serial.print(".");

  }



  Serial.println();

  Serial.println("WiFi Connected");


  Serial.print("IP Address: ");

  Serial.println(WiFi.localIP());



  // Web routes

  server.on("/", handleRoot);

  server.on("/ON", ledOn);

  server.on("/OFF", ledOff);



  server.begin();


  Serial.println("Web Server Started");

}



// ================= LOOP =================

void loop()
{

  server.handleClient();

}