//By Jan Riedel & Jo Zhou
//Adafruit Feather Huzzah 8266 and RGBW LED strip

#include <ESP8266WiFi.h> // Include WiFi library
#include <ESP8266WebServer.h> // Include WebServer library
#include <DS3231M.h> // Include the DS3231M RTC library

// SSID of WiFi network and password of WiFi network
const char* ssid = "WiFi-Name";
const char* password = "WiFi-Password"; 

//Creates an instance of the DS3231M class and server object on port 80l standard port for webbrowsers talking to web servers
DS3231M_Class DS3231M;       
ESP8266WebServer server(80);

//Assigning variables for LED pins
uint8_t WHITEpin = 15;
uint8_t BLUEpin = 13;
uint8_t REDpin = 14;
uint8_t GREENpin = 12;

//Define boolean variables for LED button status
bool Lightstatus = LOW;
bool Nightlightstatus = LOW;
bool Bluestatus = LOW;
bool Redstatus = LOW;
bool Greenstatus = LOW;

//Define variables to store current time and alarm time; Alarm time is set to 65 initially, to hide "Alarm time" in HTML when alarm is not set
uint8_t Actualhour;
uint8_t Actualminute;
uint8_t Alarmhour = 65;
uint8_t Alarmminute = 65;

//Define and set variable for delay when turning leds in "alarmlight()"
uint16_t FADESPEED = 500;

void setup()
{
  //Set LED pins to OUTPUT
  pinMode(WHITEpin, OUTPUT);
  pinMode(BLUEpin, OUTPUT);
  pinMode(REDpin, OUTPUT);
  pinMode(GREENpin, OUTPUT);

  //Initiate Serial monitor 
  Serial.begin(115200);
  Serial.println();

  //Initialize RTC communications; Red LED blinks, if DS3231M cannot be initialized
  while (!DS3231M.begin()) 
    {
      Serial.println(F("Unable to find DS3231MM. Checking again in 3s."));
      delay(1000);
      digitalWrite(REDpin, HIGH);
      delay(1000);
      digitalWrite(REDpin, LOW);
      delay(1000);
    }
  DS3231M.pinSquareWave(); // Make INT/SQW pin toggle at 1Hz
  //DS3231M.adjust(DateTime(2020, 4, 4, 13, 04, 00)); // Sets time of DS3231M
  Serial.println("DS3231M initialized.");
  digitalWrite(REDpin, LOW); // Turns red LED off, in case it stayed on in initial sequence
  
  //Initiate WiFi in Station Mode
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");

  //Functions that get called when specific url is being called
  server.on("/", handle_OnConnect); 
  server.on("/lighton", handle_lighton);
  server.on("/lightoff", handle_lightoff);
  server.on("/nightlighton", handle_nightlighton);
  server.on("/nightlightoff", handle_nightlightoff);
  server.on("/blueon", handle_blueon);
  server.on("/blueoff", handle_blueoff);
  server.on("/redon", handle_redon);
  server.on("/redoff", handle_redoff);
  server.on("/greenon", handle_greenon);
  server.on("/greenoff", handle_greenoff);
  server.on("/alarmtime", handle_alarmtime);
  server.on("/alarmreset", handle_alarmreset);
  server.onNotFound(handle_NotFound);  //404 message when url not specified with server.on requested

  server.begin(); // Start webserver
  Serial.printf("Web server started, open %s in a web browser\n", WiFi.localIP().toString().c_str());

  Serial.println("Light Status: OFF | Blue Status: OFF | RED Status: OFF | GREEN Status: OFF");
}

