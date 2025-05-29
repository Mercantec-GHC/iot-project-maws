#include "carrier.h"
#include "image.h"
#include "pet.h"

#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include "arduino_secrets.h"
#include <Adafruit_SleepyDog.h>
#include <ArduinoJson.h>

// WiFi Configuration
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
const char *server_address = SECRET_SERVER;
const int server_port = SECRET_PORT_HTTPS; // Use HTTPS port

// Use WiFiSSLClient for HTTPS
WiFiSSLClient wifi_client;
HttpClient http_client = HttpClient(wifi_client, SECRET_SERVER, SECRET_PORT_HTTPS);

animation_t *smile;
pet_t *pet;

// Add debounce timing variables
unsigned long last_button_time = 0;
const unsigned long debounce_delay = 200; // 200ms debounce

// Debugging variables
unsigned long debug_timestamp = 0;
const unsigned long DEBUG_INTERVAL = 5000; // Print debug info every 5 seconds

// Variables for sending data to server
unsigned long last_send_time = 0;
const unsigned long SEND_DATA_INTERVAL = 5 * 60 * 1000; // Send data every 5 minutes

void setup()
{
  Serial.begin(9600);

  // MKR Carrier Init
  while (!carrier.begin())
  {
    Watchdog.reset(); // Reset watchdog during long operations
  }

  carrier.display.setRotation(2);
  carrier.display.fillScreen(0x0000);

  // Initialize LEDs
  carrier.leds.begin();
  carrier.leds.clear();
  carrier.leds.show();

  center_x = carrier.display.width() / 2;
  center_y = carrier.display.height() / 2;

  pet = pet_init(50, 50, 50);
  smile = animation_create(2, 100);
  animation_add_frame(smile, epd_bitmap_image_1);
  animation_add_frame(smile, epd_bitmap_image_2);

  // WiFi Init
  connectToWiFi();

  // Configure power settings
  USBDevice.detach();
  delay(100);
  USBDevice.attach();

  // Test HTTPS connection without setInsecure() first
  Serial.println("Testing HTTPS connection...");
  if (wifi_client.connectSSL(SECRET_SERVER, SECRET_PORT_HTTPS))
  {
    Serial.println("SSL connection successful!");
    wifi_client.stop();
  }
  else
  {
    Serial.println("SSL connection failed - trying with relaxed security...");

// If connection fails, try with setInsecure if available
#ifdef WIFI_SSL_CLIENT_INSECURE_AVAILABLE
    wifi_client.setInsecure();
    if (wifi_client.connectSSL(SECRET_SERVER, SECRET_PORT_HTTPS))
    {
      Serial.println("SSL connection successful with relaxed security!");
      wifi_client.stop();
    }
    else
    {
      Serial.println("SSL connection still failed!");
    }
#else
    Serial.println("setInsecure() not available in this WiFiNINA version");
#endif
  }

  Watchdog.reset(); // Reset watchdog after setup completes
}

void connectToWiFi()
{
  Serial.println("Connecting to WiFi...");

  // Check WiFi module
  if (WiFi.status() == WL_NO_MODULE)
  {
    Serial.println("Communication with WiFi module failed!");
    while (true)
      ; // Don't continue
  }

  // Attempt to connect to WiFi network
  int attempts = 0;
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED && attempts < 10)
  {
    delay(1000);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\nWiFi connected successfully!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("\nFailed to connect to WiFi. Check credentials or network availability.");
  }
}

