#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h> 
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "time.h"
#define OLED_MOSI 13 // 9
#define OLED_CLK 14 //10
#define OLED_DC 26 //11
#define OLED_CS 12 //12
#define OLED_RESET 27 //13
#define SEALEVELPRESSURE_HPA (1013.25)
// 소리
#include <Arduino.h>
#include <SPIFFS.h>
#include "WAVFileReader.h"
#include "SinWaveGenerator.h"
#include "I2SOutput.h"
// 레코그니션
#include <AWS_IOT.h>
#include <Arduino_JSON.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
//alarm api
#define SWAP 0 // sw access point
#include <string>

// Replace with your network credentials
#if SWAP
  const char *ssid = /*"SK_WiFiGIGA8DB8"*/"HAM";
const char *password = /*"1810008032"*/"GUKIMSEOLEE"; // password should be long!!
#else
const char *ssid = /*"SK_WiFiGIGA8DB8"*/"HAM";
const char *password = /*"1810008032"*/"GUKIMSEOLEE";
#endif

WiFiServer server(80);

// i2s pins
i2s_pin_config_t i2sPins = {
    .bck_io_num = GPIO_NUM_2,
    .ws_io_num = GPIO_NUM_15,
    .data_out_num = GPIO_NUM_22,
    .data_in_num = -1};
    
I2SOutput *output;
SampleSource *sampleSource;
//

String line = "";
String currentRain;
const int analogPin = 25;

int delayTime;

int waterVal = 0;
const int touchThread = 15;
const int waterThread = 2200;

bool stop = false;
bool firstLock = false;

const int ledChannel = 0;
const int resolution = 8;
const int duty = 128;

float discomfort;

AWS_IOT testButton;


String header;
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
const long timeoutTime = 2000;

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600*9; // 3600
const int daylightOffset_sec = 0; // 3600

#define BME_CS 5 // cs for esp32 vspi
Adafruit_BME280 bme(BME_CS); // hardware SPI
Adafruit_SH1106 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2
#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH 16

String hourZeroToTwelve = "<label class=\"label\">Hour</label><div class=\"select\"> <select name=\"hour\"> <option>0</option> <option>1</option> <option>2</option> <option>3</option> <option>4</option> <option>5</option> <option>6</option> <option>7</option> <option>8</option> <option>9</option> <option>10</option> <option>11</option> <option>12</option> <option>13</option> <option>14</option> <option>15</option> <option>16</option> <option>17</option> <option>18</option> <option>19</option> <option>20</option> <option>21</option> <option>22</option> <option>23</option> </select> </div>";
String minuteZeroToSixty = "<label class=\"label\">Minute</label><div class=\"select\"> <select name=\"minute\"> <option>0</option> <option>1</option> <option>2</option> <option>3</option> <option>4</option> <option>5</option> <option>6</option> <option>7</option> <option>8</option> <option>9</option> <option>10</option> <option>11</option> <option>12</option> <option>13</option> <option>14</option> <option>15</option> <option>16</option> <option>17</option> <option>18</option> <option>19</option> <option>20</option> <option>21</option> <option>22</option> <option>23</option> <option>24</option> <option>25</option> <option>26</option> <option>27</option> <option>28</option> <option>29</option> <option>30</option> <option>31</option> <option>32</option> <option>33</option> <option>34</option> <option>35</option> <option>36</option> <option>37</option> <option>38</option> <option>39</option> <option>40</option> <option>41</option> <option>42</option> <option>43</option> <option>44</option> <option>45</option> <option>46</option> <option>47</option> <option>48</option> <option>49</option> <option>50</option> <option>51</option> <option>52</option> <option>53</option> <option>54</option> <option>55</option> <option>56</option> <option>57</option> <option>58</option> <option>59</option> </select> </div>";
String musics = "<label class=\"label\">Music</label><div class=\"select\"> <select name=\"music\"> <option>Atheism</option> <option>Buddhism</option> <option>Christian</option> </select> </div>";
String getHour = "";
String getMin = "";
String getMusic = "";