//Check status of buttons and control LEDS
void loop() 
{
  server.handleClient();
  DateTime now = DS3231M.now(); // get the current time from device

  //Save current hour and minute and save it to variable
  if (Actualhour != now.hour())
    {Actualhour = now.hour();}
  if (Actualminute != now.minute())
    {Actualminute = now.minute();}

  //Check if actual time is the same as alarmtime. If so, call "alarmlight"
  if(now.hour() == Alarmhour && now.minute() == Alarmminute)
  {alarmlight();}

  //Check if boolean variable for the HTML button has changed through calling the corresponding link (in SendHTML) and turn on LEDs
  if(Lightstatus)
    {digitalWrite(WHITEpin, HIGH);}
  else
    {digitalWrite(WHITEpin, LOW);}

  if(Nightlightstatus)
    {
    analogWrite(BLUEpin, 94);
    analogWrite(REDpin, 253);
    analogWrite(GREENpin, 83);
    }

  if(Bluestatus)
    {digitalWrite(BLUEpin, HIGH);}
  else
    {digitalWrite(BLUEpin, LOW);}

  if(Redstatus)
    {digitalWrite(REDpin, HIGH);}
  else
    {digitalWrite(REDpin, LOW);}

  if(Greenstatus)
    {digitalWrite(GREENpin, HIGH);}
  else
    {digitalWrite(GREENpin, LOW);}
}

//Definition of custom functions for calling certain URLs and turning the alarm light on
//Alarm light function that slow turns on LEDs, mimicking sunrise;
void alarmlight()                                     
{
  for (uint16_t r = 0; r < 1024; r++) 
    { 
    analogWrite(REDpin, r);
    delay(FADESPEED);
    }
  for (uint16_t g = 0; g < 1024; g++) 
    { 
    analogWrite(GREENpin, g);
    delay(FADESPEED);
    }
  for (uint16_t b = 0; b < 512; b++) 
    { 
    analogWrite(BLUEpin, b);
    delay(FADESPEED);
    } 
  for (uint16_t w = 0; w < 1024; w++) 
    { 
    analogWrite(WHITEpin, w);
    delay(FADESPEED);
    }
// Set boolean variables to HIGH, so light alarm stays on until reset  
Lightstatus = HIGH;
Bluestatus = HIGH;
Redstatus = HIGH;
Greenstatus = HIGH;
}

//These function handle request from client (through server.on) and send HTML back to client. Variables in SendHTML string will get updated as well
//Function that called when calling root (/) of webserver
void handle_OnConnect() 
{
  server.send(200, "text/html", SendHTML(Lightstatus,Nightlightstatus,Bluestatus,Redstatus,Greenstatus,Actualhour,Actualminute,Alarmhour,Alarmminute)); 
}

//Functions that get called when LED buttons get pushed
void handle_lighton() {
  Lightstatus = HIGH;
  Serial.println("Light Status: ON");
  server.send(200, "text/html", SendHTML(true,Nightlightstatus,Bluestatus,Redstatus,Greenstatus,Actualhour,Actualminute,Alarmhour,Alarmminute)); 
}

void handle_lightoff() {
  Lightstatus = LOW;
  Serial.println("Light Status: OFF");
  server.send(200, "text/html", SendHTML(false,Nightlightstatus,Bluestatus,Redstatus,Greenstatus,Actualhour,Actualminute,Alarmhour,Alarmminute)); 
}

void handle_nightlighton() {
  Nightlightstatus = HIGH;
  Serial.println("Night Light Status: ON");
  server.send(200, "text/html", SendHTML(Lightstatus,true,Bluestatus,Redstatus,Greenstatus,Actualhour,Actualminute,Alarmhour,Alarmminute)); 
}

void handle_nightlightoff() {
  Nightlightstatus = LOW;
  Serial.println("Night Light Status: OFF");
  server.send(200, "text/html", SendHTML(Lightstatus,false,Bluestatus,Redstatus,Greenstatus,Actualhour,Actualminute,Alarmhour,Alarmminute)); 
}

void handle_blueon() {
  Bluestatus = HIGH;
  Serial.println("Blue Status: ON");
  server.send(200, "text/html", SendHTML(Lightstatus,Nightlightstatus,true,Redstatus,Greenstatus,Actualhour,Actualminute,Alarmhour,Alarmminute)); 
}

