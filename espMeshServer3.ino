#include "painlessMesh.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <PubSubClient.h> 
#include "FS.h"

#define   MESH_PREFIX     "ESPMesh"
#define   MESH_PASSWORD   "24041990"

#define   MESH_PORT       5555
#define HOSTNAME "MQTT_Bridge"

//const char* hostId="api.thingspeak.com";
//String apiKey = "98KB26E9DSW91TB5";

const char *ssidAP = "ESPuser";
const char *passwordAP = "24041990";

String STATION_SSID; 
String STATION_PASSWORD;

const char* mqtt_server="Your mqtt host id here"; 

Scheduler  ts; // to control your personal task
painlessMesh  mesh;

PubSubClient client;

WiFiClient wifiClient;
ESP8266WebServer server(80);

//PubSubClient mqttClient(mqttBroker, 1883, wifiClient);

IPAddress ap_local_IP(192,168,1,4);
IPAddress ap_gateway(192,168,1,254);
IPAddress ap_subnet(255,255,255,0);

// Prototype
void receivedCallback( uint32_t from, String &msg );

void taskOnBroadcast();
void taskOnDisable();

unsigned long apTimer =0;
unsigned long apInterval = 30000;

unsigned long startInterval = 15000;
unsigned long startTimer = 0;

String valueHumid;
String valueTemp;
String valueHumid1;
String valueTemp1;

uint32_t channelId ;

//Creating the input form
const char INDEX_HTML[] =
"<!DOCTYPE HTML>"
"<html>"
  "<head>"
"<meta content=\"text/html; charset=ISO-8859-1\""
" http-equiv=\"content-type\">"
"<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\">"
"<title>ESP8266 Web Form Demo</title>"
"<style>"
"\"body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }\""
"</style>"
"</head>"
"<body>"
"<h1>ESP8266 Web Form Demo</h1>"
"<FORM action=\"/\" method=\"post\">"
"<P>"
"<label>ssid:&nbsp;</label>"
"<input maxlength=\"30\" name=\"ssid\"><br>"
"<label>Password:&nbsp;</label><input maxlength=\"30\" name=\"Password\"><br>"
"<INPUT type=\"submit\" value=\"Send\"> <INPUT type=\"reset\">"
"</P>"
"</FORM>"
"</body>";


// Send my ID every 10 seconds to inform others
Task taskBroadcast(20*TASK_SECOND,TASK_FOREVER,&taskOnBroadcast,&ts,false,NULL,&taskOnDisable);

//*****************READ STRING**********************//
String read_string(int l, int p){
  String temp;
  for (int n = p; n < l+p; ++n)
    {
     if(char(EEPROM.read(n))!=';'){
       temp += String(char(EEPROM.read(n)));
     }else n=l+p;
    }
  return temp;
}

void setup() {
  Serial.begin(115200);
  
  EEPROM.begin(512);
  SPIFFS.begin();
  Serial.println(SPIFFS.format() ? "Format Complete" : "Un successful");

  
  
  Serial.println();
  Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */

   for(uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    }
    WiFi.softAPConfig(ap_local_IP, ap_gateway, ap_subnet);
    Serial.print("Setting soft-AP ... ");
    WiFi.softAP(ssidAP, passwordAP);
    server.on("/", handleRoot);
    server.onNotFound(handleNotFound);
    server.begin();

    apTimer =millis();
    while(millis()-apTimer<= apInterval){
        server.handleClient();  
      }
  
  reconnectWiFi();
  WiFi.disconnect();
  meshInit(channelId);

  mesh.onReceive(&receivedCallback);

  mesh.onNewConnection([](size_t nodeId) {
    Serial.printf("New Connection %u\n", nodeId);
  });

  mesh.onDroppedConnection([](size_t nodeId) {
    Serial.printf("Dropped Connection %u\n", nodeId);
  });

  //taskBroadcast.enable();
}

void loop() {
  ts.execute(); // it will run mesh scheduler as well
  mesh.update();
}

//callback for task broadcast
void taskOnBroadcast(){
    DynamicJsonBuffer jsonBuffer;
    JsonObject& msg = jsonBuffer.createObject();
    msg["topic"] = "logServer";
    msg["nodeId"] = mesh.getNodeId();

    String str;
    msg.printTo(str);
    mesh.sendBroadcast(str);
    Serial.print("server json:");
    msg.printTo(Serial);

    taskWiFiCallback();
    wifiClient.stop();
  }
  
