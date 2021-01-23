#include "WifiConPortal.h"
#include "ESP8266WiFi.h"
#include "ESPAsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include "LittleFS.h"

WifiConPortal::WifiConPortal(String apSSID,String apPASS)
{
  _apSSID = apSSID;
  _apPASS = apPASS;
  _server = NULL;
  _mode = tryConnect();
  if (_mode == 1)
  {
      scanNet();
      setupSoftAP(); 
      setupWebServer();
      
  }
}
WifiConPortal::~WifiConPortal()
{}
//used for code control
int WifiConPortal::getMode()
{
  return _mode; 
}

void WifiConPortal::setupSoftAP()
{
    Serial.print("Setting soft-AP ... ");
    boolean result = WiFi.softAP(_apSSID, _apPASS);
    if(result == true)
    {
    Serial.println("Ready");
    Serial.print("Soft-AP IP address = ");
    Serial.println(WiFi.softAPIP());
    }
    else
    {
    Serial.println("Failed!");
    }
}
//scan for networks
void WifiConPortal::scanNet()
{
  int n = WiFi.scanNetworks();
  //build the dropdown menu content
  for (int i = 0; i < n; i++)
  {
    _networkStr+="<option value='";
    _networkStr+=WiFi.SSID(i);
    _networkStr+="'>";
    _networkStr+=WiFi.SSID(i);
    _networkStr+="</option>";
  }
  File temp = LittleFS.open("/temp.txt","w");
  temp.print(_networkStr);
  temp.close();
}

//attempt to connect to already configured ap
int WifiConPortal::tryConnect(){
  
  File cred = LittleFS.open("/conf/cred.txt","r");
  size_t endpos = cred.size();
  size_t pos = cred.position();
  
  while (pos < endpos)
  {
    String wifiAP(cred.readStringUntil(':'));
    String WifiPass(cred.readStringUntil(0x0a));
    int concount = 0;
    
    pos = cred.position();
    WiFi.begin(wifiAP,WifiPass);

    Serial.print("Connecting ");
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
      concount++;
      if (concount == 10)
      {
        break;
      }
    }
    Serial.println(WiFi.status());
    if(WiFi.status() == WL_CONNECTED){
        Serial.print("Connected, IP address: ");
        Serial.println(WiFi.localIP());
        cred.close();
      return 0;
    }
  }
  Serial.println("con to ap failed");
  cred.close();
  return 1;
}


String WifiConPortal::processor(const String& var)
{
  String netStr;
  if(var == "LIST"){
    //add scanned ap list
    File temp = LittleFS.open("/temp.txt","r");
    netStr = temp.readString();
    temp.close();
    return netStr;
  }
}
void WifiConPortal::handleRoot(AsyncWebServerRequest *request)
{
  request->send(LittleFS, "/www/wifi/connect.html", String(), false,std::bind(&WifiConPortal::processor, this, std::placeholders::_1));
}
void WifiConPortal::handleAddAp(AsyncWebServerRequest *request)
{
    String network;
    String networkPass;
    //consume the request
    int nbparams = request->params();
    for (int i = 0; i < nbparams; i++)
    {
      AsyncWebParameter* p = request->getParam(i);
      if( p->name() == "networkSelection"){
        network =  p->value();
      }
      if( p->name() == "password"){
        networkPass =  p->value();
      }
     }
     //append new access point to cred.txt
    File cred = LittleFS.open("/conf/cred.txt","a");
    cred.printf("%s:%s\n", network.c_str(), networkPass.c_str());
    cred.close();
    request->send(LittleFS, "/www/wifi/connect.html", String(), false);
}
void WifiConPortal::setupWebServer()
{
  _server = new AsyncWebServer(80);
  _server->on("/", HTTP_GET,std::bind(&WifiConPortal::handleRoot, this, std::placeholders::_1));
  _server->on("/addAp",HTTP_POST,std::bind(&WifiConPortal::handleAddAp, this, std::placeholders::_1));
  _server->begin();
}