void handle_blueoff() {
  Bluestatus = LOW;
  Serial.println("Blue Status: OFF");
  server.send(200, "text/html", SendHTML(Lightstatus,Nightlightstatus,false,Redstatus,Greenstatus,Actualhour,Actualminute,Alarmhour,Alarmminute)); 
}

void handle_redon() {
  Redstatus = HIGH;
  Serial.println("Red Status: ON");
  server.send(200, "text/html", SendHTML(Lightstatus,Nightlightstatus,Bluestatus,true,Greenstatus,Actualhour,Actualminute,Alarmhour,Alarmminute)); 
}

void handle_redoff() {
  Redstatus = LOW;
  Serial.println("Red Status: OFF");
  server.send(200, "text/html", SendHTML(Lightstatus,Nightlightstatus,Bluestatus,false,Greenstatus,Actualhour,Actualminute,Alarmhour,Alarmminute)); 
}

void handle_greenon() {
  Greenstatus = HIGH;
  Serial.println("Green Status: ON");
  server.send(200, "text/html", SendHTML(Lightstatus,Nightlightstatus,Bluestatus,Redstatus,true,Actualhour,Actualminute,Alarmhour,Alarmminute)); 
}

void handle_greenoff() {
  Greenstatus = LOW;
  Serial.println("Green Status: OFF");
  server.send(200, "text/html", SendHTML(Lightstatus,Nightlightstatus,Bluestatus,Redstatus,false,Actualhour,Actualminute,Alarmhour,Alarmminute)); 
}

//Function that sets alarmtime by saving set time from URL
void handle_alarmtime() {
  if (server.arg("hour")!= ""){
    Alarmhour = server.arg("hour").toInt();
  }

  if (server.arg("minute")!= ""){
    Alarmminute = server.arg("minute").toInt();
    Serial.print("Alarm time set to: ");
    Serial.print(Alarmhour);
    Serial.print(":");
    Serial.println(Alarmminute);
  }
  server.send(200, "text/html", SendHTML(Lightstatus,Nightlightstatus,Bluestatus,Redstatus,Greenstatus,Actualhour,Actualminute,Alarmhour,Alarmminute));
}

//Alarm variables get set to 65 to virtually set them to an out of range for actual time and reset the alarm
void handle_alarmreset() {
  Alarmhour = 65;
  Alarmminute = 65;
  server.send(200, "text/html", SendHTML(Lightstatus,Nightlightstatus,Bluestatus,Redstatus,Greenstatus,Actualhour,Actualminute,Alarmhour,Alarmminute));
}

//Function to handle 404 request
void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

