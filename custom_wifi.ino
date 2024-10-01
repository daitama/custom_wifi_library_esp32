
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <LittleFS.h>

//Create AsyncWebServer object on port 80
AsyncWebServer server(80);

//Serach for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "password";
const char* PARAM_INPUT_3 = "ip";
const char* PARAM_INPUT_4 = "gateway";

//Variables to save value from HTML form
String ssid;
String pass;
String ip;
String gateway;

String networks;


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
    request->send(LittleFS, "/index.html", "text/html");
  });
  server.serveStatic("/", LittleFS, "/");
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/styles.css", "text/css");
  });
  server.serveStatic("/", LittleFS, "/").setDefaultFile("/styles.css").setCacheControl("no-store");
  server.on("/getNetworks", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", networks);
  });
}

void handlePostRequest(){
      server.on("/", HTTP_POST, [](AsyncWebServerRequest *request){
      int params = request->params();
      for(int i=0; i<params; i++){
        const AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          // HTTP POST ssid value
          if (p->name() == "message"){
          networks = scanWiFiNetworks();
            //Write file to save value
          }
        }
      }
      request->send(200, "text/plain", networks);
    });
}

// Replace placeholder with LED state value
String processor(const String& var){
  if(var == "SSID"){
    String ssidPlaceholder = "";
    ssidPlaceholder += "<script>const accessPoints = ";
    ssidPlaceholder += networks;
    ssidPlaceholder += ";document.dispatchEvent(new Event(RENDER_EVENT));</script>";
    return ssidPlaceholder;
  }
  return String();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  initLittleFS();

  // Set Wi-Fi to station mode and disconnect from any existing connection
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Call the getWiFiNetworks function and print the result
  Serial.println("Scanning for Wi-Fi networks...");
  networks = scanWiFiNetworks();
  Serial.println(networks);

  Serial.println("Setting AP (Acces Point)");
  WiFi.softAP("ESP-WIFI-MANAGER", NULL);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP Address: ");
  Serial.println(IP);

  handleGetRequest();
  handlePostRequest();
  
  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:

}
