 
#include <WiFiManager.h> 
#include <ESP8266WiFi.h> 
 
#define PIN_RELAY_1 5   
#define PIN_RELAY_2 4   
#define PIN_RELAY_3 14  
#define PIN_RELAY_4 12  
#define TOUCH_SENSOR_1 16  
#define TOUCH_SENSOR_2 10  
#define TOUCH_SENSOR_3 13  
#define TOUCH_SENSOR_4 15   
#define WIFI_LED_PIN 0  
#define TOGGLE_TRIGGER_PIN 3  
 
WiFiServer server(80);  
WiFiManager wm; 
int timeout = 120, relayState1 = 1, relayState2 = 1, relayState3 = 1, relayState4 = 1; 
String header;  
bool webServerRunning = false;   
 
void relayOnOff(int relay, int &relayState, int pin) { 
    relayState = !relayState; 
    digitalWrite(pin, relayState ? HIGH : LOW); 
    Serial.printf("Switch %d %s\n", relay, relayState ? "OFF" : "ON"); 
    delay(100); 
} 
void touchControl() { 
    if (digitalRead(TOUCH_SENSOR_1) == HIGH) relayOnOff(1, relayState1, PIN_RELAY_1); 
    if (digitalRead(TOUCH_SENSOR_2) == HIGH) relayOnOff(2, relayState2, PIN_RELAY_2); 
    if (digitalRead(TOUCH_SENSOR_3) == HIGH) relayOnOff(3, relayState3, PIN_RELAY_3); 
    if (digitalRead(TOUCH_SENSOR_4) == HIGH) relayOnOff(4, relayState4, PIN_RELAY_4); 
} 
void wifiConnect() { 
    if (digitalRead(TOGGLE_TRIGGER_PIN) == HIGH) { 
        wm.setConfigPortalTimeout(timeout); 
        if (!wm.startConfigPortal("Smart-ExtBoard-AP")) ESP.reset(); 
        startWebServer(); 
    } 
} 
 
void webControl() {  
    WiFiClient client = server.available(); 
    if (client) { 
        Serial.println("New Client.");  
        String currentLine = ""; 
        while (client.connected()) { 
            if (client.available()) { 
                char c = client.read(); 
                header += c; 
                if (c == '\n' && currentLine.length() == 0) { 
                    client.println("HTTP/1.1 200 OK\nContent-type:text/html\nConnection: close\n"); 
                    if (header.indexOf("GET /1") >= 0) relayOnOff(1, relayState1, PIN_RELAY_1); 
                    if (header.indexOf("GET /2") >= 0) relayOnOff(2, relayState2, PIN_RELAY_2); 
                    if (header.indexOf("GET /3") >= 0) relayOnOff(3, relayState3, PIN_RELAY_3); 
                    if (header.indexOf("GET /4") >= 0) relayOnOff(4, relayState4, PIN_RELAY_4); 
                    client.println("<!DOCTYPE html><html><body><h1>Smart Extension Board</h1>"); 
                    for (int i = 1; i <= 4; ++i) { 
                        client.printf("<p>Switch %d %s</p><a href=\"/%d\"><button>Switch</button></a>",  
                            i, (i == 1 && relayState1) || (i == 2 && relayState2) || (i == 3 && relayState3) || (i == 4 && 
relayState4) ? "OFF" : "ON", i); 
                    } 
                    client.println("</body></html>"); 
                    break; 
                } 
                currentLine += (c != '\r') ? c : ""; 
            } 
        } 
        header = ""; client.stop(); 
    } 
} 
 
void startWebServer() { if (WiFi.status() == WL_CONNECTED) { server.begin(); webServerRunning = true; } } 
void setup() { 
    Serial.begin(115200); WiFi.mode(WIFI_STA); 
    for (int i = PIN_RELAY_1; i <= PIN_RELAY_4; ++i) pinMode(i, OUTPUT), digitalWrite(i, HIGH); 
    for (int i = TOUCH_SENSOR_1; i <= TOUCH_SENSOR_4; ++i) pinMode(i, INPUT_PULLUP); 
    pinMode(TOGGLE_TRIGGER_PIN, INPUT_PULLUP), pinMode(WIFI_LED_PIN, OUTPUT); 
    wm.autoConnect(); startWebServer(); 
} 
void loop() { wifiConnect(); touchControl(); webControl(); }
