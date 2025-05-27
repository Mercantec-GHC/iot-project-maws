#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>

// ---------- WiFi Indstillinger ----------
char ssid[] = "NSFW_Wifi"; 
char pass[] = "Keshava108"; 
// Den globale 'status' variabel er ikke længere nødvendig for loop-logikken,
// da vi nu primært stoler på WiFi.status() direkte.

// ---------- API Indstillinger (for dit Tamagotchi Projekt) ----------
const char serverAddress[] = "192.168.64.105"; // DIN PCs IP-ADRESSE
const int serverPort = 5021;                 // DIN TAMAGOTCHI APIs HTTP PORT
WiFiClient client;                           // Brug WiFiClient for HTTP

HttpClient httpClient = HttpClient(client, serverAddress, serverPort);
const char petDataPathGetLatest[] = "/api/PetData/latest"; // Endpoint for GET
const char petDataPathPost[] = "/api/PetData";         // Endpoint for POST

// ---------- Pet Data Struktur på Arduino (matcher din DomainModels/PetData.cs) ----------
struct PetDataArduino {
    int petDataId; // Vil typisk blive sat af serveren/databasen ved POST
    int hunger;
    int happiness;
    int tiredness;
    // String timestamp; // Serveren sætter timestamp
};

PetDataArduino currentPetData; // Til at gemme data hentet fra API'en
PetDataArduino dataToSend;     // Til at forberede data, der skal sendes

unsigned long lastApiGetCallTime = 0;
const long apiGetCallInterval = 15000; // Hent data hvert 15. sekund

unsigned long lastApiPostCallTime = 0;
const long apiPostCallInterval = 20000; // Send data hvert 20. sekund for test med dummy data

void setup() {
  Serial.begin(9600);
  while (!Serial) { ; } 
  Serial.println("\n[SETUP] Arduino Tamagotchi API Klient (Send & Modtag)...");

  connectToWiFi(); 

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("[SETUP] WiFi forbundet. Laver første GET API kald...");
    getLatestPetData();
    lastApiGetCallTime = millis();

    // Initialiser dataToSend med DUMMY data for første POST
    dataToSend.hunger = 8; 
    dataToSend.happiness = 7;
    dataToSend.tiredness = 6;
    // petDataId og timestamp sættes ikke fra Arduino ved POST til oprettelse

  } else {
    Serial.println("[SETUP] FEJL: Kunne ikke forbinde til WiFi. API kald springes over.");
  }
  Serial.println("[SETUP] Setup færdig.");
}

void loop() {
  unsigned long currentTime = millis();

  if (WiFi.status() != WL_CONNECTED) {
    // Forsøg kun at genforbinde hvis ikke allerede i gang med et forsøg
    static unsigned long lastReconnectAttempt = 0;
    if (currentTime - lastReconnectAttempt > 5000) { // Prøv hvert 5. sekund
        Serial.println("\n[LOOP] WiFi ikke forbundet. Forsøger at genforbinde...");
        connectToWiFi();
        lastReconnectAttempt = currentTime;
    }
    return; // Spring resten af loop over hvis ingen WiFi
  }
  
  // Hent seneste data periodisk
  if (currentTime - lastApiGetCallTime >= apiGetCallInterval) {
    Serial.println("\n[LOOP] Tid til et GET API kald for PetData.");
    getLatestPetData();
    lastApiGetCallTime = currentTime;
  }

  // Send DUMMY data periodisk for at teste POST
  if (currentTime - lastApiPostCallTime >= apiPostCallInterval) {
    Serial.println("\n[LOOP] Tid til et POST API kald for at sende DUMMY PetData.");
    
    // Sæt dummy data (petDataId er ikke relevant at sende for en ny post)
    dataToSend.hunger = 8; 
    dataToSend.happiness = 7;
    dataToSend.tiredness = 6;
    // Hvis du vil have varierende dummy data, kan du bruge random() her:
    // dataToSend.hunger = random(5, 15); 
    // dataToSend.happiness = random(5, 15);
    // dataToSend.tiredness = random(5, 15);


    sendPetDataToAPI(dataToSend);
    lastApiPostCallTime = currentTime;
  }

  delay(100); 
}

void connectToWiFi() {
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("[WiFi] Fejl: Intet WiFiNINA modul! Stopper.");
    while (true); 
  }
  
  if (WiFi.status() == WL_CONNECTED) {
     printArduinoIP();
     return;
  }
  
  Serial.print("[WiFi] Forsøger at forbinde til SSID: "); Serial.println(ssid);
  WiFi.begin(ssid, pass);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) { 
    delay(500); Serial.print("."); attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WiFi] Forbundet succesfuldt!");
    printArduinoIP(); 
  } else {
    Serial.println("\n[WiFi] Forbindelse fejlede!");
  }
}

void printArduinoIP() {
  IPAddress ip = WiFi.localIP(); 
  Serial.print("[WiFi] Arduino IP Adresse: "); Serial.println(ip); 
}