void sendPetDataToServer(pet_t *pet)
{
  Serial.println("Attempting to send data to server...");

  // Check WiFi connection first
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi disconnected. Attempting to reconnect...");
    connectToWiFi();
    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("Reconnection failed, aborting data send.");
      return;
    }
    Serial.println("WiFi reconnected.");
  }

  // Reset watchdog before creating the JSON
  Watchdog.reset();
  Serial.println("Creating JSON payload...");

  StaticJsonDocument<200> jsonDoc;
  jsonDoc["hunger"] = pet->hunger;
  jsonDoc["happiness"] = pet->happiness;
  jsonDoc["tiredness"] = pet->tiredness;
  int lifespan_percent = (int)((float)pet->lifespan / MAX_LIFESPAN * 100);
  jsonDoc["lifespan"] = lifespan_percent;

  String post_data;
  serializeJson(jsonDoc, post_data);

  Serial.print("JSON payload: ");
  Serial.println(post_data);

  // Reset watchdog before HTTP operations
  Watchdog.reset();
  Serial.println("Preparing HTTP request...");

  // Stop any previous connection and set a timeout
  http_client.stop();
  http_client.setTimeout(10000); // 10 second timeout

  Serial.println("Connecting via HTTPS...");
  Serial.println("Calling http_client.beginRequest()...");
  http_client.beginRequest();
  Serial.println("Calling http_client.post()...");
  http_client.post("/api/PetData");
  Serial.println("Calling http_client.sendHeader() for Content-Type...");
  http_client.sendHeader("Content-Type", "application/json");
  Serial.println("Calling http_client.sendHeader() for Content-Length...");
  http_client.sendHeader("Content-Length", post_data.length());
  Serial.println("Calling http_client.beginBody()...");
  http_client.beginBody();
  Serial.println("Calling http_client.print() with payload...");
  http_client.print(post_data);
  Serial.println("Calling http_client.endRequest()...");
  http_client.endRequest();
  Serial.println("HTTPS request sent.");

  // Reset watchdog after request
  Watchdog.reset();

  Serial.println("Processing HTTPS response...");
  int status_code = http_client.responseStatusCode();
  String response_body = http_client.responseBody();

  Serial.print("Status code: ");
  Serial.println(status_code);
  Serial.print("Response: ");
  Serial.println(response_body);
  Serial.println("HTTPS data sending process complete.");

  Watchdog.reset();
}

void pet_draw_age(pet_t *pet)
{
  // Calculate current age
  unsigned long current_time = millis();
  unsigned long age_duration = current_time - pet->birth_time;

  int days = age_duration / (24 * 60 * 60 * 1000);
  int hours = (age_duration % (24 * 60 * 60 * 1000)) / (60 * 60 * 1000);
  int minutes = (age_duration % (60 * 60 * 1000)) / (60 * 1000);

  // Display age on screen
  carrier.display.setTextColor(ST77XX_WHITE);
  carrier.display.setTextSize(1);
  carrier.display.setCursor(10, 10);
  carrier.display.print("Age: ");

  if (days > 0)
  {
    carrier.display.print(days);
    carrier.display.print("d ");
  }

  carrier.display.print(hours);
  carrier.display.print("h ");
  carrier.display.print(minutes);
  carrier.display.print("m");
}

void update_status_leds(pet_t *pet)
{
  // Clear all LEDs first
  carrier.leds.clear();

  // LED 0: Hunger indicator (Green = Full, Yellow = OK, Red = Hungry)
  if (pet->hunger >= 70)
  {
    carrier.leds.setPixelColor(0, 0, 255, 0); // Green
  }
  else if (pet->hunger >= 30)
  {
    carrier.leds.setPixelColor(0, 255, 255, 0); // Yellow
  }
  else
  {
    carrier.leds.setPixelColor(0, 255, 0, 0); // Red
  }

  // LED 1: Happiness indicator (Green = Happy, Yellow = OK, Red = Sad)
  if (pet->happiness >= 70)
  {
    carrier.leds.setPixelColor(1, 0, 255, 0); // Green
  }
  else if (pet->happiness >= 30)
  {
    carrier.leds.setPixelColor(1, 255, 255, 0); // Yellow
  }
  else
  {
    carrier.leds.setPixelColor(1, 255, 0, 0); // Red
  }

  // LED 2: Tiredness indicator (Green = Rested, Yellow = OK, Red = Tired)
  if (pet->tiredness <= 30)
  {
    carrier.leds.setPixelColor(2, 0, 255, 0); // Green
  }
  else if (pet->tiredness <= 70)
  {
    carrier.leds.setPixelColor(2, 255, 255, 0); // Yellow
  }
  else
  {
    carrier.leds.setPixelColor(2, 255, 0, 0); // Red
  }

  // LED 3: Lifespan indicator (Green = Healthy, Yellow = OK, Red = Near death)
  int lifespan_percent = (int)((float)pet->lifespan / MAX_LIFESPAN * 100);
  if (lifespan_percent >= 70)
  {
    carrier.leds.setPixelColor(3, 0, 255, 0); // Green
  }
  else if (lifespan_percent >= 30)
  {
    carrier.leds.setPixelColor(3, 255, 255, 0); // Yellow
  }
  else
  {
    carrier.leds.setPixelColor(3, 255, 0, 0); // Red
  }

  // LED 4: Overall health (average of all parameters)
  int overall_health = (pet->hunger + pet->happiness + (100 - pet->tiredness) + lifespan_percent) / 4;

  if (overall_health >= 70)
  {
    carrier.leds.setPixelColor(4, 0, 255, 0); // Green
  }
  else if (overall_health >= 30)
  {
    carrier.leds.setPixelColor(4, 255, 255, 0); // Yellow
  }
  else
  {
    carrier.leds.setPixelColor(4, 255, 0, 0); // Red
  }

  // Update LEDs
  carrier.leds.show();
}