//Strings hold HTML and CSS styling
String SendHTML(uint8_t lightstat,uint8_t nightlightstat,uint8_t bluestat,uint8_t redstat,uint8_t greenstat, uint8_t actualhh, uint8_t actualmm, uint8_t alarmhh, uint8_t alarmmm){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name='viewport' content='width=device-width, initial-scale=1.0, user-scalable=no'>\n";
  ptr +="<title>Lichtwecker</title>\n";
  ptr +="<style>html {background-color: black;font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;color: white;font-size: 14px} h1 {color: #fff;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 20px;}\n";
  ptr +=".button-on {background-color: #6FB98F;}\n";
  ptr +=".button-on:active {background-color: #45BD79;}\n";
  ptr +=".button-off {background-color: #F18D9E;}\n";
  ptr +=".button-off:active {background-color: #EE5972;}\n";
  ptr +=".time {margin-right: 5px;margin-left: 5px;padding: 5px 10px;border: none;border-radius: 4px;background-color: #4d4d4d;color: #cecece;}\n";
  ptr +=".submit {margin-left: 5px;padding: 6px 10px;border: none;border-radius: 4px;background-color: #4d4d4d;color: #cecece;text-decoration: none}\n";
  ptr +=".form {margin: 0px auto 20px;}\n";
  ptr +="p {font-size: 14px;color: #fff;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>ESP8266 Lichtwecker </h1>\n";
  ptr +="<p>Current Time: ";
  ptr +=(int)actualhh;
  ptr +=":";
  if (actualmm < 10) // Display leading zeros
  {ptr +="0";}
  ptr +=(int)actualmm;
  ptr +="</p>\n";
  if (alarmhh < 61) // Show alarm time when it is set to value within range of actual time
  {ptr +="<p>Alarm Time: ";
  ptr +=(int)alarmhh;
  ptr +=":";}
  if(alarmmm < 10) // Display leading zeros
  {ptr +="0";}
  if(alarmmm < 61) // Show alarm time when it is set to value within range of actual time
  {ptr +=(int)alarmmm;
  ptr +="</p>\n";}

  // HTML dropdown form to set the alarm time
  ptr +="<form class='form' method ='GET' action='/alarmtime'>";
  ptr +="<select class='time' name = hour>";
  ptr +="<option value='0'>0</option>";
  ptr +="<option value='1'>1</option>";
  ptr +="<option value='2'>2</option>";
  ptr +="<option value='3'>3</option>";
  ptr +="<option value='4'>4</option>";
  ptr +="<option value='5'>5</option>";
  ptr +="<option value='6'>6</option>";
  ptr +="<option value='7'>7</option>";
  ptr +="<option value='8'>8</option>";
  ptr +="<option value='9'>9</option>";
  ptr +="<option value='10'>10</option>";
  ptr +="<option value='11'>11</option>";
  ptr +="<option value='12'>12</option>";
  ptr +="<option value='13'>13</option>";
  ptr +="<option value='14'>14</option>";
  ptr +="<option value='15'>15</option>";
  ptr +="<option value='16'>16</option>";
  ptr +="<option value='17'>17</option>";
  ptr +="<option value='18'>18</option>";
  ptr +="<option value='19'>19</option>";
  ptr +="<option value='20'>20</option>";
  ptr +="<option value='21'>21</option>";
  ptr +="<option value='22'>22</option>";
  ptr +="<option value='23'>23</option>";
  ptr +="</select>";
  ptr +=":";
  ptr +="<select class='time' name= minute>";
  ptr +="<option value='00'>00</option>";
  ptr +="<option value='10'>10</option>";
  ptr +="<option value='20'>20</option>";
  ptr +="<option value='30'>30</option>";
  ptr +="<option value='40'>40</option>";
  ptr +="<option value='50'>50</option>";
  ptr +="</select>";
  ptr +="<input class='submit' type='submit' value='Set Alarm'>";
  ptr +="</form>\n"; 

  // Display alarm reset button when alarm is set to value within range of actual time
  if(alarmhh < 61 && alarmmm < 61)
  {ptr +="<a class='submit' href='/alarmreset'>Alarmreset</a>\n";}

  // If function that will either show the "ON" or "OFF" button for LEDs
  if(lightstat)
  {ptr +="<p>Light State: ON</p><a class='button button-off' href='/lightoff'>OFF</a>\n";}
  else
  {ptr +="<p>Light State: OFF</p><a class='button button-on' href='/lighton'>ON</a>\n";}

  if(nightlightstat)
  {ptr +="<p>Night Light State: ON</p><a class='button button-off' href='/nightlightoff'>OFF</a>\n";}
  else
  {ptr +="<p>Night Light State: OFF</p><a class='button button-on' href='/nightlighton'>ON</a>\n";}

  if(bluestat)
  {ptr +="<p>Blue State: ON</p><a class='button button-off' href='/blueoff'>OFF</a>\n";}
  else
  {ptr +="<p>Blue State: OFF</p><a class='button button-on' href='/blueon'>ON</a>\n";}

  if(redstat)
  {ptr +="<p>Red State: ON</p><a class='button button-off' href='/redoff'>OFF</a>\n";}
  else
  {ptr +="<p>Red State: OFF</p><a class='button button-on' href='/redon'>ON</a>\n";}

  if(greenstat)
  {ptr +="<p>Green State: ON</p><a class='button button-off' href='/greenoff'>OFF</a>\n";}
  else
  {ptr +="<p>Green State: OFF</p><a class='button button-on' href='/greenon'>ON</a>\n";}

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
