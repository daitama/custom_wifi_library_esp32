
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
String getWiFiNetworks() {
  // Start scanning for networks
  int numNetworks = WiFi.scanNetworks();
  
  // If no networks found, return an empty array
  if (numNetworks == 0) {
    return "[]";
  }

  // Create a string to store the array of SSIDs
  String wifiArray = "[";

  // Loop through each network and add SSID to the array string
  for (int i = 0; i < numNetworks; i++) {
    String ssid = WiFi.SSID(i);
    wifiArray += "\"" + ssid + "\"";  // Add SSID in quotes

    // Add a comma between SSIDs, but not after the last one
    if (i < numNetworks - 1) {
      wifiArray += ", ";
    }
  }

  // Close the array string
  wifiArray += "]";
  
  // Return the formatted string array
  return wifiArray;
}

// Replace placeholder with LED state value
String processor(const String& var){
  if(var == "SSID"){
    String ssidPlaceholder = "";
    ssidPlaceholder += "<script>const accessPoints = ";
    ssidPlaceholder += networks;
    ssidPlaceholder += ";</script>";
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
  networks = getWiFiNetworks();
  Serial.println(networks);

  Serial.println("Setting AP (Acces Point)");
  WiFi.softAP("ESP-WIFI-MANAGER", NULL);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP Address: ");
  Serial.println(IP);

  // Web Server Root URL

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html", false, processor);
  });
  server.serveStatic("/", LittleFS, "/");

  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:

}