void pet_handle_events_modified(pet_t *pet)
{
  // Only process button events if enough time has passed since last event
  unsigned long current_time = millis();
  if (current_time - last_button_time < debounce_delay)
  {
    return; // Skip button processing if not enough time has passed
  }

  // Knap 0: Mad
  if (carrier.Button0.getTouch())
  {
    Serial.println("Knap 0 trykket (Mad)");
    pet->hunger = constrain(pet->hunger - 20, MIN_STAT, MAX_STAT);      // Reducer sult
    pet->tiredness = constrain(pet->tiredness + 1, MIN_STAT, MAX_STAT); // Lidt træt af at spise
    pet->feedback_message = "Mums!";
    pet->feedback_timeout = millis() + FEEDBACK_DURATION;
    pet->status_change = true;
    last_button_time = current_time;
  }

  // Knap 1: Leg
  else if (carrier.Button1.getTouch())
  {
    Serial.println("Knap 1 trykket (Leg)");
    pet->happiness = constrain(pet->happiness + 4, MIN_STAT, MAX_STAT);
    pet->hunger = constrain(pet->hunger + 5, MIN_STAT, MAX_STAT);
    pet->tiredness = constrain(pet->tiredness + 3, MIN_STAT, MAX_STAT);
    pet->feedback_message = "Sjovt!";
    pet->feedback_timeout = millis() + FEEDBACK_DURATION;
    pet->status_change = true;
    last_button_time = current_time;
  }

  // Knap 3: Lur
  else if (carrier.Button3.getTouch())
  {
    Serial.println("Knap 3 trykket (Lur)");
    pet->tiredness = constrain(pet->tiredness - 10, MIN_STAT, MAX_STAT);
    pet->feedback_message = "Zzz";
    pet->feedback_timeout = millis() + FEEDBACK_DURATION;
    pet->status_change = true;
    last_button_time = current_time;
  }
}

void loop()
{
  // Debug timing
  unsigned long current_millis = millis();
  if (current_millis - debug_timestamp > DEBUG_INTERVAL)
  {
    debug_timestamp = current_millis;
    Serial.print("System uptime (seconds): ");
    Serial.println(current_millis / 1000);
    Serial.print("Free memory: ");
    Serial.println(freeMemory());
  }

  // Update buttons at the beginning of the loop
  carrier.Buttons.update();

  // Always update the animation
  animation_step(smile);

  // Check if pet is alive
  if (pet_is_alive(pet))
  {
    // Draw animation frame
    animation_draw_frame(center_x - 32, center_y - 32, 64, smile);

    // Handle button presses
    pet_handle_events_modified(pet);

    // Update UI
    pet_draw_feedback(pet);
    pet_draw_status(pet);
    pet_draw_age(pet);
    update_status_leds(pet);

    // Update pet lifespan
    pet_update_lifespan(pet);

    // Send data to server periodically
    if (current_millis - last_send_time > SEND_DATA_INTERVAL)
    {
      sendPetDataToServer(pet);
      last_send_time = current_millis;
    }
  }
  else
  {
    // Pet is dead, display death screen
    pet_draw_death(pet);

    // Turn all LEDs red when pet is dead
    carrier.leds.clear();
    for (int i = 0; i < 5; i++)
    {
      carrier.leds.setPixelColor(i, 255, 0, 0); // All red
    }
    carrier.leds.show();

    // Check for restart button press
    unsigned long current_time = millis();
    if (current_time - last_button_time > debounce_delay)
    {
      if (carrier.Button0.getTouch() || carrier.Button1.getTouch() ||
          carrier.Button2.getTouch() || carrier.Button3.getTouch() ||
          carrier.Button4.getTouch())
      {
        // Reset pet
        free(pet);
        pet = pet_init(50, 50, 50);
        carrier.display.fillScreen(0x0000);
        last_button_time = current_time;
      }
    }
  }

  // Short delay to prevent CPU overload
  delay(10);
}

// track free memory
int freeMemory()
{
  char top;
  return &top - reinterpret_cast<char *>(malloc(4));
}