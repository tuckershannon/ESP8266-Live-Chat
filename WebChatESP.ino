// Tucker Shannon 2023

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* ssid = "ssid";  // Replace with your network SSID (name)
const char* password = "password";  // Replace with your network password


ESP8266WebServer server(80);

String chatLog = "";  // This will store the chat history
String username = ""; // This will store the username

void handleRoot() {
  String page =
    "<html>"
    "<head>"
      "<script>"
        "function refreshChat() {"
          "var xhr = new XMLHttpRequest();"
          "xhr.onreadystatechange = function() {"
            "if (xhr.readyState == XMLHttpRequest.DONE) {"
              "document.getElementById('chatLog').innerHTML = xhr.responseText;"
            "}"
          "};"
          "xhr.open('GET', '/chatLog', true);"
          "xhr.send();"
        "}"
        "setInterval(refreshChat, 2000);"  // Refresh chat every 2 seconds
        "function sendMessage() {"
          "var input = document.getElementById('messageInput');"
          "var message = input.value;"
          "var xhr = new XMLHttpRequest();"
          "xhr.open('GET', '/send?message=' + encodeURIComponent(message), true);"
          "xhr.send();"
          "input.value = '';"
          "return false;"  // Prevent the form from being submitted normally
        "}"
      "</script>"
    "</head>"
    "<body>"
      "<h1>Chat Room</h1>"
      "<div id='chatLog'></div>";

  if (username == "") {  // If no username is set, show the username form
    page +=
      "<form action=\"/setUsername\" method=\"get\">"
        "<input type=\"text\" name=\"username\" placeholder=\"Username\">"
        "<input type=\"submit\" value=\"Set Username\">"
      "</form>";
  } else {  // If a username is set, show the chat form
    page +=
      "<form onsubmit=\"return sendMessage();\">"
        "<input type=\"text\" id=\"messageInput\" placeholder=\"Message\">"
        "<input type=\"submit\" value=\"Send\">"
      "</form>";
  }

  page += "</body></html>";

  server.send(200, "text/html", page);
}

void handleSetUsername() {
  username = server.arg("username");
  server.sendHeader("Location", String("/"), true);
  server.send (302, "text/plain", "");   // Redirect to the root page
}

void handleChatLog() {
  server.send(200, "text/html", chatLog);
}

void handleMessage() {
  String message = server.arg("message");

  // Generate timestamp
  time_t rawtime = time(NULL);
  struct tm * ti;
  ti = localtime (&rawtime);
  char timestamp[20];
  strftime(timestamp, 20, "%Y-%m-%d %H:%M:%S", ti);

  chatLog += String("<b>") + username + "</b> (" + timestamp + "): " + message + "<br>";  // Add the new message to the chat history
  server.send(200, "text/plain", "");   // Send back an empty response
}

void setup(void){
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  Serial.println("");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/setUsername", handleSetUsername);
  server.on("/send", handleMessage);
  server.on("/chatLog", handleChatLog);  // Add a new route for the chat log

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void){
  server.handleClient();
  MDNS.update();
}