//callback for taskWiFi

 void taskWiFiCallback(){
  mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  mesh.setHostname(HOSTNAME);
  Serial.println(mesh.getStationIP());
  Serial.println("WiFi callback started");
    client.setClient(wifiClient);
      //client connects to server 
     client.setServer(mqtt_server,1883);
  client.connect("ESP8266Client123456789");
  client.subscribe("TempRoom1");
    client.subscribe("HumidRoom1");
  client.subscribe("TempRoom2");
    client.subscribe("HumidRoom2");
  //while(millis()-startTimer<= startInterval){
    
    if(!client.connected()){
      Serial.println("client not connected");
      return;
      }
      Serial.println("client connected");
     client.publish("TempRoom1",String(valueTemp).c_str());
     client.publish("HumidRoom1",String(valueHumid).c_str());  
     client.publish("TempRoom2",String(valueTemp1).c_str());
     client.publish("HumidRoom2",String(valueHumid1).c_str());  
     client.loop();

     
  //}    
    /*if(wifiClient.connect(hostId,80)){
            String postStr = apiKey;
            postStr +="&field1=";
            postStr += String(valueHumid);
            postStr +="&field2=";
            postStr += String(valueHumid1);
            postStr +="&field3=";
            postStr += String(valueTemp);
            postStr +="&field4=";
            postStr += String(valueTemp1);
            postStr += "\r\n\r\n";

            wifiClient.print("POST /update HTTP/1.1\n");
                             wifiClient.print("Host: api.thingspeak.com\n");
                             wifiClient.print("Connection: close\n");
                             wifiClient.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
                             wifiClient.print("Content-Type: application/x-www-form-urlencoded\n");
                             wifiClient.print("Content-Length: ");
                             wifiClient.print(postStr.length());
                             wifiClient.print("\n\n");
                             wifiClient.print(postStr);
          }
          wifiClient.stop();*/
  }

void taskOnDisable(){
   taskBroadcast.disable();
  }

//callback to the received messages from different nodes
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("logServer: Received from %u msg=%s\n", from, msg.c_str());
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(msg);
  //search the jsonObject from the key it contains
  if(root.containsKey("client")){
  //client.publish("themalValues",msg.c_str())
  if(String("room1").equals(root["client"].as<String>())){
    Serial.printf("client detected!!!\n");}
    JsonArray &arrayRoot = root["temp1"];
    JsonArray &arrayRoot1 = root["humid1"];
    valueTemp = arrayRoot.get<String>(0);
    valueHumid = arrayRoot1.get<String>(0);
    Serial.println(valueTemp);
    Serial.println(valueHumid);
 }
 
 if(root.containsKey("client1")){
  if(String("room2").equals(root["client1"].as<String>())){
    Serial.printf("client1 detected!!!\n");}
    JsonArray &arrayRoot = root["temp1"];
    JsonArray &arrayRoot1 = root["humid1"];
    valueTemp1 = arrayRoot.get<String>(0);
    valueHumid1 = arrayRoot1.get<String>(0);
    Serial.println(valueTemp1);
    Serial.println(valueHumid1);
  }
  taskBroadcast.enable();
}

void meshInit(uint32_t getChannel){
  // set before init() so that you can see startup messages
  //mesh.setDebugMsgTypes( ERROR | CONNECTION | S_TIME );  
  mesh.setDebugMsgTypes(ERROR | S_TIME);
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &ts, MESH_PORT, WIFI_AP_STA, getChannel);
  
}

//*********************CONNECT TO WIFI********************//
void reconnectWiFi(){
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  char ssidWiFi[30];//Stores the router name
  char passWiFi[30];//Stores the password

        String string_Ssid="";
        String string_Password="";
        string_Ssid= read_string(30,0); 
        string_Password= read_string(30,100); 
        Serial.println("ssid: "+string_Ssid);
        Serial.println("Password: "+string_Password);
        string_Password.toCharArray(passWiFi,30);
        string_Ssid.toCharArray(ssidWiFi,30);
        Serial.println(ssidWiFi);
        Serial.println(passWiFi);
        STATION_SSID = string_Ssid;
        STATION_PASSWORD = string_Password;
        
        //Serial.println(ssidWifi.charAt(2));
  delay(400);
  WiFi.begin(ssidWiFi,passWiFi);
  while (WiFi.status() != WL_CONNECTED)
  {
      delay(500);
      Serial.print(".");
  }
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  channelId = WiFi.channel();
  Serial.println(channelId); 
}


//****************************HANDLE ROOT***************************//

void handleRoot() {
   if (server.hasArg("ssid")&& server.hasArg("Password") ) {//If all form fields contain data call handelSubmit()
    handleSubmit();
  }
  else {//Redisplay the form
    server.send(200, "text/html", INDEX_HTML);
  }
}

//**************************SUBMIT RESPONSE**************************//
void handleSubmit(){//dispaly values and write to memmory
  String response="<p>The ssid is ";
 response += server.arg("ssid");
 response +="<br>";
 response +="And the password is ";
 response +=server.arg("Password");
 response +="</P><BR>";
 response +="<H2><a href=\"/\">go home</a></H2><br>";

 server.send(200, "text/html", response);
 //calling function that writes data to memory 
 ROMwrite(String(server.arg("ssid")),String(server.arg("Password")));
}

//**************WRITE RESPONSE TO EEPROM******************//
void ROMwrite(String s, String p){
 s+=";";
 write_EEPROM(s,0);
 p+=";";
 write_EEPROM(p,100);
 EEPROM.commit();   
}

//**************WRITE TO EEPROM************************//
void write_EEPROM(String x,int pos){
  for(int n=pos;n<x.length()+pos;n++){
     EEPROM.write(n,x[n-pos]);
  }
}

//****************HANDLE NOT FOUND*********************//
void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  message +="<H2><a href=\"/\">go home</a></H2><br>";
  server.send(404, "text/plain", message);
}