// 레코그니션
char HOST_ADDRESS[]="akiub8kep5cue-ats.iot.ap-northeast-2.amazonaws.com";
char CLIENT_ID[]= "ChoiESP32";
char sTOPIC_NAME[]= "esp32/sub/data"; // subscribe topic name
char pTOPIC_NAME[]= "esp32/pub/data"; // publish topic name
int status = WL_IDLE_STATUS;
int msgCount=0,msgReceived = 0;
char payload[512];
char rcvdPayload[512];
const int waterPin = 34;
unsigned long preMil = 0;
const long intMil = 1000;
const int ledPin = 16;
String Time;
int title;
int firstSet=0;
int alarmSet=0;
//

static const unsigned char PROGMEM logo16_glcd_bmp[] =
{ B00000000, B11000000,
B00000001, B11000000,
B00000001, B11000000,
B00000011, B11100000,
B11110011, B11100000,
B11111110, B11111000,
B01111110, B11111111,
B00110011, B10011111,
B00011111, B11111100,
B00001101, B01110000,
B00011011, B10100000,
B00111111, B11100000,
B00111111, B11110000,
B01111100, B11110000,
B01110000, B01110000,
B00000000, B00110000 };
#if (SH1106_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SH1106.h!");
#endif

String printLocalTime()
{
  struct tm timeinfo;
  String nowTime = ""; 
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return "Failed to obtain time";
  }
  nowTime += ("Year:" + String(timeinfo.tm_year+ 1900) + " Month:" + String(timeinfo.tm_mon + 1));
  nowTime += (" Day: " + String(timeinfo.tm_mday)+ "  " + String(timeinfo.tm_hour) + ":" + String(timeinfo.tm_min) + ":" + String(timeinfo.tm_sec) + "\n");
  if(String(timeinfo.tm_hour) == getHour && String(timeinfo.tm_min) == getMin)
  {
    /*Serial.println("#######Saved Note is Playing.....#######");
    playAlarm();
    getRekResult();*/
    alarmSet=1;
  }
  return nowTime;
}

String temp;
void api()
{
  WiFiClient client = server.available(); // Listen for incoming clients
  
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  
  String old_Time = "";
  if (client)
  { // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client."); // print a message out in the serial port
    String currentLine = "";       // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime)
    { // loop while the client's connected
      currentTime = millis();
      if (client.available())
      {                         // if there's bytes to read from the client,
        char c = client.read(); // read a byte, then
        Serial.write(c);        // print it out the serial monitor
        header += c;
        if (c == '\n')
        { // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0)
          {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println("Refresh: 30\r");
            client.println();
            
            if (header.indexOf("GET /get?hour=") >= 0)
            {
              temp = header.substring(header.indexOf("/get?hour=")+11, header.indexOf("/get?hour=")+12);
              if(temp=="&"){
                getHour = header.substring(header.indexOf("/get?hour=")+10, header.indexOf("/get?hour=")+11);
              }
              else{
                getHour = header.substring(header.indexOf("/get?hour=")+10, header.indexOf("/get?hour=")+12);
              }
              temp = header.substring(header.indexOf("&minute=")+9, header.indexOf("&minute=")+10);
              if(temp=="&"){
                getMin = header.substring(header.indexOf("&minute=")+8, header.indexOf("&minute=")+9);
              }
              else{
                getMin = header.substring(header.indexOf("&minute=")+8, header.indexOf("&minute=")+10);
              }
              Serial.println("Got Hour: " + getHour);
              
              Serial.println("Got Min: " + getMin);
              getMusic = header.substring(header.indexOf("&music=")+7, header.indexOf("&music=")+15);
              Serial.println("Got Music: " + getMusic);
              getMusic.trim();
              Serial.println(getMusic.length());
            }
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/bulma@0.9.3/css/bulma.min.css\">");

            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50;border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");

            // Web Page Heading
            client.println("<body><h1>Alarm API</h1>");
            
            //display Clock interface
            String now_Time = printLocalTime();
            if(now_Time != old_Time)
            {
              old_Time = now_Time;
            }

            
            // javaScript
            client.println("<script>var totalTime="+String(timeinfo.tm_hour*3600+timeinfo.tm_min*60+timeinfo.tm_sec)+
            "; setInterval(function(){totalTime++; document.getElementById('timer').innerHTML='NowTime: '+Math.floor(totalTime/3600) + ':' + Math.floor(totalTime%3600/60) + ':' + totalTime%3600%60;}, 1000);</script>");
            client.println(&timeinfo, "<h2 id='timer'>NowTime: %H:%M:%S</h2>");
            client.println("Year: "+String(timeinfo.tm_year+1900)+", Month: " + String(timeinfo.tm_mon+1));
            // javaScript

            client.println("</br><h2>Set Alarm</h2>");
            client.println("<p>");
            client.println("<form action=\"/get\">");
        
            client.println(hourZeroToTwelve);
            client.println(minuteZeroToSixty);
            client.println(musics);
            client.println("<br><input type=\"submit\" value=\"Submit\">");
            client.println("<button type=\"botton\" onclick=\"location.href=\'192.168.0.30\'\">Refresh</button>");
            client.println("</form><div class=\"control\"></div></p>");
            
            client.println("</body></html>");
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } //** if (currentLine.length() == 0) {
          else
          { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } //** if (c == '\n') {
        else if (c != '\r')
        {                   // if you got anything else but a carriage return character,
          currentLine += c; // add it to the end of the currentLine
        }
      } //* if (client.available()){
    }   //** while

    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
    
  } //** if (client) {
}