void getLatestPetData() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[API GET] Kan ikke hente: Ingen WiFi."); 
    return;
  }

  Serial.print("[API GET] Sender GET til: http://"); 
  Serial.print(serverAddress); Serial.print(":"); Serial.print(serverPort); Serial.println(petDataPathGetLatest);

  httpClient.setTimeout(10000); 
  httpClient.stop(); 
  delay(100);      

  int errorCode = httpClient.get(petDataPathGetLatest); 
  
  if (errorCode != 0) {
    Serial.print("[API GET] Fejl ved httpClient.get(). Kode: "); Serial.println(errorCode);
    return;
  }

  int httpStatusCode = httpClient.responseStatusCode();
  String responseBody = httpClient.responseBody();

  Serial.print("[API GET] HTTP Status: "); Serial.println(httpStatusCode);
  
  if (httpStatusCode == 200) { 
    StaticJsonDocument<256> jsonDoc; 
    DeserializationError error = deserializeJson(jsonDoc, responseBody);
    if (error) {
      Serial.print("[API GET] deserializeJson() fejlede: "); Serial.println(error.f_str());
      Serial.print("[API GET] Modtaget Body: "); Serial.println(responseBody);
      return;
    }
    
    currentPetData.petDataId = jsonDoc["petDataId"]; 
    currentPetData.hunger = jsonDoc["hunger"];
    currentPetData.happiness = jsonDoc["happiness"];
    currentPetData.tiredness = jsonDoc["tiredness"];

    Serial.println("[API GET] PetData modtaget:");
    Serial.print("  ID: "); Serial.print(currentPetData.petDataId);
    Serial.print(", Sult: "); Serial.print(currentPetData.hunger);
    Serial.print(", Glæde: "); Serial.print(currentPetData.happiness);
    Serial.print(", Træthed: "); Serial.println(currentPetData.tiredness);
  } else { 
    Serial.print("[API GET] API kald fejlede. Body: "); Serial.println(responseBody);
  }
  httpClient.stop(); 
}

// Funktion til at sende PetData via POST med dummy data
void sendPetDataToAPI(PetDataArduino dataToPost) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[API POST] Kan ikke sende: Ingen WiFi.");
    return;
  }

  // 1. Opret JSON payload med de specifikke dummy data
  StaticJsonDocument<128> jsonPayloadDoc; 
  jsonPayloadDoc["hunger"] = dataToPost.hunger;     // Bruger værdien fra dataToPost (som er sat til dummy data)
  jsonPayloadDoc["happiness"] = dataToPost.happiness; // Bruger værdien fra dataToPost
  jsonPayloadDoc["tiredness"] = dataToPost.tiredness; // Bruger værdien fra dataToPost
  // PetDataId og Timestamp skal ikke sendes, da serveren håndterer dem for en ny post

  String jsonPayloadString;
  serializeJson(jsonPayloadDoc, jsonPayloadString);

  Serial.print("[API POST] Sender POST til: http://");
  Serial.print(serverAddress); Serial.print(":"); Serial.print(serverPort); Serial.println(petDataPathPost);
  Serial.print("[API POST] JSON Payload: "); Serial.println(jsonPayloadString);

  httpClient.setTimeout(10000);
  httpClient.stop(); 
  delay(100);

  String contentType = "application/json";
  int errorCode = httpClient.post(petDataPathPost, contentType, jsonPayloadString);

  if (errorCode != 0) {
    Serial.print("[API POST] Fejl ved httpClient.post(). Kode: "); Serial.println(errorCode);
    return;
  }

  int httpStatusCode = httpClient.responseStatusCode();
  String responseBody = httpClient.responseBody();

  Serial.print("[API POST] HTTP Status: "); Serial.println(httpStatusCode);
  Serial.print("[API POST] Svar Body: "); Serial.println(responseBody);

  if (httpStatusCode == 201) { 
    Serial.println("[API POST] Data sendt og oprettet succesfuldt på serveren!");
    // Du kan parse responseBody her for at få det nye ID, hvis din API returnerer det
    StaticJsonDocument<256> responseDoc;
    DeserializationError error = deserializeJson(responseDoc, responseBody);
    if (!error) {
      int newId = responseDoc["petDataId"]; // Antager at serveren returnerer det oprettede objekt
      // String newTimestamp = responseDoc["timestamp"].as<String>(); // Eksempel
      Serial.print("[API POST] Nyt PetDataId fra server: "); Serial.println(newId);
      // Serial.print("[API POST] Timestamp fra server: "); Serial.println(newTimestamp);
    }
  } else if (httpStatusCode == 200 || httpStatusCode == 204) { 
     Serial.println("[API POST] Data sendt succesfuldt (Status " + String(httpStatusCode) + ").");
  }
  else {
    Serial.print("[API POST] Fejl ved afsendelse af data. HTTP status: "); Serial.println(httpStatusCode);
  }
  httpClient.stop();
}
