#include <Time.h>
#include <TimeLib.h>
#include <ESP8266WiFi.h>
//ESP Web Server Library to host a web page
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>


#include <ArduinoJson.h>

#define DEBUG_ON

const char *ssid = "xxxxxx";     // Set you WiFi SSID
const char *password = "xxxxxx";

//Comun cerrado
const uint8_t valorEncendido = LOW;
const uint8_t valorApagado = HIGH;

uint8_t LED1pin = D5;
const uint8_t horaEncendido = 18;
const uint8_t horaApagado = 22;
uint8_t LED1status = valorApagado;
bool basedOnTime = true;

int actualHour;
int actualMinute;
int actualsecond;
int actualyear;
int actualMonth;
int actualday;

ESP8266WebServer server(80); //Server on port 80

void setup() {
  pinMode(LED1pin, OUTPUT);
  digitalWrite(LED1pin,LED1status);
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED)
  {
    // not connected to the network
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("Conectado (Y)");

  Serial.println("Direccion IP: ");
  Serial.println(WiFi.localIP());
  server.on("/", handle_OnConnect);

  server.on("/relay1on", handle_led1on);
  server.on("/relay1off", handle_led1off);

  server.on("/time", handle_change_basedOnTime);
  server.on("/desactivate", handle_OnDesactivate);

  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("Servidor HTTP listo");


  HTTPClient http;    //Declare object of class HTTPClient


  http.begin("http://ruedadifusion.com/JP/getTime.php");              //Specify request destination
  int httpCode = http.GET();   //Send the request
  String payload = http.getString();    //Get the response payload

  if (httpCode > 0) {

    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
      Serial.println("payload= " + payload);
      //JSON
      // Enough space for:
      // + 1 object with 3 members
      // + 2 objects with 1 member
      const int capacity = JSON_OBJECT_SIZE(6) + 2 * JSON_OBJECT_SIZE(1);
      DynamicJsonDocument doc(capacity);
      DeserializationError err = deserializeJson(doc, payload);

      actualHour = doc["hour"].as<int>();
      actualMinute = doc["minute"].as<int>();
      actualsecond = doc["seconds"].as<int>()+10;
      actualday = doc["day"].as<int>();
      actualMonth = doc["month"].as<int>();
      actualyear = doc["year"].as<int>();

      setTime(actualHour, actualMinute, actualsecond, actualday, actualMonth, actualyear);

    } else {
      Serial.printf("Error trayendo hora", http.errorToString(httpCode).c_str());
    }
  } else {
    Serial.printf("[HTTP} No fue posible realizar la peticion\n");
  }
  http.end();  //Close connection
  delay(1000);

}

void loop() {
  time_t t = now();

  //Serial.println(hora_string);
  //Setear todo
  actualHour = hour(t);
  actualMinute = minute(t);
  actualsecond = second(t);
  actualday = day(t);
  actualMonth = month(t);
  actualyear = year(t);
  if (basedOnTime == true) {

    if (actualHour >= horaEncendido && actualHour < horaApagado) {
      if (LED1status == valorApagado) {
        digitalWrite(LED1pin, valorEncendido);
        LED1status = valorEncendido;
      }
    } else {
      if (LED1status == valorEncendido) {
        if (actualday != 24 || actualday != 31) {
          digitalWrite(LED1pin, valorApagado);
          LED1status = valorApagado;
        }
      }
    }
  }

  server.handleClient();
  if (LED1status == valorEncendido)
  {
    digitalWrite(LED1pin, valorEncendido);
  }
  else
  {
    digitalWrite(LED1pin, valorApagado);
  }
  delay(1000);//cada minuto
}

void handle_OnConnect()
{
  server.send(200, "text/html", SendHTML(LED1status));
}
void handle_OnDesactivate()
{
  LED1status = valorApagado;
  /*LED2status = LOW;
    LED3status = LOW;
    LED4status = LOW;*/
  server.send(200, "text/html", SendHTML(LED1status));
}

void handle_led1on()
{
  LED1status = valorEncendido;
  server.send(200, "text/html", SendHTML(LED1status));
}

void handle_led1off()
{

  LED1status = valorApagado;
  server.send(200, "text/html", SendHTML(LED1status));
}
void handle_change_basedOnTime() {

  if (basedOnTime == true) {
    basedOnTime = false;
  } else {
    basedOnTime = true;
  }
  server.send(200, "text/html", SendHTML(LED1status));
}

void handle_NotFound()
{
  server.send(404, "text/plain", "Not found");
}

String SendHTML(uint8_t led1stat)
{
  String hora_string = "";
  hora_string += actualday;
  hora_string += "/";
  hora_string += actualMonth;
  hora_string += "/";
  hora_string += actualyear;
  hora_string += " ";
  hora_string += actualHour;
  hora_string += ":";
  hora_string += actualMinute;
  hora_string += ":";
  hora_string += actualsecond;
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>Switch Casa Gomez </title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr += ".button {display: block;width: 120px;background-color: #6E0000;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 20px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr += ".button-on {background-color: #1abc9c;}\n";
  ptr += ".button-on:active {background-color: #16a085;}\n";
  ptr += ".button-off {background-color: #34495e;}\n";
  ptr += ".button-off:active {background-color: #2c3e50;}\n";
  ptr += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<h1>Hora: ";
  ptr += hora_string;
  ptr += "</h1>\n";

  if (led1stat == valorEncendido)
  {
    ptr += "<p>RELAY 1 Status: ON</p><a class=\"button button-off\" href=\"/relay1off\">OFF</a>\n";
  }
  else
  {
    ptr += "<p>RELAY 1 Status: OFF</p><a class=\"button button-on\" href=\"/relay1on\">ON</a>\n";
  }
  if (basedOnTime == true) {
    ptr += "<p>Se rige por las horas: Si</p><a class=\"button button-off\" href=\"/time\">NO</a>\n";
  } else {
    ptr += "<p>Se rige por las horas: No</p><a class=\"button button-on\" href=\"/time\">SI</a>\n";
  }
  /*
    if (led2stat == HIGH)
    {
    ptr += "<p>RELAY 2 Status: ON</p><a class=\"button button-off\" href=\"/relay2off\">OFF</a>\n";
    }
    else
    {
    ptr += "<p>RELAY 2 Status: OFF</p><a class=\"button button-on\" href=\"/relay2on\">ON</a>\n";
    }

    if (led3stat == HIGH)
    {
    ptr += "<p>RELAY 3 Status: ON</p><a class=\"button button-off\" href=\"/relay3off\">OFF</a>\n";
    }
    else
    {
    ptr += "<p>RELAY 3 Status: OFF</p><a class=\"button button-on\" href=\"/relay3on\">ON</a>\n";
    }

    if (led4stat == HIGH)
    {
    ptr += "<p>RELAY 4 Status: ON</p><a class=\"button button-off\" href=\"/relay4off\">OFF</a>\n";
    }
    else
    {
    ptr += "<p>RELAY 4 Status: OFF</p><a class=\"button button-on\" href=\"/relay4on\">ON</a>\n";
    }
  */
  //boton de desactivar

  ptr += "<a class=\"button\" href=\"/desactivate\">DESACTIVAR</a>\n";

  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}
