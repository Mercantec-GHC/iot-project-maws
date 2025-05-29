#include "carrier.h"
#include "image.h"
#include "pet.h"


#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include "arduino_secrets.h"

// WiFi Configuration
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
const char *server_address = SECRET_SERVER;
const int server_port = SECRET_PORT;

WiFiClient wifi_client;
HttpClient http_client(wifi_client, server_address, server_port);

animation_t *smile;
pet_t *pet;

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ; // Wait for serial to connect

  // MKR Carrier Init
  while (!carrier.begin())
    ;
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
  // Check WiFi connection first
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi disconnected. Attempting to reconnect...");
    connectToWiFi();
    if (WiFi.status() != WL_CONNECTED)
    {
      return; // Exit if reconnection failed
    }
  }

  String post_data = "{";
  post_data += "\"hunger\":" + String(pet->hunger) + ",";
  post_data += "\"happiness\":" + String(pet->happiness) + ",";
  post_data += "\"tiredness\":" + String(pet->tiredness) + ",";
  // Add lifespan percentage (0-100)
  int lifespan_percent = (int)((float)pet->lifespan / MAX_LIFESPAN * 100);
  post_data += "\"lifespan\":" + String(lifespan_percent);
  post_data += "}";

  Serial.println("Sending data to server...");
  Serial.println(post_data);

  http_client.beginRequest();
  http_client.post("/api/PetData");
  http_client.sendHeader("Content-Type", "application/json");
  http_client.sendHeader("Content-Length", post_data.length());
  http_client.beginBody();
  http_client.print(post_data);
  http_client.endRequest();

  // Process response
  int status_code = http_client.responseStatusCode();
  String response_body = http_client.responseBody();

  Serial.print("Status code: ");
  Serial.println(status_code);
  Serial.print("Response: ");
  Serial.println(response_body);

  if (status_code <= 0)
  {
    Serial.println("Error connecting to server. Check server address and port.");
  }
  else if (status_code >= 200 && status_code < 300)
  {
    Serial.println("Data sent successfully!");
  }
  else
  {
    Serial.println("Server error. Check API endpoint and data format.");
  }
}

// Implementation of pet_init (if not already in pet.h)
pet_t *pet_init(int initial_hunger, int initial_happiness, int initial_tiredness)
{
  pet_t *new_pet = (pet_t *)malloc(sizeof(pet_t));
  new_pet->hunger = initial_hunger;
  new_pet->happiness = initial_happiness;
  new_pet->tiredness = initial_tiredness;
  new_pet->birth_time = millis();
  new_pet->lifespan = MAX_LIFESPAN;
  new_pet->is_alive = true;
  return new_pet;
}

void pet_update_lifespan(pet_t *pet)
{
  static unsigned long last_lifespan_update = 0;
  unsigned long current_time = millis();

  // Update lifespan every LIFESPAN_DECREMENT_INTERVAL
  if (current_time - last_lifespan_update > LIFESPAN_DECREMENT_INTERVAL)
  {
    // Calculate health score (0-100) based on pet stats
    int health_score = (pet->hunger + pet->happiness + (100 - pet->tiredness)) / 3;

    // Calculate lifespan decrement based on health
    // Lower health = faster lifespan decrease
    unsigned long decrement = 0;

    if (health_score < 20)
    {
      decrement = LIFESPAN_DECREMENT_INTERVAL * 5; // Very unhealthy - 5x faster aging
    }
    else if (health_score < 40)
    {
      decrement = LIFESPAN_DECREMENT_INTERVAL * 2; // Unhealthy - 2x faster aging
    }
    else if (health_score < 60)
    {
      decrement = LIFESPAN_DECREMENT_INTERVAL; // Normal aging
    }
    else if (health_score < 80)
    {
      decrement = LIFESPAN_DECREMENT_INTERVAL / 2; // Healthy - slower aging
    }
    else
    {
      decrement = LIFESPAN_DECREMENT_INTERVAL / 4; // Very healthy - very slow aging
    }

    // Ensure we don't underflow
    if (pet->lifespan > decrement)
    {
      pet->lifespan -= decrement;
    }
    else
    {
      pet->lifespan = 0;
      pet->is_alive = false;
    }

    last_lifespan_update = current_time;
  }
}

bool pet_is_alive(pet_t *pet)
{
  return pet->is_alive && pet->lifespan > 0;
}

void pet_draw_death(pet_t *pet)
{
  // Clear the screen
  carrier.display.fillScreen(0x0000);

  // Draw death message
  carrier.display.setTextColor(ST77XX_WHITE);
  carrier.display.setTextSize(2);
  carrier.display.setCursor(20, center_y - 40);
  carrier.display.println("Your pet has");
  carrier.display.setCursor(20, center_y - 10);
  carrier.display.println("passed away");

  // Calculate how long the pet lived
  unsigned long life_duration = millis() - pet->birth_time;
  int days_lived = life_duration / (24 * 60 * 60 * 1000);
  int hours_lived = (life_duration % (24 * 60 * 60 * 1000)) / (60 * 60 * 1000);

  carrier.display.setCursor(20, center_y + 20);
  carrier.display.print("Lived: ");
  carrier.display.print(days_lived);
  carrier.display.print("d ");
  carrier.display.print(hours_lived);
  carrier.display.println("h");

  carrier.display.setCursor(20, center_y + 50);
  carrier.display.println("Press button to");
  carrier.display.setCursor(20, center_y + 70);
  carrier.display.println("restart");
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

// Add this function before the loop() function

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

// Modify your loop() function to handle pet death
void loop()
{
  // Check if pet is alive
  if (pet_is_alive(pet))
  {
    // Regular pet activities
    animation_step(smile);
    animation_draw_frame(center_x - 32, center_y - 32, 64, smile);
    pet_handle_events(pet);
    pet_draw_feedback(pet);
    pet_draw_status(pet);

    // Display pet age
    pet_draw_age(pet);

    // Update RGB LEDs to show pet status
    update_status_leds(pet);

    // Update pet lifespan
    pet_update_lifespan(pet);

    static unsigned long last_sent = 0;
    unsigned long current_time = millis();

    // Send data every 30 seconds
    if (current_time - last_sent > 30000)
    {
      sendPetDataToServer(pet);
      last_sent = current_time;
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

    // Check if a button is pressed to restart
    carrier.loop();
    if (carrier.Button1.getTouch() || carrier.Button2.getTouch() ||
        carrier.Button3.getTouch() || carrier.Button4.getTouch() ||
        carrier.Button5.getTouch())
    {
      // Reset pet
      free(pet);
      pet = pet_init(50, 50, 50);
      carrier.display.fillScreen(0x0000);
    }
  }

  // Check buttons and sensor inputs from carrier
  carrier.loop();
}
