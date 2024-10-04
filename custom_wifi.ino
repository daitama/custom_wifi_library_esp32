//PERMASALAHAN DI initWifi()
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <LittleFS.h>

#define LED 22 //Builtin LED

// File paths to save input values premanently
const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";

//Timer variables
unsigned long previousMillis = 0;
const long interval = 10000;

//Create AsyncWebServer object on port 80
AsyncWebServer server(80);

//Variables to save value from HTML form
String ssid;
String pass;
String ip;
String gateway;

String networks;

//Variable to save states
bool isConnected = false;


// Initialize LittleFS
void initLittleFS() {
  if (!LittleFS.begin(true)) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  Serial.println("LittleFS mounted successfully");
}

// Read File from LittleFS
String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    return String();
  }
  
  String fileContent;
  while(file.available()){
    fileContent = file.readStringUntil('\n');
    break;     
  }
  return fileContent;
}

// Write file to LittleFS
void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
}

// Function to scan Wi-Fi networks and return SSIDs as a formatted string array
String scanWiFiNetworks() {
    // Start the WiFi scan
    int n = WiFi.scanNetworks();
    Serial.print("Number of networks found: ");
    Serial.println(n);

    // Create a JSON array to store the results
    String jsonArray = "[";

    // Loop through the scanned networks
    for (int i = 0; i < n; i++) {
        // Get the SSID, RSSI, channel, and security type
        String ssid = WiFi.SSID(i);
        int signalStrength = WiFi.RSSI(i);
        int channel = WiFi.channel(i);
        String securityType;

        // Determine the security type
        if (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) {
            securityType = "open";
        } else {
            securityType = "PSK"; // For simplicity, we'll consider other types as PSK
        }

        // Check if the SSID is hidden
        String isHidden = ssid.length() == 0 ? "true" : "false";

        // Append to JSON array
        jsonArray += "{";
        jsonArray += "\"ssid\":\"" + ssid + "\",";
        jsonArray += "\"securityType\":\"" + securityType + "\",";
        jsonArray += "\"signalStrength\":\"" + String(signalStrength) + "\",";
        jsonArray += "\"channel\":\"" + String(channel) + "\",";
        jsonArray += "\"isHidden\":\"" + isHidden + "\"";
        jsonArray += "}";

        if (i < n - 1) {
            jsonArray += ",";
        }
    }

    jsonArray += "]";
    return jsonArray;
}

void handleGetRequest(){
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/wifimanager.html", "text/html");
  });
  server.serveStatic("/", LittleFS, "/");
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/styles.css", "text/css");
  });
  server.serveStatic("/", LittleFS, "/").setDefaultFile("/styles.css").setCacheControl("no-store");
  server.on("/getNetworks", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", networks);
  });
  server.on("/getStatus", HTTP_GET, [](AsyncWebServerRequest *request){
    //
  });
}

void handlePostRequest(){
      server.on("/", HTTP_POST, [](AsyncWebServerRequest *request){
      int params = request->params();
      String responseMessage;
      for(int i=0; i<params; i++){
        const AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          // HTTP POST ssid value
          if (p->name() == "message"){
          networks = scanWiFiNetworks();
          responseMessage = networks;
            //Write file to save value
          }
        }
      }
      request->send(200, "text/plain", responseMessage);
    });
      server.on("/sendCredential", HTTP_POST, [](AsyncWebServerRequest *request){
      int params = request->params();
      String responseMessage;
      for(int i=0; i<params; i++){
        const AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          // HTTP POST ssid value
          if(p->name() == "ssid"){
            ssid = p->value().c_str();
            Serial.println("SSID Accepted!");
            Serial.print("SSID : ");
            Serial.println(ssid);
            //Write file to save value
            writeFile(LittleFS, ssidPath, ssid.c_str());
          }
          if(p->name() == "password"){
            pass = p->value().c_str();
            Serial.println("Password Accepted!");
            Serial.print("Password : ");
            Serial.println(pass);
            //Write file to save value
            writeFile(LittleFS, passPath, pass.c_str());
            responseMessage =  "Selesai, Data tersimpan.\n";
            responseMessage += "ESP akan di restart, Jika koneksi berhasil terhubung Lampu indikator akan menyala.\n";
            responseMessage += "Jika gagal, silahkan coba lagi dengan menyambung ke AP ini.";
          }
        }
      }
      request->send(200, "text/plain", responseMessage);
      delay(3000);
      ESP.restart();
    });
}
// Function that returns error message as a String based on the WiFi status
String getWiFiErrorMessage(wl_status_t status) {
  switch (status) {
    case WL_NO_SSID_AVAIL:
      return "Error: No SSID Available";
    case WL_CONNECT_FAILED:
      return "Error: Connection Failed. Wrong Password?";
    case WL_IDLE_STATUS:
      return "Status: Wi-Fi is Idle";
    case WL_DISCONNECTED:
      return "Error: Wi-Fi Disconnected";
    case WL_SCAN_COMPLETED:
      return "Status: Wi-Fi Scan Completed";
    case WL_CONNECTION_LOST:
      return "Error: Connection Lost";
    default:
      return "Attempting to connect...";
  }
}

bool initWifi() {
    if (ssid == "" || pass == "") {
        Serial.println("Undefined SSID or IP address.");
        return false;
    }

    Serial.println("Attempting to connect to WiFi...");
    WiFi.begin(ssid.c_str(), pass.c_str());
  
  unsigned long currentMillis = millis();
  previousMillis = currentMillis;

  while (WiFi.status() != WL_CONNECTED){
    currentMillis = millis();
    if (currentMillis - previousMillis >= interval){
      Serial.println("Failed to connect.");
      return false;
    }
  }
    // If we exit the loop, we're connected
    Serial.println("Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    isConnected = true;
    digitalWrite(LED, LOW);  // Turn on LED to indicate successful connection
    return true;
}


void setup() {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  delay(100);


  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  initLittleFS();

  //Load Values saved in LittleFS
  ssid = readFile(LittleFS, ssidPath);
  pass = readFile(LittleFS, passPath);

  if (!initWifi()) {
    // Call the getWiFiNetworks function and print the result
    WiFi.mode(WIFI_OFF);
    delay(1000); // Add a delay to ensure Wi-Fi module is ready
    WiFi.mode(WIFI_STA);
    Serial.println("Scanning for Wi-Fi networks...");
    networks = scanWiFiNetworks(); // Now should get networks if available
    Serial.println(networks);

    // Set up Access Point and start the server
    Serial.println("Setting AP (Access Point)");
    WiFi.softAP("ESP-WIFI-MANAGER", NULL);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP Address: ");
    Serial.println(IP);
    
    handleGetRequest();
    handlePostRequest();
    server.begin();
  } else{
    Serial.println("Connected!");
  }

}

void loop() {
  // put your main code here, to run repeatedly:

}