void setting()
{
    #if SWAP
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
#else
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
#endif
  server.begin();
}

// 레코그니션
void mySubCallBackHandler (char *topicName, int payloadLen, char *payLoad)
{
  strncpy(rcvdPayload,payLoad,payloadLen);
  rcvdPayload[payloadLen] = 0;
  msgReceived = 1; //flag
}

void playAlarm(){
  Serial.println("Music start");

  SPIFFS.begin();

  Serial.println("Created sample source");

  // sampleSource = new SinWaveGenerator(40000, 10000, 0.75);

  sampleSource = new WAVFileReader("/sample.wav");

  Serial.println("Starting I2S Output");
  output = new I2SOutput();
  output->start(I2S_NUM_1, i2sPins, sampleSource);
}

int alarmStopped=0;
void stopAlarm(){
  Serial.println("function stopAlarm");
  output->stopPlay(I2S_NUM_1);
  alarmStopped=1;
}

void getRekResult() {
  WiFiUDP ntpUDP;
  NTPClient timeClient (ntpUDP);
  timeClient.begin();
  timeClient.update();
  Time = String(timeClient.getEpochTime()-10);
  if(firstSet==0){
    title = timeClient.getEpochTime()-10;
    firstSet=1;
  }
  if(msgReceived == 1)
  {
    msgReceived = 0;
    Serial.print("Received Message:");
    Serial.println(rcvdPayload);
    String faceFlag = rcvdPayload;
    if(faceFlag=="100"){
      Serial.println("1st Lock unlocked");
      /*1차 잠금 해제*/
      firstLock = true;
      //stopAlarm();
    }
  }
  if((millis()-preMil) > intMil && !firstLock) {
      preMil = millis();
      sprintf(payload,"{\"payload\": \"%d.jpg\"}", title);
      if(testButton.publish(pTOPIC_NAME,payload) == 0) {
        Serial.print("Publish Message:");
        Serial.println(payload);
        title++;
      }
      else
        Serial.println("Publish failed");
    }
}
//

String parsing() {
  int del_index = line.indexOf(F("description"));
  line.remove(0, del_index);
  int end_index = line.indexOf(F("icon")); 
  String description;
  description = line.substring(14, end_index - 3); 
  Serial.println(description);
  line = "";
  return description;
}

String get_weather() {
  String description;
    Serial.println("Starting connection to server...");
    HTTPClient http;
    http.begin("https://api.openweathermap.org/data/2.5/weather?lat=37&lon=126&appid=74b9181d35bfcd15f534a40bc4e0698e");       //Specify the URL
    int httpCode = http.GET();  //Make the request
    if (httpCode > 0) {         //Check for the returning code
      line = http.getString();
      Serial.println("Success on HTTP request");
      description = parsing();
      http.end(); //Free the resources
    }
    else {
      Serial.println("Error on HTTP request");
    }
//    Serial.println(line); // 수신한 날씨 정보 시리얼 모니터 출력
  return description;
}

void InitTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  Serial.println("Year: " + String(timeinfo.tm_year+1900) + ", Month: " + String(timeinfo.tm_mon+1));
}

float discomfortCal(){ 
  discomfort = 0.81 * bme.readTemperature() + 0.01 * bme.readHumidity() * ( 0.99 * bme.readTemperature() - 14.3 ) + 46.3;
  return discomfort;
}

