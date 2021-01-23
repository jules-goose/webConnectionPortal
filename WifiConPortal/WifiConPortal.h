#ifndef WifiConPortal_h
#define WifiConPortal_h

#include "ESPAsyncWebServer.h"
/*
this class will either start it's own server and run as an ap, 
or if connection succeed, will proceed with you code
(can be a web server, or anything really)
*/

class WifiConPortal
{
protected:
    
private:
    String _networkStr ; //option string used for the ap selection
    AsyncWebServer *_server;
    String _apSSID ;//soft ap ssid
    String _apPASS ;//soft ap password
    int _mode;
    String processor(const String& var);
    int tryConnect();//attempt to connect to already saved access point
    void scanNet();//scan available access point
    void setupSoftAP();//set up a soft access point
    void setupWebServer();//set up the web server that handle adding access point
    void handleRoot(AsyncWebServerRequest *request);//handle default path, url : /
    void handleAddAp(AsyncWebServerRequest *request);// handle AddAp path, url : /addAp
    
public:
    
    WifiConPortal(String apSSID,String apPASS);
    ~WifiConPortal();
    int getMode(); //_mode getter
};


#endif