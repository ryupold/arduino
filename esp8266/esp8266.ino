#include "credentials.h"
#include <arduino.h>
#include <ESP8266WiFi.h>
#include <Esp.h>

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
    blink(250);
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  led(false);

  // this option is usefull when you want to send requests the webserver rapidly
  server.setNoDelay(true);

  server.begin(); //start webserver
}

void led(bool on)
{
  if (on)
    digitalWrite(LED_BUILTIN, LOW);
  else
    digitalWrite(LED_BUILTIN, HIGH);
}

void blink(unsigned long ms)
{
  led(true);
  delay(ms);
  led(false);
}

void loop()
{
  while(handleClients(60 * 1000)){} //let clients connect for a minute

  Serial.println("going into deep sleep");
  ESP.deepSleep(10 * 60e6);
}

int handleClients(unsigned long ms)
{
  int count = 0;
  unsigned long start = millis();

  do
  {
    WiFiClient client = server.available(); // Listen for incoming clients

    if (client)
    { // If a new client connects,
      count++;
      led(true);
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
      led(false);
    }
  } while (millis() - start < ms);

  return count;
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
      client.println("Content-type:text/plain");
      client.println("Connection: close");
      client.println();
      //--- CONTENT -------------------------------------------------------------------------------

      client.print(analogRead(A0));

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