String feelState(){
  if(discomfort < 70){
    return "Good!!";
  }
  else if(70 <= discomfort && discomfort <= 75){
    return "So So";
  }
  else if(75 < discomfort && discomfort <= 80){
    return "Bad";
  }
  else{
    return "Destroy it!!";
  }
}

void DisplayTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){  
    display.clearDisplay();
    display.setCursor(0,0);
    Serial.println("Failed to obtain time");
    display.println("Failed to obtain time");
    display.display();
    return;
  }
  display.clearDisplay();
  display.setCursor(0,0);
  
  display.println();
  display.print(String(timeinfo.tm_year+1900) + " / " + String(timeinfo.tm_mon+1) + " / ");
  display.print(&timeinfo, "%d");
  display.println();
  display.println();
  display.println(&timeinfo, "%A");
  display.println();
  display.println(&timeinfo, "%H : %M : %S");
  display.display();
}

void DisplayBME280() {
  display.clearDisplay();
  display.setCursor(0,0);
  display.println();
  display.print("Temperature = ");
  display.print(bme.readTemperature(), 1);
  display.println(" *C");
  display.println();
  display.println();
  display.print("Discomfort : ");
  display.println(discomfortCal(), 1);
  display.print(feelState());
  display.println();
  display.println();
  display.print(currentRain);
  display.display();
  delay(delayTime);
  display.clearDisplay();
  display.setCursor(0,0);
} 

void DisplayBathroom(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.println();
  display.println();
  display.println();
  display.println();
  display.print("Go To Bathroom NOW!!!");
  display.display();
  delay(delayTime);
  display.clearDisplay();
  display.setCursor(0,0);
}

void ReadWater(){
  if(stop == false){
    waterVal = analogRead(waterPin);
    Serial.println(waterVal);
    delay(500);
  }
  
  if(waterVal > waterThread && stop == false){
    stop = true;
    Serial.print("Alarm off : ");
    Serial.println(waterVal);
    stopAlarm();
  }
}


void awsConnect(){
  if(testButton.connect(HOST_ADDRESS,CLIENT_ID)== 0) {
    Serial.println("Connected to AWS");
    delay(1000);
    if(0==testButton.subscribe(sTOPIC_NAME,mySubCallBackHandler)) {
      Serial.println("Subscribe Successfull");
    }
    else {
      Serial.println("Subscribe Failed, Check the Thing Name and Certificates");
      while(1);
    }
  }
  else {
    Serial.println("AWS connection failed, Check the HOST Address");
    while(1);
  }
}

bool BMEstatus;
void BMEsetting(){
  // BME280
  BMEstatus = bme.begin(0x76); // bme280 I2C address = 0x76
  if (!BMEstatus) {
    Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
    Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
    Serial.print(" ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print(" ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print(" ID of 0x60 represents a BME 280.\n");
    Serial.println(" ID of 0x61 represents a BME 680.\n");
    while (1) delay(10);
  }
}

void startDis(){
  // Display
  display.begin(SH1106_SWITCHCAPVCC);
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(2000);
  display.clearDisplay();
  
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  
//  display.setTextColor(BLACK, WHITE); // 색깔 반전
  display.display();
}

void DisplayEnd(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("Alarm OFF!!!");
  display.println();
  display.println();
  display.print("Have a Very Nice Day!!!");
  display.println();
  display.println();
  display.print("Push Enable Button to restart");
  display.display();
  display.clearDisplay();
  display.setCursor(0,0);
}

void setup() {
  Serial.begin(115200);
  setting();
  awsConnect();
  InitTime();
  currentRain = get_weather();
  //disconnect WiFi as it's no longer needed
//  WiFi.disconnect(true);
//  WiFi.mode(WIFI_OFF);
  BMEsetting();
  delayTime = 1000;
  startDis();
  delay(2000);
}

int alCount =0;
void loop() {

  api();

  if(alarmSet==1){
    //Serial.println("#######Alarm Playing.....#######");
    if(alCount==0){
      playAlarm();
      alCount = 1;
    }
    getRekResult();
  }
  
  if(firstLock&&alarmStopped==0){
    DisplayBathroom();
    ReadWater();
  }
  
  else if(touchRead(4) < touchThread&&alarmStopped==0){
    DisplayBME280();
  }
  
  else if(alarmStopped==0){
    DisplayTime();
  }
  if(alarmStopped==1){
    DisplayEnd();
  }
}
