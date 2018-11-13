#include "credentials.h"
#include <arduino.h>
#include <ESP8266WiFi.h>

int sensorPin = A0;
WiFiServer server(80); //define webserver on port 80

String header;

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(9600);
  Serial.println();

  WiFi.begin(WLAN_SSID, WLAN_PW);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(LED_BUILTIN, HIGH);

  server.begin(); //start webserver
}

void loop()
{
  WiFiClient client = server.available(); // Listen for incoming clients

  if (client)
  {                                // If a new client connects,
    Serial.println("New Client."); // print a message out in the serial port
    String currentLine = "";       // make a String to hold incoming data from the client
    unsigned long connectedSince = millis();
    while (client.connected() && process(client, connectedSince, currentLine))
    {
      // loop while the client's connected
    }
    // Clear the header variable
    header = "";
    currentLine = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

bool process(WiFiClient client, unsigned long connectedSince, String &currentLine)
{
  //if(!client.available())
  if (millis() - connectedSince > 1000)
  {
    Serial.println("client not available anymore");
    return false;
  }

  char c = client.read(); // read a byte, then
  Serial.write(c);        // print it out the serial monitor
  header += c;
  if (c == '\n' || currentLine.length() == 0)
  { // if the byte is a newline character
    // if the current line is blank, you got two newline characters in a row.
    // that's the end of the client HTTP request, so send a response:
    if (currentLine.length() == 0)
    {
      //--- HEADER --------------------------------------------------------------------------------
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/html");
      client.println("Connection: close");
      client.println();
      //--- CONTENT -------------------------------------------------------------------------------

      // Display the HTML web page
      client.println("<!DOCTYPE html><html>");
      client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
      client.println("<link rel=\"icon\" href=\"data:,\">");

      // Web Page Heading
      client.println("<body><h1>ESP8266 Web Server</h1>");

      int value = analogRead(sensorPin);
      Serial.println(value);
      client.print("<p>");
      client.print(value);
      client.println("</p>");
      client.print("<p>");
      client.print(millis());
      client.println("ms</p>");

      client.println("</body></html>");

      client.println(); // The HTTP response ends with another blank line
      //--- END OF COMMUNICATION ------------------------------------------------------------------
      Serial.println("END OF COMMUNICATION");
      return false;
    }
    else
    { // if you got a newline, then clear currentLine
      currentLine = "";
    }
  }
  else if (c != '\r')
  {                   // if you got anything else but a carriage return character,
    currentLine += c; // add it to the end of the currentLine
  }

  return true;
}

void route()
{
  // turns the GPIOs on and off
  if (header.indexOf("GET /5/on") >= 0)
  {
    Serial.println("GPIO 5 on");
  }
  else if (header.indexOf("GET /5/off") >= 0)
  {
    Serial.println("GPIO 5 off");
  }
  else if (header.indexOf("GET /4/on") >= 0)
  {
    Serial.println("GPIO 4 on");
  }
  else if (header.indexOf("GET /4/off") >= 0)
  {
    Serial.println("GPIO 4 off");
  }
}
