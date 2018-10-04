#include <SocketIoClient.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h> 
#include <EEPROM.h>
#include <ESP8266mDNS.h>

MDNSResponder mdns;


ESP8266WebServer server(80);


const char* websocketServer = "172.18.5.249";
const int websocketPort = 8000; 
SocketIoClient socket;
 
char* ssid = "";      //Thay tên wifi ở đây
char* password = "";  //Thay tên password ở đây


int state = LOW;
int pin = 2;

void event(const char * payload, size_t length) {
  String data(payload);
 if (data.charAt(0) == '0')
  state = LOW;
 else state = HIGH;

 pin = data.substring(1).toInt();

 Serial.print("Pin ");
 Serial.println(pin);

 digitalWrite(pin, state);
}

int readLength;
int readCounter;
char readChar;

char * readEEPROMString(int &startAddress) {
  readLength = EEPROM.read(startAddress);
  Serial.print("Read data at address: ");
  Serial.println(startAddress);
  Serial.print("Read data with length: ");
  Serial.println(readLength);
  if (readLength == 0)
    return "";
  startAddress += 1;
  char * data = new char[readLength + 1];
  for (readCounter = 0; readCounter < readLength; readCounter++) {
    Serial.println(readCounter);
    Serial.println(startAddress);
    readChar = (char) EEPROM.read(startAddress);
    delay(5);
    Serial.println(readChar);
    startAddress += 1;
    data[readCounter] = readChar;
  }
  data[readLength] = 0;
  Serial.println("");

  return data;
}

void writeEEPROMString(int &startAddress, const char* data) {
  readLength = strlen(data);
  Serial.print("Writing data at address: ");
  Serial.println(startAddress);
  Serial.print("Writing data with length: ");
  Serial.println(readLength);
  if (readLength == 0)
    return;
  EEPROM.write(startAddress, readLength);
  delay(5);


  startAddress += 1;
  for (readCounter = 0; readCounter < readLength; readCounter++) {
    EEPROM.write(startAddress, data[readCounter]);
    delay(5);
    startAddress += 1;
  }
  EEPROM.commit();
  Serial.print("Writing return new address: ");
  Serial.println(startAddress);
}

bool readyToWork = false;

void setup() {

  EEPROM.begin(100);
 
  pinMode(12, OUTPUT);  //led chân 12
  pinMode(13, OUTPUT);  //led chân 13
  pinMode(2, OUTPUT);  //led chân 2
 
  Serial.begin(9600);
  delay(100);
 
  Serial.println();
  Serial.println();

  int readingAddress = 0;

  ssid = readEEPROMString(readingAddress);
  delay(5);
  password = readEEPROMString(readingAddress);
  if (strlen(ssid)) {
    digitalWrite(2, HIGH);
    readyToWork = true;
  } else {
    digitalWrite(13, HIGH);
  }
  
  if (readyToWork) {
    Serial.print("WiFi connecting to ");
    Serial.print(ssid);
    Serial.print(" password ");
    Serial.println(password);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    
    socket.begin(websocketServer, websocketPort, "/ws/chat/xxx/?transport=websocket");
    socket.on("event", event);
  } else {
    server.on("/", [](){
      String homepage = "<form><div>Name: <input type=\"text\" name=\"name\"/></div><div>Wifi: <input type=\"text\" name=\"wifi\"/>";
      homepage += "</div><div>Name: <input type=\"text\" name=\"password\"/></div><button type=\"submit\">Save</button></form>";
      String wifi = server.arg("wifi");
      String password = server.arg("password");
      Serial.println(wifi);
      Serial.println(password);
      Serial.println("Stored");

      int writingAddress = 0;
      writeEEPROMString(writingAddress, wifi.c_str());
      writeEEPROMString(writingAddress, password.c_str());

      int temp = 0;
      Serial.println("Save config");
      Serial.println(readEEPROMString(temp));
      Serial.println(readEEPROMString(temp));
      Serial.println(temp);
      
      server.send(200, "text/html", homepage);
    });
    server.begin();
    WiFi.softAP("ESP8266", "", 12, false, 4);
  }
}
 
void loop() {
  if (readyToWork)
    socket.loop();
  else
    server.handleClient();
}
